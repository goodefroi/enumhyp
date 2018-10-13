#include "table.h"

#include <fstream>
#include <sstream>
#include <thread>
#include <boost/algorithm/string.hpp>

Table::Table() {
}

Table::Table(std::string path, char delimiter, int num_records) {
	if (num_records < 1) return;
	std::ifstream infile(path);
	std::string line;
	getline(infile, line);
	record r;
	boost::split(r, line, [](char c) {return c == ','; });
	m_records.push_back(r);
	int i_current_line = 2;
	while (getline(infile, line)) {
		if (i_current_line > num_records) break;
		record r;
		boost::split(r, line, [](char c) {return c == ','; });
		if (r.size() != m_records[0].size()) {
			std::cerr << "Record in line " << i_current_line << " appears to be broken, should be " << m_records[0].size() << " but is " << r.size() << "!" << std::endl;
			return;
		}
		m_records.push_back(r);
		i_current_line++;
	}
}

Table::~Table() {
}

records::size_type Table::num_records() {
	return m_records.size();
}

record::size_type Table::num_columns() {
	if (m_records.empty()) return 0;
	return m_records[0].size();
}

void Table::print_table() {
	if (empty()) return;
	std::vector<int> max_field_lengths(m_records[0].size(), 0);
	for (record::size_type i = 0; i < m_records[0].size(); ++i) {
		for (records::size_type j = 0; j < m_records.size(); ++j) {
			max_field_lengths[i] = std::max(max_field_lengths[i], (int)m_records[j][i].size());
		}
	}
	for (auto r : m_records) {
		for (record::size_type i = 0; i < r.size() - 1; ++i) {
			std::cout << r[i] << std::string(max_field_lengths[i] - r[i].size(), ' ') << " | ";
		}
		std::cout << r[r.size() - 1] << std::endl;
	}
}

int Table::num_uniques(record::size_type i_column) {
	if (m_records.empty()) return 0;
	if (i_column < 0 || i_column >= m_records[0].size()) throw "Requested number of unique values in non-existent column!";
	std::set<std::string> fields;
	for (record r : m_records) fields.insert(r[i_column]);
	return (int)fields.size();
}

void Table::delete_static_columns() {
	if (m_records.empty()) return;
	std::vector<record::size_type> remaining_indices;
	for (record::size_type i_column = 0; i_column < m_records[0].size(); ++i_column) if (num_uniques(i_column) > 1) remaining_indices.push_back(i_column);
	records new_records;
	for (record r : m_records) {
		record new_record;
		for (auto i_column : remaining_indices) {
			new_record.push_back(r[i_column]);
		}
		new_records.push_back(new_record);
	}
	m_records = new_records;
}

bool Table::empty() const {
	return m_records.empty();
}

edge_vec Table::edges() const {
	if (empty()) return edge_vec();
	std::thread threads[NUM_THREADS];
	std::vector<edge_set> edge_sets(NUM_THREADS);
	for (int i = 0; i < NUM_THREADS; ++i) {
		edge_set edges = edge_sets[i];
		threads[i] = std::thread(&Table::generate_edges, std::ref(m_records), (records::size_type)i, std::ref(edge_sets[i]));
	}
	for (int i = 0; i < NUM_THREADS; ++i) threads[i].join();
	edge_set edges = edge_sets[0];
	for (int i = 1; i < NUM_THREADS; ++i) for (edge e : edge_sets[i]) edges.insert(e);
	return edge_vec(edges.begin(), edges.end());
}

void Table::generate_edges(const records &m_records, records::size_type i_slice, edge_set &edges) {
	for (records::size_type i_record = i_slice * m_records.size() / NUM_THREADS; i_record < (i_slice + 1) * m_records.size() / NUM_THREADS; ++i_record) {
		for (records::size_type record_distance = 1; record_distance < m_records.size() - i_record; ++record_distance) {
			edge new_edge(m_records[i_record].size());
			for (edge::size_type i_column = 0; i_column < new_edge.size(); ++i_column) {
				if (m_records[i_record][i_column] != m_records[i_record + record_distance][i_column]) new_edge[i_column] = 1;
			}
			bool insert = true;
			edge_vec to_delete;
			for (edge e : edges) {
				if ((e & new_edge) == e) { // subset edge already exists
					insert = false;
					break;
				}
				if ((e & new_edge) == new_edge) { // new edge is subset of other one
					to_delete.push_back(e);
				}
			}
			if (insert) {
				for (auto e : to_delete) edges.erase(e);
				edges.insert(new_edge);
			}
		}
	}
}

void Table::sort_columns_descending_uniqueness() {
	auto mmap = uniques_mmap();
	records new_records;
	for (record r : m_records) {
		record new_record;
		for (auto i = mmap.rbegin(); i != mmap.rend(); ++i) new_record.push_back(r[i->second]);
		new_records.push_back(new_record);
	}
	m_records = new_records;
}

void Table::sort_columns_ascending_uniqueness() {
	auto mmap = uniques_mmap();
	records new_records;
	for (record r : m_records) {
		record new_record;
		for (auto i = mmap.begin(); i != mmap.end(); ++i) new_record.push_back(r[i->second]);
		new_records.push_back(new_record);
	}
	m_records = new_records;
}

void Table::sort_records() {
	std::sort(m_records.begin(), m_records.end());
}

std::multimap<int, record::size_type> Table::uniques_mmap() {
	std::multimap<int, record::size_type> mmap;
	if (empty()) return mmap;
	for (record::size_type i = 0; i < m_records[0].size(); ++i) mmap.insert(std::pair<int, record::size_type>(num_uniques(i), i));
	return mmap;
}

void Table::save(std::string path, std::vector<std::string> header) {
	std::ofstream outfile;
	outfile.open(path);
	if (!header.empty()) {
		for (std::vector<std::string>::size_type i = 0; i < header.size() - 1; ++i) outfile << header[i] << ",";
		outfile << header[header.size() - 1] << std::endl;
	}
	for (record r : m_records) {
		for (record::size_type i = 0; i < r.size() - 1; ++i) outfile << r[i] << ",";
		outfile << r[r.size() - 1] << std::endl;
	}
	outfile.close();
}

void Table::add_record(record r) {
	m_records.push_back(r);
}

void Table::clear() {
	m_records.clear();
}