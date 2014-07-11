
#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <inttypes.h>
#include <math.h>
#include <vector>

template <typename Floating>
class Histogram {
 public:
  template<class Derived>
  class Binning {
   public:
     uint32_t FindBin(Floating val) const {
       return static_cast<const Derived *>(this)->FindBinImpl(val);
     }
     uint32_t GetMax() const {
       return static_cast<const Derived *>(this)->GetMaxImpl();
     }
   protected:
    uint32_t FindBinImpl(Floating val) const { return uint32_t(-1); };
    uint32_t GetMaxImpl() const { return 0; };
  };

  class BinningFixed : public Binning<BinningFixed> {
    friend class Binning<BinningFixed>;
   public:
    BinningFixed(Floating min, Floating width, Floating max)
      : min_(min)
      , max_(max)
      , width_(width)
      , max_bin_(ceil((max-min)/width) + 1)
    { }

   protected:
    uint32_t FindBinImpl(Floating val) const {
      if (val < min_) return 0;
      if (val >= max_) return max_bin_;
      return (val-min_)/width_ + 1;
    }
    uint32_t GetMax() const { return max_bin_; }

    const Floating min_;
    const Floating max_;
    const Floating width_;
    const uint32_t max_bin_;
  };

  class BinningDynamic : public Binning<BinningDynamic> {
    friend class Binning<BinningDynamic>;
   public:
    // TODO: move parameter
    BinningDynamic(const std::vector<Floating> &borders)
      : borders_(borders)
      , max_bin_(borders_.size())
    { }

   protected:
    const uint8_t kSteps = 16;
    uint32_t FindBinImpl(Floating val) const {
      if (val < borders_[0]) return 0;
      if (val >= borders_[max_bin_-1]) return max_bin_;

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
    uint32_t GetMax() const { return max_bin_; }
    std::vector<Floating> borders_;
    const uint32_t max_bin_;
  };


  Histogram() { }

};

#endif
