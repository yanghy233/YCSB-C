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
#include <iomanip>
#include "core/utils.h"
#include "core/timer.h"
#include "core/client.h"
#include "core/core_workload.h"
#include "db/db_factory.h"

using namespace std;

void UsageMessage(const char *command);

bool StrStartWith(const char *str, const char *pre);

string ParseCommandLine(int argc, const char *argv[], utils::Properties &props);

// Execute the workload by one client (thread)
int DelegateClient(ycsbc::DB *db, ycsbc::CoreWorkload *wl, int thread_idx, const int load_ops,
                   vector<double> *tail_latency) {
    db->Init(thread_idx);
    ycsbc::Client client(*db, *wl);
    int ops_total = 0;
    if (load_ops)   // loading stage
    {
        for (int i = 0; i < load_ops; ++i) {
            ops_total += client.DoInsert();
        }
    } else {   // running stage
        utils::Timer<double> timer;
        double last_time = 0;
        // calculate the latency of several transactions in 1 second
        while (last_time < 1000) {
            timer.Start();
            int ok = client.DoTransaction();
            double t = timer.End();             // span time(ms)
            ops_total += ok;
            if (tail_latency && ok) {
                tail_latency->push_back(t);
            }
            last_time += t;
        }

        // sort the tail_latency array in descending order
        if (tail_latency)
            sort(tail_latency->begin(), tail_latency->end(), greater<double>());
    }
    db->Close();
    return ops_total;
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

    db->SetDbType(props.GetProperty("dbname"));

    // Loading Stage
    vector<future<int>> worker_threads;
    int total_ops = stoi(props[ycsbc::CoreWorkload::RECORD_COUNT_PROPERTY]);
    cout << "[YCSB load] Loading records..." << endl;
    int load_ops_per_thread = total_ops / num_threads;
    for (int i = 0; i < num_threads; ++i) {
        if (i == num_threads - 1) {
            load_ops_per_thread = total_ops - load_ops_per_thread * i;
        }
        worker_threads.emplace_back(async(launch::async,
                                          DelegateClient, db, &wl, i, load_ops_per_thread, nullptr));
    }
    assert((int) worker_threads.size() == num_threads);

    int sum = 0;
    for (auto &n: worker_threads) {
        assert(n.valid());
        sum += n.get();
    }
    cout << "[YCSB load] Records finish loaded:\t" << sum << endl;

    /////////////////////////////////////////////////////////////////////
    if (db->DbType() == "cruisedb") {
        db->Begin(1);     // just for prepare rocksdb tokenBucket
      
       // sleep 目的：等待后台compaction完成，尽可能清空内存的情况下在开始流量控制
       // 防止冷启动问题的发生
       const int SLEEP_TIME = 120;
       sleep(SLEEP_TIME);
    
       db->Begin(2);
    }

    /////////////////////////////////////////////////////////////////////////

    // Running Stage
    int current_time = 0;
    total_ops = stoi(props[ycsbc::CoreWorkload::OPERATION_COUNT_PROPERTY]);

    // latency array in total time
    vector<double> total_latency;
    total_latency.reserve(total_ops);

    // latency array in one second: Descending order
    vector<double> sec_latency;

    // each thread tail_latency array in one second
    vector<vector<double>> tail_latency(num_threads);

    // the pointer to each thread tail_latency array
    vector<int> latency_ptr(num_threads);

    // the heap to store the tail_latency
    priority_queue<pair<double, int>> latency_pq;

    sum = 0;

    while (sum < total_ops) {
        worker_threads.clear();
        for (int i = 0; i < num_threads; ++i) {
            worker_threads.emplace_back(async(launch::async,
                                              DelegateClient, db, &wl, i, 0, &tail_latency[i]));
        }
        assert((int) worker_threads.size() == num_threads);

        int ops = 0;

        for (int i = 0; i < num_threads; ++i) {
            assert(worker_threads[i].valid());
            ops += worker_threads[i].get();
            assert(tail_latency[i].size());
            latency_ptr[i] = 0;
            latency_pq.emplace(tail_latency[i][0], i);
        }
        sum += ops;

        // the same as multi-way merge, all the latency in one second will send to sec_latency
        // sorted in descending order
        while (!latency_pq.empty()) {
            auto current = latency_pq.top();
            latency_pq.pop();
            double curr_latency = current.first;
            int thread_idx = current.second;
            sec_latency.push_back(curr_latency);
            total_latency.push_back(curr_latency);
            ++latency_ptr[thread_idx];
            if (latency_ptr[thread_idx] < (int) tail_latency[thread_idx].size()) {
                latency_pq.emplace(tail_latency[thread_idx][latency_ptr[thread_idx]], thread_idx);
            } else {
                tail_latency[thread_idx].clear();
            }
        }

        cout << fixed << setprecision(2)
             << "[YCSB run] " << current_time++ << " sec, total: " << sum << " , ops: " << ops << ", tail_latency(ms): "
             << "Max=" << sec_latency[0]
             << ", Min=" << sec_latency[sec_latency.size() - 1]
             << ", Avg=" << sec_latency[sec_latency.size() / 2]
             << ", 90%=" << sec_latency[sec_latency.size() * 0.1]
             << ", 99%=" << sec_latency[sec_latency.size() * 0.01]
             << ", 99.9%=" << sec_latency[sec_latency.size() * 0.001]
             << ", 99.99%=" << sec_latency[sec_latency.size() * 0.0001] << endl;

        // stream to cerr: ops, 90% latency, 99% latency, 99.9% latency, 99.99% latency
        cerr << fixed << setprecision(2)
             << ops << ", " << sec_latency[sec_latency.size() * 0.1] << ", " << sec_latency[sec_latency.size() * 0.01]
             << ", " << sec_latency[sec_latency.size() * 0.001] << ", " << sec_latency[sec_latency.size() * 0.0001]
             << endl;

        sec_latency.clear();
    }

    sort(total_latency.begin(), total_latency.end(), greater<double>());

    // total latency: Max, Min, Avg, 90%, 99%, 99.9%, 99.99%
    cout << fixed << setprecision(2) << "[YCSB] Total tail latency(Max): " << total_latency[0] << "ms" << endl;
    cout << fixed << setprecision(2) << "[YCSB] Total tail latency(Min): " << total_latency[total_latency.size() - 1]
         << "ms" << endl;
    cout << fixed << setprecision(2) << "[YCSB] Total tail latency(Avg): " << total_latency[total_latency.size() / 2]
         << "ms" << endl;
    cout << fixed << setprecision(2) << "[YCSB] Total tail latency(90%): " << total_latency[total_latency.size() * 0.1]
         << "ms" << endl;
    cout << fixed << setprecision(2) << "[YCSB] Total tail latency(99%): " << total_latency[total_latency.size() * 0.01]
         << "ms" << endl;
    cout << fixed << setprecision(2) << "[YCSB] Total tail latency(99.9%): "
         << total_latency[total_latency.size() * 0.001] << "ms" << endl;
    cout << fixed << setprecision(2) << "[YCSB] Total tail latency(99.99%): "
         << total_latency[total_latency.size() * 0.0001] << "ms" << endl;

    delete db;

    return 0;
}

string ParseCommandLine(int argc, const char *argv[], utils::Properties &props) {
    int argindex = 1;
    string filename;
    while (argindex < argc && StrStartWith(argv[argindex], "-")) {
        if (strcmp(argv[argindex], "-h") == 0) {
            UsageMessage(argv[0]);
            exit(0);
        } else if (strcmp(argv[argindex], "-threads") == 0) {
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
            }
            catch (const string &message) {
                cout << message << endl;
                exit(0);
            }
            input.close();
            argindex++;
        } else {
            cout << "Unknown option '" << argv[argindex] << "'" << endl;
            UsageMessage(argv[0]);
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
    cout << "  -threads n: execute using n threads (default: 1) [-threads 1]" << endl;
    cout << "  -db dbname: specify the name of the DB to use (default: basic) [-db rocksdb]" << endl;
    cout << "  -P propertyFile: load properties from the given file or multiple files. [-P workloads/workloada.spec]"
         << endl;
    cout << "  1 > res/w1.out: result log by cout" << endl;
    cout << "  2 > res/w1.txt: result(tps,latency) by cerr" << endl;
}

inline bool StrStartWith(const char *str, const char *pre) {
    return strncmp(str, pre, strlen(pre)) == 0;
}
