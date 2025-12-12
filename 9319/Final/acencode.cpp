#include<iostream>
#include<fstream>
#include<map>
#include<cstdio>
using namespace std;

int main(int argc, char* argv[]) {
    ifstream infile(argv[1]);
    char c;
    map<char, int> cnt;
    int n = 0;
    string s;
    while(infile.get(c)) {
        s += c;
        cnt[c]++;
        n ++;
    }
    map<char, pair<long double, long double>> probability;
    long double subend = 0;
    for(auto& ch : cnt) {
        probability[ch.first].first = subend;
        probability[ch.first].second = subend + ch.second / (long double)n;
        printf("%.10Lf %.10Lf\n", probability[ch.first].first, probability[ch.first].second);
        subend = probability[ch.first].second;
    }

    long double low = 0, high = 1;
    long double range = high - low;
    for(int i = 0; i < s.length(); i ++) {
        high = low + range * probability[s[i]].second;
        low = low + range * probability[s[i]].first;
        range = high - low;
        printf("%.10Lf %.10Lf\n", low, high);
    }
    printf("%.10Lf %.10Lf\n", low, high);
    return 0;
}