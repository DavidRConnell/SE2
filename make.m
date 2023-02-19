function make()
%MAKE compile all mex files in package
%   MAKE() compile mex files. If any files have already been compiled
%   they will be recompiled only if the source code has been edited
%   since last compilation.

    oldDir = pwd;
    cdPackage();
    try
        compileMexFiles();
    catch ME
        cd(oldDir);
        rethrow(ME);
    end
    cd(oldDir);

    function cdPackage()
        filepath = mfilename('fullpath');
        packageRoot = fileparts(filepath);
        cd(packageRoot);
    end

    function compileMexFiles()
        outDir = 'private';
        mexDir = 'mex';

        isCompiled = @(name) exist(fullfile(pwd, outDir, [name '.' mexext]), 'file') == 2;
        if ~isCompiled('discrete_nmi') || isStale('discrete_nmi')
            mex COPTIMFLAGS='-O3 -fwrapv' mex/discrete_nmi.c mex/mex_igraph.c -ligraph -outdir private
        end

        function TF = isStale(name)
            mexFile = dir(fullfile(pwd, outDir, [name '.' mexext]));
            cFile = dir(fullfile(pwd, mexDir, [name '.c']));

            TF = mexFile.datenum < cFile.datenum;
        end
    end
end
