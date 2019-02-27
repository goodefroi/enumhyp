#pragma once

#include "globals.h"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

void print_help(const po::options_description &option_description);
void verify_path(const fs::path &path);
std::vector<fs::path> graphs_from_path(const fs::path &path);