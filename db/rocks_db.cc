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
    Options options;
    options.max_background_jobs=1;
    if(column_families.size())
        {
        std::vector<ColumnFamilyHandle*> cf_handles;
        Status s=rocksdb::DB::Open(options,"./rocksdb_test",column_families,&cf_handles,&db);
        assert(s.ok());
        for(unsigned i=1;i<cf_handles.size();++i)
            handles[column_families[i].name]=cf_handles[i];
        }
    else
        {
        options.create_if_missing=true;
        Status s=rocksdb::DB::Open(options,"./rocksdb_test",&db);
        assert(s.ok());
        }
    }

RocksDB::~RocksDB()
    {
    column_families.clear();
    column_families.emplace_back(kDefaultColumnFamilyName, ColumnFamilyOptions());
    for(const auto &handle : handles)
        {
        column_families.emplace_back(handle.first,ColumnFamilyOptions());
        delete handle.second;
        }
    delete db;
    }

int RocksDB::Read (const std::string &table,const std::string &key,const std::vector<std::string> *fields,std::vector<KVPair> &result)
    {
    if(handles.find(table)==handles.end())
        {
        ColumnFamilyHandle* cf;
        Status s = db->CreateColumnFamily(ColumnFamilyOptions(),table, &cf);
        assert(s.ok());
        handles[table]=cf;
        }
    ColumnFamilyHandle* cf=handles[table];
    std::string values;
    Status s=db->Get(ReadOptions(),cf,Slice(key),&values);
    if(!s.ok())return s.code();
    deserializeValues(values, fields, result);
    return 0;
    }

int RocksDB::Scan(const std::string &table, const std::string &key,int record_count, const std::vector<std::string> *fields,std::vector<std::vector<KVPair>> &result)
    {
    if(handles.find(table)==handles.end())
        {
        ColumnFamilyHandle* cf;
        Status s = db->CreateColumnFamily(ColumnFamilyOptions(),table, &cf);
        assert(s.ok());
        handles[table]=cf;
        }
    ColumnFamilyHandle* cf=handles[table];
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
    if(handles.find(table)==handles.end())
        {
        ColumnFamilyHandle* cf;
        Status s = db->CreateColumnFamily(ColumnFamilyOptions(),table, &cf);
        assert(s.ok());
        handles[table]=cf;
        }
    ColumnFamilyHandle* cf=handles[table];
    std::string oldvalues;
    Status s=db->Get(ReadOptions(),cf,Slice(key),&oldvalues);
    if(!s.ok())return s.code();
    std::vector<KVPair> result;
    deserializeValues(oldvalues, NULL, result);
    updateValues(result,values);
    s=db->Put(WriteOptions(),cf,Slice(key),Slice(serializeValues(result)));
    return s.code();
    }

int RocksDB::Insert(const std::string &table, const std::string &key,std::vector<KVPair> &values)
    {
    if(handles.find(table)==handles.end())
        {
        ColumnFamilyHandle* cf;
        Status s = db->CreateColumnFamily(ColumnFamilyOptions(),table, &cf);
        assert(s.ok());
        handles[table]=cf;
        }
    ColumnFamilyHandle* cf=handles[table];
    Status s=db->Put(WriteOptions(),cf,Slice(key),Slice(serializeValues(values)));
    return s.code();
    }

int RocksDB::Delete(const std::string &table, const std::string &key)
    {
    if(handles.find(table)==handles.end())
        {
        ColumnFamilyHandle* cf;
        Status s = db->CreateColumnFamily(ColumnFamilyOptions(),table, &cf);
        assert(s.ok());
        handles[table]=cf;
        }
    ColumnFamilyHandle* cf=handles[table];
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

void RocksDB::updateValues(std::vector<KVPair> &result ,const std::vector<KVPair> &values)
    {
    for(unsigned i=0;i<result.size();++i)
        for(unsigned j=0;j<values.size();++j)
            if(result[i].first==values[j].first)
                {
                result[i].second=values[j].second;
                break;
                }
    return;
    }

std::string RocksDB::serializeValues(const std::vector<KVPair> &values)
    {
    std::string ret;
    for(unsigned i=0;i<values.size();++i)
        ret+=" "+values[i].first+" "+values[i].second;
    return ret;
    }
} // namespace ycsbc
