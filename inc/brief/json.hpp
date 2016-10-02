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
#include <chrono>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#ifdef BRIEF_RTTI
#  include <typeinfo>
#endif
#include <unordered_map>
#include <vector>
#include <experimental/string_view>
#include <experimental/optional>

#include <boost/locale.hpp>
#include <boost/preprocessor.hpp>

#include "brief/serial.hpp"

namespace brief {

#ifndef BRIEF_RTTI
#define BRIEF_JSON_TYPENAME(TYPE) #TYPE
#else
#define BRIEF_JSON_TYPENAME(TYPE) typeid(TYPE).name()
#endif

constexpr auto JSON_INLINE_THRESHOLD = 64;

struct token_t {
  enum type_t : char {
    ARRAY_OPEN, ARRAY_CLOSE,
    OBJECT_OPEN, OBJECT_CLOSE,
    COLON, COMMA,
    STRING, NUMBER, IDENTIFIER
  } type_;
  int line_, col_;
  bool escaped_ = false;
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

template<typename T>
struct json;

class Tokenizer {
 public:
  Tokenizer(const char *_input, const char *_end)
      : input_(_input), cursor_(_input), end_(_end) { }

  bool hasNext() {
    return countTrim() != (size_t) -1;
  }

  token_t next() {
    token_t result = pollCache_.value_or(poll());
    pollCache_ = std::experimental::optional<token_t> {};  // FIXME: My impl don't have reset
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
      default:
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
    pollCache_ = result;
    return result;
  }

  token_t expect(token_t::type_t except) {
    token_t token = next();
    if (token.type_ != except) {
      std::stringstream buf;
      buf << "excepted '" << getTokenTypeSymbol(except) << "' found '" << token.view_ << "'";
      throwError(token.line_, token.col_, buf.str());
    }
    return token;
  }

  void reset() {
    cursor_ = input_;
  }

  int line_ = 0, col_ = 0;

