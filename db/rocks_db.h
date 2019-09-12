//
//  rocks_db.h
//  YCSB-C
//
//  Created by Junkai Liang on 9/12/19.
//

#ifndef YCSB_C_ROCKS_DB_H_
#define YCSB_C_ROCKS_DB_H_

#include "core/db.h"

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
};

} // ycsbc

#endif // YCSB_C_ROCKS_DB_H_
