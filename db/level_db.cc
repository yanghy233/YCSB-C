//
// Created by yanghy on 9/24/24.
//

#include "level_db.h"
#include <iostream>
#include <cstdlib>

ycsbc::LevelDB::LevelDB() {
    const std::string db_path = "./ycsb-leveldb";
    options_.create_if_missing = true;
    leveldb::Status s = leveldb::DB::Open(options_, db_path, &db_);
    assert(s.ok());
    std::cout << "[YCSB] leveldb opened." << std::endl;
}

int ycsbc::LevelDB::Read(const std::string &table,
                         const std::string &key,
                         const std::vector<std::string> *fields,
                         std::vector<KVPair> &result) {
    std::string values;
    leveldb::ReadOptions readOptions;
    leveldb::Status s = db_->Get(readOptions, key, &values);
    if (!s.ok()) {
        return DB::kErrorNoData;
    }
    deserializeValues(values, fields, result);
    return DB::kOK;
}

int ycsbc::LevelDB::Scan(const std::string &table,
                         const std::string &key,
                         int record_count,
                         const std::vector<std::string> *fields,
                         std::vector<std::vector<KVPair>> &result) {
    leveldb::Iterator *it = db_->NewIterator(leveldb::ReadOptions());
    int cnt = 0;
    for (it->Seek(key); it->Valid() && cnt < record_count; it->Next()) {
        std::vector<KVPair> tmp;
        std::string values = it->value().data();
        deserializeValues(values, fields, tmp);
        result.push_back(tmp);
        ++cnt;
    }
    int ret = DB::kOK;
    if (it->status().ok()) {
        ret = DB::kOK;
    } else {
        ret = DB::kErrorConflict;
    }
    delete it;
    return ret;
}

int ycsbc::LevelDB::Update(const std::string &table, const std::string &key, std::vector<KVPair> &values) {
    // find or not
    std::string data;
    leveldb::Status s = db_->Get(leveldb::ReadOptions(), key, &data);
    if (!s.ok()) {
        return DB::kErrorNoData;
    }
    // update
    data.clear();
    data = serializeValues(values);
    db_->Put(leveldb::WriteOptions(), key, data);
    return DB::kOK;
}

int ycsbc::LevelDB::Insert(const std::string &table, const std::string &key, std::vector<KVPair> &values) {
    std::string data = serializeValues(values);
    leveldb::WriteOptions wopt;
    leveldb::Status s = db_->Put(wopt, key, data);
    assert(s.ok());
    return DB::kOK;
}

int ycsbc::LevelDB::Delete(const std::string &table, const std::string &key) {
    leveldb::Status s = db_->Delete(leveldb::WriteOptions(), key);
    return DB::kOK;
}

ycsbc::LevelDB::~LevelDB() {
    delete db_;
}

void ycsbc::LevelDB::deserializeValues(const std::string &values, const std::vector<std::string> *fields,
                                       std::vector<KVPair> &result) {
    for (auto pos = values.find(' '); pos != std::string::npos;) {
        auto next = values.find(' ', pos + 1);
        if (next == std::string::npos)break;
        std::string tmp = values.substr(pos + 1, next - pos - 1);
        bool flag = fields ? false : true;
        for (unsigned i = 0; !flag && i < fields->size(); ++i)
            if (tmp == fields->at(i))
                flag = true;
        pos = values.find(' ', next + 1);
        if (flag)
            result.emplace_back(tmp, values.substr(next + 1, pos - next - 1));
    }
}

std::string ycsbc::LevelDB::serializeValues(const std::vector<KVPair> &values) {
    std::string ret;
    for (const auto & value : values)
        ret += " " + value.first + " " + value.second;
    return ret;
}
