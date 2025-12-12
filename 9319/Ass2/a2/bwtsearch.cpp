/*
Author: Songyu Qi
Zid: z5536858

This file contains the main function of the bwtsearch program.
It implements BWT Backward Search using the checkpointing (sampling)
strategy to fit within memory limits.
*/

#include "bwt-util.h"
#include <vector>
#include <cstring>
#include <cstdio>
#include <algorithm>

using namespace std;

int main(int argc, char* argv[]) {
    char pattern[105];
    FILE* f_in = fopen(argv[1], "rb");
    
    // bwtsearch 使用较大的 checkpoint 间隔以节省内存
    BwtData search_cp = bwt_initial(f_in, CHECKPOINT_SIZE_SEARCH);

    while (scanf("%s", pattern) == 1) {
        SearchData search_data;
        search_data.start = 0;
        search_data.end = search_cp.N;
        search_data.num = search_cp.N;
        
        int len = strlen(pattern);
        
        // 从模式串的最后一个字符开始反向搜索
        for(int i = len - 1; i >= 0; i--) {
            bwtsearch(f_in, pattern[i], search_cp, search_data);
            
            // 如果匹配数为 0，提前退出
            if(search_data.num == 0) {
                break;
            }
        }
        
        printf("%u\n", search_data.num);
    }
    
    fclose(f_in);
    return 0;
}