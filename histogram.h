
#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <inttypes.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <array>
#include <type_traits>

#include <cassert>

#include "binning.h"
#include "channel.h"
#include "coord.h"

// Always allow weighted fill
// RMS?
// Check for overflow for char, short
// Automatic bin extension
// average histograms

template <
   typename       BorderT,
   typename       ChannelT,
   class          ChannelStoreT,
   class          DerivedT
>
class HistogramBase {
 //protected:
 public:
  explicit HistogramBase(const std::vector<Binning<BorderT>> &binnings) 
    : dimension_(binnings.size())
    , binnings_(binnings) 
  {
    size_ = 1;
    for (uint16_t i = 0; i < dimension_; ++i) {
      num_bins_.push_back(binnings_[i].num_bins());
      size_ *= num_bins_[i];
    }
    channels_ = std::unique_ptr< ChannelStoreBase<ChannelT, ChannelStoreT> >
      (new ChannelStoreT(size_));
  }

  uint64_t Indexes2Channel(const std::vector<uint64_t> &indexes)
    const
  {
    uint64_t result = 0;
    uint64_t factor = 1;
    for (uint16_t i = 0; i < dimension_; ++i) {
      result += factor * indexes[i];
      factor *= num_bins_[i];
    }
    return result;
  }
  std::vector<uint64_t> Channel2Indexes(uint64_t channel)
    const
  {
    std::vector<uint64_t> indexes;
    for (uint16_t i = 0; i < dimension_; ++i) {
      indexes.push_back(channel % num_bins_[i]);
      channel /= num_bins_[i];
    }
    return indexes;
  }

  void Fill(const std::vector<BorderT> &coord) {
    std::vector<uint64_t> indexes;
    for (uint16_t i = 0; i < dimension_; ++i) {
      indexes.push_back(binnings_[i].FindBin(coord[i]));
    }
    uint64_t channel = Indexes2Channel(indexes);
    channels_->Add(channel, 1);
    static_cast<DerivedT *>(this)->AddMore(channel, 1);
  }
  
  uint64_t size() const { return size_; }
  uint64_t occupied() const { return channels_->Occupied(); }
  ChannelT sum() const { return channels_->Sum(); }

 private:
  uint16_t dimension_;
  std::vector<Binning<BorderT>> binnings_;
  // Cache for binnings_[i]->num_bins()
  std::vector<uint64_t> num_bins_;
  std::unique_ptr< ChannelStoreBase<ChannelT, ChannelStoreT> > channels_;
  uint64_t size_;
};


template <
   typename       BorderT,
   typename       ChannelT,
   class          ChannelStoreT
>
class Histogram :
  public HistogramBase<
    BorderT,
    ChannelT,
    ChannelStoreT,
    Histogram<BorderT, ChannelT, ChannelStoreT>
  >
{
  typedef
    HistogramBase< BorderT, ChannelT, ChannelStoreT,
      Histogram<BorderT, ChannelT, ChannelStoreT> >
      FatherT;
  friend class HistogramBase< BorderT, ChannelT, ChannelStoreT,
      Histogram<BorderT, ChannelT, ChannelStoreT> >;
 public:
  explicit Histogram(const std::vector<Binning<BorderT>> &binnings)
    : FatherT(binnings) { }
 protected:
  void AddMore(const uint64_t channel, const ChannelT value) { }
};


template <
   typename       BorderT,
   typename       ChannelT,
   class          ChannelStoreT
>
class HistogramQsums :
  public HistogramBase<
    BorderT,
    ChannelT,
    ChannelStoreT,
    HistogramQsums<BorderT, ChannelT, ChannelStoreT>
  >
{
  friend class HistogramBase<BorderT, ChannelT, ChannelStoreT,
    HistogramQsums<BorderT, ChannelT, ChannelStoreT>>;
 protected:
  void AddMore(const uint64_t channel, const ChannelT value) {
    qsums_->Add(channel, value*value);
  }
 private:
  std::unique_ptr< ChannelStoreBase<ChannelT, ChannelStoreT> > qsums_;
};


template <
   typename       BorderT,
   typename       ChannelT,
   class          ChannelStoreT
>
class HistogramProfile :
  public HistogramBase<
    BorderT,
    ChannelT,
    ChannelStoreT,
    HistogramProfile<BorderT, ChannelT, ChannelStoreT>
  >
{
  friend class HistogramBase<
      BorderT,
      ChannelT,
      ChannelStoreT,
      HistogramProfile<BorderT, ChannelT, ChannelStoreT>
    >;
 protected:
  void AddMore(const uint64_t channel, const ChannelT value) {
    // TODO
  }
 private:
  std::unique_ptr< ChannelStoreBase<ChannelT, ChannelStoreT> > qsums_;
  
  // TODO: replace by num_fills
  std::unique_ptr< ChannelStoreBase<ChannelT, ChannelStoreT> > qsumsmeans_;
  std::unique_ptr< ChannelStoreBase<ChannelT, ChannelStoreT> > means_;
};

#endif
