//
//  rocks_db.cc
//  YCSB-C
//
//  Created by Junkai Liang on 9/19/19.
//

#include "rocks_db.h"

using namespace rocksdb;

namespace ycsbc{
static std::vector<ColumnFamilyDescriptor> column_families;

RocksDB::RocksDB()
    {
// #ifndef SILK
//     options.rate_limiter.reset(NewGenericRateLimiter(400<<20,100*1000,10,RateLimiter::Mode::kWritesOnly,true));
// #endif
    // std::string ram_path="/home/ubuntu/ramdisk";
    std::string disk_path="./rocksdb_test";
    options.max_write_buffer_number = 24;
    //options.min_write_buffer_number_to_merge = 4;
    // options.level0_file_num_compaction_trigger = 1;
    // options.db_paths.emplace_back(ram_path,1ull<<30);
    // options.db_paths.emplace_back(disk_path,1ull<<40);

    options.create_if_missing = true;
    Status s=rocksdb::DB::Open(options,disk_path,&db);
    assert(s.ok());

    s = db->CreateColumnFamily(ColumnFamilyOptions(options),"usertable", &cf);
    assert(s.ok());
/*     if(column_families.size())
        {
        std::vector<ColumnFamilyHandle*> cf_handles;
        Status s=rocksdb::DB::Open(options,disk_path,column_families,&cf_handles,&db);
        assert(s.ok());
        for(unsigned i=1;i<cf_handles.size();++i)
            handles[column_families[i].name]=cf_handles[i];
        }
    else
        {
        options.create_if_missing=true;
        Status s=rocksdb::DB::Open(options,disk_path,&db);
        assert(s.ok());
        } */
    }

RocksDB::~RocksDB()
    {
/*     column_families.clear();
    column_families.emplace_back(kDefaultColumnFamilyName, ColumnFamilyOptions(options));
    for(const auto &handle : handles)
        {
        column_families.emplace_back(handle.first,ColumnFamilyOptions(options));
        delete handle.second;
        } */
    delete cf;
    delete db;
    }

int RocksDB::Read (const std::string &table,const std::string &key,const std::vector<std::string> *fields,std::vector<KVPair> &result)
    {
/*     mu.lock();
    if(handles.find(table)==handles.end())
        {
        ColumnFamilyHandle* cf;
        Status s = db->CreateColumnFamily(ColumnFamilyOptions(options),table, &cf);
        assert(s.ok());
        handles[table]=cf;
        }
    ColumnFamilyHandle* cf=handles[table];
    mu.unlock(); */
    std::string values;
    Status s=db->Get(ReadOptions(),cf,Slice(key),&values);
    if(!s.ok())return s.code();
    deserializeValues(values, fields, result);
    return 0;
    }

int RocksDB::Scan(const std::string &table, const std::string &key,int record_count, const std::vector<std::string> *fields,std::vector<std::vector<KVPair>> &result)
    {
/*     mu.lock();
    if(handles.find(table)==handles.end())
        {
        ColumnFamilyHandle* cf;
        Status s = db->CreateColumnFamily(ColumnFamilyOptions(options),table, &cf);
        assert(s.ok());
        handles[table]=cf;
        }
    ColumnFamilyHandle* cf=handles[table];
    mu.unlock(); */
    Iterator*it=db->NewIterator(ReadOptions(),cf);
    int cnt=0;
    for(it->Seek(key);it->Valid()&&cnt<record_count;it->Next())
        {
        std::vector<KVPair> tmp;
        deserializeValues(it->value().data(), fields, tmp);
        result.push_back(tmp);
        ++cnt;
        }
    int ret=it->status().code();
    delete it;
    return ret;
    }

int RocksDB::Update(const std::string &table,const std::string &key,std::vector<KVPair> &values)
    {
/*     mu.lock();
    if(handles.find(table)==handles.end())
        {
        mu.unlock();
        return Status::kNotFound;
        }
    ColumnFamilyHandle* cf=handles[table];
    mu.unlock(); */
    Status s=db->Put(WriteOptions(),cf,Slice(key),Slice(serializeValues(values)));
    return s.code();
    // if(handles.find(table)==handles.end())
    //     {
    //     ColumnFamilyHandle* cf;
    //     Status s = db->CreateColumnFamily(ColumnFamilyOptions(options),table, &cf);
    //     assert(s.ok());
    //     handles[table]=cf;
    //     }
    // ColumnFamilyHandle* cf=handles[table];
    // std::string oldvalues;
    // Status s=db->Get(ReadOptions(),cf,Slice(key),&oldvalues);
    // if(!s.ok())return s.code();
    // std::vector<KVPair> result;
    // deserializeValues(oldvalues, NULL, result);
    // updateValues(result,values);
    // s=db->Put(WriteOptions(),cf,Slice(key),Slice(serializeValues(result)));
    // return s.code();
    }

int RocksDB::Insert(const std::string &table, const std::string &key,std::vector<KVPair> &values)
    {
/*     mu.lock();
    if(handles.find(table)==handles.end())
        {
        ColumnFamilyHandle* cf;
        Status s = db->CreateColumnFamily(ColumnFamilyOptions(options),table, &cf);
        assert(s.ok());
        handles[table]=cf;
        }
    ColumnFamilyHandle* cf=handles[table];
    mu.unlock(); */
    Status s=db->Put(WriteOptions(),cf,Slice(key),Slice(serializeValues(values)));
    return s.code();
    }

int RocksDB::Delete(const std::string &table, const std::string &key)
    {
/*     mu.lock();
    if(handles.find(table)==handles.end())
        {
        ColumnFamilyHandle* cf;
        Status s = db->CreateColumnFamily(ColumnFamilyOptions(options),table, &cf);
        assert(s.ok());
        handles[table]=cf;
        }
    ColumnFamilyHandle* cf=handles[table];
    mu.unlock(); */
    Status s=db->Delete(WriteOptions(),cf,Slice(key));
    return s.code();
    }

void RocksDB::deserializeValues(const std::string &values,const std::vector<std::string> *fields,std::vector<KVPair> &result)
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

// void RocksDB::updateValues(std::vector<KVPair> &result ,const std::vector<KVPair> &values)
//     {
//     for(unsigned i=0;i<result.size();++i)
//         for(unsigned j=0;j<values.size();++j)
//             if(result[i].first==values[j].first)
//                 {
//                 result[i].second=values[j].second;
//                 break;
//                 }
//     return;
//     }

std::string RocksDB::serializeValues(const std::vector<KVPair> &values)
    {
    std::string ret;
    for(unsigned i=0;i<values.size();++i)
        ret+=" "+values[i].first+" "+values[i].second;
    return ret;
    }
} // namespace ycsbc
