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

    if (content.empty()) return 0;

    int total_len = content.length();

    map<char, int> freq_map;
    for (char ch : content) {
        freq_map[ch]++;
    }

    map<char, Range> symbol_ranges;
    long double current_pos = 0.0;
    long double total_chars_ld = (long double)(total_len);

    // 这一步必须保证顺序是 lexicographical (map 默认就是)
    for (auto const& [key, count] : freq_map) {
        long double probability = count / total_chars_ld;
        symbol_ranges[key] = {current_pos, current_pos + probability};
        current_pos += probability;
    }

    // 3. 开始解码
    // 从命令行读取编码后的数值 (使用 stold 保证 long double 精度)
    long double code_val = std::stold(argv[2]);
    
    std::string decoded_output = "";

    // 循环次数等于原始字符串长度
    for (int i = 0; i < total_len; ++i) {
        // 遍历所有字符区间，看当前数值落在哪个区间里
        for (auto const& [key, range] : symbol_ranges) {
            // 判断条件： range.first <= code_val < range.second
            if (code_val >= range.first && code_val < range.second) {
                // 找到字符了！
                decoded_output += key;
                
                // 核心解码公式：去除偏移量，除以区间宽度（归一化）
                // NewCode = (OldCode - Low) / (High - Low)
                long double width = range.second - range.first;
                code_val = (code_val - range.first) / width;
                
                // 既然找到了，就跳出内层循环，进行下一个字符的解码
                break;
            }
        }
    }

    std::cout << "Decoded String: " << decoded_output << std::endl;

    return 0;
}