#include "main.h"

#include "hypergraph.h"
#include "table.h"

#include <random>

int main(int argc, char *argv[]) {
	try {
		std::string action;
		int randomized_permutations;
		std::string statistics_directory;
		po::options_description option_description("Available options");
		option_description.add_options()
			("help,h", "show help message")
			("action,a", po::value<std::string>(&action)->default_value("enumerate"), "generate | enumerate")
			("input,i", po::value<std::string>()->default_value(fs::current_path().string()), "path to a file or directory")
			("output,o", po::value<std::string>()->default_value(fs::current_path().string()), "path to output file/directory")
			("randomized_permutations,r", po::value<int>(&randomized_permutations)->default_value(0), "number of random permutations to use (uses input permutation by default)")
			("implementation,I", po::value<std::vector<std::string>>(), "implementation(s) to use, can be used multiple times, available: standard | legacy | brute_force")
			("statistics_directory,s", po::value<std::string>(&statistics_directory)->default_value(fs::current_path().string()), "path to a directory to write statistics to")
			("hitting_set_statistics,H", "collect hitting set statistics")
			("oracle_statistics,O", "collect oracle statistics")
			("delimiter,d", po::value<char>()->default_value(','), "table delimiter used during graph generation")
			;

		po::positional_options_description positional_options_description;
		positional_options_description.add("action", 1).add("input", 1);

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

		fs::path input = fs::system_complete(fs::path(variables_map["input"].as<std::string>()));

		if (action == "enumerate") {

			std::vector<std::string> implementations;
			if (variables_map.count("implementation")) implementations = variables_map["implementation"].as<std::vector<std::string>>();
			else implementations.push_back("standard");

			enumerate_configuration configuration;
			configuration.statistics_directory = fs::system_complete(fs::path(variables_map["statistics_directory"].as<std::string>()));
			configuration.collect_hitting_set_statistics = (bool)variables_map.count("hitting_set_statistics");
			configuration.collect_oracle_statistics = (bool)variables_map.count("oracle_statistics");

			if (variables_map.count("output") && randomized_permutations > 0) {
				std::cerr << "Graphs with randomized permutation cannot be written to output!" << std::endl;
				exit(EXIT_FAILURE);
			}

			if (configuration.collect_hitting_set_statistics && configuration.collect_oracle_statistics) std::cerr << "WARNING: Collecting hitting set and oracle statistics at the same time. This will lead to imprecise hitting set running time measurements!" << std::endl;

			std::cout << "graph";
			if (randomized_permutations > 0) std::cout << ",permutation";
			for (std::string implementation : implementations) std::cout << "," << implementation << "_running_time_ns";
			std::cout << std::endl;

			for (fs::path graph_path : files_from_path(input, GRAPH_EXTENSION)) {
				Hypergraph h = Hypergraph(graph_path.string());
				configuration.name = graph_path.stem().string();
				if (randomized_permutations == 0) {
					std::cout << remove_quotations(graph_path.stem().string());
					for (std::string implementation : implementations) {
						configuration.implementation = implementation;
						Hypergraph t;
						auto start = Clock::now();
						t = h.enumerate(configuration);
						auto end = Clock::now();
						std::cout <<"," << ns_string(start, end);
						if (variables_map.count("output")) {
							fs::path output_path = fs::system_complete(fs::path(variables_map["output"].as<std::string>()));
							if (fs::is_directory(input)) {
								if (!fs::exists(output_path)) {
									std::cerr << "Output directory " << output_path << " does not exist!" << std::endl;
									exit(EXIT_FAILURE);
								}
								if (!fs::is_directory(output_path)) {
									std::cerr << "Directory given as input, but output path " << output_path << " does not describe a directory!" << std::endl;
									exit(EXIT_FAILURE);
								}
								output_path /= fs::path(graph_path.stem().string() + "_transversal");
								output_path.replace_extension(GRAPH_EXTENSION);
							}
							else {
								if (fs::exists(output_path) && fs::is_directory(output_path)) {
									output_path /= graph_path.stem();
									output_path.replace_extension(GRAPH_EXTENSION);
								}
							}
							t.save(output_path.string());
						}
					}
					std::cout << std::endl;
				}
				else {
					std::random_device rd;
					std::mt19937 g(rd());
					permutation p;
					for (int i = 0; i < h.m_num_vertices; ++i) p.push_back(i);
					for (int i = 0; i < randomized_permutations; ++i) {
						std::shuffle(p.begin(), p.end(), g);
						h.permute(p);
						std::cout << remove_quotations(graph_path.stem().string()) << "," << permutation_string(p);
						for (std::string implementation : implementations) {
							configuration.implementation = implementation;
							auto start = Clock::now();
							h.enumerate(configuration);
							auto end = Clock::now();
							std::cout << "," << ns_string(start, end);
						}
						std::cout << std::endl;
					}
				}
			}
		}
		else if (action == "generate") {
			for (fs::path table_path : files_from_path(input, TABLE_EXTENSION)) {
				fs::path output_path = fs::system_complete(fs::path(variables_map["output"].as<std::string>()));
				if (fs::is_directory(input)) {
					if (!fs::exists(output_path)) {
						std::cerr << "Output directory " << output_path << " does not exist!" << std::endl;
						exit(EXIT_FAILURE);
					}						
					if (!fs::is_directory(output_path)) {
						std::cerr << "Directory given as input, but output path " << output_path << " does not describe a directory!" << std::endl;
						exit(EXIT_FAILURE);
					}
					output_path /= table_path.stem();
					output_path.replace_extension(GRAPH_EXTENSION);
				}
				else {
					if (fs::exists(output_path) && fs::is_directory(output_path)) {
						output_path /= table_path.stem();
						output_path.replace_extension(GRAPH_EXTENSION);
					}
				}
				std::cerr << "Generating " << table_path.stem() << "..." << std::endl;
				Table t = Table(table_path.string(), variables_map["delimiter"].as<char>());
				Hypergraph h = Hypergraph(t);
				h.save(output_path.string());
			}
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
}

void print_help(const po::options_description &option_description) {
	std::cout << "Example usages:\n\tenumhyp enumerate path/to/graph.graph\n\tenumhyp enumerate path/to/graph/directory -r 50 -i standard -i legacy -s path/to/statistics/directory -O\n\tenumhyp generate path/to/table.csv\n\tenumhyp generate path/to/table.csv -d ; -o path/to/graph.graph\n\tenumhyp generate path/to/table/directory\n";
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

std::vector<fs::path> files_from_path(const fs::path &path, std::string extension) {
	verify_path(path);
	std::vector<fs::path> paths;
	if (fs::is_directory(path)) {
		std::vector<fs::directory_entry> directory_entries;
		std::copy(fs::directory_iterator(path), fs::directory_iterator(), back_inserter(directory_entries));
		std::sort(directory_entries.begin(), directory_entries.end());
		for (fs::directory_entry& directory_entry : directory_entries) {
			fs::path file_path = directory_entry.path();
			if (file_path.extension() == extension) paths.push_back(file_path);
		}
		if (paths.empty()) std::cerr << path << " does not contain any files with extension " << extension << "!" << std::endl;
	}
	else {
		paths.push_back(path);
	}
	return paths;
}

fs::path directory_from_path(fs::path path) {
	verify_path(path);
	if (fs::is_directory(path)) return path;
	return path.parent_path();
}