
Instructions for clustering networks with SpeakEasy2:Champagne

To cluster data, 
Download SpeakEasy2: Champagne
Navigate to the SpeakEasy2 folder in matlab (or put it on your matlab path) and type: load ADJdemo
At the matlab prompt type… SpeakEasy2(ADJdemo).
You’ve clustered some data!  You’ll see results have been saved in the folder as SpeakEasy2_results.mat.  If you’d like to verify that SpeakEasy2 is producing plausible results, copy this into the matlab terminal:

load SpeakEasy2_results
figure('Position',[0 0 1000 400]) <br />
subplot(1,2,1); imagesc(ADJ) <br />
title('original input ADJ') <br />
xlabel('nodes') <br />
ylabel('nodes') <br />
subplot(1,2,2); imagesc(ADJcolorful(convenient_order{1},convenient_order{1})) <br />
title('ADJ reorganized by SE2 clusters') <br />
xlabel('reordered nodes') <br />
ylabel('reordered nodes') <br />
colormap(cmap); <br />


On the left you can see the randomly arranged network input to SE2, while the image on the right shows the same data arranged according to the clusters found by SE2.  You can see that nodes have been organized into groups (along the diagonal).  These have been color-coded with the ground truth communities, and you can see in most cases the original communities are received (nodes in a group tend to be the same color).

If you want to apply SE2 to some of your own data, just substitute your adjacency matrix for the ADJ from load ADJdemo.

If you are interested in further customizing the application of SpeakEasy2, continue reading, but the basic function above works for many data.



Runtime - 
If you are running a multi-core machine, have the matlab parallel toolbox installed, and wish to explicitly parallelize SE2 for faster execution, use the max_threads parameter.
  For instance, >>SpeakEasy2(ADJdemo,’max_threads’,50) will utilize up to 50 threads.  Performance gains from multiple cores/threads are substantial.  However, a copy of the ADJ is sent to each worker, so for very large networks, you may have to limit the number of workers to fit in memory.


Accuracy - 
The more independent partitions that are generated by SE2, the more stable and accurate the final partition solution will be.  In practice, 10 estimated partitions are generally sufficient, but if you wish to generate more partitions to slightly improve accuracy, you can use the independent_runs parameter to increase this.
  For instance, >>SpeakEasy2(ADJdemo,’independent_runs’, 20) will double the default number of estimated partitions.


Overlapping clusters - 
By default SE2 returns disjoint communities, but you can enable varying levels of overlapping output with the “multicommunity” parameter.
  For instance, >>SpeakEasy2(ADJdemo,’multicommunity’, 4) allows nodes to be members in up to 4 communities.  The usual saved file will indicate nodes with multiple communities (it’s length will be greater than the number of nodes), and if you just want a list of the multi-community nodes, that is saved as well.


Subclustering - 
Occasionally your data will have multiple scales, or large batch effects that might look like clusters, and you need to get underneath them for meaningful results.  (Often the case for bulk RNAseq, but generally not single cell RNAseq)
You can apply SpeakEasy2 to each of the top-tier clusters with subcluster.  This may be done multiple times if you wish (i.e. sub-sub clustering).
   For instance, >>SpeakEasy2(ADJdemo,’subcluster’, 3) produces three levels of clustering - the usual top-level clusters, as well as sub-clusters and sub-sub clusters.  You can access lower levels of clusters as follows:
  >> load SpeakEasy2_results
partition_tags{1}; %these are the top-level communities
partition_tags{2}; %sub clusters
partition_tags{3}; %sub-sub clusters

You may not wish to split communities below a certain size, which is facilitated by minclust.  For instance >>SpeakEasy2(ADJdemo,’subcluster’, 3,’minclust’,20) will not subcluster primary clusters with less than 20 nodes.


I/O -
If you wish to access the partition without loading a saved file, you can run a command with 1-3 outputs:

load ADJdemo
[node_tags  node_groups convenient_order]=SpeakEasy2(ADJ);

In this case node_tags{1} will contain two columns - the first is the node ID and the 2nd is an arbitrary numeric cluster assignment.  For instance (hypothetically) it could contain:
1 1
2 1
3 2 
4 3
5 3

-the first output matrix ('node_tags') above denotes a 5-node network, with three clusters.

The second output ('node_groups') lists the nodes associated with the three clusters (it is a cell of length 3).

The third output ('convenient_order') orders nodes for convenient visualization (i.e. imagesc(ADJ(convenient_order{1},convenient_order{1}) is visually interpretable).




