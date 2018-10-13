#include "hypergraph.h"

#include <fstream>
#include <sstream>
#include <map>
#include <utility>

#define NOT_EXTENDABLE 0
#define EXTENDABLE 1
#define MINIMAL 2

Hypergraph::Hypergraph() {
}

Hypergraph::Hypergraph(const Hypergraph &other) {
	m_edges = other.m_edges;
	m_num_vertices = other.m_num_vertices;
}

Hypergraph::Hypergraph(std::string path) {
	m_edges.clear();
	std::ifstream infile(path);
	std::string s_num_vertices;
	getline(infile, s_num_vertices);
	int num_vertices = std::stoi(s_num_vertices);
	if (num_vertices <= 0) throw "Tried to read graph with less than one vertex from file!";
	m_num_vertices = num_vertices;
	edge_set edges;
	for (std::string line; getline(infile, line); ) {
		std::stringstream ss(line);
		std::string node;
		edge e((edge::size_type) num_vertices);
		while (std::getline(ss, node, ',')) {
			int i_vertex = std::stoi(node);
			if (i_vertex < 0 || i_vertex >= num_vertices) throw "Found corrupt edge while reading graph from file!";
			e[(edge::size_type) i_vertex] = 1;
		}
		edges.insert(e);
	}
	for (auto e : edges) m_edges.push_back(e);
}

Hypergraph::Hypergraph(const Table &t) {
	if (t.empty()) return;
	m_num_vertices = (int)t.m_records[0].size();
	m_edges = t.edges();
	minimize();
}

Hypergraph::~Hypergraph() {
}

void Hypergraph::print_edges() const {
	print_edge_vec(m_edges);
}

void Hypergraph::save(std::string path) const {
	std::ofstream outfile;
	outfile.open(path);
	outfile << m_num_vertices << std::endl;
	for (auto e : m_edges) {
		auto num_vertices = e.count();
		if (num_vertices == 0) {
			std::cerr << "Graph contains empty edge!" << std::endl;
			continue;
		}
		auto index = e.find_first();
		edge::size_type i = 0;
		while (i < num_vertices - 1) {
			outfile << index << ",";
			index = e.find_next(index);
			++i;
		}
		outfile << index << std::endl;
	}
	outfile.close();
}

bool Hypergraph::is_hitting_set(const edge &h) const {
	for (auto e : m_edges) if (!e.intersects(h)) return false;
	return true;
}

