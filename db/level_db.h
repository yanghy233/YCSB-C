//
// Created by yanghy on 9/24/24.
//

#ifndef YCSB_C_LEVEL_DB_H
#define YCSB_C_LEVEL_DB_H

#include <leveldb/db.h>
#include <leveldb/options.h>
#include <leveldb/status.h>
#include <leveldb/cache.h>
#include <leveldb/filter_policy.h>

#include <string>

#include "core/db.h"

namespace ycsbc {

    class LevelDB : public DB {
    public:
        LevelDB();

        void Begin(int code) override {}

        int Read(const std::string &table, const std::string &key,
                 const std::vector<std::string> *fields,
                 std::vector<KVPair> &result) override;

        int Scan(const std::string &table, const std::string &key,
                 int len, const std::vector<std::string> *fields,
                 std::vector<std::vector<KVPair>> &result) override;

        int Update(const std::string &table, const std::string &key,
                   std::vector<KVPair> &values) override;

        int Insert(const std::string &table, const std::string &key,
                   std::vector<KVPair> &values) override;

        int Delete(const std::string &table, const std::string &key) override;

        ~LevelDB() override;

    private:
        leveldb::DB *db_{};
        leveldb::Options options_;

        static void deserializeValues(const std::string &values, const std::vector<std::string> *fields,
                               std::vector<KVPair> &result);

        //void updateValues(std::vector<KVPair> &result ,const std::vector<KVPair> &values);
        static std::string serializeValues(const std::vector<KVPair> &values);
    };

} // ycsbc


#endif //YCSB_C_LEVEL_DB_H
