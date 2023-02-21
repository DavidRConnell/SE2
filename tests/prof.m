function prof(network)
    oldDir = pwd;
    cd('..'); % Hack to find repo top level to get access to SpeakEasy2
    try
        n = load(fullfile('~/data/matlab/networks', network), network);
        profile clear
        profile on
        [~] = SpeakEasy2(n.(network), "independent_runs", 1, 'random_seed', 1);
        profile off
        profsave
    catch ME
        cd(oldDir)
        rethrow(ME)
    end
    cd(oldDir)
end
