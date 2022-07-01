# https://zhuanlan.zhihu.com/p/399335152
cmake_minimum_required(VERSION 3.12)
set (CMAKE_CXX_STANDARD 14)

project(fmt-commented)
set(fmt_dir fmt-0.8.0)
file(GLOB fmt_src ${fmt_dir}/*)
message(fmt_src=${fmt_src})
add_executable(fmt_test src/main.cpp ${fmt_src})
target_include_directories(fmt_test PUBLIC ${fmt_dir})