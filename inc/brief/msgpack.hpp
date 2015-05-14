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

#include <functional>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <experimental/string_view>

namespace brief {
namespace msgpack {

/**
 * Non conformant implementation, we don't need to share messages, it's just for caching and faster parsing than json.
 *  - Always use host byte order
 *  - Don't handle fixnums and assumes 16bit size containers
 */

template <typename T>
struct writer {
  static void write(std::ostream &_stream, const T &_ref) {
    throw std::runtime_error(std::string("can't put a ") + typeid(T).name());
  }
};

template <typename T>
struct reader {
  static void read(std::istream &_stream, T &_ref) {
    throw std::runtime_error(std::string("can't peek a ") + typeid(T).name());
  }
};

template <>
struct writer<bool> {
  static void write(std::ostream &_stream, const bool &_ref) {
    _stream.put(_ref ? (uint8_t) 0xC3 : (uint8_t) 0xC2);
  }
};

template <>
struct reader<bool> {
  static void read(std::istream &_stream, bool &_ref) {
    auto c = _stream.get();
    switch (c) {
      case 0xC3: _ref = true; break;
      case 0xC2: _ref = false; break;
      default:
        std::stringstream error;
        error << "expected bool, found : " << std::hex << c;
        throw std::runtime_error(error.str());
    }
  }
};

#define BRIEF_MSGPACK_BIND_NUMBER(CTYPE, MSGPACK_HEADER) \
template <> \
struct writer<CTYPE> { \
  static void write(std::ostream &_stream, const CTYPE &_ref) { \
    _stream.put(MSGPACK_HEADER); \
    _stream.write(reinterpret_cast<const char*>(&_ref), sizeof (_ref)); \
  } \
}; \
template <> \
struct reader<CTYPE> { \
  static void read(std::istream &_stream, CTYPE &_ref) { \
    auto c = _stream.get(); \
    if (c != MSGPACK_HEADER) { \
      std::stringstream error; \
      error << "expected " << #CTYPE << ", found : " << std::hex << c; \
      throw std::runtime_error(error.str()); \
    } else { \
      _stream.read(reinterpret_cast<char*>(&_ref), sizeof (_ref)); \
    } \
  } \
};

BRIEF_MSGPACK_BIND_NUMBER(uint8_t, 0xCC)
BRIEF_MSGPACK_BIND_NUMBER(uint16_t, 0xCD)
BRIEF_MSGPACK_BIND_NUMBER(uint32_t, 0xCE)
BRIEF_MSGPACK_BIND_NUMBER(uint64_t, 0xCF)
BRIEF_MSGPACK_BIND_NUMBER(int8_t, 0xD0)
BRIEF_MSGPACK_BIND_NUMBER(int16_t, 0xD1)
BRIEF_MSGPACK_BIND_NUMBER(int32_t, 0xD2)
BRIEF_MSGPACK_BIND_NUMBER(int64_t, 0xD3)
BRIEF_MSGPACK_BIND_NUMBER(float, 0xCA)
BRIEF_MSGPACK_BIND_NUMBER(double, 0xCB)

void write_string(std::ostream &_stream, const char *_ref, uint16_t _size) {
  _stream.put((uint8_t) 0xDA);
  _stream.write(reinterpret_cast<const char*>(&_size), sizeof (_size));
  _stream.write(_ref, _size);
}

#define BRIEF_MSGPACK_BIND_STRING_WRITE(CTYPE) \
template <> \
struct writer<CTYPE> { \
  static void write(std::ostream &_stream, const CTYPE &_ref) { \
    if (_ref.size() >= (2^16)) { \
      throw std::runtime_error("string too big, max is 2^16 bytes."); \
    } \
    write_string(_stream, _ref.data(), (uint16_t) _ref.size()); \
  } \
};

BRIEF_MSGPACK_BIND_STRING_WRITE(std::string)
BRIEF_MSGPACK_BIND_STRING_WRITE(std::experimental::string_view)

uint16_t read_string_header(std::istream &_stream) {
  auto c = _stream.get();
  if (c != 0xDA) {
    std::stringstream error;
    error << "expected string, found : " << std::hex << c;
    throw std::runtime_error(error.str());
  } else {
    uint16_t size;
    _stream.read(reinterpret_cast<char*>(&size), sizeof(size));
    return size;
  }
}

template <>
struct reader<std::string> {
  static void read(std::istream &_stream, std::string &_ref) {
    const uint16_t size = read_string_header(_stream);
    _ref.reserve(size);
    _stream.read(_ref.begin().base(), size);
  }
};

template <typename T, typename Iter>
void write_array(std::ostream &_stream, Iter _begin, uint16_t _size) {
  _stream.put((uint8_t) 0xDC);
  _stream.write(reinterpret_cast<const char*>(&_size), sizeof (_size));
  for (int i = 0; i < _size; i++) {
    writer<T>::write(_stream, *_begin++);
  }
}

uint16_t read_array_header(std::istream &_stream) {
  auto c = _stream.get();
  if (c != 0xDC) {
    std::stringstream error; \
    error << "expected string, found : " << std::hex << c;
    throw std::runtime_error(error.str());
  } else {
    uint16_t size;
    _stream.read(reinterpret_cast<char*>(&size), sizeof(size));
    return size;
  }
}

#define BRIEF_MSGPACK_BIND_ARRAY(CTYPE) \
template <typename T> \
struct writer<CTYPE<T>> { \
  static void write(std::ostream &_stream, const CTYPE<T> &_ref) { \
    if (_ref.size() >= (2^16)) \
      throw std::runtime_error("string too big, max is 2^16 bytes."); \
    write_array(_stream, std::back_inserter(_ref), (uint16_t) _ref.size()); \
  } \
}; \
template <typename T> \
struct reader<CTYPE<T>> { \
  static void read(std::istream &_stream, CTYPE<T> &_ref) { \
    if (_ref.size() >= (2^16)) \
      throw std::runtime_error("string too big, max is 2^16 bytes."); \
    const uint16_t size = read_array_header(_stream); \
    _ref.reserve(size); \
    for (int i = 0; i < size; i++) { \
      reader<T>::read(_stream, _ref[i]); \
    } \
  } \
};

BRIEF_MSGPACK_BIND_ARRAY(std::vector)

template <typename K, typename V, typename Iter>
void write_map(std::ostream &_stream, Iter _begin, uint16_t _size) {
  _stream.put((uint8_t) 0xDE);
  _stream.write(reinterpret_cast<const char*>(&_size), sizeof (_size));
  for (int i = 0; i < _size; i++) {
    auto it = *_begin++;
    writer<K>::write(_stream, it->first);
    writer<V>::write(_stream, it->second);
  }
}

uint16_t read_map_header(std::istream &_stream) {
  auto c = _stream.get();
  if (c != 0xDE) {
    std::stringstream error; \
    error << "expected string, found : " << std::hex << c;
    throw std::runtime_error(error.str());
  } else {
    uint16_t size;
    _stream.read(reinterpret_cast<char*>(&size), sizeof(size));
    return size;
  }
}

#define BRIEF_MSGPACK_BIND_MAP(CTYPE) \
template <typename K, typename V> \
struct writer<CTYPE<K, V>> { \
  static void write(std::ostream &_stream, const CTYPE<K, V> &_ref) { \
    if (_ref.size() >= (2^16)) \
      throw std::runtime_error("string too big, max is 2^16 bytes."); \
    write_map<K, V>(_stream, _ref.begin(), (uint16_t) _ref.size()); \
  } \
}; \
template <typename K, typename V> \
struct reader<CTYPE<K, V>> { \
  static void read(std::istream &_stream, CTYPE<K, V> &_ref) { \
    if (_ref.size() >= (2^16)) \
      throw std::runtime_error("string too big, max is 2^16 bytes."); \
    const uint16_t size = read_map_header(_stream); \
    for (int i = 0; i < size; i++) { \
      K key; \
      V value; \
      reader<K>::read(_stream, key); \
      reader<V>::read(_stream, value); \
      _ref[key] = value; \
    } \
  } \
};

BRIEF_MSGPACK_BIND_MAP(std::map)
BRIEF_MSGPACK_BIND_MAP(std::unordered_map)

template <typename Tuple, typename F, std::size_t ...Indices>
void for_each_impl(Tuple&& tuple, F&& f, std::index_sequence<Indices...>) {
  using swallow = int[];
  (void)swallow{1, (f(std::get<Indices>(std::forward<Tuple>(tuple))), void(), int{})...};
}

template <typename Tuple, typename F>
void for_each(Tuple&& tuple, F&& f) {
  constexpr std::size_t N = std::tuple_size<std::remove_reference_t<Tuple>>::value;
  for_each_impl(std::forward<Tuple>(tuple), std::forward<F>(f), std::make_index_sequence<N>{});
}

template<typename... TParams>
struct writer<std::tuple<TParams...>> {
  static void write(std::ostream &_stream, const std::tuple<TParams...> &_ref) {
    for_each(_ref, [&_stream](const auto &x) {
      writer<typename std::remove_cv<typename std::remove_reference<decltype(x)>::type>::type>::write(_stream, x);
    });
  }
};

template<typename... TParams>
struct reader<std::tuple<TParams...>> {
  static void read(std::istream &_stream, std::tuple<TParams...> &_ref) {
    for_each(_ref, [&_stream](auto &x) {
      reader<typename std::remove_reference<decltype(x)>::type>::read(_stream, x);
    });
  }
};

}  // namespace msgpack
}  // namespace brief

#define BRIEF_MSGPACK(TYPE, ...) \
namespace brief { namespace msgpack { \
template<> \
struct writer<TYPE> { \
  static void write(std::ostream &_stream, const TYPE &_) { \
    auto tuple = std::forward_as_tuple(__VA_ARGS__); \
    writer<decltype(tuple)>::write(_stream, tuple); \
  } \
}; \
template<> \
struct reader<TYPE> { \
  static void read(std::istream &_stream, TYPE &_) { \
    auto tuple = std::forward_as_tuple(__VA_ARGS__); \
    reader<decltype(tuple)>::read(_stream, tuple); \
  } \
}; \
}  /* namespace msgpack */ } /* namespace brief */
