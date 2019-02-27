#include "main.h"

#include "hypergraph.h"
#include "table.h"
#include "batch.h"
#include "test.h"
#include "interactive.h"

#include <random>

#define ILLEGAL_PATH 0
#define FILE 1
#define DIRECTORY 2

int main(int argc, char *argv[]) {
	try {
		std::string action;
		int randomized_permutations;
		std::string statistics_directory;
		po::options_description option_description("Available options");
		option_description.add_options()
			("help,h", "show help message")
			("action,a", po::value<std::string>(&action)->default_value("enumerate"), "arg: generate | enumerate")
			("path,p", po::value<std::string>()->default_value(fs::current_path().string()), "path to a file or directory")
			("randomized_permutations,r", po::value<int>(&randomized_permutations)->default_value(0), "number of random permutations to use (uses input permutation by default)")
			("statistics_directory,s", po::value<std::string>(&statistics_directory)->default_value(fs::current_path().string()), "path to a directory to write statistics to")
			("implementation,i", po::value<std::vector<std::string>>()->default_value(std::vector<std::string>(), "standard"), "implementation(s) to use, can be used multiple times")
			;

		po::positional_options_description positional_options_description;
		positional_options_description.add("action", 1).add("path", 1);

		po::variables_map variables_map;
		po::store(po::command_line_parser(argc, argv).options(option_description).positional(positional_options_description).run(), variables_map);
		po::notify(variables_map);

		if (variables_map.count("help")) {
			print_help(option_description);
			return EXIT_SUCCESS;
		}

		if (randomized_permutations < 0) {
			std::cerr << "randomized_permutations cannot be smaller than 0" << std::endl;
			return EXIT_FAILURE;
		}

		fs::path path = fs::system_complete(fs::path(variables_map["path"].as<std::string>()));

		std::cout << "action: " << action << std::endl;
		std::cout << "path: " << path << std::endl;
		std::cout << "randomized_permutations: " << randomized_permutations << std::endl;

		if (action == "enumerate") {
			std::vector<std::string> implementations;
			/*if (variables_map.count("implementation"))*/ implementations = variables_map["implementation"].as<std::vector<std::string>>();
			//else implementations.push_back("standard");
			for (fs::path graph_path : graphs_from_path(path)) {
				Hypergraph h = Hypergraph(graph_path.string());
				if (randomized_permutations == 0) {
					for (std::string implementation : implementations) {
						std::cout << "Enumerating " << graph_path.stem() << " using " << implementation << " implementation ...";
						h.enumerate(implementation);
						std::cout << "done." << std::endl;
					}
				}
				else {
					std::random_device rd;
					std::mt19937 g(rd());
					permutation p;
					for (int i = 0; i < h.m_num_vertices; ++i) p.push_back(i);
					for (int i = 0; i < randomized_permutations; ++i) {
						std::shuffle(p.begin(), p.end(), g);
						h.permute(p);
						for (std::string implementation : implementations) {
							std::cout << "Enumerating " << graph_path.stem() << " using " << implementation << " implementation and permutation " << permutation_string(p) << "...";
							h.enumerate(implementation);
							std::cout << "done." << std::endl;
						}
					}
				}
			}
		}
		else if (action == "generate") {
		}
		else {
			std::cerr << "Invalid action: " << action << ". Use --help to show available options." << std::endl;
		}
		return 0;
	}
	catch (std::exception &e) {
		std::cout << e.what() << std::endl;
		return 1;
	}
	return 0;
	//generate_graphs_from_tables(TABLE_DIRECTORY, GRAPH_DIRECTORY);
	//generate_enumeration_stats(GRAPH_DIRECTORY, STATS_DIRECTORY);
	//batch_generate_enum_vs_bf_stats();
	//batch_generate_permutation_stats();
	//test_all();
	//generate_permutation_stats_interactive(argc, argv);
	//generate_enum_vs_bf_stats_interactive(argc, argv);
	//generate_enumeration_stats_interactive(argc, argv);
	//batch_generate_impr_vs_legacy_stats();
	//batch_reduce_graphs();
	//batch_desc_uniq_vs_degree_order_stats();
}

void print_help(const po::options_description &option_description) {
	std::cout << "Usage: options_description [options]\n";
	std::cout << option_description;
}

void verify_path(const fs::path &path) {
	if (!fs::exists(path)) {
		std::cerr << path << " does not exist!" << std::endl;
		exit(EXIT_FAILURE);
	}
	if (fs::is_symlink(path) || fs::is_other(path)) {
		std::cerr << path << " does not describe a file or directory!" << std::endl;
		exit(EXIT_FAILURE);
	}
}

std::vector<fs::path> graphs_from_path(const fs::path &path) {
	verify_path(path);
	std::vector<fs::path> paths;
	if (fs::is_directory(path)) {
		std::vector<fs::directory_entry> directory_entries;
		std::copy(fs::directory_iterator(path), fs::directory_iterator(), back_inserter(directory_entries));
		std::sort(directory_entries.begin(), directory_entries.end());
		for (fs::directory_entry& directory_entry : directory_entries) {
			fs::path file_path = directory_entry.path();
			if (file_path.extension() == GRAPH_EXTENSION) paths.push_back(file_path);
		}
		if (paths.empty()) std::cerr << path << " does not contain any graphs!" << std::endl;
	}
	else {
		paths.push_back(path);
	}
	return paths;
}
