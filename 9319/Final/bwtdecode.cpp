#include<iostream>
#include<fstream>
#include<map>
#include<vector>
#include<string>
using namespace std;

int main(int argc, char* argv[]){
    ifstream infile(argv[1]);
    ofstream outfile(argv[2]);
    string L;
    char c;
    map<char, int> freq;
    while(infile.get(c)) {
        freq[c] ++;
        L += c;
    }
    //C-table
    map<char, int> C;
    int cumulation = 0;
    for(auto& fq : freq) {
        C[fq.first] = cumulation;
        cumulation += fq.second;
    }
    //Occ
    int n = L.length();
    vector<vector<int>> occ(n + 1, vector<int>(128, 0));
    for(int i = 0; i < n; i ++) {
        occ[i + 1] = occ[i];
        occ[i + 1][L[i]] ++;
    }
    //LF-Mapping
    int idx = 0;
    string decoded;
    decoded.reserve(n);
    for(int i = 0; i < n; i ++) {
        decoded += L[idx];
        idx = C[L[idx]] + occ[idx][L[idx]];
    }
    reverse(decoded.begin(), decoded.end() - 1);
    outfile << decoded;
    infile.close();
    outfile.close();
    return 0;
}