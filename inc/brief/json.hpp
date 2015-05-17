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

#include <algorithm>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>
#include <experimental/string_view>

#include <boost/preprocessor.hpp>

#include "brief/serial.hpp"

namespace brief {

struct token_t {
  enum type_t : char {
    ARRAY_OPEN, ARRAY_CLOSE,
    OBJECT_OPEN, OBJECT_CLOSE,
    COLON, COMMA,
    STRING, NUMBER, IDENTIFIER
  } type_;
  int line_, col_;
  bool escaped_ = false;  // true if string has any escaped characters (can't use as string_view)
  std::experimental::string_view view_;
};

inline std::string getTokenTypeSymbol(token_t::type_t type) {
  switch (type) {
    case token_t::ARRAY_OPEN: return "[";
    case token_t::ARRAY_CLOSE: return "]";
    case token_t::OBJECT_OPEN: return "{";
    case token_t::OBJECT_CLOSE: return "}";
    case token_t::COLON: return ":";
    case token_t::COMMA: return ",";
    case token_t::STRING: return "string";
    case token_t::NUMBER: return "number";
    case token_t::IDENTIFIER: return "identifier";
  }
  throw std::runtime_error("unknown token type");
}

inline void throwError(int _line, int _col, const std::string &_message) {
  std::stringstream buf;
  buf << "At " << _line << ":" << _col << " : " << _message << ".";
  throw std::runtime_error(buf.str());
}

class Tokenizer {
 public:
  Tokenizer(const char *_input, const char *_end)
      : input_(_input), cursor_(_input), end_(_end) { }

  bool hasNext() {
    return countTrim() != (size_t) -1;
  }

  token_t next() {
    skipSpaces();
    auto result = poll();
    const size_t size = result.view_.size();
    col_ += size;
    cursor_ += size;
    return result;
  }

  token_t poll() {
    skipSpaces();

    token_t result;
    result.col_ = col_;
    result.line_ = line_;
    size_t count = 1;
    const char *start = cursor_;

    switch (*start) {
      case '[': result.type_ = token_t::ARRAY_OPEN; break;
      case ']': result.type_ = token_t::ARRAY_CLOSE; break;
      case '{': result.type_ = token_t::OBJECT_OPEN; break;
      case '}': result.type_ = token_t::OBJECT_CLOSE; break;
      case ':': result.type_ = token_t::COLON; break;
      case ',': result.type_ = token_t::COMMA; break;
      case '"':  // string
        result.type_ = token_t::STRING;
        count = countString(result);
        break;
      case '-': case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        result.type_ = token_t::NUMBER;
        count = countNumber();
        break;
      case 't': case 'f': case 'n':  // keywords
      default:  // FIXME Unsupported in strict json
        if (!isprint(*start)) {
          throwError(line_, col_, std::string("unexpected caracter: '") + *start + "'");
        }
        result.type_ = token_t::IDENTIFIER;
        count = countIdentifier();
        break;
    }
    if (count <= 0) {
      throwError(line_, col_, "couldn't count token size");
    }
    if (cursor_ >= end_) {
      throwError(line_, col_, "unexpected end of input");
    }
    result.view_ = std::experimental::string_view(start, count);
    return result;
  }

  token_t expect(token_t::type_t except) {
    token_t token = next();
    if (token.type_ != except) {
      std::stringstream buf;
      buf << "excepted " << getTokenTypeSymbol(except) << " found " << token.view_;
      throwError(token.line_, token.col_, buf.str());
    }
    return token;
  }

  void reset() {
    cursor_ = input_;
  }

 private:
  const char *input_, *cursor_, *end_;
  int line_ = 0, col_ = 0;

  void skipSpaces() {
    size_t spaces = countTrim();
    if (spaces == (size_t)  -1)
      throwError(line_, col_, "unexpected end of input");
    cursor_ += spaces;
  }

  size_t countTrim() {
    const char *start = cursor_;
    const char *cur = start;
    while (cur != end_) {
      switch (*cur) {
        default:
          return cur - start;
        case ' ': case '\t':
          col_++;
          cur++;
          continue;
        case '\n': case '\v': case '\f':
          col_ = 0;
          line_++;
          // no break intended
        case '\r':
          cur++;
          continue;
      }
    }
    return (size_t) -1;  // Signal eof
  }

  size_t countIdentifier() {
    const char *start = cursor_;
    const char *cur = start;
    while (isalpha(*cur) && cur < end_) {
      cur++;
    }
    return cur - start;
  }

