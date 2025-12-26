# Build semua test
cd test/platform/

# C API test
gcc -I../../include test_basic.c ../../src/exs_platform.c -o test_basic

# C++ API test  
g++ -std=c++11 -I../../include test_cpp.cpp ../../src/exs_platform.c ../../src/exs_platform.cpp -o test_cpp

# License test
g++ -std=c++11 -I../../include test_license.cpp ../../src/exs_platform.c ../../src/exs_platform.cpp -o test_license

# Performance test
gcc -I../../include test_perf.c ../../src/exs_platform.c -o test_perf

# Integration test
gcc -I../../include test_integration.c ../../src/exs_platform.c -o test_integration

# Run semua test
./test_basic
./test_cpp
./test_license
./test_perf
./test_integration
