#pragma once

#include <string>

#include <boost/filesystem.hpp>

#include "globals.h"


void batch_process_files(std::string directory_path, void(*function)(boost::filesystem::path file_entry));
void generate_graphs_from_tables(std::string table_path, std::string graph_path);
void generate_enumeration_stats(std::string graph_path, std::string stats_path);
void batch_generate_enum_vs_bf_stats();
void generate_enum_vs_bf_stats(boost::filesystem::path file_entry);
void batch_generate_permutation_stats();
void generate_permutation_stats(boost::filesystem::path file_entry);
void generate_permutation_stats(boost::filesystem::path file_entry, int num_permutations, int num_repetitions);
void batch_generate_impr_vs_legacy_stats();
void generate_impr_vs_legacy_stats(boost::filesystem::path file_entry);
void batch_reduce_graphs();
void reduce_graph(boost::filesystem::path file_entry);
void batch_desc_uniq_vs_degree_order_stats();
void desc_uniq_vs_degree_order_stats(boost::filesystem::path file_entry);