/*
Author: Songyu Qi
Zid: z5536858

The core function for bwtdecode and bwtsearch.

*/
#define _POSIX_C_SOURCE 200809L

#include "bwt-util.h"

#define BUF_SIZE 4096 // buffer size for reading the file
static unsigned char buffer[BUF_SIZE];

uint32_t get_N(FILE* f_in) { 
    uint32_t N = 0;
    size_t io_len;
    fseek(f_in, 0, SEEK_SET);
    while ((io_len = fread(buffer, 1, BUF_SIZE, f_in)) > 0) {
        for (size_t io_pos = 0; io_pos < io_len; io_pos++) {
            unsigned char byte = buffer[io_pos];
            uint32_t c_len = (byte & 0x1F) + 1;
            N += c_len;
        }
    }
    fseek(f_in, 0, SEEK_SET);
    return N;
}

BwtData bwt_initial(FILE* f_in, uint32_t checkpoint_size,
                    Checkpoint* checkpoint_array) {
    BwtData idx;
    idx.N = 0;
    memset(idx.C_table_counts, 0, 4 * sizeof(uint32_t));
    idx.tail_index = (uint32_t)-1;
    idx.checkpoint_k = checkpoint_size;
    
    idx.checkpoints = checkpoint_array;
    idx.checkpoint_count = 0;
    
    fseek(f_in, 0, SEEK_SET);
    
    uint32_t current_pos = 0; // current position in the file
    uint32_t next_cp_boundary = 0;
    uint32_t current_counts[4] = {0}; // counts of each character
    long file_byte_offset = 0; // file byte offset
    size_t io_len;
    //read the long chars in Cache buffer
    while ((io_len = fread(buffer, 1, BUF_SIZE, f_in)) > 0) { 
        for (size_t io_pos = 0; io_pos < io_len; io_pos++) {
            unsigned char byte = buffer[io_pos];
            char c = RLE_indexToChar((byte >> 5) & 0x07);
            uint32_t c_len = (byte & 0x1F) + 1;
            
            long file_offset = file_byte_offset; //calculate for checkpoint file offset
            int lex_idx = (c == '\n') ? -1 : LEX_charToIndex(c);
            
            if (c == '\n' && idx.tail_index == (uint32_t)-1) {
                idx.tail_index = current_pos;
            }
            
            idx.N += c_len;
            if (lex_idx >= 0) {
                idx.C_table_counts[lex_idx] += c_len; //update C-table counts
            }
            
            while (current_pos + c_len > next_cp_boundary) { // place checkpoint when boundary lies within or at start
                uint32_t boundary_pos = next_cp_boundary;
                next_cp_boundary += checkpoint_size;
                uint32_t chars_offset = boundary_pos - current_pos;
                Checkpoint* cp = &checkpoint_array[idx.checkpoint_count]; //copy checkpoint
                for (int i = 0; i < 4; i++) {
                    cp->counts[i] = current_counts[i];
                }
                if (lex_idx >= 0) {
                    cp->counts[lex_idx] += chars_offset;
                }
                cp->file_offset = (uint32_t)file_offset; //checkpoint index in file
                cp->run_offset = (uint16_t)chars_offset; //offset of rle unit
                idx.checkpoint_count++;
            }
            //next checkpoint in next file rle point
            if (lex_idx >= 0) {
                current_counts[lex_idx] += c_len;
            }
            current_pos += c_len;
            file_byte_offset++;
        }
    }
    
    idx.C[0] = 1;
    idx.C[1] = idx.C[0] + idx.C_table_counts[0];
    idx.C[2] = idx.C[1] + idx.C_table_counts[1];
    idx.C[3] = idx.C[2] + idx.C_table_counts[2];
    
    return idx;
}

void bwt_free(BwtData* idx) {
    idx->checkpoints = NULL;
}

