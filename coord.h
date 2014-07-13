
#ifndef COORD_H
#define COORD_H

#include <array>

template <typename T, const uint16_t DimensionV>
class Coordinate {
 public:
  explicit Coordinate(const std::array<T, DimensionV> &values)
    : values(values)
  { }
 private:
  std::array<T, DimensionV> values;
};

template <typename T>
struct Coordinate <T, 1> {
  Coordinate(const T &value)
    : values({{value}})
  { }
  explicit Coordinate(const std::array<T, 1> &values)
    : values(values)
  { }

  std::array<T, 1> values;
};

template <typename T>
struct Coordinate <T, 2> {
  Coordinate(const T &val1, const T &val2)
    : values({{val1, val2}})
  { }
  explicit Coordinate(const std::array<T, 2> &values)
    : values(values)
  { }

  std::array<T, 2> values;
};

template <typename T>
struct Coordinate <T, 3> {
  Coordinate(const T &val1, const T &val2, const T &val3)
    : values({{val1, val2, val3}})
  { }
  explicit Coordinate(const std::array<T, 3> &values)
    : values(values)
  { }

  std::array<T, 3> values;
};

#endif