int Hypergraph::extendable(const edge &x, const edge &y) {
#ifdef ORACLE_STATS
	Clock::time_point oracle_timestamp = Clock::now();
#endif
	// 2
	if (x.none()) {
		// 3
		if (is_hitting_set(~y)) {
#ifdef ORACLE_STATS
			auto now = Clock::now();
			m_oracle_stats.add_record({ edge_to_string(x), edge_to_string(y), "3", ns_string(oracle_timestamp, now), "0", "0" });
#endif
			return EXTENDABLE;
		}
		// 4
#ifdef ORACLE_STATS
		auto now = Clock::now();
		m_oracle_stats.add_record({ edge_to_string(x), edge_to_string(y), "4", ns_string(oracle_timestamp, now), "0", "0" });
#endif
		return NOT_EXTENDABLE;
	}
	// 5
	edge_vec t;
	// 6
	std::vector<edge_vec> s(x.count(), edge_vec());
	std::vector<std::vector<edge_vec>::size_type> x_index_to_s_index(m_num_vertices, -1);
	std::vector<edge_vec>::size_type s_index = -1;
	for (auto x_index = x.find_first(); x_index != edge::npos; x_index = x.find_next(x_index)) x_index_to_s_index[x_index] = ++s_index;
	// 7
	for (edge e : m_edges) {
		edge intersection = e & x;
		// 9
		if (intersection.none()) {
			t.push_back(e - y);
			continue;
		}
		// 8
		if (intersection.count() == 1) s[x_index_to_s_index[intersection.find_first()]].push_back(e - y);
	}
	// 10
	for (auto sx : s) if (sx.empty()) {
#ifdef ORACLE_STATS
		auto now = Clock::now();
		m_oracle_stats.add_record({ edge_to_string(x), edge_to_string(y), "10", ns_string(oracle_timestamp, now), "0", "0" });
#endif
		return NOT_EXTENDABLE;
	}
	// 11
	if (t.empty()) {
#ifdef ORACLE_STATS
		auto now = Clock::now();
		m_oracle_stats.add_record({ edge_to_string(x), edge_to_string(y), "11", ns_string(oracle_timestamp, now), "0", "0" });
#endif
		return MINIMAL;
	}
	// 12
#ifdef ORACLE_STATS
	int iteration_count = 0;
	Clock::time_point oracle_bf_timestamp = Clock::now();
#endif
	std::vector<edge::size_type> iteration_position(x.count(), 0);
	while (true) {
#ifdef ORACLE_STATS
		iteration_count++;
#endif
		// 13
		edge w(m_num_vertices);
		bool increase_next = true;
		for (std::vector<edge_vec>::size_type i_s = 0; i_s < s.size(); ++i_s) {
			w |= s[i_s][iteration_position[i_s]];
			if (increase_next) {
				++iteration_position[i_s];
				if (iteration_position[i_s] == s[i_s].size()) iteration_position[i_s] = 0;
				else increase_next = false;
			}
		}
		// 14
		bool all_no_subset = true;
		for (edge e : t) {
			if (e.is_subset_of(w)) {
				all_no_subset = false;
				break;
			}
		}
		if (all_no_subset) {
#ifdef ORACLE_STATS
			auto now = Clock::now();
			m_oracle_stats.add_record({ edge_to_string(x), edge_to_string(y), "14", ns_string(oracle_timestamp, now), ns_string(oracle_bf_timestamp, now), std::to_string(iteration_count) });
#endif
			return EXTENDABLE;
		}
		if (increase_next) break;
	}
	// 15
#ifdef ORACLE_STATS
	auto now = Clock::now();
	m_oracle_stats.add_record({ edge_to_string(x), edge_to_string(y), "15", ns_string(oracle_timestamp, now), ns_string(oracle_bf_timestamp, now), std::to_string(iteration_count) });
#endif
	return NOT_EXTENDABLE;
}

edge_vec Hypergraph::enumerate(
#ifdef PRINT_TO_FILE
	std::ofstream &outfile
#endif
) {
	edge_vec minimal_hitting_sets;
#ifdef HITTING_SET_STATS
	m_hitting_set_stats.clear();
	m_hitting_set_stats.add_record({ "minimal_hitting_set", "delay_ns" });
#endif
#ifdef PRINT_HITTING_SETS
	std::cout << "ENUMERATION:\nminimal_hitting_set,delay_ns\n";
#endif
#ifdef PRINT_TO_FILE
	outfile << "minimal_hitting_set,delay_ns" << std::endl;
#endif
#if defined(HITTING_SET_STATS) || defined(PRINT_HITTING_SETS) || defined(PRINT_TO_FILE)
	m_hitting_set_timestamp = Clock::now();
#endif
#ifdef ORACLE_STATS
	m_oracle_stats.clear();
	m_oracle_stats.add_record({ "x", "y", "return_line", "time_ns", "bf_time_ns", "iteration_count" });
#endif
	enumerate(edge(m_num_vertices), edge(m_num_vertices), 0, minimal_hitting_sets
#ifdef PRINT_TO_FILE
		, outfile
#endif
	);
#ifdef PRINT_HITTING_SETS
	auto now = Clock::now();
	std::cout << "until return," << ns_string(m_hitting_set_timestamp, now) << std::endl;
#endif
#ifdef PRINT_TO_FILE
	auto now = Clock::now();
	outfile << "until return," << ns_string(m_hitting_set_timestamp, now) << std::endl;
#endif
	return minimal_hitting_sets;
}

