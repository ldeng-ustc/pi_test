#pragma once

#include "rocksdb/db.h"
#include "rocksdb/utilities/options_util.h"
#include "pi/sketch.h"

class Parser {
public:
    // Get secondary key. 
    Parser(size_t sk_len): sk_len_(sk_len) {}
    virtual rocksdb::Slice GetSK(const rocksdb::Slice &val) {
        return rocksdb::Slice(val.data(), sk_len_);
    }
    virtual ~Parser() {}
private:
    size_t sk_len_;
};

struct PiOptions {
    Parser *parser = nullptr;
    std::string options_file = "";
    size_t sketch_width = 60000;
    size_t sketch_height = 3;
    size_t index_threshold = 10;
    bool remove_existing_db = false;
};

struct PiStat {
    size_t sec_index_count = 0;
    size_t sec_item_count = 0;
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

    PiStat stat() const;

private:
    rocksdb::DB *db_;
    rocksdb::ColumnFamilyHandle *table_;
    rocksdb::ColumnFamilyHandle *index_;
    Parser *parser_;
    CountMinSketch *sketch_;
    PiOptions opt_;
    PiStat stat_;



    std::string EncodeIndexKey(const rocksdb::Slice& key, const rocksdb::Slice& val) {
        return parser_->GetSK(val).ToString() + key.ToString();
    }
};