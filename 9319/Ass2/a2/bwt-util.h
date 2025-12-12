/*

Author: Songyu Qi
Zid: z5536858

This header file contains the declarations of the functions and data structures used in the bwt-util.cpp file.
The file is organized into the following sections:
1. Core data structures
2. Helper tools
3. Core functionalitys

*/
#pragma once

#include <cstdio>
#include <vector>
#include <cstring>    // for memcpy, strlen
#include <algorithm>  
#include <cstdint>    // for uint32_t, uint16_t
using namespace std;

// 为不同功能设置不同的 checkpoint 大小
#ifndef CHECKPOINT_SIZE_DECODE
#define CHECKPOINT_SIZE_DECODE 300  // bwtdecode 使用较小间隔
#endif

#ifndef CHECKPOINT_SIZE_SEARCH
#define CHECKPOINT_SIZE_SEARCH 2048  // bwtsearch 使用较大间隔以节省内存
#endif

struct Checkpoint {
    // 5 * 4 + 2 = 22 bytes
    uint32_t counts[4];
    uint32_t file_offset;
    uint16_t run_offset; 
};

struct BwtData {
    uint32_t N;
    uint32_t C_table_counts[4];
    vector<Checkpoint> checkpoints;
    uint32_t tail_index;
    uint32_t C[4];
    uint32_t checkpoint_interval;  // 新增：记录使用的 checkpoint 间隔
};

struct SearchData {
    uint32_t start;
    uint32_t end;
    uint32_t num;
};

struct LF_Data {
    char l_char;
    uint32_t occ_val;
};

inline int LEX_charToIndex(char c) {
    switch (c) {
        case 'A':  return 0;
        case 'C':  return 1;
        case 'G':  return 2;
        case 'T':  return 3;
        default:   return -1;
    }
}

inline char RLE_indexToChar(int rle_code) {
    switch (rle_code) {
        case 0: return 'A';
        case 1: return 'C';
        case 2: return 'G';
        case 3: return 'T';
        case 4: return '\n';
        default: return '?';
    }
}

// 修改函数签名，添加 checkpoint_size 参数
BwtData bwt_initial(FILE* f_in, uint32_t checkpoint_size);

void LF_Mapping(FILE* f, const vector<Checkpoint>& checkpoints, 
                uint32_t k, uint32_t index,
                LF_Data* data);

void bwtdecode(FILE* f_in, FILE* f_out);

void bwtsearch(FILE* f_in, char c, const BwtData& idx, SearchData& search_data);

uint32_t Occ_Count(FILE* f, const vector<Checkpoint>& checkpoints,
                   uint32_t k, uint32_t index, char c);