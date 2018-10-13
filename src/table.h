#pragma once

#include "globals.h"

#include <map>

typedef std::vector<std::string> record;
typedef std::vector<record> records;

class Table
{
public:
	records m_records;
	records::size_type num_records();
	record::size_type num_columns();
	Table();
	Table(std::string path, char delimiter) : Table(path, delimiter, INT_MAX) {}
	Table(std::string path, char delimiter, int num_records);
	~Table();
	void print_table();
	int num_uniques(record::size_type i_column);
	void delete_static_columns();
	bool empty() const;
	edge_vec edges() const;
	static void generate_edges(const records &m_records, records::size_type i_slice, edge_set &edges);
	void sort_columns_descending_uniqueness();
	void sort_columns_ascending_uniqueness();
	void sort_records();
	std::multimap<int, record::size_type> uniques_mmap();
	void save(std::string path, std::vector<std::string> header = std::vector<std::string>());
	void add_record(record r);
	void clear();
};