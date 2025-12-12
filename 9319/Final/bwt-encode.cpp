#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

// BWT 编码函数
string bwt_encode(const string& text) {
    int n = text.length();
    
    // 1. 创建索引数组 [0, 1, ..., n-1]
    // 这比存储 n 个长字符串节省大量内存
    vector<int> indices(n);
    for (int i = 0; i < n; ++i) {
        indices[i] = i;
    }

    // 2. 根据索引对应的循环移位字符串进行排序
    // 使用 lambda 表达式定义比较规则
    sort(indices.begin(), indices.end(), [&](int i, int j) {
        // 比较从索引 i 开始的字符串 和 从索引 j 开始的字符串
        // 由于是循环移位，需要取模处理
        for (int k = 0; k < n; ++k) {
            char c1 = text[(i + k) % n];
            char c2 = text[(j + k) % n];
            if (c1 != c2) {
                return c1 < c2;
            }
        }
        return false; // 两个字符串完全相等（理论上加了唯一终止符后不会发生）
    });

    // 3. 构建 BWT 字符串（取矩阵的最后一列）
    // 矩阵每一行对应一个旋转，其最后一列字符就是该旋转起始字符的前一个字符
    string bwt_result;
    bwt_result.resize(n);
    
    for (int i = 0; i < n; ++i) {
        // 当前行的起始位置是 indices[i]
        // 最后一列的字符位置就是 (indices[i] - 1)
        // 注意处理 indices[i] == 0 的情况，通过 +n 再模 n 解决
        int last_char_idx = (indices[i] - 1 + n) % n;
        bwt_result[i] = text[last_char_idx];
    }

    return bwt_result;
}

int main(int argc, char* argv[]) {
    // 0. 参数检查
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <input_file> <output_file>" << endl;
        return 1;
    }

    // 1. 读取输入文件
    ifstream infile(argv[1], ios::binary); // 使用 binary 模式防止换行符转换问题
    if (!infile) {
        cerr << "Error: Cannot open input file " << argv[1] << endl;
        return 1;
    }

    // 一次性读取整个文件内容
    string content((istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());
    infile.close();

    // 检查是否包含终止符冲突
    if (content.find('$') != string::npos) {
        cerr << "Warning: Input contains '$'. This might affect decoding if '$' is used as sentinel." << endl;
        // 在实际工程中，通常使用 ASCII 0 或专门的逻辑处理 Sentinels
    }

    // 2. 添加终止符
    // 这一步对于能够逆转换（Decode）是必须的
    content += '$';

    // 3. 执行编码
    string encoded_text = bwt_encode(content);

    // 4. 写入输出文件
    ofstream outfile(argv[2], ios::binary);
    if (!outfile) {
        cerr << "Error: Cannot open output file " << argv[2] << endl;
        return 1;
    }

    outfile.write(encoded_text.c_str(), encoded_text.size());
    outfile.close();

    cout << "BWT Encoding completed." << endl;
    cout << "Original length (with $): " << content.size() << endl;
    cout << "Output saved to: " << argv[2] << endl;

    return 0;
}