#include "batch.h"

#include <random>

#include "table.h"
#include "hypergraph.h"

using namespace boost::filesystem;

void batch_process_files(std::string directory_path, void(*function)(path file_entry)) {
	if (exists(directory_path)) {
		if (is_directory(directory_path)) {
			std::vector<directory_entry> file_entries;
			std::copy(directory_iterator(directory_path), directory_iterator(), back_inserter(file_entries));
			std::sort(file_entries.begin(), file_entries.end());
			for (directory_entry& f : file_entries) {
				if (is_regular_file((path)f)) {
					function(f.path());
				}
			}
		}
		else {
			std::cerr << "The path " << directory_path << " does not describe a directory!" << std::endl;
		}
	}
	else {
		std::cerr << "Directory " << directory_path << " does not exist!" << std::endl;
	}
}

void generate_graphs_from_tables(std::string table_path, std::string graph_path) {
	if (exists(table_path)) {
		if (is_directory(table_path)) {
			if (!exists(graph_path)) {
				std::cerr << "Graph location " << graph_path << " should exist, but does not!" << std::endl;
				return;
			}
			if (!is_directory(graph_path)) {
				std::cerr << "Graph location " << graph_path << " is not a directory!" << std::endl;
				return;
			}
			for (directory_entry& f : directory_iterator(table_path)) {
				if (is_regular_file((path)f)) {
					std::cout << "Generating hypergraph " << f.path().stem() << " ... " << std::flush;
					auto timestamp = Clock::now();
					Table t = Table(f.path().string(), ',');
					t.delete_static_columns();
					t.sort_columns_ascending_uniqueness();
					t.sort_records();
					Hypergraph h = Hypergraph(t);
					h.reverse_vertex_order();
					h.remove_unused_vertices();
					std::string p = graph_path + "/" + f.path().stem().string() + "_no_static_desc_uniq.graph";
					h.save(p);
					std::cout << "done, took " << time_string(timestamp, Clock::now()) << "." << std::endl;
				}
			}
		}
		else {
			std::cerr << "The path " << table_path << " should contain tables, but is no directory!" << std::endl;
		}
	}
	else {
		std::cerr << "Directory " << table_path << " should contain tables, but does not exist!" << std::endl;
	}
}

void generate_enumeration_stats(std::string graph_path, std::string stats_path) {
	int num_repetitions = 100;
	if (exists(graph_path)) {
		if (is_directory(graph_path)) {
			if (!exists(stats_path)) {
				std::cerr << "Stats location " << stats_path << " should exist, but does not!" << std::endl;
				return;
			}
			if (!is_directory(stats_path)) {
				std::cerr << "Stats location " << stats_path << " is not a directory!" << std::endl;
				return;
			}
			for (directory_entry& f : directory_iterator(graph_path)) {
				std::cout << "Generating enumeration stats for " << f.path().stem() << " ... " << std::flush;
				std::string p_base = stats_path + "/" + f.path().stem().string();
				Hypergraph h = Hypergraph(f.path().string());
				auto timestamp = Clock::now();
#ifdef PRINT_TO_FILE
				std::ofstream outfile;
				outfile.open(p_base + "_hitting_set_stats_live.csv");
				h.enumerate(outfile);
				outfile.close();
				auto now = Clock::now();
#else
				int num_length = std::to_string(num_repetitions - 1).size();
				for (int i = 0; i < 10; ++i) h.enumerate();
				for (int i = 0; i < num_repetitions; ++i) {
					h.enumerate();
#ifdef HITTING_SET_STATS
					std::stringstream ss_hs;
					ss_hs << p_base << "_hitting_set_stats_" << pad(i, num_length, '0');
					h.m_hitting_set_stats.save(unique_path(ss_hs.str(), ".csv"));
#endif
#ifdef ORACLE_STATS
					std::stringstream ss_os;
					ss_os << p_base << "_oracle_stats_" << pad(i, num_length, '0');
					h.m_oracle_stats.save(unique_path(ss_os.str(), ".csv"));
#endif
				}
				auto now = Clock::now();
#endif								
				std::cout << "done, took " << time_string(timestamp, now) << " / " << ns_string(timestamp, now) << "ns"
#ifndef PRINT_TO_FILE
					<< " for " << num_repetitions << " repetitions"
#endif
					<< "." << std::endl;
			}
		}
		else {
			std::cerr << "The path " << graph_path << " should contain graphs, but is no directory!" << std::endl;
		}
	}
	else {
		std::cerr << "Directory " << graph_path << " should contain graphs, but does not exist!" << std::endl;
	}
}

