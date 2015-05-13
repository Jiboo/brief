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
#include <map>
#include <sstream>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>
#include <experimental/string_view>

namespace brief {
namespace json {

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

static std::string getTokenTypeSymbol(token_t::type_t type) {
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

void throwError(int _line, int _col, const std::string &_message) {
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
};

template <typename T>
void parse(Tokenizer &_tokenizer, T& ref) {
  auto tok = _tokenizer.next();
  throwError(tok.line_, tok.col_, std::string("can't parse a ") + typeid(T).name());
}

template <typename T>
T parse(Tokenizer &_tokenizer) {
  T result;
  parse<T>(_tokenizer, result);
  return result;
}

template <>
void parse<std::string>(Tokenizer &_tokenizer, std::string &_dest) {
  token_t token = _tokenizer.expect(token_t::type_t::STRING);
  // TODO Unescape
  _dest.assign(token.view_.data() + 1, token.view_.size() - 2);  // offsets to remove quote characters
}

template <typename T>
void parse(Tokenizer &_tokenizer, typename std::enable_if<std::is_integral<T>::value, T>::type &_dest) {
  token_t token = _tokenizer.expect(token_t::type_t::NUMBER);
  // TODO Check that atoi result would fit in _dest
  _dest = atoi(std::string(token.view_.data(), token.view_.size()).c_str());
}

template <typename T>
void parse(Tokenizer &_tokenizer, typename std::enable_if<std::is_floating_point<T>::value, T>::type &_dest) {
  token_t token = _tokenizer.next();
  // TODO Check that atof result would fit in _dest
  _dest = atof(std::string(token.view_.data(), token.view_.size()).c_str());
}

template <typename T, class OutputIt>
void parse_vector(Tokenizer &_tokenizer, OutputIt _dest) {
  _tokenizer.expect(token_t::type_t::ARRAY_OPEN);
  token_t next = _tokenizer.poll();
  while (next.type_ != token_t::type_t::ARRAY_CLOSE) {
    T local;
    parse<T>(_tokenizer, local);
    *_dest++ = local;
    next = _tokenizer.poll();
    if (next.type_ != token_t::type_t::ARRAY_CLOSE)
      _tokenizer.expect(token_t::type_t::COMMA);
  }
  _tokenizer.expect(token_t::type_t::ARRAY_CLOSE);
}

template <typename T>
std::vector<T> parse_vector(Tokenizer &_tokenizer) {
  std::vector<T> result;
  parse_vector<T>(_tokenizer, std::back_inserter(result));
  return result;
}

template <typename K>
void parse_object(Tokenizer &_tokenizer, std::function<void(const K&)> _cb) {
  _tokenizer.expect(token_t::type_t::OBJECT_OPEN);
  token_t next = _tokenizer.poll();
  while (next.type_ != token_t::type_t::OBJECT_CLOSE) {
    K key;
    parse<K>(_tokenizer, key);
    _tokenizer.expect(token_t::type_t::COLON);
    _cb(key);
    next = _tokenizer.poll();
    if (next.type_ != token_t::type_t::OBJECT_CLOSE)
      _tokenizer.expect(token_t::type_t::COMMA);
  }
  _tokenizer.expect(token_t::type_t::OBJECT_CLOSE);
}

template <typename K = std::string, typename V, class OutputIt>
void parse_map(Tokenizer &_tokenizer, OutputIt _dest) {
  parse_object<std::string>(_tokenizer, [&_tokenizer, &_dest](const K& key) {
    V value;
    parse<V>(_tokenizer, value);
    *_dest++ = {key, value};
  });
}

template <typename K, typename V>
std::unordered_map<K, V> parse_map(Tokenizer &_tokenizer) {
  std::unordered_map<K, V> result;
  parse_map<K, V>(_tokenizer, std::inserter(result, result.begin()));
  return result;
}

}
}

#define BRIEF_JSON_PARSE_START(TYPE, ARG_0_TYPE, ARG_0_C_NAME, ARG_0_JSON_NAME) \
namespace brief { namespace json { \
template<> \
void parse<TYPE>(brief::json::Tokenizer &_tokenizer, TYPE &_o) { \
  parse_object<std::string>(_tokenizer, [&_tokenizer, &_o] (const std::string &_key) { \
    if (_key == ARG_0_JSON_NAME) \
      _o. ARG_0_C_NAME = brief::json::parse<ARG_0_TYPE>(_tokenizer);

#define BRIEF_JSON_PARSE_ARG(ARG_N_TYPE, ARG_N_C_NAME, ARG_N_JSON_NAME) \
    else if (_key == ARG_N_JSON_NAME) \
      _o. ARG_N_C_NAME = brief::json::parse<ARG_N_TYPE>(_tokenizer); \

#define BRIEF_JSON_PARSE_STOP() \
  }); \
} \
}  /* namespace json */ } /* namespace brief */
