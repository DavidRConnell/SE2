function [varargout]=SpeakEasy2(ADJ,varargin)
%SPEAKEASY2 Community detection algorithm
%   You can run SpeakEasy without setting any parameters, or control exactly
%   how it functions, using the parameters below, which control how the
%   algorithm is applied, but not how it operates.
%
%   Algorithm produces INDEPENDENT_RUNS x TARGET_PARTITION partitions then
%   selects the partition that is most similar to all other partitions based on
%   NMI.
%
%   SPEAKEASY2(ADJ) Run SpeakEasy2 on adjacency matrix ADJ, saving the results
%   to FILENAME (default 'SpeakEasy2_results'). ADJ can be full or sparse,
%   weighted or unweighted, directed or undirected.
%
%   PARTITION = SPEAKEASY2(ADJ) Return resulting partition, in two column list
%   format with node IDs in first column and label IDs in the second, to
%   PARTITION.
%
%   [PARTITION, PARTITION_CELLS, CONVENIENT_ORDER] = SPEAKEASY2(ADJ)
%   PARTITION_CELLS returns cells of nodes for each cluster order by size of
%   the cluster and CONVENIENT_ORDER sorts nodes in an order useful for
%   visualizing clusters. Clusters can be visualized with:
%       >> imagesc((convenient_node_ordering{1},convenient_node_ordering{1}))
%
%   [P, CELLS, ORDER, MULTI_COMMUNITY_NODES] = SPEAKEASY2(ADJ)
%   MULTI_COMMUNITY_NODES provides a list of all nodes in multiple
%   communities. See MULTICOMMUNITY parameter.
%
%   [...] = SPEAKEASY2(..., 'PARAM1', VAL1, 'PARAM2', VAL2, ...) specifies
%   additional parameters and their values.  Valid parameters are the
%   following:
%
%       Algorithm parameters
%       Parameter             Value
%        'independent_runs'   '10' (default) number of independent runs to
%                             perform. Each run get's its own set off initial
%                             conditions.
%        'subcluster'         '1' (default) depth of clustering. Sub-cluster
%                             primary clusters if > 1.
%        'multicommunity'     '1' (default) max number of communities a node
%                             can be a member of. Find crisp partitions if 1,
%                             fuzzy if > 1.
%        'target_partitions'  '5' (default) Number of partitions to find per
%                             independent run. Total partitions is
%                             target_partitions * independent runs.
%        'target_clusters'    Expected number of clusters to find.
%        'minclust'           '5' (defualt) Smallest cluster size for
%                             subclustering. Don't attempt to subcluster a
%                             cluster that is smaller than this.
%        'discard_transient'  '3' (default) how many initial partitions to
%                             discard.
%
%      Performance parameters
%      Parameter            Value
%        'memory_efficient'  'true' (default) whether to favor memory (true)
%                            or performance (false). Specific to full ADJ.
%        'random_seed'       Seed to use for reproducing results.
%        'autoshutdown'      'true' (default) whether to shutdown the parpool.
%                            If not using parallel processing, ignored. Setting
%                            to false can improve performance if batch running
%                            SpeakEasy2 on many networks.
%        'max_threads'       Number of threads to use. Defaults to system
%                            max - 1. If not using parallel processing, ignored.
%
%      User parameters
%      Parameter            Value
%        'filename'          'SpeakEasy2_results' (default) filename to save
%                            the results to if called without output arguments.
%        'verbose'           'false' (default) Display additional information
%                            about the run.
%        'graphics'          'false' (default) Create graphics.
%        'node_confidence    'false' (default).
%
%   Example uses
%     >> SpeakEasy2(rand(30)); % just to verify it runs
%     >> load ADJdemo
%     >> % output results to workspace
%     >> [partition partition_cell_version convenient_node_order]=SpeakEasy2(ADJ);
%     >> % clustering primary clusters again
%     >> SpeakEasy2(ADJ,'subcluster',2)
%     >> % three independent threads - requires parallel toolbox
%     >> SpeakEasy2(ADJ,'maxthreads',3)

%%Settings.  In this section change how SE2 is applied, but not its fundamental behavior.
%The most likely inputs to be adjusted are at the top.
options = inputParser;
options.CaseSensitive = false;

%settings related to how many times SpeekEasy is applied
addOptional(options,'filename','SpeakEasy2_results')
addOptional(options,'independent_runs',10);
addOptional(options,'subcluster',1);
addOptional(options,'multicommunity',1)
addOptional(options,'target_partitions',5);
addOptional(options,'target_clusters',max([min([10 length(ADJ)]) round(length(ADJ)/100)]));

