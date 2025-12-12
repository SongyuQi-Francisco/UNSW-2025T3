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

void initialize_dictionary(unordered_map<uint32_t, string>& dict, uint32_t& next_code_index);
uint32_t read_next_code_from_file(ifstream& infile);

int main(int argc, char* argv[]) {
    // check input arguments
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <input_file> <output_file>" << endl;
        return 1;
    }

    // open input and output files
    ifstream infile(argv[1], ios::binary);
    ofstream outfile(argv[2], ios::binary);
    if (!infile.is_open()) { cerr << "Error: Cannot open input file." << endl; return 1; }
    if (!outfile.is_open()) { cerr << "Error: Cannot open output file." << endl; return 1; }

    // read N from input file (4 bytes) follow Big-Endian
    unsigned char header_bytes[4];
    if (!infile.read((char*)header_bytes, 4)) { // can't recognize Header
        cerr << "Error: Cannot read 4-byte header from input file." << endl;
        return 1;
    }
    // Get N from Header
    uint32_t N = (header_bytes[0] << 24) | 
                 (header_bytes[1] << 16) | 
                 (header_bytes[2] << 8) | 
                 (header_bytes[3]);
    //cout << "N: " << N << endl;
    unordered_map<uint32_t, string> dictionary; //Use hash map to store the dictionary and complit the code search
    uint32_t next_code_index = 0;
    uint32_t bytes_decoded = 0; // count N
    
    // initialize Dict (0-127)
    initialize_dictionary(dictionary, next_code_index);

    string p; // prev stream
    uint32_t c; // current stream

    if ((c = read_next_code_from_file(infile)) == (uint32_t)-1) // file is empty or only contains header
        return 0;

    // check the first code
    if (!dictionary.count(c)) {
        cerr << "Error: First code is invalid." << endl;
        return 1;
    }
    
    p = dictionary.at(c);
    outfile.write(p.c_str(), p.length());
    bytes_decoded += p.length();

    while ((c = read_next_code_from_file(infile)) != (uint32_t)-1) {
        //string output_c = dictionary.at(c);
        string output_c;
        if (dictionary.count(c)) {
            output_c = dictionary.at(c);
        } else {
            output_c = p + p[0]; //ab aba, aba = ab() + (a)b
        }
        outfile.write(output_c.c_str(), output_c.length()); //output Dict[c]
        bytes_decoded += output_c.length(); // count N
        
        dictionary.insert(make_pair(next_code_index ++, p + output_c[0])); // add p+Dict[c][0] to dictionary
        p = output_c;
        
        if (N > 0 && bytes_decoded > N) {
            initialize_dictionary(dictionary, next_code_index);
            bytes_decoded = output_c.length(); // reset counter, current output is the first of new chunk
        }
    }
    return 0;
}

void initialize_dictionary(unordered_map<uint32_t, string>& dict, uint32_t& next_code_index) {
    dict.clear();
    for (int i = 0; i <= 127; ++i) {
        dict.insert(make_pair(i, string(1, (char)i)));
    }
    next_code_index = 256; //256 up need to change
}

uint32_t read_next_code_from_file(ifstream& infile) {
    unsigned char byte1, byte2, byte3; //unsigned char 8-bits

    if (!infile.read((char*)&byte1, 1)) {
        return (uint32_t)-1; // file empty
    }

    // check prefix
    uint8_t prefix = (byte1 >> 6); // get the highest 2 bits

    if (prefix == 0b00 || prefix == 0b01) { 
        // 1 byte code: (0) + 7 bits = 8 bits
        return (uint32_t)(byte1 & 0x7F); //0x7F = 01111111
    } 
    else if (prefix == 0b10) { 
        // 2 byte code: (10) + 14 bits = 16 bits
        if (!infile.read((char*)&byte2, 1)) {
            cerr << "Error: Unexpected EOF while reading 2-byte code." << endl;
            return (uint32_t)-1;
        }
        return (uint32_t)(((byte1 & 0x3F) << 8) | byte2); 
    } 
    else if (prefix == 0b11) { 
        // 3 byte code: (11) + 22 bits = 24 bits
        if (!infile.read((char*)&byte2, 1) || !infile.read((char*)&byte3, 1)) {
            cerr << "Error: Unexpected EOF while reading 3-byte code." << endl;
            return (uint32_t)-1;
        }
        return (uint32_t)(((byte1 & 0x3F) << 16) | (byte2 << 8) | byte3); 
    }
    //cerr << "Error: Invalid LZW prefix." << endl;
    return (uint32_t)-1; 
}
