/*
Author: Songyu Qi
Zid: z5536858

This header file contains the declarations of the functions and data structures.
For bwtdecode and bwtsearch.
*/
#pragma once

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#ifndef CHECKPOINT_SIZE_DECODE
#define CHECKPOINT_SIZE_DECODE 128
#endif

#ifndef CHECKPOINT_SIZE_SEARCH
#define CHECKPOINT_SIZE_SEARCH 2048
#endif

#define OUTPUT_BUFFER_SIZE 65536

typedef struct {
    uint32_t counts[4];
    uint32_t file_offset; // checkpoint file offset in file
    uint16_t run_offset; //offset of rle unit
} Checkpoint;

typedef struct {
    uint32_t N;
    uint32_t C_table_counts[4];
    Checkpoint* checkpoints; // array of checkpoints
    uint32_t checkpoint_count; // number of checkpoints
    uint32_t tail_index; // index of '\n' in L
    uint32_t C[4]; // C-table
    uint32_t checkpoint_k;
} BwtData; 

typedef struct {
    uint32_t start;
    uint32_t end;
    uint32_t num; //end - start
} SearchData;

typedef struct {
    char l_char; // L[i]
    uint32_t occ_val; // Occ(L[i], i)
} LF_Data;

/* Helper functions */
static inline int LEX_charToIndex(char c) {
    switch (c) {
        case 'A': return 0;
        case 'C': return 1;
        case 'G': return 2;
        case 'T': return 3;
        default:  return -1; // \n
    }
}

static inline char RLE_indexToChar(int rle_code) {
    switch (rle_code) {
        case 0: return 'A';
        case 1: return 'C';
        case 2: return 'G';
        case 3: return 'T';
        case 4: return '\n';
    }
}

static inline uint32_t calculate_checkpoint_count(uint32_t N, uint32_t k) {
    return (N / k) + 2;
}

uint32_t get_N(FILE* f_in); // get the number of characters in the file

BwtData bwt_initial(FILE* f_in, uint32_t checkpoint_size,
                    Checkpoint* checkpoint_array);

void bwt_free(BwtData* idx);

void LF_Mapping(FILE* f, const Checkpoint* checkpoints, 
                uint32_t k, uint32_t index,
                LF_Data* data);

void bwtdecode(FILE* f_in, FILE* f_out);

void bwtsearch(FILE* f_in, char c, const BwtData* idx, SearchData* search_data);

//culculate the occurrence of character c in the range [0, index]
uint32_t Occ_Count(FILE* f, const Checkpoint* checkpoints,
                   uint32_t k, uint32_t index, char c); 