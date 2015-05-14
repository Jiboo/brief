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

struct MsgpackType {
  bool b_;
  int i_;
  float f_;
  bool operator ==(const MsgpackType &_other) const {
    return b_ == _other.b_ && i_ == _other.i_ && f_ == _other.f_;
  }
};
BRIEF_MSGPACK(MsgpackType, _.b_, _.i_, _.f_)

TEST(MsgPackReader, CustomTypes) {
  MsgpackType test {false, 42, 3.14};
  std::stringstream output;
  brief::msgpack::writer<MsgpackType>::write(output, test);

  std::stringstream input(output.str());
  MsgpackType read;
  brief::msgpack::reader<MsgpackType>::read(input, read);

  ASSERT_EQ(test, read);
}
