
#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <inttypes.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <array>

#include "channel.h"

template <
   typename       BorderT,
   typename       ChannelT,
   class          ChannelStoreT,
   const uint16_t DimensionV,
   class          DerivedT
>
class HistogramBase {
 public:
  class Binning {
   public:
     Binning(const BorderT min, const BorderT width, const BorderT max)
       : min_(min)
       , max_(max)
       , width_(width)
       , max_bin_(ceil((max-min)/width) + 1)
     { }
     Binning(const std::vector<BorderT> &borders)
       : borders_(borders)
       , min_(borders[0])
       , max_(borders.back())
       , width_(0)
       , max_bin_(borders_.size())
     { }

     uint64_t FindBin(const BorderT val) const {
       if (val < min_) return 0;
       if (val >= max_) return max_bin_;
       if (width_ > 0) return (val-min_)/width_ + 1;

       uint64_t lo = 0;
       uint64_t hi = max_bin_;
       while (hi-lo > kSteps) {
         uint64_t skip = (hi-lo)/kSteps;
         unsigned i = lo + skip;
         for ( ; i < hi; i += skip)
           if (borders_[i] >= val) break;
         hi = (i > hi) ? hi : i;
         lo = i - skip;
       }
       unsigned i = lo;
       for ( ; i < hi; ++i)
         if (borders_[i] > val) break;
       return i;
     }

     uint64_t num_bins() const { return max_bin_ + 1; }
   private:
    static const uint8_t kSteps = 16;

    std::vector<BorderT> borders_;
    BorderT min_;
    BorderT max_;
    BorderT width_;
    uint32_t max_bin_;
  };

 //protected:
 public:
  //explicit HistogramBase(const )
/*  HistogramBase()
    : dimensions_(0)
    , binnings_(nullptr)
    , max_bins_(nullptr)
  { }*/
  /*void SetDimensions(const uint16_t dimensions) {
    Clear();
    dimensions_ = dimensions;
    binnings_ = new Binning[dimensions_];
    max_bins_ = new uint64_t[dimensions_];

    bins_ = std::unique_ptr< ChannelStoreBase<ChannelT, ChannelStoreT> >
      (new ChannelStoreBase<ChannelT, ChannelStoreT>(dimensions));

    //TODO
    //bins_ = new ChannelT[max_bins_[dimensions_-1]];
  }
  void SetBinning(const uint16_t dimension, const Binning &binning) {
    max_bins_[dimension-1] = binning.max_bin();
    binnings_[dimension-1] = binning;
  }
  void Fill(BorderT val) {
    uint64_t bin_id = binnings_[0].FindBin(val);
    //bins_[bin_id] = bins_[bin_id]+1;
  }*/

  void SetBinning(const uint16_t dimension, const Binning &binning) {
    binnings_[dimension] = std::unique_ptr<Binning>(new Binning(binning));
    num_bins_[dimension] = binnings_[dimension]->num_bins();
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
      indexes[i] = binnings_[i]->FindBin(point[i]);
    }
    uint64_t channel = Indexes2Channel(indexes);
    channels_->Add(channel, 1);
    static_cast<DerivedT *>(this)->AddQsum(channel, 1);
  }

 private:
  std::array< std::unique_ptr<Binning>, DimensionV > binnings_;
  // Cache for binnings_[i]->num_bins()
  std::array< uint64_t, DimensionV > num_bins_;
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
  friend class Histogram<BorderT, ChannelT, ChannelStoreT, DimensionV>;
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
    qsums_->Add(channel, 1);
  }
 private:
  std::unique_ptr< ChannelStoreBase<ChannelT, ChannelStoreT> > qsums_;
};

#endif
