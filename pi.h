#pragma once

#include "rocksdb/db.h"
#include "rocksdb/utilities/options_util.h"
#include "pi/sketch.h"

class Parser {
public:
    // Get secondary key. 
    virtual rocksdb::Slice GetSK(const rocksdb::Slice &val) {
        return rocksdb::Slice(val.data(), 12);
    }
    virtual ~Parser() {}
};

struct PiOptions {
    std::string options_file = "";
    size_t sketch_width = 60000;
    size_t sketch_height = 3;
    size_t index_threshold = 10;
    bool remove_existing_db = false;
};

class Pi{
public:
    const char *kPrimaryColumnFamilyName = "default";
    const char *kIndexColumnFamilyName = "index";

    static rocksdb::DBOptions GetDefaultDBOptions();
    static rocksdb::ColumnFamilyOptions GetDefaultCFOptions();

    Pi(std::string path, PiOptions opt=PiOptions{});

    ~Pi();

    rocksdb::Status Put(const rocksdb::Slice& key, const rocksdb::Slice& val);

    rocksdb::Status Get(const rocksdb::Slice& key, std::string *val);

    rocksdb::Status Get2(const rocksdb::Slice& sk, std::vector<std::string> *vals);

private:
    rocksdb::DB *db_;
    rocksdb::ColumnFamilyHandle *table_;
    rocksdb::ColumnFamilyHandle *index_;
    Parser *parser_;
    CountMinSketch *sketch_;
    PiOptions opt_;


    std::string EncodeIndexKey(const rocksdb::Slice& key, const rocksdb::Slice& val) {
        return parser_->GetSK(val).ToString() + key.ToString();
    }
};