
#ifndef CHANNEL_H
#define CHANNEL_H

#include <inttypes.h>
#include <math.h>
#include <vector>
#include <unordered_map>

#include "kvincr.h"

template <typename ChannelT, class DerivedT>
class ChannelStoreBase {
 public:
  void Add(const uint64_t index, const ChannelT value) {
    static_cast<DerivedT *>(this)->AddImpl(index, value);
  }
  ChannelT Get(const uint64_t index) const {
    return static_cast<DerivedT *>(this)->GetImpl(index);
  }
  uint64_t Occupied() const {
    return static_cast<const DerivedT *>(this)->Occupied();
  }
  ChannelT Sum() const {
    return static_cast<const DerivedT *>(this)->Sum();
  }
};


template <typename ChannelT>
class ChannelStoreSimple :
  public ChannelStoreBase< ChannelT, ChannelStoreSimple<ChannelT> >
{
  friend class ChannelStoreBase< ChannelT, ChannelStoreSimple<ChannelT> >;
 public:
  ChannelStoreSimple(const uint64_t total_bins) : bins_(total_bins, 0) { };
  uint64_t total_bins() const { return bins_.size(); }
 protected:
  void AddImpl(const uint64_t index, const ChannelT value) {
    bins_[index] += value;
  }
  ChannelT GetImpl(const uint64_t index) {
    return bins_[index];
  }
  uint64_t Occupied() const {
    uint64_t occupied = 0;
    for (auto i : bins_) {
      if (i > 0) occupied++;
    }
    return occupied; 
  }
  ChannelT Sum() const {
    ChannelT sum{};
    for (auto i : bins_) {
      sum += i;
    }
    return sum; 
  }
 private:
  std::vector<ChannelT> bins_;
};


template <typename ChannelT>
class ChannelStoreSparse :
  public ChannelStoreBase< ChannelT, ChannelStoreSparse<ChannelT> >
{
  friend class ChannelStoreBase< ChannelT, ChannelStoreSparse<ChannelT> >;
 public:
  ChannelStoreSparse(const uint64_t num) { };
 protected:
  void AddImpl(const uint64_t index, const ChannelT value) {
    bins_[index] += value;
  }
  ChannelT GetImpl(const uint64_t index) {
    ChannelT result{};
    auto iter = bins_.find(index);
    if (iter != bins_.end())
      result = iter->second;
    return result;
  }
  uint64_t Occupied() const {
    return bins_.size(); 
  }
  ChannelT Sum() const {
    ChannelT sum{};
    for (auto i : bins_) {
      sum += i.second;
    }
    return sum; 
  }
 private:
  std::unordered_map<uint64_t, ChannelT> bins_;
};

template <typename ChannelT>
class ChannelStoreKvStore :
  public ChannelStoreBase< ChannelT, ChannelStoreKvStore<ChannelT> >
{
  friend class ChannelStoreBase< ChannelT, ChannelStoreKvStore<ChannelT> >;
 public:
  ChannelStoreKvStore(const uint64_t num) : conn_(NULL) { };
  ~ChannelStoreKvStore() {
    KvDisconnect(conn_);
  }
  
  bool Connect(const std::string &locator) { 
    conn_ = KvConnect(locator.c_str());
    return conn_ != NULL;
  }
  void SetName(const std::string &name) { name_ = name; }
  void Commit() {
    
  }
 protected:
  void AddImpl(const uint64_t index, const ChannelT value) {
    bins_[index] += value;
  }
  ChannelT GetImpl(const uint64_t index) {
    ChannelT result{};
    auto iter = bins_.find(index);
    if (iter != bins_.end())
      result = iter->second;
    return result;
  }
  uint64_t Occupied() const {
    return bins_.size(); 
  }
  ChannelT Sum() const {
    ChannelT sum{};
    for (auto i : bins_) {
      sum += i.second;
    }
    return sum; 
  }
 private:
  std::unordered_map<uint64_t, ChannelT> bins_;
  std::string name_;
  KV_CONNECTION *conn_;
};

#endif