%core and probably don't need to ever change, as these are generally arbitrary, fast and accurate
addOptional(options,'minclust',5);
addOptional(options,'discard_transient',3)

%these merely affect how we do the clustering, but not the result
addOptional(options,'memory_efficient',true,@islogical);
addOptional(options,'random_seed',[]);
addOptional(options,'autoshutdown',true,@islogical);
addOptional(options,'max_threads',feature('numcores') - 1);

%for extra output
addOptional(options,'verbose',false,@islogical);
addOptional(options,'graphics',false,@islogical);
addOptional(options,'node_confidence',false,@islogical);

%% Parse arguments
parse(options,varargin{:});

if isempty(options.Results.random_seed)
     rng(randi(10000,1))
         addOptional(options,'seed_set_by_user',0);
else
    rng(options.Results.random_seed)
    addOptional(options,'seed_set_by_user',1);
end

parallel_enabled = ~isempty(ver("parallel"));
if all(~contains(options.UsingDefaults, 'max_threads')) && ~parallel_enabled
    warning(sprintf("%s\n%s", ...
                    "Parallel toolbox required to take advantage of multiple threads.", ...
                    "Ignoring value of 'max_threads' argument."));
end

if options.Results.max_threads > feature('numcores')
    error("Threads set to more than available in argument 'max_threads'.")
end

parse(options,varargin{:});
options=options.Results; %for convenience

%%  Call SpeakEasy to generate primary clusters (and subsequently if options.layers>1)

ADJ = ADJ_characteristics(ADJ); %some memory properties optimized on characteristics of ADJ

if options.independent_runs*options.target_partitions>100
    disp('you probably only need max value of 100 paritions, so you may be able to reduce option.independnent runs or options.target_partitions')
end

if options.minclust<3
    error('you set minclust < 3 - this is logically odd and may cause problems, so please increase');         %min size for sub-clustering(if already small, dont need to subcluster)
end

if options.max_threads==1
    error('set parallel equal to desired number of threads, not just 1, which indicates a single thread');
end

if options.multicommunity<1
    disp('Alert - you set the multicom option equal a value less than 1 - this will NOT enable overlapping community detection.')
    disp('If you want to do that select an integer greater than 1, equal to the # community a node may belong to')
end

