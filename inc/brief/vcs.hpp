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

#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include "repo_generated.h"

namespace brief {

class Context;

/**
 * Implements how to drive a version control system.
 */
class VCS {
 public:
  using Factory = std::function<std::shared_ptr<VCS>(Context&, std::string)>;

  virtual void reset() = 0;
  virtual void reset(const std::string &tag) = 0;
  virtual std::string first_tag() = 0;
  virtual std::string last_tag() = 0;
  virtual int compareTags(const std::string &tag1, const std::string &tag2) = 0;

  virtual std::vector<boost::filesystem::path> diff() = 0;
};

}  // namespace brief
