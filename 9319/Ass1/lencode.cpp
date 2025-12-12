#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <unordered_map>
#include <stdexcept>
#include <utility>

using namespace std;

const uint32_t MAX_1_BYTE_CODE = 128;      // 2^7
const uint32_t MAX_2_BYTE_CODE = 16384;    // 2^14
const uint32_t MAX_DICTIONARY_SIZE = 4194304; // 2^22 LZW max code value + 1

// variable-length code parameter
struct VLCParams {
    uint32_t payload; // final code with prefix 
    int total_bits;   
};


VLCParams get_vlc_params(uint32_t code);
void write_code(ofstream& outfile, uint32_t code);
void initialize_dictionary(unordered_map<string, uint32_t>& dict, uint32_t& next_code_index);


int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <input_file> <output_file> <reset_frequency_N>" << endl;
        return 1;
    }
    
    uint32_t N;
    try {
        N = stoul(argv[3]); // convert string to uint32_t
    } catch (const exception& e) {
        cerr << "Error: Invalid reset frequency N." << endl;
        return 1;
    }

    // open input and output files
    ifstream infile(argv[1], ios::binary);
    ofstream outfile(argv[2], ios::binary);
    if (!infile.is_open()) { cerr << "Error: Cannot open input file." << endl; return 1; }
    if (!outfile.is_open()) { cerr << "Error: Cannot open output file." << endl; return 1; }

    // write N to output file (4 bytes) follow Big-Endian
    unsigned char header_bytes[4];
    // 00 00 00 00 total 32-bits
    header_bytes[0] = (unsigned char)((N >> 24) & 0xFF);
    header_bytes[1] = (unsigned char)((N >> 16) & 0xFF);
    header_bytes[2] = (unsigned char)((N >> 8) & 0xFF);
    header_bytes[3] = (unsigned char)(N & 0xFF);
    outfile.write((char*)header_bytes, 4);

   
    unordered_map<string, uint32_t> dictionary; //Use hash map to store the dictionary and complit the code search
    uint32_t next_code_index = 0;
    uint32_t bytes_encoded = 0; // count N
    
    // initialize Dict (0-127)
    initialize_dictionary(dictionary, next_code_index);

    string p; // prev stream
    char c; // current char

    if (infile.get(c)) { // pre-read first char
        p = string(1, c);
        bytes_encoded++;
    } else {
        cerr << "Error: Input file is empty." << endl;
        return 1;
    }

    while (infile.get(c)) {
        bytes_encoded++; // count each source byte processed
        string p_c = p + string(1, c);
        
        if (N > 0 && bytes_encoded > N) {
            while (!p.empty()) { //output unreset part
                string longest_hash = "";
                for (size_t len = p.length(); len >= 1; len--) {
                    string prefix = p.substr(0, len);
                    if (dictionary.count(prefix)) {
                        longest_hash = prefix;
                        break;
                    }
                }
                
                 uint32_t code_to_output = dictionary.at(longest_hash);
                 write_code(outfile, code_to_output);
                 p = p.substr(longest_hash.length());
            }
            
            initialize_dictionary(dictionary, next_code_index);
            bytes_encoded = 1; 
            p = string(1, c); 
            continue; //new chunk's first input
        }
        
        if (dictionary.count(p_c)) {  //p+c matched, continue to match longer sequence
            p = p_c;
        } else { // p+c not matched, output p, add p+c, reset p=c

            if (next_code_index < MAX_DICTIONARY_SIZE) {
                dictionary.insert(make_pair(p_c, next_code_index++));
            }
    
            uint32_t code_to_output = dictionary.at(p);
            write_code(outfile, code_to_output); // output p's code
            p = string(1, c);
        }
    }

    if (!p.empty()) {
        uint32_t final_code = dictionary.at(p);
        write_code(outfile, final_code);
    }

    return 0;
}

// Initialize dictionary for ASCII characters
void initialize_dictionary(unordered_map<string, uint32_t>& dict, uint32_t& next_code_index) {
    dict.clear();
    for (int i = 0; i <= 127; ++i) {
        dict.insert(make_pair(string(1, (char)i), i));
    }
    next_code_index = 256; //256 up need to change
}

// Variable-length code parameter calculation function
VLCParams get_vlc_params(uint32_t code) {
    VLCParams result = {0, 0};
    
    // 0-127 : 0 prefix + 7 bits = 8 bits
    if (code < MAX_1_BYTE_CODE) { 
        result.payload = code;          
        result.total_bits = 8; 
    } 
    // 128-16383 : 10 prefix + 14 bits = 16 bits
    else if (code < MAX_2_BYTE_CODE) { 
        result.payload = (code & 0x3FFF) | (2 << 14); 
        result.total_bits = 16; 
    } 
    // 16384-2^22-1 : 11 prefix + 22 bits = 24 bits
    else { 
        result.payload = (code & 0x3FFFFF) | (3 << 22); 
        result.total_bits = 24; 
    }
    
    return result;
}

void write_code(ofstream& outfile, uint32_t code) {
    VLCParams vlc = get_vlc_params(code);
    uint32_t payload = vlc.payload;
    int total_bits = vlc.total_bits;

    // variable-length code is always 8-bit aligned, can output directly
    if (total_bits == 8) {
        // 1-byte : output directly
        unsigned char byte1 = (unsigned char)(payload & 0xFF);
        outfile.write((char*)&byte1, 1);
    }
    else if (total_bits == 16) {
        // 2-bytes : (10) + 14 = 16-bites
        unsigned char byte1 = (unsigned char)((payload >> 8) & 0xFF);
        unsigned char byte2 = (unsigned char)(payload & 0xFF);
        outfile.write((char*)&byte1, 1);
        outfile.write((char*)&byte2, 1);
    }
    else if (total_bits == 24) {
        // 3bytes：(11) + 22 = 24-bits
        unsigned char byte1 = (unsigned char)((payload >> 16) & 0xFF);
        unsigned char byte2 = (unsigned char)((payload >> 8) & 0xFF);
        unsigned char byte3 = (unsigned char)(payload & 0xFF);
        outfile.write((char*)&byte1, 1);
        outfile.write((char*)&byte2, 1);
        outfile.write((char*)&byte3, 1);
    }
}
