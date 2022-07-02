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
        // string s = fmt::Format("{:#x}", 42);
        // 0b101010
        // string s = fmt::Format("{:#b}", 42);
        // 052
        // string s = fmt::Format("{:#o}", 42);
        // +42
        // string s = fmt::Format("{:+}", 42);
        // 00042
        // string s = fmt::Format("{:05}", 42);
        // ___42.
        // string s = fmt::Format("{:>5}", 42);
        // _42__.
        // string s = fmt::Format("{:^5}", 42);
        // $+42$
        // string s = fmt::Format("{:$^+5}", 42);
        // +$$42 这种 numeric 填充(=)有点奇葩符号和内容分开了
        string s = fmt::Format("{:$=+5}", 42);
        // 42___.
        // string s = fmt::Format("{:<5}", 42);
        cout << s << endl;
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
    void test_format_char(){
        string s = fmt::Format("AB{:^10c}BA", 'x');
        cout << s << endl;
    }
    void test_format_pointer(){
        int a = 10;
        // 非 char 指针, fmt 要求把指针转成 void *
        string s = fmt::Format("AB{:$^20p}BA", (void *)&a);
        cout << s << endl;
    }
    void test_format_string(){
        string s = fmt::Format("AB{:$^30s}BA", "hello world");
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
        // test_format_char();
        // test_format_double();
        test_format_int();
        // test_format_pointer();
        // test_format_string();
    }
}
namespace test_print{
    void test(){
        // Print() 返回的是一个 Formatter 对象
        // 之后调用 Formatter::operator<<(), 添加 Arg
        // 最后这个临时的 Formatter 对象析构, 调用 Formatter::~Formatter()
        // ~Formatter() 中解析格式化字符串, 并往终端输出
        fmt::Print("default format: {}\n") << 42;
        fmt::PrintColored(fmt::YELLOW, "default format: {}\n") << 42;
    }
}
int main(){
    test_format::test();
    // test_print::test();
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
    // printf("\033[34mhello world\033[0m");
}