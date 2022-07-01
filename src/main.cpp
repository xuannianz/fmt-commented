#include <iostream>
#include <chrono>
#include <string>
#include <type_traits>
#include "format.h"

using namespace std;
using namespace std::literals::chrono_literals;

namespace test_readme{
    void test(){
        // string s = fmt::Format("the answer is {}.", 42);
        // 十六进制输出, 并且开头加上 0x, 0x2a
        // string s = fmt::Format("the answer is {:#x}.", 42);
        // 0b101010
        // string s = fmt::Format("the answer is {:#b}.", 42);
        // 052
        // string s = fmt::Format("the answer is {:#o}.", 42);
        // +42
        // string s = fmt::Format("the answer is {:+}.", 42);
        // 00042
        // string s = fmt::Format("the answer is {:05}.", 42);
        // ___42.
        // string s = fmt::Format("the answer is {:>5}.", 42);
        // _42__.
        // string s = fmt::Format("the answer is {:^5}.", 42);
        // 42___.
        // string s = fmt::Format("the answer is {:<5}.", 42);
        string s = fmt::Format("the answer is {}.", 42.3);
        cout << s << endl;
        // string s2 = fmt::Format("I'd rather be {1} than {0}.", "right", "happy");
        // cout << s2 << endl;
        // fmt::Print("default format: {} {}\n") << 42s << 100ms;
    }
}
int main(){
    // char c = '我';
    // wchar_t x = L'璇';
    // unsigned char *p = (unsigned char *)&x;
    // for (int i = 0; i < sizeof(x); ++i){
    //     printf("%02x ", p[i]);
    // }
    // printf("\n");
    // cout << sizeof(char) << endl;
    // cout << sizeof(wchar_t) << endl;
    test_readme::test();
    // cout << char_traits<char>::length("hi") << endl;
}