void batch_generate_enum_vs_bf_stats() {
	batch_process_files(GRAPH_DIRECTORY, generate_enum_vs_bf_stats);
}

void generate_enum_vs_bf_stats(boost::filesystem::path file_entry) {
#if defined(HITTING_SET_STATS) || defined(ORACLE_STATS) || defined(BRUTE_FORCE_STATS)
	std::cerr << "ERROR: Cannot test enumeration vs. brute force runtime. Code collects unnecessary statistics that impact results!" << std::endl;
	return;
#endif
	std::cout << "Generating enumeration vs. brute force stats for " << file_entry.stem() << " ... " << std::flush;
	Hypergraph h = Hypergraph(file_entry.string());
	h.remove_unused_vertices();
	auto timestamp_start = Clock::now();
#ifdef PRINT_TO_FILE
	std::ofstream enum_outfile;
	std::string p_base = std::string(STATS_DIRECTORY) + "/" + file_entry.stem().string();
	std::string p_enum = unique_path(p_base + "_enum_hitting_set_stats_live", ".csv");
	enum_outfile.open(p_enum);
	edge_vec enum_vec = h.enumerate(enum_outfile);
	enum_outfile.close();
	auto timestamp_mid = Clock::now();
	std::ofstream bf_outfile;
	std::string p_bf = unique_path(p_base + "_bf_hitting_set_stats_live", ".csv");
	bf_outfile.open(p_bf);
	edge_vec bf_vec = h.brute_force_mhs(bf_outfile);
	bf_outfile.close();
#else
	edge_vec enum_vec = h.enumerate();
	auto timestamp_mid = Clock::now();
	edge_vec bf_vec = h.brute_force_mhs();
#endif
	auto timestamp_end = Clock::now();
	edge_set enum_set(enum_vec.begin(), enum_vec.end());
	edge_set bf_set(bf_vec.begin(), bf_vec.end());
	/*if (enum_set != bf_set) {
		std::cout << "ENUMERATION AND BRUTE FORCE IMPLEMENTATION RETURNED DIFFERENT RESULTS!" << std::endl;
		// std::cout << "Enumeration:" << std::endl;
		// for (edge e : enum_set) std::cerr << edge_to_string(e) << std::endl;
		// std::cout << "Brute Force:" << std::endl;
		// for (edge e : bf_set) std::cerr << edge_to_string(e) << std::endl;
		return;
	}*/
	std::cout << "\n... done, took " << time_string(timestamp_start, timestamp_end) << ". Results:" << std::endl;
	std::cout << "\tEnumeration:\t" << time_string(timestamp_start, timestamp_mid) << std::endl;
	std::cout << "\t\t\t" << ns_string(timestamp_start, timestamp_mid) << "ns" << std::endl;
	std::cout << "\tBrute Force:\t" << time_string(timestamp_mid, timestamp_end) << std::endl;
	std::cout << "\t\t\t" << ns_string(timestamp_mid, timestamp_end) << "ns" << std::endl;
	std::cout << "\tRunning time increase of bf vs. enum:\t" << (double)duration_ns(timestamp_mid, timestamp_end) / (double)duration_ns(timestamp_start, timestamp_mid) << std::endl;
}

void batch_generate_permutation_stats(){
	batch_process_files(GRAPH_DIRECTORY, generate_permutation_stats);
}

void generate_permutation_stats(boost::filesystem::path file_entry) {
	generate_permutation_stats(file_entry, 1000, 100);
}