 private:
  const char *input_, *cursor_, *end_;
  std::experimental::optional<token_t> pollCache_;

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
        case '/': {
          char next = *(cur + 1);
          if (next == '/') {
            while (*cur != '\n') {
              cur++;
            }
            cur++;
            continue;
          } else if (next == '*') {
            while (*cur != '*' || *(cur + 1) != '/')
              cur++;
            cur += 2;
            continue;
          }
        }
        default:
          return cur - start;
      }
    }
    return (size_t) - 1;  // Signal eof
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
    const char *cur = start + 1;
    while (cur < end_) {
      char ccur = *cur;
      if (ccur == '"') {
        if (*(cur - 1) != '\\') {
          break;
        } else {
          if (cur - start > 2 && *(cur - 2) == '\\')
            break;
        }
      } else if (ccur == '\\') {
        tok.escaped_ = true;
      }
      cur++;
    }
    return cur - start + 1;
  }

  inline bool isNumberChar(int _c) {
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
    throwError(tok.line_, tok.col_, std::string("can't parse a ") + BRIEF_JSON_TYPENAME(T));
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
#define BRIEF_JSON_BIND_INT(CTYPE, FUNC) \
  template <> \
  struct json<CTYPE> { \
    static void parse(Tokenizer &_tokenizer, CTYPE &_dest) { \
      token_t token = _tokenizer.expect(token_t::type_t::NUMBER); \
      _dest = static_cast<CTYPE>(std::FUNC(token.view_.begin(), nullptr, 10)); \
    } \
    static void serialize(std::ostream &_stream, const CTYPE &_ref, int _indent = 0) { \
      _stream << _ref; \
    } \
  };

BRIEF_JSON_BIND_INT(int8_t, strtol)
BRIEF_JSON_BIND_INT(int16_t, strtol)
BRIEF_JSON_BIND_INT(int32_t, strtol)
BRIEF_JSON_BIND_INT(int64_t, strtol)
BRIEF_JSON_BIND_INT(long long, strtoll)
BRIEF_JSON_BIND_INT(uint8_t, strtoul)
BRIEF_JSON_BIND_INT(uint16_t, strtoul)
BRIEF_JSON_BIND_INT(uint32_t, strtoul)
BRIEF_JSON_BIND_INT(uint64_t, strtoul)
BRIEF_JSON_BIND_INT(unsigned long long, strtoull)

#define BRIEF_JSON_BIND_FLOAT(CTYPE, FUNC) \
  template <> \
  struct json<CTYPE> { \
    static void parse(Tokenizer &_tokenizer, CTYPE &_dest) { \
      token_t token = _tokenizer.expect(token_t::type_t::NUMBER); \
      _dest = std::FUNC(token.view_.begin(), nullptr); \
    } \
    static void serialize(std::ostream &_stream, const CTYPE &_ref, int _indent = 0) { \
      _stream << _ref; \
    } \
  };

BRIEF_JSON_BIND_FLOAT(float, strtof)
BRIEF_JSON_BIND_FLOAT(double, strtod)
BRIEF_JSON_BIND_FLOAT(long double, strtold)

inline std::string json_escape(const std::experimental::string_view &_str) {
  std::string result;
  result.reserve(_str.size());
  const char *cur = _str.begin();
  const char *end = _str.end();
  while (cur != end) {
    char ccur = *cur;
    if (ccur == '"') {
      result += "\\\"";
    } else if (ccur == '\\') {
      result += "\\\\";
    } else if (ccur == '/') {
      result += "\\/";
    } else if (ccur == '\b') {
      result += "\\b";
    } else if (ccur == '\f') {
      result += "\\f";
    } else if (ccur == '\n') {
      result += "\\n";
    } else if (ccur == '\r') {
      result += "\\r";
    } else if (ccur == '\t') {
      result += "\\t";
    } else if (ccur < 0x20) {
      result += "\\u00";
      char tmp = ccur >> 4;
      result.push_back(tmp < 0xA ? tmp + '0' : tmp + 'a');
      tmp = ccur & static_cast<char>(0xF);
      result.push_back(tmp < 0xA ? tmp + '0' : tmp + 'a');
    } else {
      result.push_back(ccur);
    }
    cur++;
  }
  return result;
}

inline char16_t json_unescape_parse(const token_t &_tok, const char *_cur) {
  char buff[4];
  std::copy_n(_cur, 4, buff);
  char16_t conv = 0;
  for (auto i = 0; i < 4; ++i) {
    conv <<= 4;
    char hexChar = buff[i];
    if (hexChar >= '0' && hexChar <= '9')
      conv |= hexChar - '0';
    else if (hexChar >= 'a' && hexChar <= 'f')
      conv |= hexChar - 'a' + 0xA;
    else if (hexChar >= 'A' && hexChar <= 'F')
      conv |= hexChar - 'A' + 0xA;
    else
      throwError(_tok.line_, _tok.col_, std::string("invalid char in escape sequence: ") + hexChar);
  }
  return conv;
}

inline size_t json_unescape_utf32(const token_t &_tok, const char *_cur, std::string &_result) {
  if (std::experimental::string_view {_cur + 4, 2} != "\\u")
    throwError(_tok.line_, _tok.col_, std::string("expected another unicode escape sequence for utf32 char"));

  char16_t code[2];
  code[0] = json_unescape_parse(_tok, _cur);
  code[1] = json_unescape_parse(_tok, _cur + 6);
  auto conv = boost::locale::conv::utf_to_utf<char, char16_t>(code, code + 2);
  _result += conv;
  return 9;
}

inline size_t json_unescape_utf16(const token_t &_tok, const char *_cur, std::string &_result) {
  char16_t code = json_unescape_parse(_tok, _cur);
  if (code < 0x80) {
    _result.push_back(static_cast<char>(code & 0xFF));
  } else {
    _result += boost::locale::conv::utf_to_utf<char, char16_t>(&code, &code + 1);
  }
  return 3;
}

inline std::string json_unescape(const token_t &_tok) {
  std::string result;
  const char *start = _tok.view_.data() + 1;
  const char *end = _tok.view_.data() + _tok.view_.size() - 1;
  result.reserve(end - start);
  const char *cur = start;
  while (cur != end) {
    char ccur = *cur;
    if (ccur == '\\') {
      cur++;
      switch (*cur) {
        case '"':
          result.push_back('"');
          break;
        case '\\':
          result.push_back('\\');
          break;
        case 'b':
          result.push_back('\b');
          break;
        case 'f':
          result.push_back('\f');
          break;
        case 'n':
          result.push_back('\n');
          break;
        case 'r':
          result.push_back('\r');
          break;
        case 't':
          result.push_back('\t');
          break;
        case 'u': {
          ccur = *(++cur);
          if (ccur == '8' || ccur == '9' || (ccur >= 'A' && ccur <= 'F')) {
            cur += json_unescape_utf32(_tok, cur, result);
          } else {
            cur += json_unescape_utf16(_tok, cur, result);
          }
        } break;
        default:
          throwError(_tok.line_, _tok.col_, std::string("unexpected escape sequence: ") + *cur);
      }
    } else {
      result.push_back(*cur);
    }
    cur++;
  }
  return result;
}

template <>
struct json<std::string> {
  static void parse(Tokenizer &_tokenizer, std::string &_dest) {
    token_t token = _tokenizer.expect(token_t::type_t::STRING);
    if (!token.escaped_) {
      _dest.assign(token.view_.data() + 1, token.view_.size() - 2);  // offsets to remove quote characters
    } else {
      _dest = json_unescape(token);
    }
  }
  static void serialize(std::ostream &_stream, const std::string &_ref, int _indent = 0) {
    if (_ref.find('\\') == std::string::npos) {
      _stream << '"' << _ref << '"';
    } else {
      _stream << '"' << json_escape(_ref) << '"';
    }
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
    std::ostringstream buf;
    auto end = std::end(_ref);
    auto last = end - 1;
    for (auto it = std::begin(_ref); it != end; it++) {
      json<T>::serialize(buf, *it, _indent + 1);
      if (it != last)
        buf << ", ";

      if (buf.tellp() > JSON_INLINE_THRESHOLD)
        break;
    }
    auto bufstr = buf.str();

    if (bufstr.size() < JSON_INLINE_THRESHOLD && bufstr.find('\n') == std::string::npos) {
      _stream << bufstr;
    } else {
      _stream << '\n';
      indent(_stream, _indent + 1);
      for (auto it = std::begin(_ref); it != end; it++) {
        json<T>::serialize(_stream, *it, _indent + 1);
        if (it != last) {
          _stream << ",\n";
          indent(_stream, _indent + 1);
        } else {
          _stream << '\n';
          indent(_stream, _indent);
        }
      }
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

  // Use to compare fields to default values
  const V defaultValue {};

  // Pre-serialize a compact form to check if it could be inlined.
  std::ostringstream buf;
  for (auto it = _being; it != _end; ) {
    if (it->second != defaultValue) {
      json<K>::serialize(buf, it->first, _indent + 1);
      buf << ": ";
      json<V>::serialize(buf, it->second, _indent + 1);
      if (++it != _end)
        buf << ", ";
    } else {
      ++it;
    }

    if (buf.tellp() > JSON_INLINE_THRESHOLD)
      break;
  }
  auto bufstr = buf.str();

  if (bufstr.size() < JSON_INLINE_THRESHOLD && bufstr.find('\n') == std::string::npos) {
    _stream << bufstr;
  } else {
    _stream << '\n';
    indent(_stream, _indent + 1);
    for (auto it = _being; it != _end;) {
      // FIXME Defaults to random stuff for primitives, idem in BRIEF_JSON_PROP_SERIALIZE
      if (it->second != defaultValue) {
        json<K>::serialize(_stream, it->first, _indent + 1);
        _stream << ": ";
        json<V>::serialize(_stream, it->second, _indent + 1);
        if (++it != _end) {
          _stream << ",\n";
          indent(_stream, _indent + 1);
        } else {
          _stream << '\n';
          indent(_stream, _indent);
        }
      } else {
        ++it;
      }
    }
  }

  _stream << '}';
}

#define BRIEF_JSON_BIND_MAP(CTYPE) \
template <typename K, typename V> \
struct json<CTYPE<K, V>> { \
  static void parse(Tokenizer &_tokenizer, CTYPE<K, V> &_dest) { \
    parse_map<K, V>(_tokenizer, std::inserter(_dest, _dest.begin())); \
  } \
  static void serialize(std::ostream &_stream, const CTYPE<K, V> &_ref, int _indent = 0) { \
    serialize_map<K, V>(_stream, std::begin(_ref), std::end(_ref), _indent); \
  } \
};

BRIEF_JSON_BIND_MAP(std::map)
BRIEF_JSON_BIND_MAP(std::multimap)
BRIEF_JSON_BIND_MAP(std::unordered_map)
BRIEF_JSON_BIND_MAP(std::unordered_multimap)

#define BRIEF_JSON_PROP_PARSE(R, ELSE, PROP) \
  BOOST_PP_EXPR_IF(ELSE, else) if (_key == BRIEF_PROP_NAME(PROP)) \
    _o. BRIEF_PROP_FIELD(PROP) = brief::parse<BRIEF_PROP_TYPE(PROP)>(_tokenizer);

#define BRIEF_JSON_ENUM_PARSE(R, ELSE, ENUM) \
  BOOST_PP_EXPR_IF(ELSE, else) if (value == BRIEF_ENUM_NAME(ENUM)) \
    _ref = BRIEF_ENUM_VALUE(ENUM);

#define BRIEF_JSON_PROP_SERIALIZE(R, SIZE, I, PROP) \
  if (_ref. BRIEF_PROP_FIELD(PROP) != defaultValue. BRIEF_PROP_FIELD(PROP)) { \
    brief::indent(_stream, _indent + 1); \
    brief::json<std::string>::serialize(_stream, BRIEF_PROP_NAME(PROP), _indent + 1); \
    _stream << ": "; \
    brief::json<BRIEF_PROP_TYPE(PROP)>::serialize(_stream, _ref. BRIEF_PROP_FIELD(PROP), _indent + 1); \
    _stream << ",\n"; \
  }

// #pragma disable goodpractices

#define BRIEF_JSON_PROP_SERIALIZE_INLINE(R, SIZE, I, PROP) \
  if (_ref. BRIEF_PROP_FIELD(PROP) != defaultValue. BRIEF_PROP_FIELD(PROP)) { \
    brief::json<std::string>::serialize(buf, BRIEF_PROP_NAME(PROP), _indent + 1); \
    buf << ": "; \
    brief::json<BRIEF_PROP_TYPE(PROP)>::serialize(buf, _ref. BRIEF_PROP_FIELD(PROP), _indent + 1); \
    buf << ","; \
  } \
  if (buf.tellp() > JSON_INLINE_THRESHOLD) \
    goto abort_inline;

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
        else { \
          throwError(_tokenizer.line_, _tokenizer.col_, std::string("Unknown object key: ") + _key) ;\
        } \
      }); \
    } \
    static void serialize(std::ostream &_stream, const TYPE &_ref, int _indent = 0) { \
      _stream << '{'; \
      const TYPE defaultValue {}; \
      std::ostringstream buf; \
      BOOST_PP_LIST_FOR_EACH_I(BRIEF_JSON_PROP_SERIALIZE_INLINE, \
                               BOOST_PP_ARRAY_SIZE(PROPERTIES), \
                               BOOST_PP_ARRAY_TO_LIST(PROPERTIES)) \
    abort_inline: \
      auto bufstr = buf.str(); \
      if (bufstr.size() < JSON_INLINE_THRESHOLD && bufstr.find('\n') == std::string::npos) { \
        bufstr.resize(bufstr.size() - 1); \
        _stream << bufstr; \
      } else { \
        _stream << '\n'; \
        BOOST_PP_LIST_FOR_EACH_I(BRIEF_JSON_PROP_SERIALIZE, \
                                 BOOST_PP_ARRAY_SIZE(PROPERTIES), \
                                 BOOST_PP_ARRAY_TO_LIST(PROPERTIES)) \
        _stream.seekp(-2, std::ios_base::end); _stream << '\n'; \
        brief::indent(_stream, _indent); \
      } \
      _stream << "}"; \
    } \
  };

