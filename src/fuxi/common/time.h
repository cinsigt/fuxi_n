
#pragma once

#include <limits>
#include <string>

namespace next {
namespace fuxi {

class Time {
 public:
  static const Time MAX;
  static const Time MIN;
  Time() = default;
  explicit Time(uint64_t nanoseconds);
  explicit Time(int nanoseconds);
  explicit Time(double seconds);
  Time(uint32_t seconds, uint32_t nanoseconds);
  Time(const Time& other);
  Time& operator=(const Time& other);

  /**
   * @brief get the current time.
   *
   * @return return the current time.
   */
  static Time Now();
  static Time MonoTime();

  static uint64_t DurationFromTimespec(struct ::timespec time_out);
  static struct ::timespec ToTimespec(uint64_t ns);

  /**
   * @brief Sleep Until time.
   *
   * @param time the Time object.
   */
  static void SleepUntil(const Time& time);

  /**
   * @brief convert time to second.
   *
   * @return return a double value unit is second.
   */
  double ToSecond() const;

  /**
   * @brief convert time to nanosecond.
   *
   * @return return a unit64_t value unit is nanosecond.
   */
  uint64_t ToNanosecond() const;

  /**
   * @brief convert time to a string.
   *
   * @return return a string.
   */
  std::string ToString() const;

  /**
   * @brief determine if time is 0
   *
   * @return return true if time is 0
   */
  bool IsZero() const;

  bool operator==(const Time& rhs) const;
  bool operator!=(const Time& rhs) const;
  bool operator>(const Time& rhs) const;
  bool operator<(const Time& rhs) const;
  bool operator>=(const Time& rhs) const;
  bool operator<=(const Time& rhs) const;

 private:
  uint64_t nanoseconds_ = 0;
};

std::ostream& operator<<(std::ostream& os, const Time& rhs);

}  // namespace fuxi
}  // namespace next
