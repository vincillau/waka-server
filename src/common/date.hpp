// Copyright 2022 Vincil Lau
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <fmt/core.h>

#include <cassert>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <string>

#ifndef WAKA_SRC_COMMON_DATE_HPP_
#define WAKA_SRC_COMMON_DATE_HPP_

namespace waka::common {

class Date {
 public:
  Date(int year, int month, int day) : year_(year), month_(month), day_(day) {}

  [[nodiscard]] int year() const { return year_; }
  [[nodiscard]] int month() const { return month_; }
  [[nodiscard]] int day() const { return day_; }

  Date& operator++() {
    day_++;
    validate();
    return *this;
  }

  Date& operator++(int) {
    day_++;
    validate();
    return *this;
  }

  [[nodiscard]] bool isLeapYear() const {
    if (year_ % 4 != 0) {
      return false;
    } else if (year_ % 400 == 0) {
      return true;
    } else if (year_ % 100 == 0) {
      return false;
    }
    return true;
  }

  [[nodiscard]] std::string toString() const {
    return fmt::format("{:04d}-{:02d}-{:02d}", year_, month_, day_);
  }

  [[nodiscard]] static Date fromUnixMilli(std::int64_t msec) {
    time_t t = msec / 1000;
    struct tm* tm = localtime(&t);
    return {tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday};
  }

  [[nodiscard]] std::int64_t unixMilli() const {
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));
    tm.tm_year = year_ - 1900;
    tm.tm_mon = month_ - 1;
    tm.tm_mday = day_;
    time_t t = mktime(&tm);
    assert(t != -1);
    return static_cast<int64_t>(t) * 1000;
  }

  static Date today() {
    time_t t = time(nullptr);
    struct tm* tm = localtime(&t);
    return {tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday};
  }

 private:
  // 处理进位，使格式正确
  void validate() {
    assert(day_ > 0 && day_ <= 32);
    assert(month_ > 0 && month_ <= 12);
    int month_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    month_days[1] += isLeapYear();
    int month_day = month_days[month_ - 1];
    if (day_ < month_day) {
      return;
    }
    assert(day_ == month_day);
    day_ = 1;
    if (month_ > 12) {
      month_ = 1;
      year_++;
    }
  }

  int year_;
  int month_;
  int day_;
};

}  // namespace waka::common

#endif  // WAKA_SRC_COMMON_DATE_HPP_