void Hypergraph::enumerate(const edge &x, const edge &y, edge::size_type r, edge_vec &minimal_hitting_sets
#ifdef PRINT_TO_FILE
	, std::ofstream &outfile
#endif
) {
	edge xv = x;
	xv[r] = 1;
	switch (extendable(xv, y)) {
	case MINIMAL:
	{
		minimal_hitting_sets.push_back(xv);
#ifdef HITTING_SET_STATS
		auto now = Clock::now();
		m_hitting_set_stats.add_record({ edge_to_string(xv), ns_string(m_hitting_set_timestamp, now) });
		m_hitting_set_timestamp = Clock::now();
#endif
#ifdef PRINT_HITTING_SETS
		auto now = Clock::now();
		std::cout << edge_to_string(xv) << "," << ns_string(m_hitting_set_timestamp, now) << "\n";
		m_hitting_set_timestamp = Clock::now();
#endif
#ifdef PRINT_TO_FILE
		auto now = Clock::now();
		outfile << edge_to_string(xv) << "," << ns_string(m_hitting_set_timestamp, now) << std::endl;
		m_hitting_set_timestamp = Clock::now();
#endif

		break;
	}
	case EXTENDABLE:
		enumerate(xv, y, r + 1, minimal_hitting_sets
#ifdef PRINT_TO_FILE
			, outfile
#endif
		);
		break;
	case NOT_EXTENDABLE:
		edge yv = y;
		yv[r] = 1;
		enumerate(x, yv, r + 1, minimal_hitting_sets
#ifdef PRINT_TO_FILE
			, outfile
#endif
		);
		return;
	}
	edge yv = y;
	yv[r] = 1;
	if (extendable(x, yv)) enumerate(x, yv, r + 1, minimal_hitting_sets
#ifdef PRINT_TO_FILE
		, outfile
#endif
	);
}

edge_vec Hypergraph::enumerate_legacy(
#ifdef PRINT_TO_FILE
	std::ofstream &outfile
#endif
) {
	edge_vec minimal_hitting_sets;
#ifdef HITTING_SET_STATS
	m_hitting_set_stats.clear();
	m_hitting_set_stats.add_record({ "minimal_hitting_set", "delay_ns" });
#endif
#ifdef PRINT_HITTING_SETS
	std::cout << "ENUMERATION:\nminimal_hitting_set,delay_ns\n";
#endif
#ifdef PRINT_TO_FILE
	outfile << "minimal_hitting_set,delay_ns" << std::endl;
#endif
#if defined(HITTING_SET_STATS) || defined(PRINT_HITTING_SETS) || defined(PRINT_TO_FILE)
	m_hitting_set_timestamp = Clock::now();
#endif
#ifdef ORACLE_STATS
	m_oracle_stats.clear();
	m_oracle_stats.add_record({ "x", "y", "return_line", "time_ns", "bf_time_ns", "iteration_count" });
#endif
	enumerate_legacy(edge(m_num_vertices), edge(m_num_vertices), 0, minimal_hitting_sets
#ifdef PRINT_TO_FILE
		, outfile
#endif
	);
	return minimal_hitting_sets;
}

void Hypergraph::enumerate_legacy(const edge &x, const edge &y, edge::size_type r, edge_vec &minimal_hitting_sets
#ifdef PRINT_TO_FILE
	, std::ofstream &outfile
#endif
) {
	if (r == m_num_vertices) {
		minimal_hitting_sets.push_back(x);
		#ifdef HITTING_SET_STATS
		auto now = Clock::now();
		m_hitting_set_stats.add_record({ edge_to_string(x), ns_string(m_hitting_set_timestamp, now) });
		m_hitting_set_timestamp = Clock::now();
#endif
#ifdef PRINT_HITTING_SETS
		auto now = Clock::now();
		std::cout << edge_to_string(x) << "," << ns_string(m_hitting_set_timestamp, now) << "\n";
		m_hitting_set_timestamp = Clock::now();
#endif
#ifdef PRINT_TO_FILE
		auto now = Clock::now();
		outfile << edge_to_string(x) << "," << ns_string(m_hitting_set_timestamp, now) << std::endl;
		m_hitting_set_timestamp = Clock::now();
#endif
		return;
	}
	edge xv = x;
	edge yv = y;
	xv[r] = 1;
	yv[r] = 1;
	if (extendable(xv, y)) enumerate_legacy(xv, y, r + 1, minimal_hitting_sets
#ifdef PRINT_TO_FILE
			, outfile
#endif
		);
	if (extendable(x, yv)) enumerate_legacy(x, yv, r + 1, minimal_hitting_sets
#ifdef PRINT_TO_FILE
		, outfile
#endif
	);
}

