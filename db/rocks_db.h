//
//  rocks_db.h
//  YCSB-C
//
//  Created by Junkai Liang on 9/12/19.
//

#ifndef YCSB_C_ROCKS_DB_H_
#define YCSB_C_ROCKS_DB_H_

#include "core/db.h"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"
#include "rocksdb/iterator.h"
#include "rocksdb/rate_limiter.h"
#include <unordered_map>
#include <string>

namespace ycsbc {

class RocksDB : public DB {
 public:
  RocksDB();
  int Read(const std::string &table, const std::string &key,
           const std::vector<std::string> *fields,
           std::vector<KVPair> &result);
  int Scan(const std::string &table, const std::string &key,
           int len, const std::vector<std::string> *fields,
           std::vector<std::vector<KVPair>> &result);
  int Update(const std::string &table, const std::string &key,
             std::vector<KVPair> &values);
  int Insert(const std::string &table, const std::string &key,
             std::vector<KVPair> &values);
  int Delete(const std::string &table, const std::string &key);
  ~RocksDB();
 private:
  rocksdb::DB* db;
  std::unordered_map<std::string,rocksdb::ColumnFamilyHandle*> handles;
  void deserializeValues(const std::string &values,const std::vector<std::string> *fields,std::vector<KVPair> &result);
  void updateValues(std::vector<KVPair> &result ,const std::vector<KVPair> &values);
  std::string serializeValues(const std::vector<KVPair> &values);
};

} // ycsbc

#endif // YCSB_C_ROCKS_DB_H_
