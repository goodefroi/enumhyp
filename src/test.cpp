#include "test.h"

#include "globals.h"
#include "table.h"
#include "batch.h"

void test_all() {
	std::cout << "--- BEGINNING OF TEST OUTPUT ---" << std::endl;
	auto timestamp = Clock::now();
	test_enumerate_legacy();
	test_generation();
	test_helpers();	
	test_enumerations();
	test_brute_force_vs_enumeration();
	std::cout << "--- END OF TEST OUTPUT, TOTAL TIME: " << time_string(timestamp, Clock::now()) << " ---" << std::endl;
}

void test_enumerations() {
	std::string graph_directory = std::string(TEST_DIRECTORY) + "/graphs";
	batch_process_files(graph_directory, test_enumeration);
}

void test_enumeration(boost::filesystem::path file_entry) {
#ifdef PRINT_TO_FILE
	std::cerr << "Please disable PRINT_TO_FILE for enumeration testing!" << std::endl;
	return;
#else
	std::cout << "Enumerating graph " << file_entry.stem() << " ... ";
	auto timestamp = Clock::now();
	Hypergraph h = Hypergraph(file_entry.string());
	Hypergraph t = Hypergraph();
	t.m_num_vertices = h.m_num_vertices;
	t.m_edges = h.enumerate();
	std::cout << "done, took " << m_string(timestamp, Clock::now()) << "minutes." << std::endl;
	test_transversal_graph_contains_only_hitting_sets(h, t);
	test_transversal_graph_is_minimal(h, t);
#endif
}


void test_transversal_graph_contains_only_hitting_sets(const Hypergraph &h, const Hypergraph &t) {
	std::cout << "Testing wether all enumerated minimal hitting sets are indeed hitting sets ... ";
	for (edge e : t.m_edges) {
		if (!h.is_hitting_set(e)) {
			std::cout << " TRANSVERSAL HYPERGRAPH CONTAINS EDGE " << edge_to_string(e) << " WHICH IS NOT A HITTING SET FOR THE ORIGINAL GRAPH!" << std::endl;
			return;
		}
	}
	std::cout << "done, everything looks fine." << std::endl;
}

void test_transversal_graph_is_minimal(const Hypergraph &h, const Hypergraph &t) {
	std::cout << "Testing wether the transversal hypergraph is minimal ... ";
	Hypergraph tmp = Hypergraph(t);
	tmp.minimize();
	if (tmp.m_edges.size() != t.m_edges.size()) {
		std::cout << " TRANSVERSAL HYPERGRAPH IS NOT MINIMAL!" << std::endl;
		return;
	}
	std::cout << "done, everything looks fine." << std::endl;
}

void test_helpers() {
	test_last_vertex();
	test_minimize();
}

void test_last_vertex() {
	bool passing = true;
	edge e(10);
	passing = passing && (last_vertex(e) == edge::npos);
	e[4] = 1;
	passing = passing && (e.find_first() == last_vertex(e));
	e[6] = 1;
	passing = passing && (e.find_first() != last_vertex(e));
	passing = passing && (last_vertex(e) == 6);
	if (passing) std::cout << "Tested last vertex helper function, everything looks fine." << std::endl;
	else std::cout << "LAST VERTEX FUNCTION RETURNS WRONG VALUE!" << std::endl;
}

void test_minimize() {
	std::string tables[] = { "ncvoter_allc_1k" };
	for (std::string table : tables) {
		std::string table_path = std::string(TEST_DIRECTORY) + "/tables/" + table + ".csv";
		Table t = Table(table_path, ',');
		std::cout << "Generating edges from table " << table << " ... " << std::flush;
		auto timestamp = Clock::now();
		edge_vec edges = t.edges();
		std::cout << "done, took " << time_string(timestamp, Clock::now()) << ".\nMinimizing edges with standard minimization function ... " << std::flush;
		Hypergraph h_standard = Hypergraph();
		h_standard.m_edges = edges;
		timestamp = Clock::now();
		h_standard.minimize();
		std::cout << "done, took " << time_string(timestamp, Clock::now()) << ".\nMinimizing edges with legacy minimization function ... " << std::flush;
		Hypergraph h_legacy = Hypergraph();
		h_legacy.m_edges = edges;
		timestamp = Clock::now();
		h_legacy.minimize_legacy();
		std::cout << "done, took " << time_string(timestamp, Clock::now()) << "." << std::endl;
		edge_set edges_standard(h_standard.m_edges.begin(), h_standard.m_edges.end());
		edge_set edges_legacy(h_legacy.m_edges.begin(), h_legacy.m_edges.end());
		if (edges_standard == edges_legacy) {
			std::cout << "Minimized edge sets are identical." << std::endl;
		}
		else {
			std::cout << "MINIMIZED EDGES ARE NOT IDENTICAL!" << std::endl;
		}
	}
}

