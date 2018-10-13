#pragma once

#include <boost/filesystem.hpp>

#include "hypergraph.h"

void test_all();

void test_enumerations();
void test_enumeration(boost::filesystem::path file_entry);
void test_transversal_graph_contains_only_hitting_sets(const Hypergraph &h, const Hypergraph &t);
void test_transversal_graph_is_minimal(const Hypergraph &h, const Hypergraph &t);

void test_helpers();
void test_last_vertex();
void test_minimize();
void test_enumerate_legacy();

void test_brute_force_vs_enumeration();

void test_generation();