void Hypergraph::minimize_legacy() {
	std::vector<bool> marker(m_edges.size(), true);
	for (edge_vec::size_type i = 0; i < m_edges.size(); ++i) {
		for (edge_vec::size_type j = 0; j < m_edges.size(); ++j) {
			if (
				(i != j)
				&& ((m_edges[i] & m_edges[j]) == m_edges[i])
				) marker[j] = false;
		}
	}
	edge_vec new_edges;
	for (edge_vec::size_type i = 0; i < m_edges.size(); ++i) if (marker[i]) new_edges.push_back(m_edges[i]);
	m_edges = new_edges;
}

void Hypergraph::minimize() {
	edge_set new_edges;
	for (edge old_edge : m_edges) {
		bool insert = true;
		edge_vec to_delete;
		for (edge new_edge : new_edges) {
			if (new_edge.is_subset_of(old_edge)) { // subset edge already exists
				insert = false;
				break;
			}
			if (old_edge.is_subset_of(new_edge)) { // edge is subset of other one
				to_delete.push_back(new_edge);
			}
		}
		if (insert) {
			for (auto e : to_delete) new_edges.erase(e);
			new_edges.insert(old_edge);
		}
	}
	m_edges = edge_vec(new_edges.begin(), new_edges.end());
}

void Hypergraph::permute(permutation p) {
	if (m_edges.empty()) return;
	if (p.size() != m_num_vertices) {
		std::cerr << "Cannot apply permutation of length " << p.size() << " to graph with " << m_num_vertices << " vertices!" << std::endl;
		return;
	}
	edge_vec new_edges;
	for (edge e : m_edges) {
		edge new_edge(m_num_vertices);
		for (auto i = e.find_first(); i != edge::npos; i = e.find_next(i)) new_edge[p[i]] = 1;
		new_edges.push_back(new_edge);
	}
	m_edges = new_edges;
}

void Hypergraph::reverse_vertex_order() {
	permutation p;
	for (int i = m_num_vertices - 1; i >= 0; --i) p.push_back(i);
	permute(p);
}

