# enumhyp
Source code used to for the performance measurements in [Bläsius, Thomas; Friedrich, Tobias; Lischeid, Julius; Meeks, Kitty; Schirneck, Martin. *Efficiently Enumerating Hitting Sets of Hypergraphs Arising in Data Profiling.* Algorithm Engineering and Experiments (ALENEX) 2019](https://hpi.de/friedrich/news/2018/alenex.html?tx_extbibsonomycsl_publicationlist%5BuserName%5D=puma-friedrich&tx_extbibsonomycsl_publicationlist%5BintraHash%5D=0aecd21152fdb3b41484d610834d7fec&tx_extbibsonomycsl_publicationlist%5BfileName%5D=ALENEX19_UCC.pdf&tx_extbibsonomycsl_publicationlist%5Bcontroller%5D=Document&cHash=af82a4759d1b33a3042ed64d7ce5060c). This tool generates unique column combination (UCC) hypergraphs from CSVs, enumerates all minimal hitting sets of hypergraphs using the enumeration algorithm described in the paper above or using a brute force algorithm.

## Build
### CMake
The CMake build is tested with VS 2017 and Make on Ubuntu on Windows. Requires [boost](https://www.boost.org/), specifically the `dynamic_bitset` library (header-only) and `filesystem` ([needs to be compiled](https://www.boost.org/doc/libs/1_68_0/more/getting_started/unix-variants.html#easy-build-and-install), consider using `--with-libraries=filesystem` to reduce compilation time). After installing boost, navigate to the top-level directory of `enumhyp`. Run `cmake .` to generate VS project files or Makefiles, depending on your OS. If CMake could not find your boost installation, `cmake . -DBOOST_ROOT:PATHNAME=/path/to/boost_1_68_0` should do the trick.
### main.cpp
Edit `src/main.cpp` to choose what the tool does.
### globals.h
Use `HITTING_SET_STATS`, `ORACLE_STATS` etc. in `src/globals.h` to choose what kind of statistics is collected. These switches have a significant performance impact, so enable only the ones you need.

## Hypergraph files
Graphs are saved as plain text files, the number of vertices in the first line is followed by one edge per line. An edge is a comma-separated list of vertex indices.
