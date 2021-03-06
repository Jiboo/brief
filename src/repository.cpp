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

#include "brief/model/repository.hpp"

namespace brief {

Task Repository::getTask(const std::string &_name) {
  auto it = tasks_.find(_name);
  if (it != tasks_.end())
    return it->second;

  it = exports_.find(_name);
  if (it != exports_.end())
    return it->second;

  throw std::out_of_range(std::string("Unknown task: ") + _name);
}

}  // namespace brief
