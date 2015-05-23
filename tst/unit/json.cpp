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

#include "brief/json.hpp"

#include <string>
#include <vector>

#include <gtest/gtest.h>

TEST(JsonReader, Tokenizer) {
  const char *input = "0\n\n\rlol\t   \t\"test\" 0.484e9 {}[]";
  brief::Tokenizer tokenizer(input, input + strlen(input));

  ASSERT_EQ(brief::token_t::type_t::NUMBER, tokenizer.next().type_);
  ASSERT_EQ(brief::token_t::type_t::IDENTIFIER, tokenizer.next().type_);
  ASSERT_EQ(brief::token_t::type_t::STRING, tokenizer.next().type_);
  ASSERT_EQ(brief::token_t::type_t::NUMBER, tokenizer.next().type_);
  ASSERT_EQ(brief::token_t::type_t::OBJECT_OPEN, tokenizer.next().type_);
  ASSERT_EQ(brief::token_t::type_t::OBJECT_CLOSE, tokenizer.next().type_);
  ASSERT_EQ(brief::token_t::type_t::ARRAY_OPEN, tokenizer.next().type_);
  ASSERT_EQ(brief::token_t::type_t::ARRAY_CLOSE, tokenizer.next().type_);
}

TEST(JsonReader, Parser) {
  const char *test = "42 3.14 \"test\" [1, 2, 4] {\"a\": 1, \"b\": 2}";
  brief::Tokenizer tokenizer(test, test + strlen(test));

  ASSERT_EQ(42, brief::parse<int>(tokenizer));
  ASSERT_FLOAT_EQ(3.14, brief::parse<float>(tokenizer));
  ASSERT_EQ("test", brief::parse<std::string>(tokenizer));

  std::vector<int> v = brief::parse<std::vector<int>>(tokenizer);
  ASSERT_EQ(1, v[0]);
  ASSERT_EQ(2, v[1]);
  ASSERT_EQ(4, v[2]);

  std::unordered_map<std::string, int> m = brief::parse<std::unordered_map<std::string, int>>(tokenizer);
  ASSERT_EQ(1, m["a"]);
  ASSERT_EQ(2, m["b"]);
}

struct JsonType {
  int a_;
  float b_;
  enum enum_t : uint8_t {
    NONE, TEST, TEST2
  } e_;
  bool operator ==(const JsonType &_other) const {
    return a_ == _other.a_ && b_ == _other.b_ && e_ == _other.e_;
  }
};

#define JsonType_enum_t_VALUES \
  (3, ( \
    (JsonType::enum_t::NONE, "none"), \
    (JsonType::enum_t::TEST, "test"), \
    (JsonType::enum_t::TEST2, "test2")) \
  )
BRIEF_JSON_ENUM(JsonType::enum_t, JsonType_enum_t_VALUES)

#define JsonType_PROPERTIES \
  (3, ( \
    (int, a_, "a"), \
    (float, b_, "b"), \
    (JsonType::enum_t, e_, "e")) \
  )
BRIEF_JSON(JsonType, JsonType_PROPERTIES)

TEST(JsonReader, CustomTypes) {
  const char *test = "{\"a\": 42, \"b\": 3.14, \"e\": \"test\"}";
  brief::Tokenizer tokenizer(test, test + strlen(test));

  JsonType expected = {42, 3.14, JsonType::enum_t::TEST};
  ASSERT_EQ(expected, brief::parse<JsonType>(tokenizer));
}
