// math.hpp
#pragma once
#include <cstddef>
#include <cstdlib>

// Global pointer for our hidden pool (inline variables require C++17 or later)
inline std::byte global_pool[1024 * 1024]; 
inline size_t pool_offset = 0;

// MUST use 'inline' for standalone functions in headers!
inline void* custom_malloc(size_t size) {
    if (pool_offset + size > 1024 * 1024) return nullptr;
    
    void* ptr = &global_pool[pool_offset];
    pool_offset += size;
    return ptr;
}

inline void custom_free(void* ptr) {
    // Basic arena free (no-op)
}