void generate_permutation_stats(boost::filesystem::path file_entry, int num_permutations, int num_repetitions) {
#ifdef PRINT_TO_FILE
	std::cerr << "Please disable file printing when generating permutation stats!" << std::endl;
	return;
#else
	auto timestamp = Clock::now();
	std::cout << "Generating permutation running stats for " << file_entry.stem() << ", using " << num_permutations << " permutations and " << num_repetitions << " repetitions, currently at: 0" << std::flush;
	Hypergraph h = Hypergraph(file_entry.string());
	h.remove_unused_vertices();
	permutation p;
	std::ofstream outfile;
	std::string outfile_path = unique_path(std::string(STATS_DIRECTORY) + "/" + file_entry.stem().string() + "_" + std::to_string(num_permutations) + "_permutations", ".csv");
	outfile.open(outfile_path);
	outfile << "permutation,enumeration_time_ns" << std::endl;
	for (int i = 0; i < h.m_num_vertices; ++i) p.push_back(i);
	uint64_t total_length = 0;
	for (int i = 0; i < 10; ++i) h.enumerate(); //warm up cache for small graphs
	for (int i = 0; i < num_repetitions; ++i) {
		auto timestamp_start = Clock::now();
		h.enumerate();
		auto timestamp_end = Clock::now();
		total_length += std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp_end - timestamp_start).count();
	}
	outfile << permutation_string(p) << "," << total_length / num_repetitions << std::endl;
#ifdef HITTING_SET_STATS
	std::string directory_path = unique_path(std::string(STATS_DIRECTORY) + "/" + file_entry.stem().string() + "_" + std::to_string(num_permutations) + "_permutations", "");
	if (exists(directory_path)) {
		if (!is_directory(directory_path)) {
			std::cerr << "Path " << directory_path << " should be a directory, but is not!" << std::endl;
			return;
		}
	}
	else create_directory(directory_path);
	std::string stats_base_path = directory_path + "/" + file_entry.stem().string() + "_hitting_set_stats_permutation_";
	int index_length = std::to_string(num_permutations-1).size();
	h.m_hitting_set_stats.save(stats_base_path + pad(0, index_length, '0') + ".csv");
#endif
	std::random_device rd;
	std::mt19937 g(rd());
	for (int i = 0; i < num_permutations; ++i) {
		for (int j = 0; j < std::to_string(i).size(); ++j) std::cout << "\b";
		std::cout << i + 1 << std::flush;
		std::shuffle(p.begin(), p.end(), g);
		h.permute(p);
		uint64_t total_length = 0;
		for (int i = 0; i < 10; ++i) h.enumerate(); //warm up cache for small graphs
		for (int i = 0; i < num_repetitions; ++i) {
			auto timestamp_start = Clock::now();
			h.enumerate();
			auto timestamp_end = Clock::now();
			total_length += std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp_end - timestamp_start).count();
		}
		outfile << permutation_string(p) << "," << total_length / num_repetitions << std::endl;
#ifdef HITTING_SET_STATS
		h.m_hitting_set_stats.save(stats_base_path + pad(i + 1, index_length, '0') + ".csv");
#endif
	}
	outfile.close();
	std::cout << " ... done, total time: " << time_string(timestamp, Clock::now()) << "." << std::endl;
#endif
}

void batch_generate_impr_vs_legacy_stats() {
	batch_process_files(GRAPH_DIRECTORY, generate_impr_vs_legacy_stats);
}

