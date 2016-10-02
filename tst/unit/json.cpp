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

#include <gtest/gtest.h>

#include "brief/json.hpp"

TEST(JsonReader, Tokenizer) {
  const std::string trivial = "0\n\n\rlol\t   \t\"test\" 0.484e9 {}[]";
  brief::Tokenizer tok1(trivial.data(), trivial.data() + trivial.size());

  ASSERT_EQ(brief::token_t::type_t::NUMBER, tok1.next().type_);
  ASSERT_EQ(brief::token_t::type_t::IDENTIFIER, tok1.next().type_);
  ASSERT_EQ(brief::token_t::type_t::STRING, tok1.next().type_);
  ASSERT_EQ(brief::token_t::type_t::NUMBER, tok1.next().type_);
  ASSERT_EQ(brief::token_t::type_t::OBJECT_OPEN, tok1.next().type_);
  ASSERT_EQ(brief::token_t::type_t::OBJECT_CLOSE, tok1.next().type_);
  ASSERT_EQ(brief::token_t::type_t::ARRAY_OPEN, tok1.next().type_);
  ASSERT_EQ(brief::token_t::type_t::ARRAY_CLOSE, tok1.next().type_);

  const std::string special = R"special("escaping\nin\nstring" "✓" // single line comments
/* multi line
   comments */ 42
)special";

  brief::Tokenizer tok2(special.data(), special.data() + special.size());

  auto escaped = tok2.next();
  ASSERT_EQ(brief::token_t::type_t::STRING, escaped.type_);
  ASSERT_TRUE(escaped.escaped_);

  auto utf8 = tok2.next();
  ASSERT_EQ(brief::token_t::type_t::STRING, utf8.type_);
  ASSERT_EQ(std::string("\"✓\""), utf8.view_);

  ASSERT_EQ(brief::token_t::type_t::NUMBER, tok2.next().type_);
}

TEST(JsonReader, Parser) {
  const std::string test = "42 3.14 \"test\" [1, 2, 4] {\"a\": 1, \"b\": 2}";
  brief::Tokenizer tok1(test.data(), test.data() + test.size());

  ASSERT_EQ(42, brief::parse<int>(tok1));
  ASSERT_FLOAT_EQ(3.14, brief::parse<float>(tok1));
  ASSERT_EQ("test", brief::parse<std::string>(tok1));

  std::vector<int> v = brief::parse<std::vector<int>>(tok1);
  ASSERT_EQ(1, v[0]);
  ASSERT_EQ(2, v[1]);
  ASSERT_EQ(4, v[2]);

  std::unordered_map<std::string, int> m = brief::parse<std::unordered_map<std::string, int>>(tok1);
  ASSERT_EQ(1, m["a"]);
  ASSERT_EQ(2, m["b"]);

  const std::string escaping = R"end("\u0001\u0012\u0008\u0016\"\\")end";
  brief::Tokenizer tok3(escaping.data(), escaping.data() + escaping.size());

  ASSERT_EQ("\x01\x12\x08\x16\"\\", brief::parse<std::string>(tok3));
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
