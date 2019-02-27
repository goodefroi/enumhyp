#pragma once

#include <iostream>
#include <vector>
#include <set>
#include <iomanip>
#include <sstream>
#include <chrono>

#include <boost/dynamic_bitset.hpp>

#ifdef _DEBUG
#define TABLE_DIRECTORY "../data/tables"
#define GRAPH_DIRECTORY "../data/graphs"
#define STATS_DIRECTORY "../data/stats"
#define TEST_DIRECTORY "../data/test"
#else
#define TABLE_DIRECTORY "tables"
#define GRAPH_DIRECTORY "graphs"
#define STATS_DIRECTORY "stats"
#define TEST_DIRECTORY "test"
#endif

#define NUM_THREADS 24

#define HITTING_SET_STATS		// collect information about minimal hitting sets found by the enumeration algorithm
#define ORACLE_STATS			// collect information about every single extension oracle call
#define BRUTE_FORCE_STATS_		// collect information about minimal hitting sets found by the brute force algorithm
#define PRINT_HITTING_SETS_		// print minimal hitting sets to console as they are found	DISABLE FOR BENCHMARKING
#define PRINT_TO_FILE_			// same as above, but print to file							DISABLE FOR BENCHMARKING

#define GRAPH_EXTENSION ".graph"
#define TABLE_EXTENSION ".csv"

typedef boost::dynamic_bitset<> edge;
typedef std::vector<edge> edge_vec;
typedef std::set<edge> edge_set;
typedef std::vector<edge::size_type> permutation;

typedef std::chrono::high_resolution_clock Clock;

void print_edge(const edge &e);
void print_edge_vec(const edge_vec &v);
std::string edge_to_string(const edge &e);
std::string ns_string(Clock::time_point sooner, Clock::time_point later);
std::string m_string(Clock::time_point sooner, Clock::time_point later);
std::string s_string(Clock::time_point sooner, Clock::time_point later);
std::string time_string(Clock::time_point sooner, Clock::time_point later);
uint64_t duration_ns(Clock::time_point sooner, Clock::time_point later);

edge::size_type last_vertex(edge e);
std::string permutation_string(const permutation &p);
std::string pad(std::string s, int lenght, char c);
std::string pad(std::string s, int length);
std::string pad(int i, int length, char c);
std::string pad(int i, int length);

std::string unique_path(std::string p, std::string suffix);
