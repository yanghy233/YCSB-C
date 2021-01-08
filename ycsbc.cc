//
//  ycsbc.cc
//  YCSB-C
//
//  Created by Jinglei Ren on 12/19/14.
//  Copyright (c) 2014 Jinglei Ren <jinglei@ren.systems>.
//

#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <queue>
#include <future>
#include <functional>
#include <algorithm>
#include <unistd.h>
#include "core/utils.h"
#include "core/timer.h"
#include "core/client.h"
#include "core/core_workload.h"
#include "db/db_factory.h"

using namespace std;

void UsageMessage(const char *command);
bool StrStartWith(const char *str, const char *pre);
string ParseCommandLine(int argc, const char *argv[], utils::Properties &props);

// int DelegateClient(ycsbc::DB *db, ycsbc::CoreWorkload *wl, const int num_ops,
//     bool is_loading) {
//   db->Init();
//   ycsbc::Client client(*db, *wl);
//   int oks = 0;
//   for (int i = 0; i < num_ops; ++i) {
//     if (is_loading) {
//       oks += client.DoInsert();
//     } else {
//       oks += client.DoTransaction();
//     }
//   }
//   db->Close();
//   return oks;
// }

int DelegateClient(ycsbc::DB *db, ycsbc::CoreWorkload *wl, const int load_ops, vector<double> *tail_latency) {
  db->Init();
  ycsbc::Client client(*db, *wl);
  int oks = 0;
  if (load_ops) {
    for (int i = 0; i < load_ops; ++i) {
          oks += client.DoInsert();
        }
  } else {
    utils::Timer<double> timer;
    double last_time = 0;
    while (last_time < 1000) {
      timer.Start();
      int ok = client.DoTransaction();
      double t = timer.End();
      oks += ok;
      if (ok) {
        tail_latency->push_back(t);
      }
      last_time += t;
    }
    sort(tail_latency->begin(), tail_latency->end(), greater<double>());
  }
  db->Close();
  return oks;
}

int main(const int argc, const char *argv[]) {
  utils::Properties props;
  string file_name = ParseCommandLine(argc, argv, props);

  ycsbc::DB *db = ycsbc::DBFactory::CreateDB(props);
  if (!db) {
    cout << "Unknown database name " << props["dbname"] << endl;
    exit(0);
  }

  ycsbc::CoreWorkload wl;
  wl.Init(props);

  const int num_threads = stoi(props.GetProperty("threadcount", "1"));

  // Loads data
  vector<future<int>> actual_ops;
  int total_ops = stoi(props[ycsbc::CoreWorkload::RECORD_COUNT_PROPERTY]);
  for (int i = 0; i < num_threads; ++i) {
    actual_ops.emplace_back(async(launch::async,
        DelegateClient, db, &wl, total_ops / num_threads, nullptr));
  }
  assert((int)actual_ops.size() == num_threads);

  int sum = 0;
  for (auto &n : actual_ops) {
    assert(n.valid());
    sum += n.get();
  }
  cout << "# Loading records:\t" << sum << endl;

  #define SLEEP_TIME 60
  #define OPERATION_TIME 500

  sleep(SLEEP_TIME);

  // Peforms transactions
  total_ops = stoi(props[ycsbc::CoreWorkload::OPERATION_COUNT_PROPERTY]);
  
  vector<double> total_latency;
  total_latency.reserve(total_ops);

  vector<double> sec_latency;
  vector<vector<double> > tail_latency(num_threads);

  vector<int> ltc_ptr(num_threads);
  priority_queue<pair<double,int> > ltc_pq;

  sum = 0;

  for( int op_time = 0; op_time < OPERATION_TIME; ++op_time) {
    actual_ops.clear();
    for (int i = 0; i < num_threads; ++i) {
      actual_ops.emplace_back(async(launch::async,
          DelegateClient, db, &wl, 0, &tail_latency[i]));
    }
    assert((int)actual_ops.size() == num_threads);

    int ops = 0;

    for (int i = 0; i < num_threads; ++i) {
      assert(actual_ops[i].valid());
      ops += actual_ops[i].get();
      assert(tail_latency[i].size());
      ltc_ptr[i] = 0;
      ltc_pq.emplace(tail_latency[i][0],i);
    }
    sum += ops;

    while(!ltc_pq.empty()) {
      auto now = ltc_pq.top();
      ltc_pq.pop();
      sec_latency.push_back(now.first);
      total_latency.push_back(now.first);
      ++ltc_ptr[now.second];
      if(ltc_ptr[now.second] < (int)tail_latency[now.second].size()) {
        ltc_pq.emplace(tail_latency[now.second][ltc_ptr[now.second]],now.second);
      } else {
        tail_latency[now.second].clear();
      }
    }

    cout << "# Doing transactions:\tsum: " << sum << "\tops: " << ops << "\ttail_latency: " << sec_latency[ops*0.01] << " ms" << endl;
    cerr << ops << '\t' << sec_latency[ops*0.01] << endl;

    sec_latency.clear();
  }

  sort(total_latency.begin(),total_latency.end(),greater<double>());
  cout << "Total tail latency: " << total_latency[total_latency.size()*0.01] << "ms" << endl;
  cerr << total_latency[total_latency.size()*0.01] << endl;
  return 0;
}

