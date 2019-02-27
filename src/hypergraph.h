#pragma once

#include <vector>

#include "globals.h"
#include "table.h"

class Hypergraph
{
public:
	int m_num_vertices;
	edge_vec m_edges;
#ifdef HITTING_SET_STATS
	Table m_hitting_set_stats;
#endif
#ifdef ORACLE_STATS
	Table m_oracle_stats;
#endif
#ifdef BRUTE_FORCE_STATS
	Table m_bf_stats;
#endif
	Hypergraph();
	Hypergraph(const Hypergraph &other);
	Hypergraph(std::string path);
	Hypergraph(const Table &t);
	Hypergraph(int num_vertices, edge_vec edges);
	~Hypergraph();
	bool is_hitting_set(const edge &h) const;
	void print_edges() const;
	void save(std::string path) const;
	Hypergraph enumerate(const std::string &implementation);
	edge_vec enumerate(
#ifdef PRINT_TO_FILE
		std::ofstream &outfile
#endif
	);
	edge_vec enumerate_legacy(
#ifdef PRINT_TO_FILE
		std::ofstream &outfile
#endif
	);
	void minimize();
	void minimize_legacy();
	void permute(permutation p);
	void reverse_vertex_order();
	void order_by_degree();
	edge_vec brute_force_mhs(
#ifdef PRINT_TO_FILE
		std::ofstream &outfile
#endif
	);
	void remove_unused_vertices();
private:
#if defined(HITTING_SET_STATS) || defined(PRINT_HITTING_SETS) || defined(PRINT_TO_FILE)
	Clock::time_point m_hitting_set_timestamp;
#endif
	int extendable(const edge &x, const edge &y);
	void enumerate(const edge &x, const edge &y, edge::size_type r, edge_vec &minimal_hitting_sets
#ifdef PRINT_TO_FILE
		, std::ofstream &outfile
#endif
	);
	void enumerate_legacy(const edge &x, const edge &y, edge::size_type r, edge_vec &minimal_hitting_sets
#ifdef PRINT_TO_FILE
		, std::ofstream &outfile
#endif
	);
};
