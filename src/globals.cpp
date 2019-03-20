#include "globals.h"

#include <boost/filesystem.hpp>

void print_edge(const edge &e) {
	std::cout << edge_to_string(e) << std::endl;
}

void print_edge_vec(const edge_vec &v) {
	int i = 0;
	std::string s_largest_edge_index = std::to_string(v.size() - 1);
	int size_width = (int)s_largest_edge_index.length();
	for (auto e : v) {
		std::stringstream ss;
		ss << std::setw(size_width) << i;
		std::cout << ss.str() << ": ";
		print_edge(e);
		++i;
	}
}

std::string edge_to_string(const edge &e) {
	if (e.none()) return "";
	std::string s;
	for (auto i = e.find_first(); i != edge::npos; i = e.find_next(i)) {
		s += std::to_string(i);
		s += " ";
	}
	s.pop_back();
	return s;
}

std::string ns_string(Clock::time_point sooner, Clock::time_point later) {
	return std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(later - sooner).count());
}

std::string time_string(Clock::time_point sooner, Clock::time_point later) {
	auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(later - sooner).count();
	std::stringstream ss_h, ss_m, ss_s, ss_ms, ss_us, ss_ns;
	ss_h << (ns / 3600000000000);
	ss_m << std::setfill('0') << std::setw(2) << (ns / 60000000000 % 60);
	ss_s << std::setfill('0') << std::setw(2) << (ns / 1000000000 % 60);
	ss_ms << std::setfill('0') << std::setw(3) << (ns / 1000000 % 1000);
	ss_us << std::setfill('0') << std::setw(3) << (ns / 1000 % 1000);
	ss_ns << std::setfill('0') << std::setw(3) << (ns % 1000);
	return ss_h.str() + ":" + ss_m.str() + ":" + ss_s.str() + " " + ss_ms.str() + ":" + ss_us.str() + ":" + ss_ns.str();
}

uint64_t duration_ns(Clock::time_point sooner, Clock::time_point later) {
	return (uint64_t) std::chrono::duration_cast<std::chrono::nanoseconds>(later - sooner).count();
}

edge::size_type last_vertex(edge e) {
	if (e.none()) return edge::npos;
	edge::size_type i = e.find_first();
	edge::size_type previous_i = i;
	while (i != edge::npos) {
		previous_i = i;
		i = e.find_next(i);
	}
	return previous_i;
}

std::string permutation_string(const permutation &p) {
	std::string s;
	for (auto i : p) {
		s += std::to_string(i);
		s += " ";
	}
	s.pop_back();
	return s;
}

std::string remove_quotations(std::string s) {
	if (s.front() != '"' || s.back() != '"' || s.length() < 2) return s;
	return s.substr(1, s.length() - 2);
}