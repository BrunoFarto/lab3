cmake_minimum_required(VERSION 3.18)
project(qsort LANGUAGES C CXX)

include_directories(include)

add_executable(qs
        src/qs.c
        src/qs-seq.c
)

####################
## OpenMP
####################

find_package(OpenMP)
add_executable(qs_par
        src/qs.c
        src/qs-omp.c
)
target_link_libraries(qs_par PUBLIC OpenMP::OpenMP_C)

add_executable(primes
        src/primes.cpp
)
target_link_libraries(primes PUBLIC OpenMP::OpenMP_CXX)
