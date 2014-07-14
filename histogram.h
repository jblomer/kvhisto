
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
    for (decltype(DimensionV) i = 0; i < DimensionV; ++i) {
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
    for (decltype(DimensionV) i = 0; i < DimensionV; ++i) {
      result += factor * indexes[i];
      factor *= num_bins_[i];
    }
    return result;
  }
  std::array<uint64_t, DimensionV> Channel2Indexes(uint64_t channel)
    const
  {
    std::array<uint64_t, DimensionV> indexes;
    for (decltype(DimensionV) i = 0; i < DimensionV; ++i) {
      indexes[i] = channel % num_bins_[i];
      channel /= num_bins_[i];
    }
    return indexes;
  }

  void Fill(const Coordinate<BorderT, DimensionV> &coord) {
    std::array<uint64_t, DimensionV> indexes;
    for (decltype(DimensionV) i = 0; i < DimensionV; ++i) {
      indexes[i] = binnings_[i].FindBin(coord.values[i]);
    }
    uint64_t channel = Indexes2Channel(indexes);
    channels_->Add(channel, 1);
    static_cast<DerivedT *>(this)->AddMore(channel, 1);
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
  void AddMore(const uint64_t channel, const ChannelT value) { }
};


template <
   typename       BorderT,
   typename       ChannelT,
   class          ChannelStoreT,
   const uint16_t DimensionV
>
class HistogramQsums :
  public HistogramBase<
    BorderT,
    ChannelT,
    ChannelStoreT,
    DimensionV,
    HistogramQsums<BorderT, ChannelT, ChannelStoreT, DimensionV>
  >
{
  friend class HistogramBase<BorderT, ChannelT, ChannelStoreT, DimensionV,
    HistogramQsums<BorderT, ChannelT, ChannelStoreT, DimensionV>>;
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
   class          ChannelStoreT,
   const uint16_t DimensionV
>
class HistogramProfile :
  public HistogramBase<
    BorderT,
    ChannelT,
    ChannelStoreT,
    DimensionV,
    HistogramProfile<BorderT, ChannelT, ChannelStoreT, DimensionV>
  >
{
  friend class HistogramBase<
      BorderT,
      ChannelT,
      ChannelStoreT,
      DimensionV,
      HistogramProfile<BorderT, ChannelT, ChannelStoreT, DimensionV>
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
