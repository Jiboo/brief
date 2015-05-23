/*
 * Brief: Hobby build system.
 * Copyright (C) 2015 Jean-Baptiste "Jiboo" Lepesme
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>

#include "brief/json.hpp"

namespace brief {

#define BRIEF_W(LOGGER, OPS) do { LOGGER.w() << OPS << std::endl; } while (false)
#define BRIEF_I(LOGGER, OPS) do { LOGGER.i() << OPS << std::endl; } while (false)
#define BRIEF_V(LOGGER, OPS) do { LOGGER.v() << OPS << std::endl; } while (false)

#ifdef NDEBUG
#define BRIEF_D(LOGGER, OPS) do { } while ( false )
#else
#define BRIEF_D(LOGGER, OPS) do { LOGGER.d() << OPS << std::endl; } while (false)
#endif

#define BRIEF_OS_APPEND_MAP(CTYPE) \
  template <typename K, typename V> \
  inline std::ostream& operator<<(std::ostream &_out, const CTYPE<K, V> &_v) { \
    std::stringstream buf; \
    json<CTYPE<K, V>>::serialize(buf, _v); \
    return _out << buf.str(); \
  }

BRIEF_OS_APPEND_MAP(std::map)
BRIEF_OS_APPEND_MAP(std::multimap)
BRIEF_OS_APPEND_MAP(std::unordered_map)
BRIEF_OS_APPEND_MAP(std::unordered_multimap)

#define BRIEF_OS_APPEND_VEC(CTYPE) \
  template <typename T> \
  inline std::ostream& operator<<(std::ostream &_out, const CTYPE<T> &_v) { \
    std::stringstream buf; \
    json<CTYPE<T>>::serialize(buf, _v); \
    return _out << buf.str(); \
  }

BRIEF_OS_APPEND_VEC(std::vector)
BRIEF_OS_APPEND_VEC(std::list)
BRIEF_OS_APPEND_VEC(std::set)

#define BRIEF_OS_APPEND_TYPE(CTYPE) \
  inline std::ostream& operator<<(std::ostream &_out, const CTYPE &_v) { \
    std::stringstream buf; \
    json<CTYPE>::serialize(buf, _v); \
    return _out << buf.str(); \
  }

class Repository;
class Description;
class Task;
class Dependency;
class TaskFilters;
BRIEF_OS_APPEND_TYPE(Repository)
BRIEF_OS_APPEND_TYPE(Description)
BRIEF_OS_APPEND_TYPE(Task)
BRIEF_OS_APPEND_TYPE(Dependency)
BRIEF_OS_APPEND_TYPE(TaskFilters)

/**
 * Class responsible for logging messages.
 */
class Logger {
 public:
  enum level_t : uint8_t {
    W = 10,
    I = 20,
    V = 30,
    D = 40
  };

  Logger(std::ostream &_stream, level_t _level)
      : target_(_stream), current_(_level) {
    start_ = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(start_);
    v() << "Log start: " << std::ctime(&time);
  }

  std::ostream& operator()(level_t _log_level) {
    if (_log_level <= current_) {
      auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_);
      std::string loglevel;
      switch (_log_level) {
        case W: loglevel = "WARN"; break;
        case I: loglevel = "INFO"; break;
        case V: loglevel = "VERB"; break;
        case D: loglevel = "DBUG"; break;
      }
      return target_ << '[' << std::setw(10) << diff.count() << "] " << loglevel << " ";
    }
    return null_;
  }

  std::ostream& w() { return this->operator()(W); }
  std::ostream& i() { return this->operator()(I); }
  std::ostream& v() { return this->operator()(V); }
  std::ostream& d() { return this->operator()(D); }

 private:
  std::ofstream null_;
  std::ostream& target_;
  level_t current_;
  std::chrono::system_clock::time_point start_;
};

}  // namespace brief
