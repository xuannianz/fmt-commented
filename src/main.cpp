#include <iostream>
#include <chrono>
#include <string>
#include <type_traits>
#include "format.h"

using namespace std;
using namespace std::literals::chrono_literals;

namespace test_readme{
    void test(){
        string s = fmt::Format("the answer is {}.", 42);
        cout << s << endl;
        string s2 = fmt::Format("I'd rather be {1} than {0}.", "right", "happy");
        cout << s2 << endl;
        // fmt::Print("default format: {} {}\n") << 42s << 100ms;
    }
}
int main(){
    // test_readme::test();
    cout << char_traits<char>::length("hi") << endl;
}