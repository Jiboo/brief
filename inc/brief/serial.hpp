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

#include <boost/preprocessor.hpp>

namespace brief {

class Tag;
class Task;

using __namedtag = std::unordered_map<std::string, Tag>;
using __strmap = std::unordered_map<std::string, std::string>;
using __namedtasks = std::unordered_map<std::string, Task>;
using __namedtasksmap = std::map<std::string, Task>;

#define BRIEF_PROP_TYPE(TUPLE) BOOST_PP_TUPLE_ELEM(3, 0, TUPLE)
#define BRIEF_PROP_FIELD(TUPLE) BOOST_PP_TUPLE_ELEM(3, 1, TUPLE)
#define BRIEF_PROP_NAME(TUPLE) BOOST_PP_TUPLE_ELEM(3, 2, TUPLE)

#define BRIEF_ENUM_VALUE(TUPLE) BOOST_PP_TUPLE_ELEM(2, 0, TUPLE)
#define BRIEF_ENUM_NAME(TUPLE) BOOST_PP_TUPLE_ELEM(2, 1, TUPLE)

}  // namespace brief
