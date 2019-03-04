#pragma once

#include <iostream>
#include <vector>
#include <set>
#include <iomanip>
#include <sstream>
#include <chrono>

#include <boost/dynamic_bitset.hpp>
#include <boost/filesystem.hpp>

#define NUM_THREADS 24

#define GRAPH_EXTENSION ".graph"
#define TABLE_EXTENSION ".csv"

namespace fs = boost::filesystem;

typedef boost::dynamic_bitset<> edge;
typedef std::vector<edge> edge_vec;
typedef std::set<edge> edge_set;
typedef std::vector<edge::size_type> permutation;

typedef std::chrono::high_resolution_clock Clock;

struct enumerate_configuration {
	std::string name;
	std::string implementation;
	fs::path statistics_directory;
	bool collect_hitting_set_statistics;
	bool collect_oracle_statistics;
	bool collect_brute_force_statistics;
};

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