edge_vec Hypergraph::brute_force_mhs(
#ifdef PRINT_TO_FILE
	std::ofstream &outfile
#endif
) {
	// brute force all minimal hitting sets
	// this works a bit similar to the apriori algorithm
	int check_time_and_memory_counter = 0;
	auto exit_timestamp = Clock::now();

#ifdef BRUTE_FORCE_STATS
	m_hitting_set_stats.add_record({ "minimal_hitting_set", "delay_ns" });
#endif
#ifdef PRINT_HITTING_SETS
	std::cout << "BRUTE FORCE:\nminimal_hitting_set,delay_ns\n";
#endif
#ifdef PRINT_TO_FILE
	outfile << "minimal_hitting_set,delay_ns" << std::endl;
#endif
#if defined(BRUTE_FORCE_STATS) || defined(PRINT_HITTING_SETS) || defined(PRINT_TO_FILE)
	auto timestamp = Clock::now();
#endif
	edge_vec minimal_hitting_sets;
	edge_vec incomplete_hitting_sets;
	for (edge::size_type i = 0; i < m_num_vertices; ++i) {
		edge e(m_num_vertices);
		e[i] = 1;
		if (is_hitting_set(e)) {
			minimal_hitting_sets.push_back(e);
#ifdef PRINT_HITTING_SETS
			auto now = Clock::now();
			std::cout << edge_to_string(e) << "," << ns_string(timestamp, now) << "\n";
			timestamp = Clock::now();
#endif
#ifdef PRINT_TO_FILE
			auto now = Clock::now();
			outfile << edge_to_string(e) << "," << ns_string(timestamp, now) << std::endl;
			timestamp = Clock::now();
#endif
		}
		else incomplete_hitting_sets.push_back(e);
	}
	if (incomplete_hitting_sets.empty()) return minimal_hitting_sets;
	for (int set_size = 1; set_size < m_num_vertices; ++set_size) {
		edge_vec::size_type current_level_cutoff = minimal_hitting_sets.size();
		edge_vec new_incomplete_hitting_sets;
		for (edge_vec::size_type i_first_set = 0; i_first_set < incomplete_hitting_sets.size(); ++i_first_set) {
			for (edge_vec::size_type i_second_set = i_first_set + 1; i_second_set < incomplete_hitting_sets.size(); ++i_second_set) {
				edge first_set_minus_last_vertex = incomplete_hitting_sets[i_first_set];
				first_set_minus_last_vertex[last_vertex(first_set_minus_last_vertex)] = 0;
				if (first_set_minus_last_vertex.is_subset_of(incomplete_hitting_sets[i_second_set])) {
					edge candidate = incomplete_hitting_sets[i_first_set] | incomplete_hitting_sets[i_second_set];
					bool add_candidate = true;
					for (edge_vec::size_type i = 0; i < current_level_cutoff; ++i) {
						if (minimal_hitting_sets[i].is_subset_of(candidate)) {
							add_candidate = false;
							break;
						}
					}
					if (add_candidate) {
						if (is_hitting_set(candidate)) {
							minimal_hitting_sets.push_back(candidate);
							if (minimal_hitting_sets.size() == 81) {
								std::cout << "";
							};
#ifdef BRUTE_FORCE_STATS
							auto now = Clock::now();
							m_bf_stats.add_record({ edge_to_string(candidate), ns_string(timestamp, now) });
							timestamp = Clock::now();
#endif
#ifdef PRINT_HITTING_SETS
							auto now = Clock::now();
							std::cout << edge_to_string(candidate) << "," << ns_string(timestamp, now) << std::endl;
							timestamp = Clock::now();
#endif
#ifdef PRINT_TO_FILE
							auto now = Clock::now();
							outfile << edge_to_string(candidate) << "," << ns_string(timestamp, now) << std::endl;
							timestamp = Clock::now();
#endif
						}
						else new_incomplete_hitting_sets.push_back(candidate);
					}
				}
				else break;
			}
			check_time_and_memory_counter++;
			if (check_time_and_memory_counter > 1000) {
				check_time_and_memory_counter = 0;
				if (std::chrono::duration_cast<std::chrono::minutes>(Clock::now() - exit_timestamp).count() >= 60 * 12) {
					std::cerr << "Aborting due to time constraints, took " << time_string(exit_timestamp, Clock::now()) << "." << std::endl;
					return edge_vec();
				}				
			}
		}
		if (new_incomplete_hitting_sets.empty()) {
#ifdef PRINT_HITTING_SETS
			auto now = Clock::now();
			std::cout << "until return," << ns_string(timestamp, now) << std::endl;
#endif
#ifdef PRINT_TO_FILE
			auto now = Clock::now();
			outfile << "until return," << ns_string(timestamp, now) << std::endl;
#endif
			return minimal_hitting_sets;
		}
		incomplete_hitting_sets.swap(new_incomplete_hitting_sets);
	}
	return minimal_hitting_sets;
}

void Hypergraph::remove_unused_vertices() {
	permutation p;
	edge union_all(m_num_vertices);
	for (edge e : m_edges) union_all |= e;
	for (int i = 0; i < m_num_vertices; ++i) {
		if (union_all[i]) p.push_back(i);
	}
	edge_vec new_edges;
	for (edge e : m_edges) {
		edge new_edge(p.size());
		for (int i = 0; i < p.size(); ++i) {
			if (e[p[i]]) new_edge[i] = 1;
		}
		new_edges.push_back(new_edge);
	}
	m_num_vertices = (int)p.size();
	m_edges = new_edges;
}

void Hypergraph::order_by_degree() {
	std::multimap<int, int> mmap;
	for (int i = 0; i < m_num_vertices; ++i) {
		int counter = 0;
		for (edge e : m_edges) if (e[i]) ++counter;
		mmap.insert(std::pair<int, int>(counter, i));
	}
	permutation p;
	for (auto pair = mmap.rbegin(); pair != mmap.rend(); ++pair) p.push_back(pair->second);
	std::cout << "Degree order: " << permutation_string(p) << std::endl;
	permute(p);
}
