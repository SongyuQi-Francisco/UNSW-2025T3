/*

Author: Songyu Qi
Zid: z5536858

内存优化版本：为 bwtsearch 和 bwtdecode 使用不同的 checkpoint 间隔
*/
#include "bwt-util.h"
#include <cstdio>
#include <vector>
#include <cstring>
#include <algorithm>
#include <unistd.h>

using namespace std;

#ifndef BUF_SIZE
static const size_t BUF_SIZE = 4096;
#endif
unsigned char buffer[BUF_SIZE];

BwtData bwt_initial(FILE* f_in, uint32_t checkpoint_size) {
    BwtData idx;
    idx.N = 0;
    memset(idx.C_table_counts, 0, 4 * sizeof(uint32_t));
    idx.tail_index = (uint32_t)-1;
    idx.checkpoint_interval = checkpoint_size;  // 记录使用的间隔
    
    fseek(f_in, 0, SEEK_END);
    long file_size = ftell(f_in);
    fseek(f_in, 0, SEEK_SET);
    
    uint32_t estimated_checkpoints = (uint32_t)((file_size * 4) / checkpoint_size) + 10;
    idx.checkpoints.reserve(estimated_checkpoints);
    
    uint32_t current_pos = 0;
    uint32_t next_cp_boundary = 0;
    uint32_t current_counts[4] = {0};
    long file_byte_offset = 0;
    size_t io_len;
    
    while ((io_len = fread(buffer, 1, BUF_SIZE, f_in)) > 0) {
        for (size_t io_pos = 0; io_pos < io_len; io_pos++) {
            unsigned char byte = buffer[io_pos];
            char c = RLE_indexToChar((byte >> 5) & 0x07);
            uint32_t c_len = (byte & 0x1F) + 1;
            
            long file_offset = file_byte_offset;
            int lex_idx = (c == '\n') ? -1 : LEX_charToIndex(c);
            
            if (c == '\n' && idx.tail_index == (uint32_t)-1) {
                idx.tail_index = current_pos;
            }
            
            idx.N += c_len;
            if (lex_idx >= 0) {
                idx.C_table_counts[lex_idx] += c_len;
            }
            
            // 使用传入的 checkpoint_size 生成 checkpoint
            while (current_pos + c_len > next_cp_boundary) {
                uint32_t boundary_pos = next_cp_boundary;
                next_cp_boundary += checkpoint_size;
                uint32_t chars_offset = boundary_pos - current_pos;
                
                Checkpoint cp;
                for (int i = 0; i < 4; i++) {
                    cp.counts[i] = current_counts[i];
                }
                if (lex_idx >= 0) {
                    cp.counts[lex_idx] += chars_offset;
                }
                cp.file_offset = (uint32_t)file_offset;
                cp.run_offset = (uint16_t)chars_offset;
                idx.checkpoints.push_back(cp);
            }
            
            if (lex_idx >= 0) {
                current_counts[lex_idx] += c_len;
            }
            current_pos += c_len;
            file_byte_offset++;
        }
    }
    
    // 构建 C-Table
    idx.C[0] = 1;
    idx.C[1] = idx.C[0] + idx.C_table_counts[0];
    idx.C[2] = idx.C[1] + idx.C_table_counts[1];
    idx.C[3] = idx.C[2] + idx.C_table_counts[2];
    
    return idx;
}

void LF_Mapping(FILE* f, const vector<Checkpoint>& checkpoints,
                uint32_t k, uint32_t index,
                LF_Data* data)
{
    int fd = fileno(f);
    uint32_t block_idx = index / k;
    const Checkpoint& cp = checkpoints[block_idx];
    
    uint32_t cur_cnt[4];
    memcpy(cur_cnt, cp.counts, sizeof(cur_cnt));
    
    const uint32_t target_offset = index - block_idx * k;
    uint32_t gap_pos = 0;
    
    unsigned char local_buf[4096];
    ssize_t bytes_read = pread(fd, local_buf, sizeof(local_buf), cp.file_offset);
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
    char c = RLE_indexToChar((byte >> 5) & 0x07);
    uint32_t c_len = (byte & 0x1F) + 1;
    
    int noused_rle = c_len - cp.run_offset;
    int lex_idx = (c == '\n') ? -1 : LEX_charToIndex(c);
    
    while (true) {
        if (target_offset < gap_pos + noused_rle) {
            data->l_char = c;
            if (lex_idx >= 0) {
                cur_cnt[lex_idx] += (target_offset - gap_pos);
            }
            data->occ_val = (lex_idx >= 0) ? cur_cnt[lex_idx] : 0;
            return;
        }
        
        if (lex_idx >= 0) {
            cur_cnt[lex_idx] += noused_rle;
        }
        gap_pos += noused_rle;
        
        if (buf_pos >= buf_len) {
            off_t new_offset = cp.file_offset + buf_pos;
            bytes_read = pread(fd, local_buf, sizeof(local_buf), new_offset);
            if (bytes_read <= 0) {
                data->l_char = '?';
                data->occ_val = 0;
                return;
            }
            buf_pos = 0;
            buf_len = bytes_read;
        }
        
        byte = local_buf[buf_pos++];
        c = RLE_indexToChar((byte >> 5) & 0x07);
        c_len = (byte & 0x1F) + 1;
        lex_idx = (c == '\n') ? -1 : LEX_charToIndex(c);
        noused_rle = c_len;
    }
}

