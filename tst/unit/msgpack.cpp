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

#include "brief/msgpack.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

TEST(MsgPack, Primitives) {
  const int kCount = 1000;
  std::stringstream output;
  std::string str = "hello world";
  for (int j = 0; j < kCount; j++) {
    brief::msgpack<int>::write(output, j * 42);
    brief::msgpack<double>::write(output, j * 3.14);
    brief::msgpack<std::string>::write(output, str);
  }

  int i;
  double d;
  std::string s;
  std::stringstream input(output.str());
  for (int j = 0; j < kCount; j++) {
    brief::msgpack<int>::read(input, i);
    brief::msgpack<double>::read(input, d);
    brief::msgpack<std::string>::read(input, s);
    ASSERT_EQ(j * 42, i);
    ASSERT_DOUBLE_EQ(j * 3.14, d);
    ASSERT_EQ("hello world", s);
  }
}

TEST(MsgPack, Arrays) {
  const int kCount = 1000;
  std::vector<int> data(kCount);
  for (int i = 0; i < kCount; i++)
    data[i] = i;

  std::stringstream output;
  brief::msgpack<std::vector<int>>::write(output, data);

  std::vector<int> copy(kCount);
  std::stringstream input(output.str());
  brief::msgpack<std::vector<int>>::read(input, copy);

  ASSERT_EQ(data, copy);
}

TEST(MsgPack, Maps) {
  const int kCount = 1000;
  std::unordered_map<int, int> data;
  for (int i = 0; i < kCount; i++)
    data[i] = i;

  std::stringstream output;
  brief::msgpack<std::unordered_map<int, int>>::write(output, data);

  std::unordered_map<int, int> copy;
  std::stringstream input(output.str());
  brief::msgpack<std::unordered_map<int, int>>::read(input, copy);

  ASSERT_EQ(data, copy);
}

struct MsgpackType {
  bool b_;
  int i_;
  float f_;
  std::experimental::optional<int> o_;
  bool operator ==(const MsgpackType &_other) const {
    return b_ == _other.b_ && i_ == _other.i_ && f_ == _other.f_ && o_ == _other.o_;
  }
};
#define MsgpackType_PROPERTIES \
  (4, ( \
    (bool, b_, "b"), \
    (int, i_, "i"), \
    (float, f_, "f"), \
    (std::experimental::optional<int>, o_, "o")) \
  )
BRIEF_MSGPACK(MsgpackType, MsgpackType_PROPERTIES)

TEST(MsgPack, CustomTypes) {
  MsgpackType test {false, 42, 3.14};
  std::stringstream output;
  brief::msgpack<MsgpackType>::write(output, test);

  test.o_ = 15;
  std::stringstream outputWithOptional;
  brief::msgpack<MsgpackType>::write(outputWithOptional, test);

  ASSERT_GE(outputWithOptional.str().size(), output.str().size());

  std::stringstream input(outputWithOptional.str());
  MsgpackType read;
  brief::msgpack<MsgpackType>::read(input, read);

  ASSERT_EQ(test, read);
}
