/*
   kmp.cpp
   used for generating the output file for bwtsearch
 */

 #include <iostream>
 #include <string>
 #include <vector>
 #include <fstream>    
 #include <sstream>    
 

 std::vector<int> computeLPSArray(const std::string& pattern) {
     int m = pattern.length();
     std::vector<int> lps(m, 0); 
     int length = 0;
     int i = 1;
 
     while (i < m) {
         if (pattern[i] == pattern[length]) {
             length++;
             lps[i] = length;
             i++;
         } else {
             if (length != 0) {
                 length = lps[length - 1];
             } else {
                 lps[i] = 0;
                 i++;
             }
         }
     }
     return lps;
 }
 
 int KMPSearch(const std::string& text, const std::string& pattern) {
     int n = text.length();
     int m = pattern.length();
     
     if (m == 0) return 0; 
 

     std::vector<int> lps = computeLPSArray(pattern);
 
     int count = 0;
     int i = 0; 
     int j = 0; 
 
     while (i < n) {
         if (pattern[j] == text[i]) {
             i++;
             j++;
         }
 
         if (j == m) {
             count++;
             j = lps[j - 1];
         } else if (i < n && pattern[j] != text[i]) {
             if (j != 0) {
                 j = lps[j - 1];
             } else {
                 i = i + 1;
             }
         }
     }
     return count;
 }
 

 int main(int argc, char* argv[]) {

     std::ios_base::sync_with_stdio(false);
     std::cin.tie(NULL);
     std::cout.tie(NULL);
 
     // 1. 检查参数
     if (argc != 2) {
         std::cerr << "Usage: " << argv[0] << " <original_text_file.txt>" << std::endl;
         return 1;
     }
 
     std::ifstream file_stream(argv[1]);
     if (!file_stream) {
         std::cerr << "Error: Cannot open text file " << argv[1] << std::endl;
         return 1;
     }
     
     std::stringstream text_buffer;
     text_buffer << file_stream.rdbuf();
     std::string text = text_buffer.str(); 
     file_stream.close();
    
     std::string pattern;
     while (std::cin >> pattern) {
         

         int count = KMPSearch(text, pattern);
         std::cout << count << "\n";
     }
 
     return 0;
 }