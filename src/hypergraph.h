#pragma once

#include <vector>

#include <boost/filesystem.hpp>

#include "globals.h"
#include "table.h"

namespace fs = boost::filesystem;

class Hypergraph
{
public:
	int m_num_vertices;
	edge_vec m_edges;
	Hypergraph();
	Hypergraph(const Hypergraph &other);
	Hypergraph(std::string path);
	Hypergraph(const Table &t);
	Hypergraph(int num_vertices, edge_vec edges);
	~Hypergraph();
	bool is_hitting_set(const edge &h) const;
	void print_edges() const;
	void save(std::string path) const;
	Hypergraph enumerate(enumerate_configuration configuration);
	edge_vec enumerate();
	edge_vec enumerate_legacy();
	void minimize();
	void permute(permutation p);
	edge_vec brute_force_mhs();
private:
	enumerate_configuration m_configuration;
	Table m_hitting_set_stats;
	Table m_oracle_stats;
	Table m_bf_stats;
	Clock::time_point m_hitting_set_timestamp;
	Clock::time_point m_oracle_timestamp;
	Clock::time_point m_oracle_bf_timestamp;
	int m_iteration_count;
	int extendable(const edge &x, const edge &y);
	void enumerate(const edge &x, const edge &y, edge::size_type r, edge_vec &minimal_hitting_sets);
	void enumerate_legacy(const edge &x, const edge &y, edge::size_type r, edge_vec &minimal_hitting_sets);
	int Hypergraph::maximum_iteration_count(std::vector<edge_vec> s);
	int Hypergraph::summed_sx_sizes(std::vector<edge_vec> s);
	int Hypergraph::total_number_of_vertices_in_s(std::vector<edge_vec> s);
	int Hypergraph::total_number_of_vertices_in_t(edge_vec t);
	void Hypergraph::save_statistics();
};
