function p = leidenCpm(adj)
%LEIDENCPM Wrapper around leiden to optimize with CPM instead of modularity
    p = igraph.leiden(adj, 'method', 'cpm');
end
