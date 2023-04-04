function regression(network, varargin)
%REGRESSION compare current SE2 solution to previous solutions
%
%   REGRESSION(NETWORK) Run SE2 on the network named NETWORK in
%   ~/data/matlab/networks. If the directory ./cache/NETWORK doesn't exist,
%   then runs SE2 with randomly generated seeds and saves the results in the
%   network's cache. If the cache directory does already exist, it runs using
%   the same seeds as from the initial run and then compares the old results
%   to the new runs.
%
%   REGRESSION(..., 'INVALIDATECACHE', TF) If true delete the cache then
%   reperform the initial run.
%   Note: chooses new random seeds.
%
%   REGRESSION(..., 'REPS', N) How many runs to perform. This can only be
%   performed if the cache hasn't been created or is being invalidated.

    isnonnegativeint = @(x) isnumeric(x) && isscalar(x) && ...
        (x == floor(x)) && (x > 0);
    p = inputParser;
    p.addParameter('InvalidateCache', false, @islogical);
    p.addParameter('Reps', 5, isnonnegativeint)
    p.parse(varargin{:});

    [cache, iscached] = getCache(network);
    if p.Results.InvalidateCache && iscached
        delete(cache);
        iscached = false;
    end

    if all(~contains(p.UsingDefaults, 'Reps')) && iscached
        error('Cache exists; cannot select number of reps.')
    end

    oldDir = pwd;
    cd('..'); % Hack to find repo top level to get access to SpeakEasy2
    try
        n = load(fullfile('~/data/matlab/networks', network), network);
        if ~iscached
            initializeCache(n.(network), cache, p.Results.Reps);
        else
            compareToCache(n.(network), cache);
        end
    catch ME
        cd(oldDir)
        rethrow(ME)
    end
    cd(oldDir)
end

function [cache, iscached] = getCache(name)
    testDir = fileparts(mfilename('fullpath'));
    cache = fullfile(testDir, 'cache', [name '.mat']);
    iscached = exist(cache, 'file');
end

function initializeCache(adj, cache, reps)
    warning('No cache found for network; generating cache.');
    seeds = randi([1, 9999], 1, reps);

    partitions = struct();
    for s = seeds
        p = SpeakEasy2(adj, ...
                       'random_seed', s, ...
                       'independent_runs', 3, ...
                       'target_partitions', 3);
        partitions.(sprintf("p_%d", s)) = p{1};
    end
    save(cache, '-struct', 'partitions');
end

function compareToCache(adj, cache)
    partitions = load(cache);
    results = zeros(length(fieldnames(partitions)), 1, 'logical');
    i = 0;
    for f = fieldnames(partitions)'
        i = i + 1;
        seed = str2num(regexp(f{1}, '\d*', 'once', 'match'));
        p_new = SpeakEasy2(adj, ...
                       'random_seed', seed, ...
                       'independent_runs', 3, ...
                       'target_partitions', 3);
        p_new = p_new{1};
        p_cached = partitions.(f{1});

        results(i) = isequal(p_new, p_cached);
    end

    fprintf("\n");
    if all(results)
        disp('All cached partitions were reproduced.');
    elseif all(~results)
        fprintf(2, '\nNo cached partitions were reproduced\n');
    else
        fprintf(2, '\n%d of %d partitions reproduced.\n', sum(results), length(results));
    end
end
