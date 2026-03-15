cmake_minimum_required(VERSION 3.30)
project(shoals)

add_executable(main main.cpp)
target_compile_features(main PRIVATE cxx_std_23)