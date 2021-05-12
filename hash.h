#pragma once
#include "rocksdb/slice.h"
#include "xxhash.h"

inline uint64_t Hash64(const char* data, size_t n, uint64_t seed=0) {
    return XXH64(data, n, seed);
}

// Useful for splitting up a 64-bit hash
inline uint32_t Upper32of64(uint64_t v) {
  return static_cast<uint32_t>(v >> 32);
}
inline uint32_t Lower32of64(uint64_t v) { return static_cast<uint32_t>(v); }

inline uint64_t GetSliceHash64(const rocksdb::Slice& key) {
  return Hash64(key.data(), key.size());
}