string ParseCommandLine(int argc, const char *argv[], utils::Properties &props) {
  int argindex = 1;
  string filename;
  while (argindex < argc && StrStartWith(argv[argindex], "-")) {
    if (strcmp(argv[argindex], "-threads") == 0) {
      argindex++;
      if (argindex >= argc) {
        UsageMessage(argv[0]);
        exit(0);
      }
      props.SetProperty("threadcount", argv[argindex]);
      argindex++;
    } else if (strcmp(argv[argindex], "-db") == 0) {
      argindex++;
      if (argindex >= argc) {
        UsageMessage(argv[0]);
        exit(0);
      }
      props.SetProperty("dbname", argv[argindex]);
      argindex++;
    } else if (strcmp(argv[argindex], "-host") == 0) {
      argindex++;
      if (argindex >= argc) {
        UsageMessage(argv[0]);
        exit(0);
      }
      props.SetProperty("host", argv[argindex]);
      argindex++;
    } else if (strcmp(argv[argindex], "-port") == 0) {
      argindex++;
      if (argindex >= argc) {
        UsageMessage(argv[0]);
        exit(0);
      }
      props.SetProperty("port", argv[argindex]);
      argindex++;
    } else if (strcmp(argv[argindex], "-slaves") == 0) {
      argindex++;
      if (argindex >= argc) {
        UsageMessage(argv[0]);
        exit(0);
      }
      props.SetProperty("slaves", argv[argindex]);
      argindex++;
    } else if (strcmp(argv[argindex], "-P") == 0) {
      argindex++;
      if (argindex >= argc) {
        UsageMessage(argv[0]);
        exit(0);
      }
      filename.assign(argv[argindex]);
      ifstream input(argv[argindex]);
      try {
        props.Load(input);
      } catch (const string &message) {
        cout << message << endl;
        exit(0);
      }
      input.close();
      argindex++;
    } else {
      cout << "Unknown option '" << argv[argindex] << "'" << endl;
      exit(0);
    }
  }

  if (argindex == 1 || argindex != argc) {
    UsageMessage(argv[0]);
    exit(0);
  }

  return filename;
}

void UsageMessage(const char *command) {
  cout << "Usage: " << command << " [options]" << endl;
  cout << "Options:" << endl;
  cout << "  -threads n: execute using n threads (default: 1)" << endl;
  cout << "  -db dbname: specify the name of the DB to use (default: basic)" << endl;
  cout << "  -P propertyfile: load properties from the given file. Multiple files can" << endl;
  cout << "                   be specified, and will be processed in the order specified" << endl;
}

inline bool StrStartWith(const char *str, const char *pre) {
  return strncmp(str, pre, strlen(pre)) == 0;
}
