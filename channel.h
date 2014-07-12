
#ifndef CHANNEL_H
#define CHANNEL_H

#include <inttypes.h>
#include <math.h>
#include <vector>

template <typename ChannelT, class DerivedT>
class ChannelStoreBase {
 public:
  ChannelStoreBase(const uint64_t num) { }
  void Add(const uint64_t index, const ChannelT value) {
    static_cast<DerivedT *>(this)->AddImpl(index, value);
  }
  ChannelT Get(const uint64_t index) const {
    return static_cast<DerivedT *>(this)->GetImpl(index);
  }
};


template <typename ChannelT>
class ChannelStoreSimple :
  public ChannelStoreBase< ChannelT, ChannelStoreSimple<ChannelT> >
{
  friend class ChannelStoreBase< ChannelT, ChannelStoreSimple<ChannelT> >;
 public:
  ChannelStoreSimple(const uint64_t num) : bins_(num, 0) { };
 protected:
  void AddImpl(const uint64_t index, const ChannelT value) {
    bins_[index] += value;
  }
  ChannelT GetImpl(const uint64_t index) {
    return bins_[index];
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
