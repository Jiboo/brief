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

#include <gtest/gtest.h>

TEST(MsgPack, Primitives) {
  const int kCount = 1000;
  std::stringstream output;
  std::string str = "hello world";
  for(int j = 0; j < kCount; j++) {
    brief::msgpack::writer<int>::write(output, j * 42);
    brief::msgpack::writer<double>::write(output, j * 3.14);
    brief::msgpack::writer<std::string>::write(output, str);
  }

  int i;
  double d;
  std::string s;
  std::stringstream input(output.str());
  for(int j = 0; j < kCount; j++) {
    brief::msgpack::reader<int>::read(input, i);
    brief::msgpack::reader<double>::read(input, d);
    brief::msgpack::reader<std::string>::read(input, s);
    ASSERT_EQ(j * 42, i);
    ASSERT_DOUBLE_EQ(j * 3.14, d);
    ASSERT_EQ("hello world", s);
  }
}

TEST(MsgPack, Arrays) {
  const int kCount = 1000;
  std::vector<int> data(kCount);
  for(int i = 0; i < kCount; i++)
    data[i] = i;

  std::stringstream output;
  brief::msgpack::writer<std::vector<int>>::write(output, data);

  std::vector<int> copy(kCount);
  std::stringstream input(output.str());
  brief::msgpack::reader<std::vector<int>>::read(input, copy);

  ASSERT_EQ(data, copy);
}

TEST(MsgPack, Maps) {
  const int kCount = 1000;
  std::unordered_map<int, int> data;
  for(int i = 0; i < kCount; i++)
    data[i] = i;

  std::stringstream output;
  brief::msgpack::writer<std::unordered_map<int, int>>::write(output, data);

  std::unordered_map<int, int> copy;
  std::stringstream input(output.str());
  brief::msgpack::reader<std::unordered_map<int, int>>::read(input, copy);

  ASSERT_EQ(data, copy);
}

struct MsgpackType {
  bool b_;
  int i_;
  float f_;
  bool operator ==(const MsgpackType &_other) const {
    return b_ == _other.b_ && i_ == _other.i_ && f_ == _other.f_;
  }
};
BRIEF_MSGPACK(MsgpackType, _.b_, _.i_, _.f_)

TEST(MsgPack, CustomTypes) {
  MsgpackType test {false, 42, 3.14};
  std::stringstream output;
  brief::msgpack::writer<MsgpackType>::write(output, test);

  std::stringstream input(output.str());
  MsgpackType read;
  brief::msgpack::reader<MsgpackType>::read(input, read);

  ASSERT_EQ(test, read);
}
