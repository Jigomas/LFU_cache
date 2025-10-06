#include <iostream>

#include "../include/ideal_cache.hpp"



size_t RunIdeal() ;



int main() {
    size_t hit_count    = 0;

    //std::cout << "\n=== Running Ideal Cache Test ===\n";
    hit_count = RunIdeal();
    std::cout << ">>" << hit_count << std::endl;

    return 0;
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
        //cache.DumpCache();
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