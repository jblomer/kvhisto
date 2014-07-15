
#ifndef BINNING_H
#define BINNING_H

#include <inttypes.h>
#include <math.h>
#include <vector>

template <typename BorderT>
class Binning {
 public:
   Binning()
     : min_(0)
     , max_(0)
     , width_(0)
     , max_bin_(1)
   { }
   Binning(const BorderT min, const BorderT width, const BorderT max)
     : min_(min)
     , max_(max)
     , width_(width)
     , max_bin_(ceil((max-min)/width) + 1)
   {
     assert(width_ > 0);
     assert(min < max);
   }
   explicit Binning(const std::vector<BorderT> &borders)
     : borders_(borders)
     , min_(borders[0])
     , max_(borders.back())
     , width_(0)
     , max_bin_(borders_.size())
   {
     assert(!borders.empty());
   }

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
  static const uint8_t kSteps = 8;

  std::vector<BorderT> borders_;
  BorderT min_;
  BorderT max_;
  BorderT width_;
  uint32_t max_bin_;
};

#endif
