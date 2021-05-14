#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <filesystem>
#include <cassert>

#include "fmt/format.h"
#include "pi.h"
#include "util.h"

using namespace std;
using namespace rocksdb;
using fmt::format;

namespace fs = std::filesystem;


DBOptions Pi::GetDefaultDBOptions() {
    DBOptions options;
    options.create_if_missing = true;
    options.create_missing_column_families = true;
    return options;
}

ColumnFamilyOptions Pi::GetDefaultCFOptions() {
    ColumnFamilyOptions options;
    return options;
}

Pi::Pi(string path, PiOptions opt) {
    if(opt.remove_existing_db && fs::exists(path)) {
        cout << format("removing old database in {} ...\n", fs::path(path).generic_string());
        fs::remove_all(path);
    }

    vector<ColumnFamilyDescriptor> cf_descriptors;
    vector<ColumnFamilyHandle*> cf_handles;
    if(opt.options_file != "") {
        DBOptions options;
        Status s;
        s = LoadOptionsFromFile(ConfigOptions(), opt.options_file, &options, &cf_descriptors);
        if(!s.ok()) {
            throw Exception(s.ToString());
        }
        s = DB::Open(options, path, cf_descriptors, &cf_handles, &db_);
        if(!s.ok()) {
            throw Exception(s.ToString());
        }

    } else {
        vector<ColumnFamilyDescriptor> cf_descriptors;
        cf_descriptors.push_back(
            ColumnFamilyDescriptor(kPrimaryColumnFamilyName, GetDefaultCFOptions()));
        cf_descriptors.push_back(
            ColumnFamilyDescriptor(kIndexColumnFamilyName, GetDefaultCFOptions()));

        Status s;
        s = DB::Open(GetDefaultDBOptions(), path, cf_descriptors, &cf_handles, &db_);
        if(!s.ok()) {
            throw Exception(s.ToString());
        }
    }

    table_ = nullptr;
    index_ = nullptr;
    for(auto cf: cf_handles) {
        if (cf->GetName() == kPrimaryColumnFamilyName) {
            if(table_ != nullptr) delete table_;
            table_ = cf;
        } else if(cf->GetName() == kIndexColumnFamilyName) {
            if(index_ != nullptr) delete index_;
            index_ = cf;
        } else {
            delete cf;
        }
    }
    if(index_ == nullptr) {
        throw Exception(
            format("Can not find column family '{}'\n", kIndexColumnFamilyName));
    }

    parser_ = opt.parser;
    if(parser_ == nullptr) {
        throw Exception("no parser");
    }
    sketch_ = new CountMinSketch(opt.sketch_height, opt.sketch_width);
    opt_ = opt;
}

Pi::~Pi() {
    delete parser_;
    delete table_;
    delete index_;
    db_->Close();
    delete db_;
}

Status Pi::Put(const Slice& key, const Slice& val) {
    Status s;
    s = db_->Put(WriteOptions(), table_, key, val);
    if(!s.ok()) return s;
    s = db_->Put(WriteOptions(), index_, EncodeIndexKey(key, val), "");
    return s;
}

Status Pi::Get(const Slice& key, string *val) {
    Status s = db_->Get(ReadOptions(), key, val);
    return s;
}

Status Pi::Get2(const Slice& sk, vector<string> *vals) {
    sketch_->Add(sk);
    uint64_t read_count = sketch_->Estimate(sk);

    Iterator *it = db_->NewIterator(ReadOptions(), index_);
    string val;

    it->Seek(sk);
    bool fast_read = it->Valid() && it->key() == sk;

    if(fast_read) {
        // Secondary index has value
        for(it->Next(); it->Valid() && it->key().starts_with(sk); it->Next()) {
            vals->push_back(it->value().ToString());
        }
    } else {
        WriteBatch wb;
        int val_count = 0;
        for (; it->Valid() && it->key().starts_with(sk); it->Next()) {
            Slice pk = it->key();
            pk.remove_prefix(sk.size());
            db_->Get(ReadOptions(), pk, &val);
            vals->push_back(val);
            val_count ++;
            wb.Put(index_, it->key(), val);
        }
        if(read_count * val_count > opt_.index_threshold) {
            //cout << format("create duplication for {}", sk.ToString()) << endl; 
            stat_.sec_index_count ++;
            stat_.sec_item_count += wb.Count();
            wb.Put(index_, sk, "");
            db_->Write(WriteOptions(), &wb);
        }
    }
    return it->status();
}

PiStat Pi::stat() const{
    return stat_;
}



