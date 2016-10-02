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

#include <chrono>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

namespace brief {

class Context;

/**
 * Implements how to drive a version control system.
 */
class VCS {
 public:
  using Factory = std::function<std::shared_ptr<VCS>(Context&, std::string)>;

  virtual ~VCS() {}
  virtual void reset() = 0;
  virtual void checkout(const std::string &_tag) = 0;
  virtual std::chrono::system_clock::time_point date(const std::string &_revId) = 0;
  virtual void fillTags(std::unordered_map<std::string, Tag> &_dest) = 0;

  virtual std::vector<boost::filesystem::path> diff() = 0;
};

}  // namespace brief
