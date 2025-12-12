/*
Author: Songyu Qi
Zid: z5536858

This file contains the main function of the bwtdecode program.
*/

#include "bwt-util.h"

int main(int argc, char* argv[]) {
    const char* in_filename = argv[1];
    const char* out_filename = argv[2];

    FILE* f_in = fopen(in_filename, "rb");
    FILE* f_out = fopen(out_filename, "wb");
    
    bwtdecode(f_in, f_out);
    
    fclose(f_in);
    fclose(f_out);
    
    return 0;
}