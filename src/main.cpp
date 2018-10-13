#include "globals.h"

#include "hypergraph.h"
#include "table.h"
#include "batch.h"
#include "test.h"
#include "interactive.h"

int main(int argc, char *argv[]) {
	//generate_graphs_from_tables(TABLE_DIRECTORY, GRAPH_DIRECTORY);
	//generate_enumeration_stats(GRAPH_DIRECTORY, STATS_DIRECTORY);
	//batch_generate_enum_vs_bf_stats();
	//batch_generate_permutation_stats();
	//test_all();
	//generate_permutation_stats_interactive(argc, argv);
	//generate_enum_vs_bf_stats_interactive(argc, argv);
	generate_enumeration_stats_interactive(argc, argv);
	//batch_generate_impr_vs_legacy_stats();
	//batch_reduce_graphs();
	//batch_desc_uniq_vs_degree_order_stats();
#ifdef _DEBUG
	system("pause");
#endif
	return 0;
}
