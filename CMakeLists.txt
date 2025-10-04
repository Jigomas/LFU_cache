cmake_minimum_required(VERSION 3.15)
project(CacheComparison VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(cache_comparison
    src/lfu_cache.cpp
)

target_include_directories(cache_comparison 
    PRIVATE 
        include
)

# Флаги компилятора
target_compile_options(cache_comparison
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

# Настройки для разных типов сборки
target_compile_definitions(cache_comparison
    PRIVATE
        $<$<CONFIG:Debug>:DEBUG>
)

target_compile_options(cache_comparison
    PRIVATE
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O2>
)