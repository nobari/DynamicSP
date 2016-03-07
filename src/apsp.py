#!/usr/bin/env python

import networkx as nx
import sys

# Read in the list of edges
edges = []
while True:
    line = sys.stdin.readline()

    # Stop when we hit the end of the graph
    if line.strip() == 'S':
        break

    # Store the edge to be added
    edges.append(map(int, line.split()))

# Construct a new graph object
G = nx.DiGraph()
G.add_edges_from(edges)

# Signify that we are ready for queries
print('R')
sys.stdout.flush()

# Iterate through all queries/updates
while True:
    line = sys.stdin.readline()

    # Check if we're done
    if line.strip() == '':
        break

    # When done with a batch flush output and continue
    if line.strip() == 'F':
        sys.stdout.flush()
        continue

    cmd, n1, n2 = line.split()
    n1 = int(n1)
    n2 = int(n2)

    if cmd == 'D':
        try:
            G.remove_edge(n1, n2)
        except nx.exception.NetworkXError:
            # Don't fail if the edge does not exist
            pass
    elif cmd == 'Q':
        try:
            # Get the shortest path (remove one since we count nodes)
            print(len(nx.shortest_path(G, n1, n2)) - 1)
        except (nx.exception.NetworkXError, nx.exception.NetworkXNoPath):
            # Node is not reachable or some nodes do not exist
            print(-1)
    if cmd == 'A':
        try:
            G.add_edge(n1, n2)
        except nx.exception.NetworkXError:
            # Don't fail if the nodes do not exist
            pass