void LF_Mapping(FILE* f, const Checkpoint* checkpoints,
                uint32_t k, uint32_t index,
                LF_Data* data)
{
    int fd = fileno(f);
    uint32_t block_idx = index / k;
    const Checkpoint* cp = &checkpoints[block_idx];
    
    uint32_t cur_cnt[4];
    memcpy(cur_cnt, cp->counts, sizeof(cur_cnt));
    
    const uint32_t target_offset = index - block_idx * k;
    uint32_t gap_pos = 0;
    
    unsigned char local_buf[4096];
    off_t abs_off = cp->file_offset;
    ssize_t bytes_read = pread(fd, local_buf, sizeof(local_buf), abs_off);
    if (bytes_read <= 0) {
        data->l_char = '?';
        data->occ_val = 0;
        return;
    }
    
    size_t buf_pos = 0;
    size_t buf_len = bytes_read;
    
    if (buf_pos >= buf_len) {
        data->l_char = '?';
        data->occ_val = 0;
        return;
    }
    
    unsigned char byte = local_buf[buf_pos++];
    abs_off++;
    char c = RLE_indexToChar((byte >> 5) & 0x07);
    uint32_t c_len = (byte & 0x1F) + 1;
    
    int unused_rle = c_len - cp->run_offset;
    int lex_idx = (c == '|') ? -1 : LEX_charToIndex(c);
    
    while (1) {
        if (target_offset < gap_pos + unused_rle) {
            data->l_char = c;
            if (lex_idx >= 0) {
                cur_cnt[lex_idx] += (target_offset - gap_pos);
            }
            data->occ_val = (lex_idx >= 0) ? cur_cnt[lex_idx] : 0;
            return;
        }
        
        if (lex_idx >= 0) {
            cur_cnt[lex_idx] += unused_rle;
        }
        gap_pos += unused_rle;
        
        if (buf_pos >= buf_len) {
            bytes_read = pread(fd, local_buf, sizeof(local_buf), abs_off);
            if (bytes_read <= 0) {
                data->l_char = '?';
                data->occ_val = 0;
                return;
            }
            buf_pos = 0;
            buf_len = bytes_read;
        }
        
        byte = local_buf[buf_pos++];
        abs_off++;
        c = RLE_indexToChar((byte >> 5) & 0x07);
        c_len = (byte & 0x1F) + 1;
        lex_idx = (c == '\n') ? -1 : LEX_charToIndex(c);
        unused_rle = c_len;
    }
}

uint32_t Occ_Count(FILE* f, const Checkpoint* checkpoints,
                   uint32_t k, uint32_t index, char c)
{
    if (index == (uint32_t)-1) {
        return 0;
    }
    
    int target_lex_idx = LEX_charToIndex(c);
    if (target_lex_idx < 0) {
        return 0;
    }
    
    int fd = fileno(f);
    uint32_t block_idx = index / k;
    const Checkpoint* cp = &checkpoints[block_idx];
    
    uint32_t count = cp->counts[target_lex_idx];
    
    const uint32_t target_offset = index - block_idx * k;
    uint32_t gap_pos = 0;
    
    unsigned char local_buf[4096];
    off_t abs_off = cp->file_offset;
    ssize_t bytes_read = pread(fd, local_buf, sizeof(local_buf), abs_off);
    if (bytes_read <= 0) {
        return count;
    }
    
    size_t buf_pos = 0;
    size_t buf_len = bytes_read;
    
    if (buf_pos >= buf_len) {
        return count;
    }
    
    unsigned char byte = local_buf[buf_pos++];
    abs_off++;
    char current_c = RLE_indexToChar((byte >> 5) & 0x07);
    uint32_t c_len = (byte & 0x1F) + 1;
    
    int unused_rle = c_len - cp->run_offset;
    int lex_idx = (current_c == '\n') ? -1 : LEX_charToIndex(current_c);
    
    while (1) {
        if (target_offset < gap_pos + unused_rle) {
            uint32_t chars_in_this_run = target_offset - gap_pos + 1;
            if (lex_idx >= 0 && lex_idx == target_lex_idx) {
                count += chars_in_this_run;
            }
            return count;
        }
        
        if (lex_idx >= 0 && lex_idx == target_lex_idx) {
            count += unused_rle;
        }
        gap_pos += unused_rle;
        
        if (buf_pos >= buf_len) {
            bytes_read = pread(fd, local_buf, sizeof(local_buf), abs_off);
            if (bytes_read <= 0) {
                return count;
            }
            buf_pos = 0;
            buf_len = bytes_read;
        }
        
        byte = local_buf[buf_pos++];
        abs_off++;
        current_c = RLE_indexToChar((byte >> 5) & 0x07);
        c_len = (byte & 0x1F) + 1;
        lex_idx = (current_c == '\n') ? -1 : LEX_charToIndex(current_c);
        unused_rle = c_len;
    }
}