for main_iter=1:options.subcluster   %main loop over clustering / subclustering
    if main_iter==1
        disp('calling main routine at level 1')  %in identically named outputs, last one so multicom will overwrite discree, if it is different
        [partition_tags{main_iter} partition_tags{main_iter} partition_cells{main_iter} partition_cells{main_iter} multicom_nodes_all{main_iter}  median_of_all_NMI{main_iter} confidence_score_temp{main_iter}]=SpeakEasy2_bootstrap(ADJ,main_iter,options);
        %partition_tags" comes out sorted by nodeID, whereas when we created it in sub_iter from partition_cells, it is unsorted, then we sort it

        if options.node_confidence==1
            for m=1:length(partition_cells{main_iter})   %partition_cells is collection of NodeID's, so max 1000
                %three columns - col1:NodeID (not IDX, except for discrete output!) col2:confidence   col3:cluster label
                nodeID_and_confidence_score_temp{m,1}= [partition_cells{main_iter}{m} confidence_score_temp{main_iter}{m}  repmat(m,length(partition_cells{main_iter}{m}),1)  ];
                %you get specific confidence value for each node in each
                %cluster, so there will be some NodeID's with multiple different confidence scores if multicommunity is enabled
                %practically speaking the fourth column of nodeID_and_confidence_score_temp has to be calculated in separate step

            end
            confidence_score{main_iter}=(vertcat(nodeID_and_confidence_score_temp{:}));

            clear nodeID_and_confidence_score_temp;
            clear confidence_score_temp
        end
    else  %main_iter>1 subclustering and sub-subclustering etc
        disp(['doing subclustering at level ' num2str(main_iter)])

        %could make next line parfor to process subclusters in  parallel, but you never know how large those are, so can be problematic as you could get big replicated subsets of ADJ
        for sub_iter=1:length(partition_cells{main_iter-1})  %go through each of the previously determined clusters, hence{main_iter-1}

            current_nodes=partition_cells{main_iter-1}{sub_iter};  %these are the top-level Node ID's of the set of nodes we will subcluster

            if length(current_nodes)>options.minclust   %subcluster big clusters; used to have mean cluster density criterion to avoid subclustering a really dense cluster... reasons to do each, you could add back here if desired

                %"partition_tags_temp" isn't used directly - just "partition_cells_temp" - which later defines "partition_tags"
                [partition_tags_temp{sub_iter,1}, ...
                 partition_tags_temp{sub_iter,1}, ...
                 partition_cells_temp{sub_iter,1}, ...
                 partition_cells_temp{sub_iter,1}, ...
                 ~, ~, ...
                 confidence_score_temp{sub_iter,1}] ...
                 =SpeakEasy2_bootstrap(ADJ(current_nodes,current_nodes),...
                                       main_iter,...
                                       options);
                partition_tags_temp{sub_iter,1}(:,1)=current_nodes(partition_tags_temp{sub_iter,1}(:,1));                       %the subclusters you've found are at various locations in the full ADJ - need to adjust indexes to reflect this
                %seems "partition_cells_temp" should be updated to "current_nodes" too, which is done in the for-k loop or else condition

                %"sub_iter" can also be thought of as idx of cluster we're working on
                for k=1:length(partition_cells_temp{sub_iter,1}) %update to main node ID's
                    if options.node_confidence==1
                        nodeID_and_confidence_score_temp{sub_iter,1}{k,1}=[current_nodes(partition_cells_temp{sub_iter,1}{k}) confidence_score_temp{sub_iter}{k}];
                    end
                    partition_cells_temp{sub_iter,1}{k}=current_nodes(partition_cells_temp{sub_iter,1}{k});  %"k" has the main clusters of the sub cluster, so really subsubclusters since we're working on a select part of the ADJ to begin with
                end
            else %if module is too small for subclustering
                partition_cells_temp{sub_iter,1}{1}=current_nodes;
                if options.node_confidence==1
                    nodeID_and_confidence_score_temp{sub_iter,1}{1}=[current_nodes(:) zeros(length(current_nodes),1)];
                    %would like to know what clusterID each confidence score will be a member
                    %of, since you might have overlapping clusters, meaning the same nodeID will have mulitle confidence scores, but final clusterID's
                    %aren't assigned until we're out of this loop
                end
            end

            if options.node_confidence==1
                tracker_confid=(vertcat(nodeID_and_confidence_score_temp{:}));
            end
            tracker_partition_cells=vertcat(partition_cells_temp{:});
        end

        %now we've made it past the sub_iter loop, we're going to combine the results (subclusters) so they're more convenient
        partition_cells{main_iter}=vertcat(partition_cells_temp{:});
        partition_cells_temp=[];

        if options.node_confidence==1
            confidence_score{main_iter}=(vertcat(nodeID_and_confidence_score_temp{:}));
            confidence_score_temp=[];
        end

        for m=1:length(partition_cells{main_iter}) %indices stored in the mth cell get assigned label==m
            nodeID_and_new_cluster_label{m,1}=[partition_cells{main_iter}{m} repmat(m,length(partition_cells{main_iter}{m}),1) ]; %two cols - nodeID and assignment
            if options.node_confidence==1
                nodeID_and_new_confidence{m,1}  = [confidence_score{main_iter}{m} repmat(m,length(partition_cells{main_iter}{m}),1) ]     ;
                %nodeID's will be in sync:          nodeID_and_new_confidence{m,1}  = [partition_cells{main_iter}{m} confidence_score{main_iter}{m} repmat(m,length(partition_cells{main_iter}{m}),1) ]
            end
        end

        if options.node_confidence==1
            confidence_score{main_iter}=(vertcat(nodeID_and_new_confidence{:})); %was below
        end

        partition_tags{main_iter}=vertcat(nodeID_and_new_cluster_label{:});
    end %for if/else main_iter processing

    convenient_order{main_iter}=vertcat(partition_cells{main_iter}{:});
    partition_tags{main_iter}=sortrows(partition_tags{main_iter});
end

if parallel_enabled && options.autoshutdown
    delete(gcp('nocreate'))
end

if options.node_confidence==1
    save([options.filename '_node_confidence.mat'], 'confidence_score')
end

if nargout==0
    save([options.filename '.mat'], 'partition_tags', 'partition_cells', 'convenient_order')
else
    varargout{1}=partition_tags ;
    varargout{2}=partition_cells ;
    varargout{3}=convenient_order ;
end

if options.multicommunity>1
    if nargout>3
        varargout{4}=multicom_nodes_all;
    else
        save([options.filename '_multicom_nodes_all.mat'], 'multicom_nodes_all')
    end
end
end