#define BRIEF_JSON_ENUM_INTERNAL(TYPE, VALUES) \
  template<> \
  struct json<TYPE> { \
    static void parse(brief::Tokenizer &_tokenizer, TYPE &_ref) { \
      std::string value; \
      json<std::string>::parse(_tokenizer, value); \
      std::transform(value.begin(), value.end(), value.begin(), ::tolower); \
      BRIEF_JSON_ENUM_PARSE(r, 0, BOOST_PP_ARRAY_ELEM(0, VALUES)) \
      BOOST_PP_LIST_FOR_EACH(BRIEF_JSON_ENUM_PARSE, 1, BOOST_PP_ARRAY_TO_LIST(BOOST_PP_ARRAY_POP_FRONT(VALUES))) \
      else { \
        throwError(_tokenizer.line_, _tokenizer.col_, std::string("Unknown enum value: ") + value); \
      } \
    } \
    static void serialize(std::ostream &_stream, const TYPE &_ref, int _indent = 0) { \
      switch (_ref) { \
        BOOST_PP_LIST_FOR_EACH(BRIEF_JSON_ENUM_SERIALIZE, nullptr, BOOST_PP_ARRAY_TO_LIST(VALUES)) \
      } \
    } \
  };

}  // namespace brief

#define BRIEF_JSON(TYPE, PROPERTIES) \
  namespace brief { \
  BRIEF_JSON_INTERNAL(TYPE, PROPERTIES) \
  }  // namespace brief

#define BRIEF_JSON_ENUM(TYPE, VALUES) \
  namespace brief { \
  BRIEF_JSON_ENUM_INTERNAL(TYPE, VALUES) \
  }  // namespace brief
