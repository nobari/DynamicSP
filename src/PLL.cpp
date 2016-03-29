#include "PLL.h"

#include <algorithm>
#include <tbb/task_group.h>

using namespace std;
using tbb::task_group;

const uint8_t PLL::INF8 = 100;

PLL::PLL(vector<pair<uint32_t, uint32_t>> &edge_list) : num_vertices(0)
{
	// Map vertices' indices -> [0;n-1]
	for (auto &i : edge_list){
		if (!mapping.count(i.first)){
			mapping[i.first] = num_vertices++;
		}
		if (!mapping.count(i.second)){
			mapping[i.second] = num_vertices++;
		}
		i.first = mapping[i.first];
		i.second = mapping[i.second];
	}

	// Prepare adjacency lists and index space
	uint32_t &V = num_vertices;
	adj.resize(V);
	adj_r.resize(V);
	for (auto &i : edge_list){
		adj[i.first].push_back(i.second);
		adj_r[i.second].push_back(i.first);
	}
}

void PLL::pruned_bfs(vector<vector<uint32_t>> &adj,	vector<index_t> &labels, vector<bool> &visited)
{
	vector<bool> used(num_vertices);

	vector<pair<vector<uint32_t>, vector<uint8_t>>> tmp_idx(num_vertices,
		make_pair(vector<uint32_t>(1, num_vertices), vector<uint8_t>(1, INF8)));
	visited.resize(num_vertices);
	vector<uint32_t> queue(num_vertices);
	vector<uint8_t> dst_r(num_vertices + 1, INF8);
	for (uint32_t r = 0; r < num_vertices; ++r){
		if (used[r]){
			continue;
		}
		const pair<vector<uint32_t>, vector<uint8_t>> &tmp_idx_r = tmp_idx[r];
		for (size_t i = 0; i < tmp_idx_r.first.size(); ++i){
			dst_r[tmp_idx_r.first[i]] = tmp_idx_r.second[i];
		}

		uint32_t que_t0 = 0, que_t1 = 0, que_h = 0;
		queue[que_h++] = r;
		visited[r] = true;
		que_t1 = que_h;

		for (uint32_t d = 0; que_t0 < que_h; ++d){
			for (uint32_t que_i = que_t0; que_i < que_t1; ++que_i){
				uint32_t v = queue[que_i];
				pair<vector<uint32_t>, vector<uint8_t>> &tmp_idx_v = tmp_idx[v];

				// Prune?
				if (used[v]){
					continue;
				}
				for (size_t i = 0; i < tmp_idx_v.first.size(); ++i){
					uint32_t w = tmp_idx_v.first[i];
					uint32_t td = tmp_idx_v.second[i] + dst_r[w];
					if (td <= d){
						goto pruned1;
					}
				}

				// Traverse
				tmp_idx_v.first.back() = r;
				tmp_idx_v.second.back() = d;
				tmp_idx_v.first.push_back(num_vertices);
				tmp_idx_v.second.push_back(INF8);
				for (auto &i : adj[v]){
					if (!visited[i]){
						queue[que_h++] = i;
						visited[i] = true;
					}
				}
			pruned1:
				{}
			}
			que_t0 = que_t1;
			que_t1 = que_h;
		}

		for (uint32_t i = 0; i < que_h; ++i){
			visited[queue[i]] = false;
		}
		for (auto &i : tmp_idx_r.first){
			dst_r[i] = INF8;
		}
		used[r] = true;
	}

	for (uint32_t v = 0; v < num_vertices; ++v){
		size_t k = tmp_idx[v].first.size();
		labels[v].vertex.resize(k);
		labels[v].distance.resize(k);
		for (uint32_t i = 0; i < k; ++i){
			labels[v].vertex[i] = tmp_idx[v].first[i];
		}
		for (auto &i : labels[v].vertex){
			if (i == num_vertices){
				i = UINT32_MAX;
			}
		}
		tmp_idx[v].first.clear();
		for (uint32_t i = 0; i < k; ++i){
			labels[v].distance[i] = tmp_idx[v].second[i];
		}
		tmp_idx[v].second.clear();
	}
}

