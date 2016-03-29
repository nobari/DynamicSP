#pragma once
#include <cstdint>
#include <vector>
#include <map>

class PLL
{
	static const uint8_t INF8 = 100;
	std::map<uint32_t, uint32_t> mapping;
	struct index_t{
		std::vector<uint32_t> vertex_in, vertex_out;
		std::vector<uint8_t> distance_in, distance_out;
	};
	std::vector<index_t> index;
	uint32_t num_vertices;

	std::vector<std::vector<uint32_t>> adj, adj_r;
	std::vector<bool> visited;
	std::vector<std::pair<uint32_t, uint8_t>> queue;
	std::vector<int64_t> root_label;

	void partial_bfs(uint32_t root, uint32_t vertex, uint32_t distance);
	void partial_bfs_r(uint32_t root, uint32_t vertex, uint32_t distance);
public:
	PLL();
	void construct_index(std::vector<std::pair<uint32_t, uint32_t>> &edge_list);
	int64_t query_distance(uint32_t source, uint32_t destination);
	void insert_edge(uint32_t source, uint32_t destination);
	~PLL();
};

