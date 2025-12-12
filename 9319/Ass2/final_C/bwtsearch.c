/*
Author: Songyu Qi
Zid: z5536858

This file contains the main function of the bwtsearch program.
*/

#include "bwt-util.h"

int main(int argc, char* argv[]) {
    
    char pattern[105];
    FILE* f_in = fopen(argv[1], "rb");
    uint32_t N = get_N(f_in);
    uint32_t checkpoint_count = calculate_checkpoint_count(N, CHECKPOINT_SIZE_SEARCH);
    Checkpoint* checkpoints = (Checkpoint*)malloc(checkpoint_count * sizeof(Checkpoint));
    BwtData search_cp = bwt_initial(f_in, CHECKPOINT_SIZE_SEARCH, checkpoints);

    while (scanf("%s", pattern) == 1) {
        SearchData search_data;
        search_data.start = 0;
        search_data.end = search_cp.N;
        search_data.num = search_cp.N;
        
        int len = strlen(pattern);
        
        for (int i = len - 1; i >= 0; i--) {
            bwtsearch(f_in, pattern[i], &search_cp, &search_data);
            
            if (search_data.num == 0) {
                break;
            }
        }
        
        printf("%u\n", search_data.num);
    }
    
    free(checkpoints);
    fclose(f_in);
    return 0;
}