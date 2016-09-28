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

#include <boost/filesystem/path.hpp>

#include "brief/model/repository.hpp"

namespace brief {

class Context;

/**
 * Class responsible for parsing JSON build files, and schedule tasks building.
 */
class Builder {
 public:
  static constexpr auto CACHE_SUFFIX = ".cache";

  explicit Builder(Context &_ctx) : ctx_(_ctx) {}

  void buildCache(const boost::filesystem::path &_repodesc, const std::vector<std::string> &_flavors);
  void loadCachedDesc();
  void loadCachedDesc(const boost::filesystem::path &_repodesc);

  void build(const std::string &_task, const std::vector<std::string> &_flavors);

 private:
  Context &ctx_;
  Repository repo_;
};

}  // namespace brief
