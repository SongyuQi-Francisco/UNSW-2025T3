#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <iomanip>
using namespace std;
typedef pair<long double, long double> Range;
int main(int argc, char* argv[]) {
    ifstream infile(argv[1]);
    string content;
    char c;
    while (infile.get(c)) {
        content += c;
    }
    infile.close();
    map<char, int> freq_map;
    for (char ch : content) {
        freq_map[ch]++;
    }
    map<char, Range> symbol_ranges;
    long double current_pos = 0.0;
    long double total_chars = content.length();

    for (auto const& [key, count] : freq_map) {
        long double probability = (long double)count / total_chars;
        symbol_ranges[key] = {current_pos, current_pos + probability};
        current_pos += probability;
    }
    long double low = 0.0;
    long double high = 1.0;

    for (char ch : content) {
        long double range = high - low;
        long double new_high = low + range * symbol_ranges[ch].second;
        long double new_low  = low + range * symbol_ranges[ch].first;

        high = new_high;
        low = new_low;
    }

    cout << low << " " << high << endl;

    return 0;
}