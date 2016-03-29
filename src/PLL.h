#pragma once
#include <cstdint>
#include <vector>
#include <map>

class PLL
{
	static const uint8_t INF8;

	std::map<uint32_t, uint32_t> mapping;
	struct index_t{
		std::vector<uint32_t> vertex;
		std::vector<uint8_t> distance;
	};
	std::vector<index_t> labels_in, labels_out;
	uint32_t num_vertices;

	std::vector<std::vector<uint32_t>> adj, adj_r;

	std::vector<bool> visited_in, visited_out;
	std::vector<std::pair<uint32_t, uint8_t>> queue_in, queue_out;
	std::vector<int64_t> root_label_in, root_label_out;

	void pruned_bfs(std::vector<std::vector<uint32_t>> &adj, 
		std::vector<index_t> &labels, std::vector<bool> &visited);

	void partial_bfs(uint32_t root, uint32_t vertex, uint32_t distance, 
		std::vector<std::vector<uint32_t>> &adj, std::vector<index_t> &labels, 
		std::vector<bool> &visited, std::vector<std::pair<uint32_t, uint8_t>> &queue, 
		std::vector<int64_t> &root_label);

	void update_labels(uint32_t source, uint32_t destination,
		const index_t &idx_src, const index_t &idx_dst,
		std::vector<std::vector<uint32_t>> &adj, std::vector<index_t> &labels,
		std::vector<bool> &visited, std::vector<std::pair<uint32_t, uint8_t>> &queue,
		std::vector<int64_t> &root_label);
public:
	PLL(std::vector<std::pair<uint32_t, uint32_t>> &edge_list);
	void construct_index();
	int64_t query_distance(uint32_t source, uint32_t destination);
	void insert_edge(uint32_t source, uint32_t destination);
	void remove_edge(uint32_t source, uint32_t destination);
	~PLL();
};

