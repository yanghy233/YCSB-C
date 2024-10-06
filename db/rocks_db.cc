//
//  rocks_db.cc
//  YCSB-C
//

#include "rocks_db.h"
#include <iostream>

using namespace rocksdb;

namespace ycsbc {
// static std::vector<ColumnFamilyDescriptor> column_families;

/**
 * open and create a new rocksdb instance
 * create a new column family named "usertable"
 */
    RocksDB::RocksDB() {
        // options_.rate_limiter.reset(NewGenericRateLimiter(300<<20,100*1000,10,RateLimiter::Mode::kWritesOnly,true));

        std::string ram_path = "./ramdisk_path";           // for ram
        std::string disk_path = "./rocksdb_disk_path";     // for disk

        // options_.max_write_buffer_number = 24;
        //options_.min_write_buffer_number_to_merge = 4;
        // options_.level0_file_num_compaction_trigger = 1;

        // db_paths: control the storage position of SST files
        // TODO @yhy db_paths.size() should >= 2, also db_paths can emplace back the same path (eg. disk_path push twice)
        options_.db_paths.emplace_back(ram_path, 2ull << 30);   // 2GB
        options_.db_paths.emplace_back(disk_path, 1ull << 40);  // 1TB

        // debug
//        options_.max_bytes_for_level_base = 1024 * 10; // 1KB
//        options_.max_bytes_for_level_multiplier = 3;
        // L0 无穷大
        // options_.level0_file_num_compaction_trigger = INT32_MAX;
            
        options_.create_if_missing = true;
        Status s = rocksdb::DB::Open(options_, disk_path, &db_);
        assert(s.ok());

        s = db_->CreateColumnFamily(ColumnFamilyOptions(options_), "usertable", &cf_);
        assert(s.ok());
    }

    RocksDB::~RocksDB() {
        delete cf_;
        delete db_;
    }

    void RocksDB::Begin(int code) {
        // TODO @yhy add more db_type
        if (DbType() == "cruisedb") {
            Status s = db_->TbBegin(cf_, code);
            assert(s.ok());
        }
    }

    int RocksDB::Read(const std::string &table, const std::string &key, const std::vector<std::string> *fields,
                      std::vector<KVPair> &result) {
        std::string values;
        Status s = db_->Get(ReadOptions(), cf_, Slice(key), &values);
        if (!s.ok())return s.code();
        deserializeValues(values, fields, result);
        return 0;
    }

    int RocksDB::Scan(const std::string &table, const std::string &key, int record_count,
                      const std::vector<std::string> *fields, std::vector<std::vector<KVPair>> &result) {
        Iterator *it = db_->NewIterator(ReadOptions(), cf_);
        int cnt = 0;
        for (it->Seek(key); it->Valid() && cnt < record_count; it->Next()) {
            std::vector<KVPair> tmp;
            std::string values = it->value().data();
            deserializeValues(values, fields, tmp);
            result.push_back(tmp);
            ++cnt;
        }
        int ret = it->status().code();
        delete it;
        return ret;
    }

    int RocksDB::Update(const std::string &table, const std::string &key, std::vector<KVPair> &values) {
        // Read the existing value
        std::string v1;
        Status s = db_->Get(ReadOptions(), cf_, Slice(key), &v1);
        if (!s.ok()) {
            return s.code(); // Return error code if the key does not exist
        }

        // Deserialize the existing value
        std::vector<KVPair> existing_values;
        deserializeValues(v1, nullptr, existing_values);

        // Update the existing values with the new values
        for (const auto &new_value : values) {
            bool found = false;
            for (auto &existing_value : existing_values) {
                if (existing_value.first == new_value.first) {
                    existing_value.second = new_value.second;
                    found = true;
                    break;
                }
            }
            if (!found) {
                existing_values.push_back(new_value);
            }
        }

        // Serialize the updated values
        std::string updated_value = serializeValues(existing_values);

        // Write the updated value back to the database
        s = db_->Put(WriteOptions(), cf_, Slice(key), Slice(updated_value));
        return s.code();
    }

    int RocksDB::Insert(const std::string &table, const std::string &key, std::vector<KVPair> &values) {
        std::string sval = serializeValues(values);
        Status s = db_->Put(WriteOptions(), cf_, Slice(key), Slice(sval));
        return s.code();
    }

    int RocksDB::Delete(const std::string &table, const std::string &key) {
        Status s = db_->Delete(WriteOptions(), cf_, Slice(key));
        return s.code();
    }

    void RocksDB::deserializeValues(const std::string &values, const std::vector<std::string> *fields,
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


    std::string RocksDB::serializeValues(const std::vector<KVPair> &values) {
        std::string ret;
        for (unsigned i = 0; i < values.size(); ++i)
            ret += " " + values[i].first + " " + values[i].second;
        return ret;
    }

    void RocksDB::End() {
        // Debug 获取每层文件数量
        for (int i = 0; i < 7; ++i) {
            std::string property;
            bool status = db_->GetProperty(cf_, "rocksdb.num-files-at-level" + std::to_string(i), &property);
            if (status) {
                std::cout << "Level " << i << " file count: " << property << std::endl;
            } else {
                std::cerr << "Error getting property for level " << i << std::endl;
            }
        }
    }
} // namespace ycsbc