void bwtdecode(FILE* f_in, FILE* f_out)
{
    uint32_t N_est = get_N(f_in); //get N and calculate checkpoint count
    
    uint32_t checkpoint_count = calculate_checkpoint_count(N_est, CHECKPOINT_SIZE_DECODE);
    
    Checkpoint* checkpoints = (Checkpoint*)malloc(checkpoint_count * sizeof(Checkpoint));
   
    BwtData idx = bwt_initial(f_in, CHECKPOINT_SIZE_DECODE, checkpoints);
    uint32_t N = idx.N;
    
    char* out_chunk = (char*)malloc(OUTPUT_BUFFER_SIZE);

    //skip \n
    fseek(f_out, (long)(N - 1), SEEK_SET);
    char nl = '\n';
    fwrite(&nl, 1, 1, f_out);

    uint32_t cur_idx = 0;
    LF_Data data;
    uint32_t pos = (N > 0) ? (N - 2) : 0; // next file position to write
    int rem = OUTPUT_BUFFER_SIZE;          // remaining slots in out_chunk

    for (uint32_t i = 0; i < (N >= 1 ? N - 1 : 0); i++) {
        LF_Mapping(f_in, idx.checkpoints, idx.checkpoint_k, cur_idx, &data);
        out_chunk[--rem] = data.l_char; //write to Cache by downstream

        //write to file by upstream
        if (rem == 0) {
            uint32_t write_start = pos;
            fseek(f_out, (long)write_start, SEEK_SET);
            fwrite(out_chunk, 1, OUTPUT_BUFFER_SIZE, f_out);
            rem = OUTPUT_BUFFER_SIZE;
        }

        int next_lex_idx = LEX_charToIndex(data.l_char);
        if (next_lex_idx < 0) {
            cur_idx = 0;
        } else {
            cur_idx = idx.C[next_lex_idx] + data.occ_val;
        }
        if (pos > 0) pos--;
    }

    //write remaining unfull block to file by upstream
    if (rem != OUTPUT_BUFFER_SIZE) {
        int remain = OUTPUT_BUFFER_SIZE - rem;
        fseek(f_out, 0, SEEK_SET);
        fwrite(out_chunk + rem, 1, remain, f_out);
    }

    free(out_chunk);
    free(checkpoints);
}

void bwtsearch(FILE* f_in, char c, const BwtData* idx, SearchData* search_data)
{
    uint32_t start = search_data->start;
    uint32_t end = search_data->end;
    
    if (start >= end) { // can't find pattern
        search_data->start = 0;
        search_data->end = 0;
        search_data->num = 0;
        return;
    }
    
    int lex_idx = LEX_charToIndex(c);
    if (lex_idx < 0) {
        search_data->start = 0;
        search_data->end = 0;
        search_data->num = 0;
        return;
    }
    
    uint32_t start_occ = 0;
    if (start > 0) {
        start_occ = Occ_Count(f_in, idx->checkpoints, idx->checkpoint_k, start - 1, c);
    }
    
    uint32_t end_occ = 0;
    if (end > 0) {
        end_occ = Occ_Count(f_in, idx->checkpoints, idx->checkpoint_k, end - 1, c);
    }
    
    uint32_t new_start = idx->C[lex_idx] + start_occ;
    uint32_t new_end = idx->C[lex_idx] + end_occ;
    
    search_data->start = new_start;
    search_data->end = new_end;

    search_data->num = (new_end > new_start) ? (new_end - new_start) : 0;
}