void generate_impr_vs_legacy_stats(boost::filesystem::path file_entry) {
	int num_permutations = 10;
	int num_repetitions = 10;
#ifdef PRINT_TO_FILE
	std::cerr << "Please disable file printing when generating permutation stats!" << std::endl;
	return;
#else
	auto timestamp = Clock::now();
	std::cout << "Generating permutation running stats for " << file_entry.stem() << ", using " << num_permutations << " permutations and " << num_repetitions << " repetitions, currently at: 0" << std::flush;
	Hypergraph h = Hypergraph(file_entry.string());
	h.remove_unused_vertices();
	permutation p;
	std::ofstream outfile;
	std::string outfile_path = unique_path(std::string(STATS_DIRECTORY) + "/" + file_entry.stem().string() + "_i_vs_l_" + std::to_string(num_permutations) + "_perm_" + std::to_string(num_repetitions) + "_rep", ".csv");
	outfile.open(outfile_path);
	outfile << "permutation,impr_time_ns,legacy_time_ns" << std::endl;
	for (int i = 0; i < h.m_num_vertices; ++i) p.push_back(i);
	uint64_t impr_ns = 0;
	uint64_t legacy_ns = 0;
	for (int i = 0; i < num_repetitions; ++i) {
		auto timestamp_start = Clock::now();
		h.enumerate();
		auto timestamp_end = Clock::now();
		impr_ns += (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp_end - timestamp_start).count();
		timestamp_start = Clock::now();
		h.enumerate_legacy();
		timestamp_end = Clock::now();
		legacy_ns += (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp_end - timestamp_start).count();
	}
	outfile << permutation_string(p) << "," << impr_ns / num_repetitions << "," << legacy_ns / num_repetitions << std::endl;
	for (int i = 0; i < num_permutations; ++i) {
		for (int j = 0; j < std::to_string(i).size(); ++j) std::cout << "\b";
		std::cout << i + 1 << std::flush;
		std::random_shuffle(p.begin(), p.end());
		h.permute(p);
		uint64_t impr_ns = 0;
		uint64_t legacy_ns = 0;
		for (int j = 0; j < num_repetitions; ++j) {
			auto timestamp_start = Clock::now();
			h.enumerate();
			auto timestamp_end = Clock::now();
			impr_ns += (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp_end - timestamp_start).count();
			timestamp_start = Clock::now();
			h.enumerate_legacy();
			timestamp_end = Clock::now();
			legacy_ns += (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp_end - timestamp_start).count();
		}
		outfile << permutation_string(p) << "," << impr_ns / num_repetitions << "," << legacy_ns / num_repetitions << std::endl;
	}
	outfile.close();
	std::cout << " ... done, total time: " << time_string(timestamp, Clock::now()) << "." << std::endl;
#endif
}

void batch_reduce_graphs() {
	batch_process_files(GRAPH_DIRECTORY, reduce_graph);
}

void reduce_graph(boost::filesystem::path file_entry) {
	Hypergraph h = Hypergraph(file_entry.string());
	std::cout << "Reduced graph " << file_entry.stem().string() << " from " << h.m_num_vertices << " to ";
	h.remove_unused_vertices();
	std::cout << h.m_num_vertices << " vertices." << std::endl;
	h.save(std::string(GRAPH_DIRECTORY) + "/" + file_entry.stem().string() + "_no_dead.graph");
}

void batch_desc_uniq_vs_degree_order_stats() {
	batch_process_files(GRAPH_DIRECTORY, desc_uniq_vs_degree_order_stats);
}

void desc_uniq_vs_degree_order_stats(boost::filesystem::path file_entry) {
	int num_repetitions = 1;
#ifdef PRINT_TO_FILE
	std::cerr << "Please disable file printing when generating desc. uniq. vs. degree order stats!" << std::endl;
	return;
#else
	auto timestamp = Clock::now();
	std::cout << "Generating desc. uniq. vs. degree order stats for " << file_entry.stem() << ", using " << num_repetitions << " repetitions..." << std::endl;
	Hypergraph h_desc_uniq = Hypergraph(file_entry.string());
	h_desc_uniq.remove_unused_vertices();
	Hypergraph h_degree_order = Hypergraph(h_desc_uniq);
	h_degree_order.order_by_degree();
	uint64_t desc_uniq_ns = 0;
	uint64_t degree_order_ns = 0;
	for (int i = 0; i < num_repetitions; ++i) {
		auto timestamp_start = Clock::now();
		h_desc_uniq.enumerate();
		auto timestamp_end = Clock::now();
		desc_uniq_ns += (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp_end - timestamp_start).count();
		timestamp_start = Clock::now();
		h_degree_order.enumerate();
		timestamp_end = Clock::now();
		degree_order_ns += (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp_end - timestamp_start).count();
	}
	std::cout << "...done, total time: " << time_string(timestamp, Clock::now()) << "." << std::endl;
	std::cout << "Desc. uniq.:\t" << desc_uniq_ns / num_repetitions << "ns" << std::endl;
	std::cout << "Degree order:\t" << degree_order_ns / num_repetitions << "ns" << std::endl;
	std::cout << "Desc. uniq. / degree order:\t" << (double)desc_uniq_ns / (double)degree_order_ns << std::endl;
#endif
}
