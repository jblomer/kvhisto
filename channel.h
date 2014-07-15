
#ifndef CHANNEL_H
#define CHANNEL_H

#include <inttypes.h>
#include <math.h>
#include <vector>

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
  uint64_t Sum() const {
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
  uint64_t Sum() const {
    uint64_t sum = 0;
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
  friend class ChannelStoreBase< ChannelT, ChannelStoreSimple<ChannelT> >;
 public:
  ChannelStoreSparse(const uint64_t num) : bins_(num, 0) { };
 protected:
  void AddImpl(const uint64_t index, const ChannelT value) {
    bins_[index] += value;
  }
 private:
  std::vector<ChannelT> bins_;
};

#endif
