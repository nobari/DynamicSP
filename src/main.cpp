#include <iostream>
#include <climits>
#include <algorithm>
#include "historical_pruned_landmark_labeling.h"
#include <map>
#include <tbb/task_group.h>

using namespace std;
using tbb::task_group;

int query_snapshot(int V,
                   const vector<historical_pruned_landmark_labeling::label_entry_t> &s1,
                   const vector<historical_pruned_landmark_labeling::label_entry_t> &s2,
                                                        int v, int w, int t) {
    if (v < 0 || w < 0 || V <= v || V <= w) return -1;

    int d = INT_MAX;

    size_t i1 = 0, i2 = 0;
    while (i1 < s1.size() && i2 < s2.size()) {
        if (s1[i1].t > t) ++i1;
        else if (s2[i2].t > t) ++i2;
        else if (s1[i1].v < s2[i2].v) ++i1;
        else if (s1[i1].v > s2[i2].v) ++i2;
        else {
            int v = s1[i1].v;
            if (v == INT_MAX) break;

            d = min(d, int(s1[i1].d) + int(s2[i2].d));
            for (++i1; s1[i1].v == v; ++i1);
            for (++i2; s2[i2].v == v; ++i2);
        }
    }
    return d == INT_MAX ? -1 : d;
}

int main() {
    map<int, int> mapping;
    int V = 0;
    std::vector<std::tuple<int, int, int> > es_out, es_in;
    ios_base::sync_with_stdio(false);

    int source, destination;
    while (cin >> source >> destination){
        if (!mapping.count(source)){
            mapping[source] = V++;
        }
        if (!mapping.count(destination)){
            mapping[destination] = V++;
        }
        source = mapping[source];
        destination = mapping[destination];
        es_out.emplace_back(0, source, destination);
        es_in.emplace_back(0, destination, source);
    }
    historical_pruned_landmark_labeling hpll_out, hpll_in;
    {
        task_group g;
        g.run([&](){hpll_out.construct_index(es_out);});
        g.run([&](){hpll_in.construct_index(es_in);});
        g.wait();
    }

    cout << "R" << endl;

    char operation;
    cin.clear();
    cin >> operation;
    while (cin >> operation){
        if (operation == 'F'){
            cout.flush();
        }
        else{
            cin >> source >> destination;
            switch (operation){
                case 'Q':
                    if (!mapping.count(source) || !mapping.count(destination)){ // No such vertices
                        return -1;
                    }
                    else if (source == destination){
                        return 0;
                    }
                    source = mapping[source];
                    destination = mapping[destination];
                    cout << query_snapshot(hpll_out.V,
                                   hpll_in.labels[source], hpll_out.labels[destination], source, destination, 0) << endl;
                    break;
                case 'A':
                    if (!mapping.count(source)){
                        mapping[source] = V++;
                    }
                    if (!mapping.count(destination)){
                        mapping[destination] = V++;
                    }
                    source = mapping[source];
                    destination = mapping[destination];
                    {
                        task_group g;
                        g.run([&](){hpll_out.insert_edge(source, destination, 0);});
                        g.run([&](){hpll_in.insert_edge(destination, source, 0);});
                        g.wait();
                    }
                    break;
                case 'D':
                    if (!(mapping.count(source) && mapping.count(destination))){ // Some vertex does not exist
                        continue;
                    }
                    source = mapping[source];
                    destination = mapping[destination];
                    {
                        task_group g;
                        g.run([&](){hpll_out.remove_edge(source, destination, 0);});
                        g.run([&](){hpll_in.remove_edge(destination, source, 0);});
                        g.wait();
                    }
                    break;
            }
        }
    }

    return 0;
}