
#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <inttypes.h>
#include <math.h>
#include <vector>

template <typename BorderType, typename ChannelType>
class Histogram {
 public:
  class Binning {
   public:
     Binning() {

     }
     Binning(BorderType min, BorderType width, BorderType max)
       : min_(min)
       , max_(max)
       , width_(width)
       , max_bin_(ceil((max-min)/width) + 1)
     { }
     Binning(const std::vector<BorderType> &borders)
       : borders_(borders)
       , min_(borders[0])
       , max_(borders.back())
       , width_(0)
       , max_bin_(borders_.size())
     { }

     uint32_t FindBin(BorderType val) const {
       if (val < min_) return 0;
       if (val >= max_) return max_bin_;
       if (width_ > 0) return (val-min_)/width_ + 1;

       uint32_t lo = 0;
       uint32_t hi = max_bin_;
       while (hi-lo > kSteps) {
         uint32_t skip = (hi-lo)/kSteps;
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

     uint32_t max_bin() const { return max_bin_; }
   private:
    static const uint8_t kSteps = 16;

    std::vector<BorderType> borders_;
    BorderType min_;
    BorderType max_;
    BorderType width_;
    uint32_t max_bin_;
  };

  Histogram()
    : dimensions_(0)
    , binnings_(nullptr)
    , max_bins_(nullptr)
    , bins_(nullptr)
  { }
  ~Histogram() {
    //Clear();
  }
  void SetDimensions(const uint8_t dimensions) {
    Clear();
    dimensions_ = dimensions;
    binnings_ = new Binning[dimensions_];
    max_bins_ = new uint32_t[dimensions_];

    //TODO
    bins_ = new ChannelType[max_bins_[dimensions_-1]];
  }
  void SetBinning(const uint8_t dimension, const Binning &binning) {
    max_bins_[dimension-1] = binning.max_bin();
    binnings_[dimension-1] = binning;
  }
  void Fill(BorderType val) {
    uint32_t bin_id = binnings_[0].FindBin(val);
    bins_[bin_id]++;
  }

 private:
  void Clear() {
    delete[] binnings_;
    delete[] max_bins_;
    delete[] bins_;
  }
  uint8_t dimensions_;
  Binning *binnings_;
  uint32_t *max_bins_;

  ChannelType *bins_;
};

#endif
