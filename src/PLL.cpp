#include "PLL.h"

using namespace std;

PLL::PLL() : num_vertices(0)
{
}

void PLL::construct_index(vector<pair<uint32_t, uint32_t>> &edge_list)
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
	index.resize(V);

	vector<bool> used(V);

	// Pruned labeling - in
	vector<pair<vector<uint32_t>, vector<uint8_t>>> tmp_idx(V,
		make_pair(vector<uint32_t>(1, V), vector<uint8_t>(1, INF8)));
	visited.resize(V);
	vector<uint32_t> queue(V);
	vector<uint8_t> dst_r(V + 1, INF8);
	{
		for (uint32_t r = 0; r < V; ++r){
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
					tmp_idx_v.first.push_back(V);
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

		for (uint32_t v = 0; v < V; ++v){
			size_t k = tmp_idx[v].first.size();
			index[v].vertex_in.resize(k);
			index[v].distance_in.resize(k);
			for (uint32_t i = 0; i < k; ++i){
				index[v].vertex_in[i] = tmp_idx[v].first[i];
			}
			for (auto &i : index[v].vertex_in){
				if (i == V){
					i = UINT32_MAX;
				}
			}
			tmp_idx[v].first.clear();
			for (uint32_t i = 0; i < k; ++i){
				index[v].distance_in[i] = tmp_idx[v].second[i];
			}
			tmp_idx[v].second.clear();
		}
	}

	fill(used.begin(), used.end(), false);

	// Pruned labeling - out
	fill(tmp_idx.begin(), tmp_idx.end(), 
		make_pair(vector<uint32_t>(1, V), vector<uint8_t>(1, INF8)));
	fill(visited.begin(), visited.end(), false);
	fill(queue.begin(), queue.end(), 0);
	fill(dst_r.begin(), dst_r.end(), INF8);
	{
		for (uint32_t r = 0; r < V; ++r){
			if (used[r]){
				continue;
			}
			index_t &idx_r = index[r];
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
					index_t &idx_v = index[v];

					// Prune?
					if (used[v]){
						continue;
					}
					for (size_t i = 0; i < tmp_idx_v.first.size(); ++i){
						uint32_t w = tmp_idx_v.first[i];
						uint32_t td = tmp_idx_v.second[i] + dst_r[w];
						if (td <= d){
							goto pruned2;
						}
					}

					// Traverse
					tmp_idx_v.first.back() = r;
					tmp_idx_v.second.back() = d;
					tmp_idx_v.first.push_back(V);
					tmp_idx_v.second.push_back(INF8);
					for (auto &i : adj_r[v]){
						if (!visited[i]){
							queue[que_h++] = i;
							visited[i] = true;
						}
					}
				pruned2:
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

		for (uint32_t v = 0; v < V; ++v){
			size_t k = tmp_idx[v].first.size();
			index[v].vertex_out.resize(k);
			index[v].distance_out.resize(k);
			for (uint32_t i = 0; i < k; ++i){
				index[v].vertex_out[i] = tmp_idx[v].first[i];
			}
			for (auto &i : index[v].vertex_out){
				if (i == V){
					i = UINT32_MAX;
				}
			}
			tmp_idx[v].first.clear();
			for (uint32_t i = 0; i < k; ++i){
				index[v].distance_out[i] = tmp_idx[v].second[i];
			}
			tmp_idx[v].second.clear();
		}
	}
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

	const index_t &idx_src = index[source], &idx_dst = index[destination];
	int64_t d = INF8;

	for (uint32_t i1 = 0, i2 = 0;;){
		uint32_t v1 = idx_src.vertex_out[i1], v2 = idx_dst.vertex_in[i2];
		if (v1 == v2){
			if (v1 == UINT32_MAX){ // Sentinel
				break;
			}
			uint32_t td = idx_src.distance_out[i1] + idx_dst.distance_in[i2];
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

void PLL::partial_bfs(uint32_t root, uint32_t vertex, uint32_t distance)
{
	if (queue.size() < num_vertices){
		queue.resize(num_vertices);
		visited.resize(num_vertices);
		root_label.resize(num_vertices);
	}

	const index_t &idx_r = index[root];
	for (int64_t i = idx_r.vertex_in.size() - 1; i >= 0; --i){
		if (idx_r.vertex_in[i] == UINT32_MAX){
			continue;
		}
		root_label[idx_r.vertex_in[i]] = idx_r.distance_in[i];
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
			index_t &idx_v = index[v];

			uint32_t i = 0;
			for (; idx_v.vertex_in[i] <= root; ++i){
				uint32_t l_v = idx_v.vertex_in[i];
				uint8_t l_d = idx_v.distance_in[i];
				if (root_label[l_v] != -1 && root_label[l_v] + l_d <= d){
					goto prune1;
				}
				if (l_v == root){
					break;
				}
			}

			for (int64_t j = idx_v.vertex_in.size() - 1; j - 1 >= i; --j){
				idx_v.vertex_in[j] = idx_v.vertex_in[j - 1];
			}
			for (int64_t j = idx_v.vertex_in.size() - 1; j - 1 >= i; --j){
				idx_v.distance_in[j] = idx_v.distance_in[j - 1];
			}
			idx_v.vertex_in.push_back(UINT32_MAX);
			idx_v.distance_in.push_back(0);
			idx_v.vertex_in[i] = root;
			idx_v.distance_in[i] = d;
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
	for (int64_t i = 0; i < idx_r.vertex_in.size(); ++i){
		if (idx_r.vertex_in[i] == UINT32_MAX){
			continue;
		}
		root_label[idx_r.vertex_in[i]] = -1;
	}
}

void PLL::partial_bfs_r(uint32_t root, uint32_t vertex, uint32_t distance)
{
	if (queue.size() < num_vertices){
		queue.resize(num_vertices);
		visited.resize(num_vertices);
		root_label.resize(num_vertices);
	}

	const index_t &idx_r = index[root];
	for (int64_t i = idx_r.vertex_out.size() - 1; i >= 0; --i){
		if (idx_r.vertex_out[i] == UINT32_MAX){
			continue;
		}
		root_label[idx_r.vertex_out[i]] = idx_r.distance_out[i];
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
			index_t &idx_v = index[v];

			uint32_t i = 0;
			for (; idx_v.vertex_out[i] <= root; ++i){
				uint32_t l_v = idx_v.vertex_out[i];
				uint8_t l_d = idx_v.distance_out[i];
				if (root_label[l_v] != -1 && root_label[l_v] + l_d <= d){
					goto prune2;
				}
				if (l_v == root){
					break;
				}
			}

			for (int64_t j = idx_v.vertex_out.size() - 1; j - 1 >= i; --j){
				idx_v.vertex_out[j] = idx_v.vertex_out[j - 1];
			}
			for (int64_t j = idx_v.vertex_out.size() - 1; j - 1 >= i; --j){
				idx_v.distance_out[j] = idx_v.distance_out[j - 1];
			}
			idx_v.vertex_out.push_back(UINT32_MAX);
			idx_v.distance_out.push_back(0);
			idx_v.vertex_out[i] = root;
			idx_v.distance_out[i] = d;
		}

		for (int64_t i = 0; i < adj_r[v].size(); ++i){
			uint32_t w = adj_r[v][i];
			if (!visited[w]){
				queue[que_t++] = make_pair(w, d + 1);
				visited[w] = true;
			}
		}

	prune2:
		{}
	}

	for (uint32_t i = 0; i < que_t; ++i){
		visited[queue[i].first] = false;
	}
	for (int64_t i = 0; i < idx_r.vertex_out.size(); ++i){
		if (idx_r.vertex_out[i] == UINT32_MAX){
			continue;
		}
		root_label[idx_r.vertex_out[i]] = -1;
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
			index.emplace_back();
			index.back().vertex_in.push_back(x);
			index.back().vertex_in.push_back(UINT32_MAX);
			index.back().distance_in.push_back(0);
			index.back().distance_in.push_back(0);
			index.back().vertex_out.push_back(x);
			index.back().vertex_out.push_back(UINT32_MAX);
			index.back().distance_out.push_back(0);
			index.back().distance_out.push_back(0);
		}
		adj.resize(num_vertices);
		adj_r.resize(num_vertices);
	}

	source = mapping[source];
	destination = mapping[destination];

	adj[source].push_back(destination);
	adj_r[destination].push_back(source);
	const index_t &idx_src = index[source], &idx_dst = index[destination];

	// v_k ... (a,b) ... u
	fill(visited.begin(), visited.end(), false);
	fill(root_label.begin(), root_label.end(), 0);
	uint32_t i_src = 0, i_dst = 0;
	int64_t prv_v = -1;
	while (true){
		pair<uint32_t, uint8_t> label_src = make_pair(idx_src.vertex_in[i_src],
			idx_src.distance_in[i_src]),
			label_dst = make_pair(idx_dst.vertex_in[i_dst], idx_dst.distance_in[i_dst]);

		if (i_src > 0 && idx_src.vertex_in[i_src] == prv_v){
			++i_src;
		}
		else if (i_dst > 0 && idx_dst.vertex_in[i_dst] == prv_v){
			++i_dst;
		}
		else if (label_src.first < label_dst.first){ // u -> v
			partial_bfs(prv_v = label_src.first, destination, label_src.second + 1);
			++i_src;
		}
		else if (label_src.first > label_dst.first){ // v -> u
			partial_bfs(prv_v = label_dst.first, source, label_dst.second + 1);
			++i_dst;
		}
		else{ // u <-> v
			if (label_src.first == UINT32_MAX){ // Sentinel
				break;
			}
			if (label_src.second + 1 < label_dst.second){
				partial_bfs(label_src.first, destination, label_src.second + 1);
			}
			if (label_src.second + 1 < label_dst.second){
				partial_bfs(label_dst.first, source, label_dst.second + 1);
			}
			prv_v = label_src.first;
			++i_src;
			++i_dst;
		}
	}

	// u ... (a,b) ... v_k
	fill(visited.begin(), visited.end(), false);
	fill(root_label.begin(), root_label.end(), 0);
	i_src = 0; 
	i_dst = 0; 
	prv_v = -1;
	while (true){
		pair<uint32_t, uint8_t> label_src = make_pair(idx_src.vertex_out[i_src],
			idx_src.distance_out[i_src]),
			label_dst = make_pair(idx_dst.vertex_out[i_dst], idx_dst.distance_out[i_dst]);

		if (i_src > 0 && idx_src.vertex_out[i_src] == prv_v){
			++i_src;
		}
		else if (i_dst > 0 && idx_dst.vertex_out[i_dst] == prv_v){
			++i_dst;
		}
		else if (label_src.first < label_dst.first){ // u -> v
			partial_bfs_r(prv_v = label_src.first, source, label_src.second + 1);
			++i_src;
		}
		else if (label_src.first > label_dst.first){ // v -> u
			partial_bfs_r(prv_v = label_dst.first, destination, label_dst.second + 1);
			++i_dst;
		}
		else{ // u <-> v
			if (label_src.first == UINT32_MAX){ // Sentinel
				break;
			}
			if (label_src.second + 1 < label_dst.second){
				partial_bfs_r(label_src.first, source, label_src.second + 1);
			}
			if (label_src.second + 1 < label_dst.second){
				partial_bfs_r(label_dst.first, destination, label_dst.second + 1);
			}
			prv_v = label_src.first;
			++i_src;
			++i_dst;
		}
	}
}

PLL::~PLL()
{
}
