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

#include <boost/filesystem/path.hpp>

#include "brief/task.hpp"

namespace brief {

class Context;

/**
 * Class responsible for installing and removing repositories from the trunks directory.
 */
class Trunks {
 public:
  Trunks(Context &_ctx);

  void has(const Dependency &_dep);
  Task retreive(const Dependency &_dep);
  void add(...);
  void remove(...);
  void list(...);
};

}  // namespace brief
