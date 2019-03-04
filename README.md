# enumhyp
Source code used to for the performance measurements in [Bläsius, Thomas; Friedrich, Tobias; Lischeid, Julius; Meeks, Kitty; Schirneck, Martin. *Efficiently Enumerating Hitting Sets of Hypergraphs Arising in Data Profiling.* Algorithm Engineering and Experiments (ALENEX) 2019](https://hpi.de/friedrich/research/enumdat). This tool generates unique column combination (UCC) hypergraphs from CSVs and enumerates all minimal hitting sets of hypergraphs using the enumeration algorithm described in the paper above. Use commit `8e6dfdba80a16d9d5eb1060561d9dbd747d564ef` to reproduce results.

## Build using CMake
The CMake build is tested with VS 2017 on Windows and with Make on Ubuntu on Windows. Requires [boost](https://www.boost.org/), including the compiled libraries `filesystem` and `program_options` (consider using `--with-libraries=filesystem,program_options` to reduce compilation time). After installing boost, navigate to the top-level directory of `enumhyp`. Run `cmake .` to generate VS project files or Makefiles, depending on your OS. If CMake could not find your boost installation, `cmake . -DBOOST_ROOT:PATHNAME=/path/to/boost` should do the trick.

## Use
Use `enumhyp --help` to show available options. Executing `enumhyp generate table.csv` will generate a UCC hypergraph `table.graph` and place it in your current working directory. Following that, you can use `enumhyp enumerate table.graph` to enumerate all minimal hitting sets for the hypergraph (results are discarded by default, use `-o path/to/file/or/directory` to save transversal hypergraphs). Save hitting set delays using the `-H` switch and extensive oracle statistics by using `-O`. Different enumeration algorithm implementations can be compared using `-I`. The vertex order of input graphs can be randomized using `-r`.

## Hypergraph files
Graphs are saved as plain text files, the number of vertices in the first line is followed by one edge per line. An edge is a comma-separated list of vertex indices.
