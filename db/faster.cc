//
//  faster.cc
//  YCSB-C
//
//  Created by Junkai Liang on 13/1/21.
//

#include "faster.h"
#include<iostream>

namespace ycsbc{

Faster::Faster() {
	// FASTER store has a hash table with approx. kInitCount / 2 entries and a log of size 16 GB
	size_t init_size = next_power_of_two(kInitCount / 2);
	store = new store_t(init_size, 17179869184, "/home/ljk/YCSB-C/faster_test/");
}

Faster::~Faster() {
	delete store;
}

void Faster::Init(int thread_idx) {
	SetThreadAffinity(thread_idx);
	Guid guid = store->StartSession();
	return;
}

void Faster::Close() {
	store->CompletePending(true);
	store->StopSession();
	return;
}

int Faster::Read (const std::string &table,const std::string &key,const std::vector<std::string> *fields,std::vector<KVPair> &result)
    {
	auto callback = [](IAsyncContext* ctxt, Status result) {
          CallbackContext<ReadContext> context{ ctxt };
        };

    ReadContext context{ key[0] };

    Status s = store->Read(context, callback, 1);
    store->Refresh();

	int code=(int)s;
	if(code)return code;
    // deserializeValues(context.value(), fields, result);
    return 0;
    }

int Faster::Scan(const std::string &table, const std::string &key,int record_count, const std::vector<std::string> *fields,std::vector<std::vector<KVPair>> &result)
    {
	return 7;
    }

int Faster::Update(const std::string &table,const std::string &key,std::vector<KVPair> &values)
    {
    auto callback = [](IAsyncContext* ctxt, Status result) {
          CallbackContext<RmwContext> context{ ctxt };
        };

    RmwContext context{ key[0], 5 };
    Status result = store->Rmw(context, callback, 1);
    store->Refresh();
    }

int Faster::Insert(const std::string &table, const std::string &key,std::vector<KVPair> &values)
    {
	auto callback = [](IAsyncContext* ctxt, Status result) {
          CallbackContext<UpsertContext> context{ ctxt };
        };

    UpsertContext context{ key[0], 0 };
    Status s = store->Upsert(context, callback, 1);
    store->Refresh();
	int code=(int)s;
	return code;
    }

int Faster::Delete(const std::string &table, const std::string &key)
    {
	auto callback = [](IAsyncContext* ctxt, Status result) {
          CallbackContext<UpsertContext> context{ ctxt };
        };

    UpsertContext context{ key[0], 0 };
    Status s = store->Upsert(context, callback, 1);
    store->Refresh();
	int code=(int)s;
	return code;
    }

/// Affinitize to hardware threads on the same core first, before
/// moving on to the next core.
void Faster::SetThreadAffinity(size_t core) {

  // For now, assume 36 cores. (Set this correctly for your test system.)
  constexpr size_t kCoreCount = 40;
  cpu_set_t mask;
  CPU_ZERO(&mask);
  switch(core % 2) {
  case 0:
    // 0 |-> 0
    // 2 |-> 2
    // 4 |-> 4
    core = core;
    break;
  case 1:
    // 1 |-> 28
    // 3 |-> 30
    // 5 |-> 32
    core = (core - 1) + kCoreCount;
    break;
  }
  CPU_SET(core, &mask);

  ::sched_setaffinity(0, sizeof(mask), &mask);
}

void Faster::deserializeValues(const std::string &values,const std::vector<std::string> *fields,std::vector<KVPair> &result)
    {
    for(auto pos=values.find(' ');pos!=std::string::npos;)
        {
        auto next=values.find(' ',pos+1);
        std::string tmp=values.substr(pos+1,next-pos-1);
        bool flag=fields?false:true;
        for(unsigned i=0;!flag&&i<fields->size();++i)
            if(tmp==fields->at(i))
                flag=true;
        pos=values.find(' ',next+1);
        if(flag)
            result.emplace_back(tmp,values.substr(next+1,pos-next-1));
        }
    return;
    }

std::string Faster::serializeValues(const std::vector<KVPair> &values)
    {
    std::string ret;
    for(unsigned i=0;i<values.size();++i)
        ret+=" "+values[i].first+" "+values[i].second;
    return ret;
    }
} // namespace ycsbc
