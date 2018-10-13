#include "interactive.h"

#include "globals.h"
#include "hypergraph.h"
#include "batch.h"

using namespace boost::filesystem;

void generate_permutation_stats_interactive(int argc, char *argv[]) {
	if (argc != 3) {
		std::cerr << "Must pass exactly two values!" << std::endl;
		return;
	}
	std::string p = argv[1];
	if (!exists(p)) {
		std::cerr << "File " << p << " does not exist!" << std::endl;
		return;
	}
	int num_permutations = std::stoi(argv[2]);
	if (num_permutations < 1) {
		std::cerr << "Cannot run " << num_permutations << " permutations!" << std::endl;
		return;
	}
	generate_permutation_stats(path(p), num_permutations, 10);
}

void generate_enum_vs_bf_stats_interactive(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << "Must pass exactly one value, the path of the graph to test enumeration vs. brute force on." << std::endl;
		return;
	}
	std::string p = argv[1];
	if (!exists(p)) {
		std::cerr << "File " << p << " does not exist!" << std::endl;
		return;
	}
	generate_enum_vs_bf_stats(path(p));
}

void generate_enumeration_stats_interactive(int argc, char *argv[]) {
	int num_repetitions = 1;
	auto timestamp = Clock::now();
	if (argc <= 1) {
		std::cerr << "First argument has to be a path to a hypergraph, followed by space-separated permutation. For a hypergraph with 4 vertices, this could look like this: " << argv[0] << " path/to/graph.graph 2 3 0 1" << std::endl;
		return;
	}
	path p = path(argv[1]);
	if (!exists(p)) {
		std::cerr << "File " << p << " does not exist!" << std::endl;
		return;
	}
	Hypergraph h = Hypergraph(p.string());
	h.remove_unused_vertices();
	permutation perm;
	if (argc == 2 + h.m_num_vertices + 1) {
		num_repetitions = std::atoi(argv[argc - 1]);
		if (num_repetitions < 1) {
			std::cerr << "ERROR: Number of repetitions must be at least one!" << std::endl;
			return;
		}
		for (int i = 2; i < argc - 1; ++i) perm.push_back(std::atoi(argv[i]));
		std::set<edge::size_type> vertices(perm.begin(), perm.end());
		if (vertices.size() != perm.size()) std::cerr << "WARNING: Given permutation includes duplicate vertices!" << std::endl;
	}
	else if (argc == 2 + h.m_num_vertices) {
		for (int i = 2; i < argc; ++i) perm.push_back(std::atoi(argv[i]));
		std::set<edge::size_type> vertices(perm.begin(), perm.end());
		if (vertices.size() != perm.size()) std::cerr << "WARNING: Given permutation includes duplicate vertices!" << std::endl;
	}
	else if (argc == 2) {
		for (edge::size_type i = 0; i < h.m_num_vertices; ++i) perm.push_back(i);
	}
	else {
		std::cerr << "Given permutation does not match hypergraph vertex count. Reduced graph has " << h.m_num_vertices << " vertices, permutation has " << argc - 2 << ". First argument has to be a path to a hypergraph, followed by an optional space-separated permutation. For a hypergraph with 4 vertices, this could look like this: " << argv[0] << " path/to/graph.graph 2 3 0 1" << std::endl;
		return;
	}	
	h.permute(perm);
	std::cout << "Generating enumeration stats for " << p.stem() << " using permutation " << permutation_string(perm) << " and " << num_repetitions << " repetitions ... " << std::flush;
	std::string p_base = std::string(STATS_DIRECTORY) + "/" + p.stem().string();
	
#ifdef PRINT_TO_FILE
	std::ofstream outfile;
	std::string hsl_path = unique_path(p_base + "_hitting_set_stats_live", ".csv");
	outfile.open(hsl_path);
	h.enumerate(outfile);
	outfile.close();
#else
	int num_length = std::to_string(num_repetitions - 1).size();
	for (int i = 0; i < 1; ++i) h.enumerate();
	for (int i = 0; i < num_repetitions; ++i) {
		auto timestamp_start = Clock::now();
		h.enumerate();
		auto timestamp_end = Clock::now();
#ifdef HITTING_SET_STATS
		std::stringstream ss_hs;
		ss_hs << p_base << "_hitting_set_stats_" << pad(i, num_length, '0');
		h.m_hitting_set_stats.save(unique_path(ss_hs.str(), ".csv"), std::vector <std::string>({ "permutation: " + permutation_string(perm) }));
#endif
#ifdef ORACLE_STATS
		std::stringstream ss_os;
		ss_os << p_base << "_oracle_stats_" << pad(i, num_length, '0');
		h.m_oracle_stats.save(unique_path(ss_os.str(), ".csv"), std::vector <std::string>({ "permutation: " + permutation_string(perm) }));
#endif
	}
#endif
	std::cout << "done, took " << time_string(timestamp, Clock::now()) << "." << std::endl;
}