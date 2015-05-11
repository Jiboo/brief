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

#include <functional>
#include <regex>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <boost/filesystem/path.hpp>

#include "repo_generated.h"
#include "brief/toolchain.hpp"
#include "brief/vcs.hpp"

namespace brief {

class Context {
 public:
  Context();

  using PrefixCallback = std::function<std::string(const Repository& repo, const Task& task, const std::string &)>;

  void registerToolchain(const std::string &name, const Toolchain::Factory &factory);
  void registerVCSHandling(const std::regex &pattern, const VCS::Factory &factory);

  void registerVar(const std::string &name, const std::string &value);
  void registerVarPrefix(const std::string &prefix, PrefixCallback cb);
  std::string preprocessString(const Repository& repo, const Task& task, const std::string &value);
  std::string lookupVar(const Repository& repo, const Task& task, const std::string &name);

 private:
  std::unordered_map<std::string, Toolchain::Factory> toolchainFactories;
  std::unordered_map<std::string, PrefixCallback> varPrefixes;
  std::unordered_map<std::string, std::string> knownVars;
  std::vector<std::tuple<std::regex, VCS::Factory>> vcsFactories;
};

}  // namespace brief
