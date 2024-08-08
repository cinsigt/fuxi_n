
#include "fuxi/common/time.h"

#include <time.h>

#include <chrono>
#include <iomanip>
#include <limits>
#include <sstream>
#include <thread>

namespace next {
namespace fuxi {

using std::chrono::high_resolution_clock;
using std::chrono::steady_clock;
using std::chrono::system_clock;

const Time Time::MAX = Time(std::numeric_limits<uint64_t>::max());
const Time Time::MIN = Time(1);

Time::Time(uint64_t nanoseconds) { nanoseconds_ = nanoseconds; }

Time::Time(int nanoseconds) {
  nanoseconds_ = static_cast<uint64_t>(nanoseconds);
}

Time::Time(double seconds) {
  nanoseconds_ = static_cast<uint64_t>(seconds * 1000000000UL);
}

Time::Time(uint32_t seconds, uint32_t nanoseconds) {
  nanoseconds_ = static_cast<uint64_t>(seconds) * 1000000000UL + nanoseconds;
}

Time::Time(const Time& other) { nanoseconds_ = other.nanoseconds_; }

Time& Time::operator=(const Time& other) {
  this->nanoseconds_ = other.nanoseconds_;
  return *this;
}

Time Time::Now() {
  auto now = high_resolution_clock::now();
  auto nano_time_point =
      std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
  auto epoch = nano_time_point.time_since_epoch();
  uint64_t now_nano =
      std::chrono::duration_cast<std::chrono::nanoseconds>(epoch).count();
  return Time(now_nano);
}

Time Time::MonoTime() {
  auto now = steady_clock::now();
  auto nano_time_point =
      std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
  auto epoch = nano_time_point.time_since_epoch();
  uint64_t now_nano =
      std::chrono::duration_cast<std::chrono::nanoseconds>(epoch).count();
  return Time(now_nano);
}

static constexpr uint32_t NSEC_PER_SEC = 1000000000UL;
static constexpr uint32_t NSEC_PER_MSEC = 1000000UL;
static constexpr uint32_t NSEC_PER_USEC = 1000UL;

/*static*/ uint64_t Time::DurationFromTimespec(struct ::timespec spec) {
  uint64_t now = (uint64_t)spec.tv_sec * NSEC_PER_SEC + spec.tv_nsec;
  return now;
}

/*static*/ struct ::timespec Time::ToTimespec(uint64_t ns) {
  timespec ts;
  ts.tv_sec = ns / NSEC_PER_SEC;
  ts.tv_nsec = ns - ts.tv_sec * NSEC_PER_SEC;
  return ts;
}

double Time::ToSecond() const {
  return static_cast<double>(nanoseconds_) / 1000000000UL;
}

bool Time::IsZero() const { return nanoseconds_ == 0; }

uint64_t Time::ToNanosecond() const { return nanoseconds_; }

std::string Time::ToString() const {
  auto nano = std::chrono::nanoseconds(nanoseconds_);
  std::chrono::time_point<system_clock, std::chrono::nanoseconds> tp(nano);
  auto time = std::time_t(
      std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch())
          .count());
  struct tm stm;
  auto ret = localtime_r(&time, &stm);
  if (ret == nullptr) {
    return std::to_string(static_cast<double>(nanoseconds_) / 1000000000.0);
  }

  std::stringstream ss;
#if __GNUC__ >= 5
  ss << std::put_time(ret, "%F %T");
  ss << "." << std::setw(9) << std::setfill('0') << nanoseconds_ % 1000000000UL;
#else
  char date_time[128];
  strftime(date_time, sizeof(date_time), "%F %T", ret);
  ss << std::string(date_time) << "." << std::setw(9) << std::setfill('0')
     << nanoseconds_ % 1000000000UL;
#endif
  return ss.str();
}

void Time::SleepUntil(const Time& time) {
  auto nano = std::chrono::nanoseconds(time.ToNanosecond());
  std::chrono::time_point<system_clock, std::chrono::nanoseconds> tp(nano);
  std::this_thread::sleep_until(tp);
}

bool Time::operator==(const Time& rhs) const {
  return nanoseconds_ == rhs.nanoseconds_;
}

bool Time::operator!=(const Time& rhs) const {
  return nanoseconds_ != rhs.nanoseconds_;
}

bool Time::operator>(const Time& rhs) const {
  return nanoseconds_ > rhs.nanoseconds_;
}

bool Time::operator<(const Time& rhs) const {
  return nanoseconds_ < rhs.nanoseconds_;
}

bool Time::operator>=(const Time& rhs) const {
  return nanoseconds_ >= rhs.nanoseconds_;
}

bool Time::operator<=(const Time& rhs) const {
  return nanoseconds_ <= rhs.nanoseconds_;
}

std::ostream& operator<<(std::ostream& os, const Time& rhs) {
  os << rhs.ToString();
  return os;
}

}  // namespace fuxi
}  // namespace next
