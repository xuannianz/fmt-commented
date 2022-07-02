#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <type_traits>
#include "format.h"

using namespace std;
using namespace std::literals::chrono_literals;

namespace test_format{
    void test_format_int(){
        // 十六进制输出, 并且开头加上 0x, 0x2a
        // string s = fmt::Format("{:#x}.", 42);
        // 0b101010
        // string s = fmt::Format("{:#b}.", 42);
        // 052
        // string s = fmt::Format("{:#o}.", 42);
        // +42
        // string s = fmt::Format("{:+}.", 42);
        // 00042
        // string s = fmt::Format("{:05}.", 42);
        // ___42.
        // string s = fmt::Format("{:>5}.", 42);
        // _42__.
        // string s = fmt::Format("{:^5}.", 42);
        // 42___.
        // string s = fmt::Format("{:<5}.", 42);
    }
    void test_format_double(){
        // string s = fmt::Format("AB{}BA", 42.3);
        // 下面这个用法有错, 有了 fill, 就必须有 align
        // string s = fmt::Format("AB{:$8g}BA", -42.3);
        // string s = fmt::Format("AB{:$<8g}BA", -42.3);
        // string s = fmt::Format("AB{:$>8g}BA", -42.3);
        string s = fmt::Format("AB{:$=8g}BA", -42.3);
        cout << s << endl;
    }
    void test_format_readme(){
        string s = fmt::Format("the answer is {}.", 42);
        cout << s << endl;
        string s2 = fmt::Format("I'd rather be {1} than {0}.", "right", "happy");
        cout << s2 << endl;
        // fmt::Print("default format: {} {}\n") << 42s << 100ms;
    }
    void test(){
        test_format_double();
    }
}
int main(){
    test_format::test();
    // char c = '我';
    // wchar_t x = L'璇';
    // unsigned char *p = (unsigned char *)&x;
    // for (int i = 0; i < sizeof(x); ++i){
    //     printf("%02x ", p[i]);
    // }
    // printf("\n");
    // cout << sizeof(char) << endl;
    // cout << sizeof(wchar_t) << endl;
    // test_readme::test();
    // cout << char_traits<char>::length("hi") << endl;
    // cout << std::signbit(-3.2) << endl;
    // cout << std::signbit(3.2) << endl;
    // cout << std::signbit(-0.0) << endl;
    // cout << std::signbit(0.0) << endl;
    // double a = 3.2;
    // printf("haha%#.3ghaha", a);
    // char arr[3] = {0};
    // size_t n = snprintf(arr, 3, "hello");
    // cout << n << endl;
    // cout << arr[0] << endl;
    // cout << arr[1] << endl;
}