  size_t countString(token_t &tok) {
    const char *start = cursor_;
    const char *cur = start;
    const char prev = *cur++;
    while (*cur != '"' && prev != '\\' && cur < end_) {
      if (*cur == '\\')
        tok.escaped_ = true;
      cur++;
    }
    return cur - start + 1;
  }

  bool isNumberChar(int _c) {
    return isdigit(_c) || _c == '-' || _c == '.' || _c == 'e' || _c == 'E' || _c == '+';
  }

  size_t countNumber() {
    const char *start = cursor_;
    const char *cur = start;
    while (isNumberChar(*cur) && cur < end_) {  // We'll let the format checking to parser.
      cur++;
    }
    return cur - start;
  }
};  // class Tokenizer


template <typename T>
struct json {
  static void parse(Tokenizer &_tokenizer, T &_ref) {
    auto tok = _tokenizer.next();
    throwError(tok.line_, tok.col_, std::string("can't parse a ") + typeid(T).name());
  }
  static void serialize(std::ostream &_stream, const T &_ref, int _indent = 0) {
    _stream << _ref;
  }
};

template <typename T>
T parse(Tokenizer &_tokenizer) {
  T value;
  json<T>::parse(_tokenizer, value);
  return value;
}

// TODO Check that atoi result would fit in _dest
#define BRIEF_JSON_BIND_INT(CTYPE) \
  template <> \
  struct json<CTYPE> { \
    static void parse(Tokenizer &_tokenizer, CTYPE &_dest) { \
      token_t token = _tokenizer.expect(token_t::type_t::NUMBER); \
      _dest = atoi(std::string(token.view_.data(), token.view_.size()).c_str()); \
    } \
    static void serialize(std::ostream &_stream, const CTYPE &_ref, int _indent = 0) { \
      _stream << _ref; \
    } \
  };

BRIEF_JSON_BIND_INT(uint8_t)
BRIEF_JSON_BIND_INT(uint16_t)
BRIEF_JSON_BIND_INT(uint32_t)
BRIEF_JSON_BIND_INT(uint64_t)
BRIEF_JSON_BIND_INT(int8_t)
BRIEF_JSON_BIND_INT(int16_t)
BRIEF_JSON_BIND_INT(int32_t)
BRIEF_JSON_BIND_INT(int64_t)

#define BRIEF_JSON_BIND_FLOAT(CTYPE) \
  template <> \
  struct json<CTYPE> { \
    static void parse(Tokenizer &_tokenizer, CTYPE &_dest) { \
      token_t token = _tokenizer.expect(token_t::type_t::NUMBER); \
      _dest = atof(std::string(token.view_.data(), token.view_.size()).c_str()); \
    } \
    static void serialize(std::ostream &_stream, const CTYPE &_ref, int _indent = 0) { \
      _stream << _ref; \
    } \
  };

BRIEF_JSON_BIND_FLOAT(float)
BRIEF_JSON_BIND_FLOAT(double)

template <>
struct json<std::string> {
  static void parse(Tokenizer &_tokenizer, std::string &_dest) {
    token_t token = _tokenizer.expect(token_t::type_t::STRING);
    // TODO Unescape
    _dest.assign(token.view_.data() + 1, token.view_.size() - 2);  // offsets to remove quote characters
  }
  static void serialize(std::ostream &_stream, const std::string &_ref, int _indent = 0) {
    // TODO Escaping
    _stream << '"' << _ref << '"';
  }
};

inline void indent(std::ostream &_stream, int _indent) {
  for (int i = 0; i < _indent; i++)
    _stream << "  ";
}

template <typename T>
struct json<std::vector<T>> {
  static void parse(Tokenizer &_tokenizer, std::vector<T> &_dest) {
    _tokenizer.expect(token_t::type_t::ARRAY_OPEN);
    token_t next = _tokenizer.poll();
    while (next.type_ != token_t::type_t::ARRAY_CLOSE) {
      T local;
      json<T>::parse(_tokenizer, local);
      _dest.emplace_back(std::move(local));
      next = _tokenizer.poll();
      // FIXME The or is for objects with a trailing , before }
      if (next.type_ != token_t::type_t::ARRAY_CLOSE || next.type_ == token_t::type_t::COMMA) {
        _tokenizer.expect(token_t::type_t::COMMA);
        next = _tokenizer.poll();
      }
    }
    _tokenizer.expect(token_t::type_t::ARRAY_CLOSE);
  }
  static void serialize(std::ostream &_stream, const std::vector<T> &_ref, int _indent = 0) {
    _stream << '[';

    // Pre-serialize a compact form to check if could be inlined.
    std::stringstream buf;
    auto end = std::end(_ref);
    auto last = end - 1;
    for (auto it = std::begin(_ref); it != end; it++) {
      json<T>::serialize(buf, *it, _indent + 1);
      if (it != last)
        buf << ", ";
    }
    auto bufstr = buf.str();

    if (bufstr.size() < 64) {
      _stream << bufstr;
    } else {
      _stream << '\n';
      indent(_stream, _indent + 1);
      for (auto it = std::begin(_ref); it != end; it++) {
        json<T>::serialize(_stream, *it, _indent + 1);
        if (it != last) {
          _stream << ",\n";
          indent(_stream, _indent + 1);
        }
      }
      indent(_stream, _indent);
    }

    _stream << ']';
  }
};

template <typename K>
void parse_object(Tokenizer &_tokenizer, std::function<void(const K&)> _cb) {
  _tokenizer.expect(token_t::type_t::OBJECT_OPEN);
  token_t next = _tokenizer.poll();
  while (next.type_ != token_t::type_t::OBJECT_CLOSE) {
    K key;
    json<K>::parse(_tokenizer, key);
    _tokenizer.expect(token_t::type_t::COLON);
    _cb(key);
    next = _tokenizer.poll();
    // FIXME The or is for objects with a trailing , before }
    if (next.type_ != token_t::type_t::OBJECT_CLOSE || next.type_ == token_t::type_t::COMMA) {
      _tokenizer.expect(token_t::type_t::COMMA);
      next = _tokenizer.poll();
    }
  }
  _tokenizer.expect(token_t::type_t::OBJECT_CLOSE);
}

template <typename K, typename V, class OutputIt>
void parse_map(Tokenizer &_tokenizer, OutputIt _dest) {
  parse_object<K>(_tokenizer, [&_tokenizer, &_dest](const K& key) {
    V value;
    json<V>::parse(_tokenizer, value);
    *_dest++ = {key, value};
  });
}
template <typename K, typename V, class InputIt>
static void serialize_map(std::ostream &_stream, const InputIt _being, const InputIt _end, int _indent = 0) {
  _stream << '{';

  // Pre-serialize a compact form to check if could be inlined.
  std::stringstream buf;
  for (auto it = _being; it != _end; ) {
    const V defaultValue;  // Don't serialize if default values, useful for Task and Description
    if (it->second != defaultValue) {
      json<K>::serialize(buf, it->first, _indent + 1);
      buf << ": ";
      json<V>::serialize(buf, it->second, _indent + 1);
      if (++it != _end)
        buf << ", ";
    } else {
      ++it;
    }
  }
  auto bufstr = buf.str();

  if (bufstr.size() < 64) {
    _stream << bufstr;
  } else {
    _stream << '\n';
    indent(_stream, _indent + 1);
    for (auto it = _being; it != _end;) {
      const V defaultValue;  // Don't serialize if default values, useful for Task and Description
      // FIXME Defaults to random stuff for primitives, idem in BRIEF_JSON_PROP_SERIALIZE
      if (it->second != defaultValue) {
        json<K>::serialize(_stream, it->first, _indent + 1);
        _stream << ": ";
        json<V>::serialize(_stream, it->second, _indent + 1);
        if (++it != _end) {
          _stream << ",\n";
          indent(_stream, _indent + 1);
        }
      } else {
        ++it;
      }
    }
    indent(_stream, _indent);
  }

  _stream << '}';
}

template <typename K, typename V>
struct json<std::unordered_map<K, V>> {
  static void parse(Tokenizer &_tokenizer, std::unordered_map<K, V> &_dest) {
    parse_map<K, V>(_tokenizer, std::inserter(_dest, _dest.begin()));
  }
  static void serialize(std::ostream &_stream, const std::unordered_map<K, V> &_ref, int _indent = 0) {
    serialize_map<K, V>(_stream, std::begin(_ref), std::end(_ref), _indent);
  }
};

template <typename K, typename V>
struct json<std::map<K, V>> {
  static void parse(Tokenizer &_tokenizer, std::map<K, V> &_dest) {
    parse_map<K, V>(_tokenizer, std::inserter(_dest, _dest.begin()));
  }
  static void serialize(std::ostream &_stream, const std::map<K, V> &_ref, int _indent = 0) {
    serialize_map<K, V>(_stream, std::begin(_ref), std::end(_ref), _indent);
  }
};

#define BRIEF_JSON_FRIENDS_INTERNAL() \
  template<typename T> friend void json<T>::parse(Tokenizer &_tokenizer, T&); \
  template<typename T> friend void json<T>::serialize(std::ostream &_stream, const T&, int _indent);

#define BRIEF_JSON_PROP_PARSE(R, ELSE, PROP) \
  BOOST_PP_EXPR_IF(ELSE, else) if (_key == BRIEF_PROP_NAME(PROP)) \
    _o. BRIEF_PROP_FIELD(PROP) = brief::parse<BRIEF_PROP_TYPE(PROP)>(_tokenizer);

#define BRIEF_JSON_ENUM_PARSE(R, ELSE, ENUM) \
  BOOST_PP_EXPR_IF(ELSE, else) if (value == BRIEF_ENUM_NAME(ENUM)) \
    _ref = BRIEF_ENUM_VALUE(ENUM);

#define BRIEF_JSON_PROP_SERIALIZE(R, SIZE, I, PROP) \
  if (_ref. BRIEF_PROP_FIELD(PROP) != BRIEF_PROP_TYPE(PROP) ()) { \
    brief::indent(_stream, _indent + 1); \
    brief::json<std::string>::serialize(_stream, BRIEF_PROP_NAME(PROP), _indent + 1); \
    _stream << ": "; \
    brief::json<BRIEF_PROP_TYPE(PROP)>::serialize(_stream, _ref. BRIEF_PROP_FIELD(PROP), _indent + 1); \
    _stream << BOOST_PP_EXPR_IF(BOOST_PP_NOT_EQUAL(I, BOOST_PP_SUB(SIZE, 1)), ",") "\n"; \
  }

#define BRIEF_JSON_ENUM_SERIALIZE(R, UNUSED, ENUM) \
  case BRIEF_ENUM_VALUE(ENUM): \
    brief::json<std::string>::serialize(_stream, BRIEF_ENUM_NAME(ENUM), _indent + 1); \
    break;

#define BRIEF_JSON_INTERNAL(TYPE, PROPERTIES) \
  template<> \
  struct json<TYPE> { \
    static void parse(brief::Tokenizer &_tokenizer, TYPE &_o) { \
      parse_object<std::string>(_tokenizer, [&_tokenizer, &_o] (const std::string &_key) { \
        BRIEF_JSON_PROP_PARSE(r, 0, BOOST_PP_ARRAY_ELEM(0, PROPERTIES)) \
        BOOST_PP_LIST_FOR_EACH(BRIEF_JSON_PROP_PARSE, 1, BOOST_PP_ARRAY_TO_LIST(BOOST_PP_ARRAY_POP_FRONT(PROPERTIES))) \
      }); \
    } \
    static void serialize(std::ostream &_stream, const TYPE &_ref, int _indent = 0) { \
      _stream << "{\n"; \
      BOOST_PP_LIST_FOR_EACH_I(BRIEF_JSON_PROP_SERIALIZE, \
                               BOOST_PP_ARRAY_SIZE(PROPERTIES), \
                               BOOST_PP_ARRAY_TO_LIST(PROPERTIES)) \
      brief::indent(_stream, _indent); _stream << "}"; \
    } \
  };

#define BRIEF_JSON_ENUM_INTERNAL(TYPE, VALUES) \
  template<> \
  struct json<TYPE> { \
    static void parse(brief::Tokenizer &_tokenizer, TYPE &_ref) { \
      std::string value; \
      json<std::string>::parse(_tokenizer, value); \
      std::transform(value.begin(), value.end(), value.begin(), ::toupper); \
      BRIEF_JSON_ENUM_PARSE(r, 0, BOOST_PP_ARRAY_ELEM(0, VALUES)) \
      BOOST_PP_LIST_FOR_EACH(BRIEF_JSON_ENUM_PARSE, 1, BOOST_PP_ARRAY_TO_LIST(BOOST_PP_ARRAY_POP_FRONT(VALUES))) \
    } \
    static void serialize(std::ostream &_stream, const TYPE &_ref, int _indent = 0) { \
      switch (_ref) { \
        BOOST_PP_LIST_FOR_EACH(BRIEF_JSON_ENUM_SERIALIZE, nullptr, BOOST_PP_ARRAY_TO_LIST(VALUES)) \
      } \
    } \
  };

}  // namespace brief

#define BRIEF_JSON_FRIENDS() \
  template<typename T> friend void brief::json<T>::parse(brief::json::Tokenizer &_tokenizer, T&); \
  template<typename T> friend void brief::json<T>::serialize(std::ostream &_stream, const T&, int _indent);

#define BRIEF_JSON(TYPE, PROPERTIES) \
  namespace brief { \
  BRIEF_JSON_INTERNAL(TYPE, PROPERTIES) \
  }  // namespace brief

#define BRIEF_JSON_ENUM(TYPE, VALUES) \
  namespace brief { \
  BRIEF_JSON_ENUM_INTERNAL(TYPE, VALUES) \
  }  // namespace brief
