set (CMAKE_CXX_STANDARD 11)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.67.0 REQUIRED COMPONENTS filesystem program_options)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
add_executable(enumhyp main.cpp globals.cpp hypergraph.cpp table.cpp)
target_link_libraries(enumhyp ${Boost_LIBRARIES} Threads::Threads)