// 优化的 Occ_Count：计算字符 c 在 [0, index] 中的出现次数
uint32_t Occ_Count(FILE* f, const vector<Checkpoint>& checkpoints,
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
    const Checkpoint& cp = checkpoints[block_idx];
    
    // 从 checkpoint 获取初始计数
    uint32_t count = cp.counts[target_lex_idx];
    
    const uint32_t target_offset = index - block_idx * k;
    uint32_t gap_pos = 0;
    
    unsigned char local_buf[4096];
    ssize_t bytes_read = pread(fd, local_buf, sizeof(local_buf), cp.file_offset);
    if (bytes_read <= 0) {
        return count;
    }
    
    size_t buf_pos = 0;
    size_t buf_len = bytes_read;
    
    if (buf_pos >= buf_len) {
        return count;
    }
    
    unsigned char byte = local_buf[buf_pos++];
    char current_c = RLE_indexToChar((byte >> 5) & 0x07);
    uint32_t c_len = (byte & 0x1F) + 1;
    
    int noused_rle = c_len - cp.run_offset;
    int lex_idx = (current_c == '\n') ? -1 : LEX_charToIndex(current_c);
    
    while (true) {
        // 如果目标位置在当前 run 内
        if (target_offset < gap_pos + noused_rle) {
            uint32_t chars_in_this_run = target_offset - gap_pos + 1;
            if (lex_idx >= 0 && lex_idx == target_lex_idx) {
                count += chars_in_this_run;
            }
            return count;
        }
        
        // 累加当前 run 中目标字符的出现次数
        if (lex_idx >= 0 && lex_idx == target_lex_idx) {
            count += noused_rle;
        }
        gap_pos += noused_rle;
        
        // 读取下一个 run
        if (buf_pos >= buf_len) {
            off_t new_offset = cp.file_offset + buf_pos;
            bytes_read = pread(fd, local_buf, sizeof(local_buf), new_offset);
            if (bytes_read <= 0) {
                return count;
            }
            buf_pos = 0;
            buf_len = bytes_read;
        }
        
        byte = local_buf[buf_pos++];
        current_c = RLE_indexToChar((byte >> 5) & 0x07);
        c_len = (byte & 0x1F) + 1;
        lex_idx = (current_c == '\n') ? -1 : LEX_charToIndex(current_c);
        noused_rle = c_len;
    }
}

void bwtdecode(FILE* f_in, FILE* f_out)
{
    // bwtdecode 使用较小的 checkpoint 间隔以提高速度
    BwtData idx = bwt_initial(f_in, CHECKPOINT_SIZE_DECODE);
    uint32_t N = idx.N;
    
    const int OUT_BUFFER_SIZE = 16384;
    char* out_buffer = new char[OUT_BUFFER_SIZE];
    int buf_idx = OUT_BUFFER_SIZE - 1;
    
    LF_Data data;
    out_buffer[buf_idx--] = '\n';
    
    uint32_t cur_idx = 0;
    
    for (uint32_t i = 0; i < N - 1; i++) {
        LF_Mapping(f_in, idx.checkpoints, idx.checkpoint_interval, cur_idx, &data);
        
        if (data.l_char == '?') {
            break;
        }
        
        out_buffer[buf_idx--] = data.l_char;
        
        if (buf_idx < 0) {
            uint32_t write_start = N - 1 - i;
            fseek(f_out, write_start, SEEK_SET);
            fwrite(out_buffer, 1, OUT_BUFFER_SIZE, f_out);
            buf_idx = OUT_BUFFER_SIZE - 1;
        }
        
        int next_lex_idx = LEX_charToIndex(data.l_char);
        if (next_lex_idx < 0) {
            cur_idx = 0;
        } else {
            cur_idx = idx.C[next_lex_idx] + data.occ_val;
        }
    }
    
    if (buf_idx < OUT_BUFFER_SIZE - 1) {
        int remain = OUT_BUFFER_SIZE - 1 - buf_idx;
        fseek(f_out, 0, SEEK_SET);
        fwrite(out_buffer + buf_idx + 1, 1, remain, f_out);
    }
    delete[] out_buffer;
}

void bwtsearch(FILE* f_in, char c, const BwtData& idx, SearchData& search_data)
{
    uint32_t start = search_data.start;
    uint32_t end = search_data.end;
    
    if (start >= end) {
        search_data.start = 0;
        search_data.end = 0;
        search_data.num = 0;
        return;
    }
    
    int lex_idx = LEX_charToIndex(c);
    if (lex_idx < 0) {
        search_data.start = 0;
        search_data.end = 0;
        search_data.num = 0;
        return;
    }
    
    // 计算字符 c 在 [0, start-1] 中的出现次数
    uint32_t start_occ = 0;
    if (start > 0) {
        start_occ = Occ_Count(f_in, idx.checkpoints, idx.checkpoint_interval, start - 1, c);
    }
    
    // 计算字符 c 在 [0, end-1] 中的出现次数
    uint32_t end_occ = 0;
    if (end > 0) {
        end_occ = Occ_Count(f_in, idx.checkpoints, idx.checkpoint_interval, end - 1, c);
    }
    
    // 使用 BWT backward search 公式计算新区间
    uint32_t new_start = idx.C[lex_idx] + start_occ;
    uint32_t new_end = idx.C[lex_idx] + end_occ;
    
    search_data.start = new_start;
    search_data.end = new_end;
    search_data.num = (new_end > new_start) ? (new_end - new_start) : 0;
}