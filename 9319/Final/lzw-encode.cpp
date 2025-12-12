#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    std::ifstream infile(argv[1]);

    std::map<std::string, int> dictionary;
    for (int i = 0; i < 256; i++) {
        std::string s(1, (unsigned char) i);
        dictionary[s] = i;
    }

    int next_code = 256;
    char c;
    std::string p;
    std::vector<int> output;
    while (infile.get(c)) {
        std::string pc = p + c;
        if (dictionary.find(pc) != dictionary.end()) {
            p = pc;
        }
        else {
            dictionary[pc] = next_code++;
            output.push_back(dictionary[p]);
            p = c;
        }
    }
    output.push_back(dictionary[p]);

    for (auto i : output) {
        std::cout << i << ' ';
    }
    std::cout<< std::endl;

    return 0;
}