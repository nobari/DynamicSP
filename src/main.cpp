#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <memory>
#include <cstdint>
#include <tbb/parallel_for.h>
#include <tbb/concurrent_queue.h>
#include <boost/align/aligned_allocator.hpp>

using namespace std;
using boost::alignment::aligned_allocator;

class Graph
{
public:
	Graph() : current(new tbb::concurrent_bounded_queue<uint32_t>()), next(new tbb::concurrent_bounded_queue<uint32_t>()) {}
	void add_edge(uint32_t source, uint32_t destination)
	{
		if (!mapping.count(source)){
			mapping[source] = list.size();
			add_vertex();
		}
		if (!mapping.count(destination)){
			mapping[destination] = list.size();
			add_vertex();
		}
		source = mapping[source];
		destination = mapping[destination];
		auto it = lower_bound(list[source].begin(), list[source].end(), destination);
		if (it == list[source].end() || *it != destination){
			list[source].insert(it, destination);
		}
	}
	void delete_edge(uint32_t source, uint32_t destination)
	{
		if (mapping.count(source) && mapping.count(destination)){
			source = mapping[source];
			destination = mapping[destination];
			auto it = lower_bound(list[source].begin(), list[source].end(), destination);
			if (it != list[source].end() && *it == destination){
				list[source].erase(it);
			}
		}
	}
	int64_t bfs(uint32_t source, uint32_t destination)
	{
		if (source == destination){
			return 0;
		}
		if (mapping.count(source) && mapping.count(destination)){
			source = mapping[source];
			destination = mapping[destination];
			uint32_t next_level = 1;
			current->clear();
			current->push(source);
			next->clear();
			fill(visited.begin(), visited.end(), 0);
			visited[source] = true;

			while (!current->empty()){
				tbb::parallel_for(uint32_t(0), uint32_t(current->size()), uint32_t(1), [&](uint32_t i){
					uint32_t index;
					current->pop(index);
					for (auto &neighbor : list[index]){
						if (!visited[neighbor]){ // race condition does not affect result
							visited[neighbor] = true;
							next->push(neighbor);
						}
					}
				});

				if (visited[destination]){
					return next_level;
				}
				++next_level;
				swap(current, next);
			}
		}
		return -1;
	}

private:
	vector<vector<uint32_t, aligned_allocator<uint32_t, 16>>> list;
	vector<uint8_t, aligned_allocator<uint8_t, 16>> visited;
	unique_ptr<tbb::concurrent_bounded_queue<uint32_t>> current, next;
	map<uint32_t, uint32_t, less<uint32_t>, aligned_allocator<pair<const uint32_t, uint32_t>, 16>> mapping;

	void add_vertex()
	{
		list.push_back(vector<uint32_t, aligned_allocator<uint32_t, 16>>());
		visited.push_back(0);
	}
};

int main()
{
	ios_base::sync_with_stdio(false);

	Graph g;
	uint32_t source, destination;
	while (cin >> source >> destination){
		g.add_edge(source, destination);
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
				cout << g.bfs(source, destination) << endl;
				break;
			case 'A':
				g.add_edge(source, destination);
				break;
			case 'D':
				g.delete_edge(source, destination);
				break;
			}
		}
	}

	return 0;
}