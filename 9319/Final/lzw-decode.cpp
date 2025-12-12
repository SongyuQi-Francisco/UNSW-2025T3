#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>
using namespace std;

int main(int argc, char* argv[]) {
    ifstream infile(argv[1]);
    vector<int> codes;
    int code;
    while (infile >> code) {
        codes.push_back(code);
    }
    unordered_map<int, string> dict;
    for (int i=0; i<256; i++) {
        string s(1, (unsigned char) i);
        dict[i] = s;
    }

    string output;
    
    code = codes[0];
    string c;
    string p = dict[code];
    output += p;
    int N = codes.size();
    int next_code = 256;
    for (int i = 1; i < N; i++) {
        code = codes[i];
        if (dict.count(code)) {
            c = dict.at(code);
        } else {
            c = p + p[0];
        }
        cout<< c <<endl;
        output += c;
        dict.insert(make_pair(next_code ++, p+c[0]));
        p = c;
    }
    cout<< output <<endl;

    return 0;
}