#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>

int main(int argc, char* argv[]) {
    std::string pattern = argv[2];
    std::ifstream file(argv[1], std::ios::binary);
    std::string S;
    std::map <char, int> freq;
    char c;

    while (file.get(c)) {
        S += c;
        freq[c]++;
    }

    // Build the C table using freq
    int cumulative = 0;
    std::map <char, int> C;
    for (auto& p : freq) {
        C[p.first] = cumulative;
        cumulative += p.second;
    }
    int N = (int) L.size();
    if (N==0) std::cout<< 0 << std::endl;
    // Build the Occ array that supports the select function
    std::vector<std::vector<int>> occ(128, std::vector(N+1, 0));
    for (int i = 0; i < N; i++) {
        unsigned char uc = (unsigned char) L[i];
        for (auto& vec : occ) {
            vec[i+1] = vec[i];
        }
        occ[uc][i+1]++;
    }

    // build backward search anonymous function
    auto backwardsearch = [&]() -> int {
        int first = 0;
        int last = N;
        int m = (int) pattern.size();
        if (m == 0) return 0;
        for (int i = m-1; i >= 0; i--) {
            unsigned char uc = (unsigned char) pattern[i];
            first = C[pattern[i]] + occ[uc][first];
            last = C[pattern[i]] + occ[uc][last];
            if (first >= last) {
                return 0;
            }
        }
        return last - first;
    }

    int ans = backwardsearch();
    std::out<<ans<<std::endl;
    return 0;
}