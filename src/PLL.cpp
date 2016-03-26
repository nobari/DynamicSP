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
	vector<vector<uint32_t>> adj(V), adj_r(V);
	for (auto &i : edge_list){
		adj[i.first].push_back(i.second);
		adj_r[i.second].push_back(i.first);
	}
	index.resize(V);

	vector<bool> used(V);

	// Pruned labeling - in
	vector<pair<vector<uint32_t>, vector<uint8_t>>> tmp_idx(V,
		make_pair(vector<uint32_t>(1, V), vector<uint8_t>(1, INF8)));
	vector<bool> visited(V);
	vector<uint32_t> queue(V);
	vector<uint8_t> dst_r(V + 1, INF8);
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
			tmp_idx[v].first.clear();
			for (uint32_t i = 0; i < k; ++i){
				index[v].distance_in[i] = tmp_idx[v].second[i];
			}
			tmp_idx[v].second.clear();
		}
	}

	fill(used.begin(), used.end(), false);

	// Pruned labeling - out
	fill(tmp_idx.begin(), tmp_idx.end(), make_pair(vector<uint32_t>(1, V), vector<uint8_t>(1, INF8)));
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
			if (v1 == num_vertices){ // Sentinel
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

PLL::~PLL()
{
}
