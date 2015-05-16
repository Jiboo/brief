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
#include <experimental/optional>
#include <experimental/string_view>

#include <boost/preprocessor.hpp>

namespace brief {

/**
 * Non conformant implementation, we don't need to share messages, it's just for caching and faster parsing than json.
 *  - Always use host byte order
 *  - Don't handle fixnums and assumes 16bit size containers
 */

template <typename T>
struct msgpack {
  static void write(std::ostream &_stream, const T &_ref) {
    throw std::runtime_error(std::string("can't put a ") + typeid(T).name());
  }
  static void read(std::istream &_stream, T &_ref) {
    throw std::runtime_error(std::string("can't peek a ") + typeid(T).name());
  }
};

template <>
struct msgpack<bool> {
  static void write(std::ostream &_stream, const bool &_ref) {
    _stream.put(_ref ? (uint8_t) 0xC3 : (uint8_t) 0xC2);
  }
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
  struct msgpack<CTYPE> { \
    static void write(std::ostream &_stream, const CTYPE &_ref) { \
      _stream.put(MSGPACK_HEADER); \
      _stream.write(reinterpret_cast<const char*>(&_ref), sizeof(_ref)); \
    } \
    static void read(std::istream &_stream, CTYPE &_ref) { \
      auto c = _stream.get(); \
      if (c != MSGPACK_HEADER) { \
        std::stringstream error; \
        error << "expected " << #CTYPE << ", found : " << std::hex << c; \
        throw std::runtime_error(error.str()); \
      } else { \
        _stream.read(reinterpret_cast<char*>(&_ref), sizeof(_ref)); \
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

inline void write_string(std::ostream &_stream, const char *_ref, uint16_t _size) {
  _stream.put((uint8_t) 0xDA);
  _stream.write(reinterpret_cast<const char*>(&_size), sizeof(_size));
  _stream.write(_ref, _size);
}

inline uint16_t read_string_header(std::istream &_stream) {
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
struct msgpack<std::string> {
  static void write(std::ostream &_stream, const std::string &_ref) {
    if (_ref.size() >= (2 << 16)) {
      std::stringstream buf;
      buf << "string too big (" << _ref.size() << "), max is 2^16 bytes.";
      throw std::runtime_error(buf.str());
    }
    write_string(_stream, _ref.data(), (uint16_t) _ref.size());
  }
  static void read(std::istream &_stream, std::string &_ref) {
    const uint16_t size = read_string_header(_stream);
    _ref.resize(size);
    _stream.read(_ref.begin().base(), size);
  }
};

template <typename T, typename Iter>
void write_array(std::ostream &_stream, Iter _begin, uint16_t _size) {
  _stream.put((uint8_t) 0xDC);
  _stream.write(reinterpret_cast<const char*>(&_size), sizeof(_size));
  for (int i = 0; i < _size; i++) {
    msgpack<T>::write(_stream, *_begin++);
  }
}

inline uint16_t read_array_header(std::istream &_stream) {
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

template <typename T>
struct msgpack<std::vector<T>> {
  static void write(std::ostream &_stream, const std::vector<T> &_ref) {
    if (_ref.size() >= (2 << 16)) {
      std::stringstream buf;
      buf << "array too big (" << _ref.size() << "), max is 2^16 elements.";
      throw std::runtime_error(buf.str());
    }
    write_array<T>(_stream, _ref.begin(), (uint16_t) _ref.size());
  }
  static void read(std::istream &_stream, std::vector<T> &_ref) {
    const uint16_t size = read_array_header(_stream);
    _ref.resize(size);
    for (int i = 0; i < size; i++)
      msgpack<T>::read(_stream, _ref[i]);
  }
};


template <typename K, typename V, typename Iter>
void write_map(std::ostream &_stream, Iter _begin, uint16_t _size) {
  _stream.put((uint8_t) 0xDE);
  _stream.write(reinterpret_cast<const char*>(&_size), sizeof(_size));
  for (int i = 0; i < _size; i++) {
    auto &it = *_begin++;
    msgpack<K>::write(_stream, it.first);
    msgpack<V>::write(_stream, it.second);
  }
}

inline uint16_t read_map_header(std::istream &_stream) {
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
  struct msgpack<CTYPE<K, V>> { \
    static void write(std::ostream &_stream, const CTYPE<K, V> &_ref) { \
      if (_ref.size() >= (2 << 16))  { \
        std::stringstream buf; \
        buf << "map too big (" << _ref.size() << "), max is 2^16 elements."; \
        throw std::runtime_error(buf.str()); \
      }\
      write_map<K, V>(_stream, _ref.begin(), (uint16_t) _ref.size()); \
    } \
    static void read(std::istream &_stream, CTYPE<K, V> &_ref) { \
      const uint16_t size = read_map_header(_stream); \
      for (int i = 0; i < size; i++) { \
        K key; \
        V value; \
        msgpack<K>::read(_stream, key); \
        msgpack<V>::read(_stream, value); \
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
struct msgpack<std::tuple<TParams...>> {
  static void write(std::ostream &_stream, const std::tuple<TParams...> &_ref) {
    for_each(_ref, [&_stream](const auto &x) {
      msgpack<typename std::remove_cv<typename std::remove_reference<decltype(x)>::type>::type>::write(_stream, x);
    });
  }
  static void read(std::istream &_stream, std::tuple<TParams...> &_ref) {
    for_each(_ref, [&_stream](auto &x) {
      msgpack<typename std::remove_reference<decltype(x)>::type>::read(_stream, x);
    });
  }
};

template <typename T>
struct msgpack<std::experimental::optional<T>> {
  static void write(std::ostream &_stream, const std::experimental::optional<T> &_ref) {
    const bool present = static_cast<bool>(_ref);
    msgpack<bool>::write(_stream, present);
    if (present)
      msgpack<T>::write(_stream, _ref.value());
  }
  static void read(std::istream &_stream, std::experimental::optional<T> &_ref) {
    bool present;
    msgpack<bool>::read(_stream, present);
    if (present) {
      T val;
      msgpack<T>::read(_stream, val);
      _ref = val;
    }
  }
};

#ifndef BRIEF_PROP_TYPE
#define BRIEF_PROP_TYPE(TUPLE) BOOST_PP_TUPLE_ELEM(3, 0, TUPLE)
#define BRIEF_PROP_FIELD(TUPLE) BOOST_PP_TUPLE_ELEM(3, 1, TUPLE)
#define BRIEF_PROP_NAME(TUPLE) BOOST_PP_TUPLE_ELEM(3, 2, TUPLE)
#endif

#define BRIEF_MSGPACK_PROP_CONSOLIDATE(R, SIZE, I, TUPLE) \
  _ref. BRIEF_PROP_FIELD(TUPLE) BOOST_PP_COMMA_IF(BOOST_PP_NOT_EQUAL(I, BOOST_PP_SUB(SIZE, 1)))

#define BRIEF_MSGPACK_INTERNAL(TYPE, PROPERTIES) \
  template<> \
  struct msgpack<TYPE> { \
    static void write(std::ostream &_stream, const TYPE &_ref) { \
      auto tuple = std::forward_as_tuple( \
        BOOST_PP_LIST_FOR_EACH_I(BRIEF_MSGPACK_PROP_CONSOLIDATE, BOOST_PP_ARRAY_SIZE(PROPERTIES), \
                                 BOOST_PP_ARRAY_TO_LIST(PROPERTIES))); \
      msgpack<decltype(tuple)>::write(_stream, tuple); \
    } \
    static void read(std::istream &_stream, TYPE &_ref) { \
      auto tuple = std::forward_as_tuple( \
        BOOST_PP_LIST_FOR_EACH_I(BRIEF_MSGPACK_PROP_CONSOLIDATE, BOOST_PP_ARRAY_SIZE(PROPERTIES), \
                                 BOOST_PP_ARRAY_TO_LIST(PROPERTIES))); \
      msgpack<decltype(tuple)>::read(_stream, tuple); \
    } \
  };

#define BRIEF_MSGPACK_FRIENDS_INTERNAL() \
  template<typename T> friend void msgpack<T>::write(std::ostream&, const T&); \
  template<typename T> friend void msgpack<T>::read(std::istream&, T&);

}  // namespace brief

#define BRIEF_MSGPACK(TYPE, PROPERTIES) \
  namespace brief { \
  BRIEF_MSGPACK_INTERNAL(TYPE, PROPERTIES) \
  }  // namespace brief

#define BRIEF_MSGPACK_FRIENDS() \
  template<typename T> friend void brief::msgpack<T>::write(std::ostream&, const T&); \
  template<typename T> friend void brief::msgpack<T>::read(std::istream&, T&);
