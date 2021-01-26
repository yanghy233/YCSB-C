//
//  faster.h
//  YCSB-C
//
//  Created by Junkai Liang on 13/1/21.
//

#ifndef YCSB_C_FASTER_H_
#define YCSB_C_FASTER_H_

#include "core/db.h"
#include "faster_work.h"
#include <string>

namespace ycsbc {

class Faster : public DB {
 public:
  Faster();
  void Init(int thread_idx);
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
  void Close();
  ~Faster();
 private:
 	static constexpr uint64_t kInitCount = 250000000;
	store_t * store;
    static void SetThreadAffinity(size_t core);
	void deserializeValues(const std::string &values,const std::vector<std::string> *fields,std::vector<KVPair> &result);
  	std::string serializeValues(const std::vector<KVPair> &values);
};

} // ycsbc

#endif // YCSB_C_FASTER_H_
