#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "../include/lfu_cache.hpp"
#include "../include/ideal_cache.hpp"



enum class CacheRunning {
    ERROR_NUM = 0,
    STANDARD_CACHE_TEST = 1,
    IDEAL_CACHE_TEST = 2,
    COMPARE_CACHES = 3
};



size_t  RunLfu();
size_t  RunIdeal();
void    RunComparison(int &total_hits_lfu, int &total_hits_ideal);



int main() {
    int total_hits_lfu        = 0; 
    int total_hits_ideal      = 0;
    int input           = 0;
    size_t hit_count    = 0;

    std::cout << "Print number: 1, 2 or 3; \n"
              << "1 - run LFU cache test; \n"
              << "2 - run ideal cache test; \n"
              << "3 - compare both caches with test file" << std::endl;

    if (!(std::cin >> input)) {
        std::cout << "Invalid input! Please enter a number." << std::endl;
        return 1;
    }

    CacheRunning user_choice = static_cast<CacheRunning>(input);

    switch(user_choice) {
        case CacheRunning::STANDARD_CACHE_TEST: {
            std::cout << "\n=== Running Standard LFU Test ===\n";
            hit_count = RunLfu();
            std::cout << "Test completed. Hits: " << hit_count << std::endl;
            break;
        }
        case CacheRunning::IDEAL_CACHE_TEST: {
            std::cout << "\n=== Running Ideal Cache Test ===\n";
            hit_count = RunIdeal();
            std::cout << "Test completed. Hits: " << hit_count << std::endl;
            break;
        }
        case CacheRunning::COMPARE_CACHES: {
            std::cout << "\n=== Running Cache Comparison ===\n";
            RunComparison(total_hits_lfu, total_hits_ideal);

            std::cout << "Total hits LFU: " << total_hits_lfu << std::endl;
            std::cout << "Total hits ideal: " << total_hits_ideal << std::endl;
            std::cout << "---" << std::endl;
            break;
        }
        default: {
            std::cout << "\n=== Invalid choice: " << input << " ===\n"
                      << "Please enter 1, 2 or 3." << std::endl;
            return 1;
        }
    }
    
    return 0;
}



size_t RunLfu() 
{
    size_t cache_size = 0;
    size_t element_count = 0;
    size_t hit_count = 0;

    if (!(std::cin >> cache_size >> element_count)) {
        std::cout << "Error reading cache parameters " << std::endl;
        return 0;
    }

    LfuCache<int, int> cache(cache_size);

    for (size_t i = 0; i < element_count; ++i) {
        cache.DumpCache();
        int element = 0;
        if (!(std::cin >> element)) {
            std::cout << "Error reading element " << i + 1 << std::endl;
            break;
        }

        if (cache.Get(element) != nullptr) 
            ++hit_count;
        else 
            cache.Put(element, element);
    }
    return hit_count;
}



size_t RunIdeal() 
{
    size_t cache_size = 0;
    size_t element_count = 0;
    size_t hit_count = 0;

    if (!(std::cin >> cache_size >> element_count)) {
        std::cout << "Error reading cache parameters " << std::endl;
        return 0;
    }

    IdealCache<int, int> cache(cache_size);
    std::vector<int> elements;
    std::unordered_map<int, std::vector<size_t>> access_map;

    for (size_t i = 0; i < element_count; ++i) {
        cache.DumpCache();
        int element = 0;
        if (!(std::cin >> element)) {
            std::cout << "Error reading element " << i + 1 << std::endl;
            break;
        }
        elements.push_back(element);
        access_map[element].push_back(i);
    }

    for (auto& [key, positions] : access_map) {
        cache.LoadAccessPattern(key, positions);
    }

    for (int element : elements) {
        if (cache.Get(element) != nullptr) 
            ++hit_count;
        else 
            cache.Put(element, element);
    }

    return hit_count;
}


void RunComparison(int &total_hits_lfu, int &total_hits_ideal) {


    std::ifstream file("../test/test.txt");
    if (!file.is_open()) {
        std::cout << "Error opening file: test/test.txt" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        std::cout << "Test case: " << line << std::endl;
        
        std::istringstream iss(line);
        size_t cache_size = 0;
        size_t element_count = 0;
        
        if (!(iss >> cache_size >> element_count)) {
            std::cout << "Error reading cache parameters!" << std::endl;
            continue;
        }

        std::vector<int> elements;
        for (size_t i = 0; i < element_count; ++i) {
            int element = 0;
            if (!(iss >> element)) {
                std::cout << "Error reading element " << i + 1 << std::endl;
                break;
            }
            elements.push_back(element);
        }

        // LFU
        LfuCache<int, int> lfu_cache(cache_size);
        size_t lfu_hits = 0;
        for (int element : elements) {
            if (lfu_cache.Get(element) != nullptr) 
                ++lfu_hits;
            else 
                lfu_cache.Put(element, element);
        }

        // Ideal
        IdealCache<int, int> ideal_cache(cache_size);
        std::unordered_map<int, std::vector<size_t>> access_map;
        for (size_t i = 0; i < elements.size(); ++i) {
            access_map[elements[i]].push_back(i);
        }
        for (auto& [key, positions] : access_map) {
            ideal_cache.LoadAccessPattern(key, positions);
        }
        
        size_t ideal_hits = 0;
        for (int element : elements) {
            if (ideal_cache.Get(element) != nullptr) 
                ++ideal_hits;
            else 
                ideal_cache.Put(element, element);
        }

        std::cout << "LFU hits: " << lfu_hits << std::endl;
        std::cout << "Ideal hits: " << ideal_hits << std::endl;
        std::cout << "---" << std::endl;

        total_hits_lfu   += lfu_hits;
        total_hits_ideal += ideal_hits;
    }
    file.close();
}