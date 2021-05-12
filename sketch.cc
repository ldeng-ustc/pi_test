#include "sketch.h"

CountMinSketch::CountMinSketch(size_t row, size_t col): row_(row), col_(col) {
    data_.resize(row * col);
}

void CountMinSketch::Add(rocksdb::Slice key, int val) {
    uint64_t hash = GetSliceHash64(key);
    uint64_t hash1 = Lower32of64(hash);
    uint64_t hash2 = Upper32of64(hash);
    for(size_t i = 0; i < row_; i++) {
        // Kirsch A., Mitzenmacher M. (2006) Less Hashing, Same Performance: Building a Better Bloom Filter.  
        uint64_t h = (hash1 + hash2 * i + i * i) % col_;
        data_[i*col_ + h] += val;
    }
}

uint64_t CountMinSketch::Estimate(rocksdb::Slice key) {
    uint64_t count = UINT64_MAX;
    uint64_t hash = GetSliceHash64(key);
    uint64_t hash1 = Lower32of64(hash);
    uint64_t hash2 = Upper32of64(hash);
    for(uint64_t i = 0; i < row_; i++) {
        // Kirsch A., Mitzenmacher M. (2006) Less Hashing, Same Performance: Building a Better Bloom Filter. 
        uint64_t h = (hash1 + hash2 * i + i * i) % col_;
        count = std::min(count, data_[i*col_ + h]);
        // std::cout << h << " " << data_[i*col_ + h] << std::endl;
    }
    return count;
}

void CountMinSketch::Clear() {
    std::fill(data_.begin(), data_.end(), 0);
}