void PLL::construct_index()
{
	labels_in.resize(num_vertices);
	labels_out.resize(num_vertices);

	task_group tg;
	tg.run([&](){pruned_bfs(adj, labels_in, visited_in); });
	tg.run([&](){pruned_bfs(adj_r, labels_out, visited_out); });
	tg.wait();

	fill(visited_in.begin(), visited_in.end(), false);
	fill(visited_out.begin(), visited_out.end(), false);
	fill(root_label_in.begin(), root_label_in.end(), 0);
	fill(root_label_out.begin(), root_label_out.end(), 0);
}

int64_t PLL::query_distance(uint32_t source, uint32_t destination)
{
	if (!(mapping.count(source) && mapping.count(destination))){
		return -1;
	}
	else if (source == destination){
		return 0;
	}

	source = mapping[source];
	destination = mapping[destination];

	const index_t &idx_src = labels_out[source], &idx_dst = labels_in[destination];
	int64_t d = INF8;

	for (uint32_t i1 = 0, i2 = 0;;){
		uint32_t v1 = idx_src.vertex[i1], v2 = idx_dst.vertex[i2];
		if (v1 == v2){
			if (v1 == UINT32_MAX){ // Sentinel
				break;
			}
			uint32_t td = idx_src.distance[i1] + idx_dst.distance[i2];
			if (td < d){
				d = td;
			}
			++i1;
			++i2;
		}
		else {
			i1 += v1 < v2 ? 1 : 0;
			i2 += v1 > v2 ? 1 : 0;
		}
	}

	if (d >= INF8 - 2){
		d = -1;
	}
	return d;
}

void PLL::partial_bfs(uint32_t root, uint32_t vertex, uint32_t distance, vector<vector<uint32_t>> &adj, 
	vector<index_t> &labels, vector<bool> &visited, vector<pair<uint32_t, uint8_t>> &queue,	
	vector<int64_t> &root_label)
{
	if (queue.size() < num_vertices){
		queue.resize(num_vertices);
		visited.resize(num_vertices);
		root_label.resize(num_vertices);
	}

	const index_t &idx_r = labels[root];
	for (int64_t i = idx_r.vertex.size() - 1; i >= 0; --i){
		if (idx_r.vertex[i] == UINT32_MAX){
			continue;
		}
		root_label[idx_r.vertex[i]] = idx_r.distance[i];
	}

	uint32_t que_h = 0, que_t = 0;
	queue[que_t++] = make_pair(vertex, distance);
	visited[vertex] = true;

	while (que_h < que_t){
		uint32_t v = queue[que_h].first;
		uint8_t d = queue[que_h].second;
		++que_h;

		// Pruning test & new label
		{
			index_t &idx_v = labels[v];

			uint32_t i = 0;
			for (; idx_v.vertex[i] <= root; ++i){
				uint32_t l_v = idx_v.vertex[i];
				uint8_t l_d = idx_v.distance[i];
				if (root_label[l_v] != -1 && root_label[l_v] + l_d <= d){
					goto prune1;
				}
				if (l_v == root){
					break;
				}
			}

			for (int64_t j = idx_v.vertex.size() - 1; j - 1 >= i; --j){
				idx_v.vertex[j] = idx_v.vertex[j - 1];
			}
			for (int64_t j = idx_v.vertex.size() - 1; j - 1 >= i; --j){
				idx_v.distance[j] = idx_v.distance[j - 1];
			}
			idx_v.vertex.push_back(UINT32_MAX);
			idx_v.distance.push_back(0);
			idx_v.vertex[i] = root;
			idx_v.distance[i] = d;
		}

		for (int64_t i = 0; i < adj[v].size(); ++i){
			uint32_t w = adj[v][i];
			if (!visited[w]){
				queue[que_t++] = make_pair(w, d + 1);
				visited[w] = true;
			}
		}

	prune1:
		{}
	}

	for (uint32_t i = 0; i < que_t; ++i){
		visited[queue[i].first] = false;
	}
	for (int64_t i = 0; i < idx_r.vertex.size(); ++i){
		if (idx_r.vertex[i] == UINT32_MAX){
			continue;
		}
		root_label[idx_r.vertex[i]] = -1;
	}
}

