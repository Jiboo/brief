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

#include "brief/repository.hpp"
#include "brief/toolchain.hpp"
#include "brief/vcs.hpp"
#include "brief/builder.hpp"
#include "brief/trunks.hpp"
#include "brief/logger.hpp"

namespace brief {

class Context {
 public:
  Logger logger_;
  Builder builder_;
  Trunks trunks_;

  explicit Context(Logger::level_t _level = Logger::D);

  using PrefixCallback = std::function<std::string(const Repository &, const Task &, const std::string &_name)>;

  void registerToolchain(const std::string &_name, const Toolchain::Factory &_factory);
  void registerVCSHandling(const std::regex &_pattern, const VCS::Factory &_factory);

  std::shared_ptr<VCS> getVCS(const std::string &_uri);
  std::shared_ptr<Toolchain> getToolchain(const std::string &_name);

  void registerVar(const std::string &_name, const std::string &_value);
  void registerVarPrefix(const std::string &_prefix, PrefixCallback _cb);
  std::string preprocessString(const Repository& _repo, const Task& _task, const std::string &_value);
  std::string lookupVar(const Repository& _repo, const Task& _task, const std::string &_name);

 private:
  std::unordered_map<std::string, Toolchain::Factory> toolchainFactories_;
  std::unordered_map<std::string, PrefixCallback> varPrefixes_;
  std::unordered_map<std::string, std::string> knownVars_;
  std::vector<std::tuple<std::regex, VCS::Factory>> vcsFactories_;
};

}  // namespace brief
