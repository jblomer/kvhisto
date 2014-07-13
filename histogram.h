
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

template <
   typename       BorderT,
   typename       ChannelT,
   class          ChannelStoreT,
   const uint16_t DimensionV,
   class          DerivedT
>
class HistogramBase {
 //protected:
 public:
  explicit HistogramBase(
    const std::array<Binning<BorderT>, DimensionV> &binnings)
    : binnings_(binnings)
  {
    uint64_t total_bins = 1;
    for (auto i : binnings_) {
      num_bins_[i] = binnings_[i].num_bins();
      total_bins *= num_bins_[i];
    }
    channels_ = std::unique_ptr< ChannelStoreBase<ChannelT, ChannelStoreT> >
      (new ChannelStoreT(total_bins));
  }

  uint64_t Indexes2Channel(const std::array<uint64_t, DimensionV> &indexes)
    const
  {
    uint64_t result = 0;
    uint64_t factor = 1;
    for (auto i : indexes) {
      result += factor * indexes[i];
      factor *= num_bins_[i];
    }
    return result;
  }
  std::array<uint64_t, DimensionV> Channel2Indexes(uint64_t channel)
    const
  {
    std::array<uint64_t, DimensionV> indexes;
    for (auto i : indexes) {
      indexes[i] = channel % num_bins_[i];
      channel /= num_bins_[i];
    }
    return indexes;
  }

  void Fill(const std::array<BorderT, DimensionV> &point) {
    std::array<uint64_t, DimensionV> indexes;
    for (auto i : indexes) {
      indexes[i] = binnings_[i].FindBin(point[i]);
    }
    uint64_t channel = Indexes2Channel(indexes);
    channels_->Add(channel, 1);
    static_cast<DerivedT *>(this)->AddQsum(channel, 1);
  }

 private:
  std::array<Binning<BorderT>, DimensionV> binnings_;
  // Cache for binnings_[i]->num_bins()
  std::array<uint64_t, DimensionV> num_bins_;
  std::unique_ptr< ChannelStoreBase<ChannelT, ChannelStoreT> > channels_;
};


template <
   typename       BorderT,
   typename       ChannelT,
   class          ChannelStoreT,
   const uint16_t DimensionV
>
class Histogram :
  public HistogramBase<
    BorderT,
    ChannelT,
    ChannelStoreT,
    DimensionV,
    Histogram<BorderT, ChannelT, ChannelStoreT, DimensionV>
  >
{
  typedef
    HistogramBase< BorderT, ChannelT, ChannelStoreT, DimensionV,
      Histogram<BorderT, ChannelT, ChannelStoreT, DimensionV> >
      FatherT;
  friend class HistogramBase< BorderT, ChannelT, ChannelStoreT, DimensionV,
      Histogram<BorderT, ChannelT, ChannelStoreT, DimensionV> >;
 public:
  explicit Histogram(const std::array<Binning<BorderT>, DimensionV> &binnings)
    : FatherT(binnings) { }
 protected:
  void AddQsum(const uint64_t channel, const ChannelT value) { }
};


template <
   typename       BorderT,
   typename       ChannelT,
   class          ChannelStoreT,
   const uint16_t DimensionV
>
class HistogramWeighted :
  public HistogramBase<
    BorderT,
    ChannelT,
    ChannelStoreT,
    DimensionV,
    Histogram<BorderT, ChannelT, ChannelStoreT, DimensionV>
  >
{
  friend class Histogram<BorderT, ChannelT, ChannelStoreT, DimensionV>;
 protected:
  void AddQsum(const uint64_t channel, const ChannelT value) {
    qsums_->Add(channel, value*value);
  }
 private:
  std::unique_ptr< ChannelStoreBase<ChannelT, ChannelStoreT> > qsums_;
};

#endif