void PLL::update_labels(uint32_t source, uint32_t destination,
	const index_t &idx_src, const index_t &idx_dst,
	vector<vector<uint32_t>> &adj, vector<index_t> &labels,
	vector<bool> &visited, vector<pair<uint32_t, uint8_t>> &queue,
	vector<int64_t> &root_label)
{
	uint32_t i_src = 0, i_dst = 0;
	int64_t prv_v = -1;
	while (true){
		uint32_t l_src_v = idx_src.vertex[i_src], l_dst_v = idx_dst.vertex[i_dst];
		uint8_t l_src_d = idx_src.distance[i_src], l_dst_d = idx_dst.distance[i_dst];

		if (i_src > 0 && idx_src.vertex[i_src] == prv_v){
			++i_src;
		}
		else if (i_dst > 0 && idx_dst.vertex[i_dst] == prv_v){
			++i_dst;
		}
		else if (l_src_v < l_dst_v){ // u -> v
			partial_bfs(prv_v = l_src_v, destination, l_src_d + 1, adj, labels, visited, queue, root_label);
			++i_src;
		}
		else if (l_src_v > l_dst_v){ // v -> u
			partial_bfs(prv_v = l_dst_v, source, l_dst_d + 1, adj, labels, visited, queue, root_label);
			++i_dst;
		}
		else{ // u <-> v
			if (l_src_v == UINT32_MAX){ // Sentinel
				break;
			}
			if (l_src_d + 1 < l_dst_d){
				partial_bfs(l_src_v, destination, l_src_d + 1, adj, labels, visited, queue, root_label);
			}
			if (l_src_d + 1 < l_dst_d){
				partial_bfs(l_dst_v, source, l_dst_d + 1, adj, labels, visited, queue, root_label);
			}
			prv_v = l_src_v;
			++i_src;
			++i_dst;
		}
	}
}

void PLL::insert_edge(uint32_t source, uint32_t destination)
{
	if (!(mapping.count(source) && mapping.count(destination))){
		uint32_t x = num_vertices;
		if (!mapping.count(source)){
			mapping[source] = num_vertices++;
		}
		if (!mapping.count(destination)){
			mapping[destination] = num_vertices++;
		}
		for (; x < num_vertices; ++x){
			labels_in.emplace_back();
			labels_out.emplace_back();
			labels_in.back().vertex.push_back(x);
			labels_in.back().vertex.push_back(UINT32_MAX);
			labels_in.back().distance.push_back(0);
			labels_in.back().distance.push_back(0);
			labels_out.back().vertex.push_back(x);
			labels_out.back().vertex.push_back(UINT32_MAX);
			labels_out.back().distance.push_back(0);
			labels_out.back().distance.push_back(0);
		}
		adj.resize(num_vertices);
		adj_r.resize(num_vertices);
	}

	source = mapping[source];
	destination = mapping[destination];

	adj[source].push_back(destination);
	adj_r[destination].push_back(source);
	
	task_group tg;
	tg.run([&](){update_labels(source, destination, labels_in[source], labels_in[destination], 
		adj, labels_in, visited_in, queue_in, root_label_in); });
	tg.run([&](){update_labels(destination, source, labels_out[source], labels_out[destination], 
		adj_r, labels_out, visited_out, queue_out, root_label_out); });
	tg.wait();
}

void PLL::remove_edge(uint32_t source, uint32_t destination)
{
	if (!(mapping.count(source) && mapping.count(destination))){ // Some vertex does not exist
		return;
	}

	source = mapping[source];
	destination = mapping[destination];

	auto it = find(adj[source].begin(), adj[source].end(), destination);
	if (it == adj[source].end()){ // No such edge
		return;
	}
	adj[source].erase(it);
	it = find(adj_r[destination].begin(), adj_r[destination].end(), source);
	adj_r[destination].erase(it);
	// Rebuild the index...
	labels_in.clear();
	labels_out.clear();
	fill(visited_in.begin(), visited_in.end(), false);
	fill(visited_out.begin(), visited_out.end(), false);
	construct_index();
}

PLL::~PLL()
{
}
