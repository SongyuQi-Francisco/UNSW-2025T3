/*

Author: Songyu Qi
Zid: z5536858

This file contains the main function of the bwtdecode program.
*/

#include "bwt-util.h"
#include <iostream> // for std::cerr, std::endl
using namespace std;

// our memory budget: strict limit = 14.9MB 
const long long SAFE_RAM_BUDGET = 14942200L; 

int main(int argc, char* argv[]) {
    
    const char* in_filename = argv[1];
    const char* out_filename = argv[2];

    CountData count_data = count_chars(in_filename);
    
    long long N = count_data.N;
    run_checkpoint(in_filename, out_filename, count_data, SAFE_RAM_BUDGET);

    return 0;
}