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

#include <map>
#include <unordered_map>
#include <string>

#include <boost/preprocessor.hpp>

namespace brief {

class Tag;
class Task;

using __namedtags = std::unordered_map<std::string, Tag>;
using __strmap = std::unordered_map<std::string, std::string>;
using __namedtasks = std::unordered_map<std::string, Task>;
using __namedtasksmap = std::map<std::string, Task>;

#define BRIEF_PROP_TYPE(TUPLE) BOOST_PP_TUPLE_ELEM(3, 0, TUPLE)
#define BRIEF_PROP_FIELD(TUPLE) BOOST_PP_TUPLE_ELEM(3, 1, TUPLE)
#define BRIEF_PROP_NAME(TUPLE) BOOST_PP_TUPLE_ELEM(3, 2, TUPLE)

#define BRIEF_ENUM_VALUE(TUPLE) BOOST_PP_TUPLE_ELEM(2, 0, TUPLE)
#define BRIEF_ENUM_NAME(TUPLE) BOOST_PP_TUPLE_ELEM(2, 1, TUPLE)

#define BRIEF_EQUALS_CONSOLIDATE(R, DATA, I, TUPLE) \
  BOOST_PP_TUPLE_ELEM(2, 1, DATA) . BRIEF_PROP_FIELD(TUPLE) \
  BOOST_PP_COMMA_IF(BOOST_PP_NOT_EQUAL(I, BOOST_PP_SUB(BOOST_PP_TUPLE_ELEM(2, 0, DATA), 1)))

#define BRIEF_EQUALS_INTERNAL(TYPE, PROPERTIES) \
  inline bool operator==(const TYPE &_left, const TYPE &_right) { \
    auto tuple_left = std::forward_as_tuple( \
        BOOST_PP_LIST_FOR_EACH_I(BRIEF_EQUALS_CONSOLIDATE, \
                                 (BOOST_PP_ARRAY_SIZE(PROPERTIES), _left), \
                                 BOOST_PP_ARRAY_TO_LIST(PROPERTIES))); \
    auto tuple_right = std::forward_as_tuple( \
        BOOST_PP_LIST_FOR_EACH_I(BRIEF_EQUALS_CONSOLIDATE, \
                                 (BOOST_PP_ARRAY_SIZE(PROPERTIES), _right), \
                                 BOOST_PP_ARRAY_TO_LIST(PROPERTIES))); \
    return tuple_left == tuple_right; \
  } \
  inline bool operator!=(const TYPE &_left, const TYPE &_right) { \
    return !(_left == _right); \
  }

#define BRIEF_EQUALS_FRIENDS_INTERNAL(TYPE) \
  friend bool operator==(const TYPE &_left, const TYPE &_right);

}  // namespace brief
