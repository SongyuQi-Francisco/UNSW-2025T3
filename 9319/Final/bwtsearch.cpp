#include<iostream>
#include<fstream>
#include<map>
#include<vector>
#include<string>
using namespace std;

int main(int argc, char* argv[]){
    ifstream infile(argv[1]);
    string pattern = argv[2];
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
    int low = 0, high = n;
    int m = pattern.length();
    for(int i = m - 1; i >= 0; i --) {
        low = C[pattern[i]] + occ[low][pattern[i]];
        high = C[pattern[i]] + occ[high][pattern[i]];
        if(low > high)
            return 0;
    }
    int ans = high - low;
    cout << ans << endl;
    infile.close();
    return 0;
}