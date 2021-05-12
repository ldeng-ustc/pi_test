#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

#include "pi/hash.h"
#include "rocksdb/slice.h"

class CountMinSketch {
public:
    CountMinSketch(size_t row, size_t col);

    void Add(rocksdb::Slice key, int val=1);

    uint64_t Estimate(rocksdb::Slice key);

    void Clear();

private:
    std::vector<uint64_t> data_;
    uint64_t row_;
    uint64_t col_;
};