void test_brute_force_vs_enumeration() {
#ifdef PRINT_TO_FILE
	std::cerr << "Please disable PRINT_TO_FILE when running enumeration vs. brute force tests!" << std::endl;
	return;
#else
	std::string graphs[] = { "passagierzahlen_10k_no_static_desc_uniq", "booking_call_a_bike_10k_no_static_desc_uniq", "civil_service_list_10k_no_static_desc_uniq", "fd_reduced_30_100k_no_static_desc_uniq" };
	for (std::string graph : graphs) {
		std::string graph_path = std::string(TEST_DIRECTORY) + "/graphs/" + graph + ".graph";
		Hypergraph h = Hypergraph(graph_path);
		h.remove_unused_vertices();
		std::cout << "Enumerating graph " << graph << " ... ";
		auto timestamp = Clock::now();
		edge_vec enum_vec = h.enumerate();
		auto now = Clock::now();
		auto enum_ns = duration_ns(timestamp, now);
		std::cout << "done, took " << time_string(timestamp, now) << "." << std::endl;
		std::cout << "Brute forcing graph " << graph << " ... ";
		timestamp = Clock::now();
		edge_vec bf_vec = h.brute_force_mhs();
		now = Clock::now();
		auto bf_ns = duration_ns(timestamp, now);
		std::cout << "done, took " << time_string(timestamp, now) << "." << std::endl;
		edge_set enum_set(enum_vec.begin(), enum_vec.end());
		edge_set bf_set(bf_vec.begin(), bf_vec.end());
		if (enum_set == bf_set) std::cout << "Found minimal hitting sets are identical, brute force took " << ((double)bf_ns / (double)enum_ns) << "x as much time." << std::endl;
		else {
			std::cout << "BRUTE FORCE AND ENUMERATION IMPLEMENTATION RETURN DIFFERENT RESULTS!" << std::endl;
			return;
		}
	}
#endif
}

void test_generation() {
	Table t = Table();
	t.add_record({ "a", "a", "c" });
	t.add_record({ "b", "a", "a" });
	t.add_record({ "a", "b", "a" });
	t.add_record({ "a", "b", "c" });
	Hypergraph h = Hypergraph(t);
	edge_set target;
	edge x(3);
	edge y(3);
	x[1] = 1;
	y[2] = 1;
	target.insert(x);
	target.insert(y);
	edge_set actual(h.m_edges.begin(), h.m_edges.end());
	if ((actual != target) || h.m_num_vertices != 3) {
		std::cout << "GRAPH GENERATION IS INCORRECT!" << std::endl;
		return;
	}
	h.remove_unused_vertices();
	edge a(2);
	edge b(2);
	a[0] = 1;
	b[1] = 1;
	target.clear();
	target.insert(a);
	target.insert(b);
	actual = edge_set(h.m_edges.begin(), h.m_edges.end());
	if ((actual != target) || (h.m_num_vertices != 2)) {
		std::cout << "UNUSED VERTEX REMOVAL IS INCORRECT!" << std::endl;
		return;
	}
	std::cout << "Tested graph generation and unused vertex removal, everything looks fine." << std::endl;
}

void test_enumerate_legacy() {
#ifdef PRINT_TO_FILE
	std::cerr << "Please disable PRINT_TO_FILE when running enumeration vs. brute force tests!" << std::endl;
	return;
#else
	std::string graphs[] = { "this_is_a_dummy_graph_filename_that_does_not_exist" };
	for (std::string graph : graphs) {
		std::string graph_path = std::string(TEST_DIRECTORY) + "/graphs/" + graph + ".graph";
		Hypergraph h = Hypergraph(graph_path);
		h.remove_unused_vertices();
		std::cout << "Enumerating graph " << graph << " using improved algorithm ... ";
		auto timestamp = Clock::now();
		edge_vec impr_vec = h.enumerate();
		auto now = Clock::now();
		auto impr_ns = duration_ns(timestamp, now);
		std::cout << "done, took " << time_string(timestamp, now) << "." << std::endl;
		std::cout << "Enumerating graph " << graph << " using legacy algorithm ... ";
		timestamp = Clock::now();
		edge_vec legacy_vec = h.enumerate_legacy();
		now = Clock::now();
		auto legacy_ns = duration_ns(timestamp, now);
		std::cout << "done, took " << time_string(timestamp, now) << "." << std::endl;
		if (impr_vec == legacy_vec) std::cout << "Found minimal hitting sets are identical, legacy algorithm took " << ((double)legacy_ns / (double)impr_ns) << "x as much time." << std::endl;
		else {
			std::cout << "BRUTE FORCE AND ENUMERATION IMPLEMENTATION RETURN DIFFERENT RESULTS!" << std::endl;
			return;
		}
	}
#endif
}