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


#include <string>
#include <iostream>

#include "brief/context.hpp"

namespace brief {

Context::Context(Logger::level_t _level)
    : logger_(std::cout, _level), builder_(*this), trunks_(*this) {
}

void Context::registerToolchain(const std::string &_name, const Toolchain::Factory &_factory) {
  toolchainFactories_.emplace(_name, _factory);
}

std::shared_ptr<Toolchain> Context::getToolchain(const std::string &_name) {
  auto it = toolchainFactories_.find(_name);
  if (it == toolchainFactories_.end())
    throw std::runtime_error(std::string("Toolchain ") + _name + " not registered.");
  return it->second(*this);
}

void Context::registerVCSHandling(const std::regex &_pattern, const VCS::Factory &_factory) {
  vcsFactories_.push_back(std::make_tuple(_pattern, _factory));
}

std::shared_ptr<VCS> Context::getVCS(const std::string &_uri) {
  for (const auto &tuple : vcsFactories_) {
    if (std::regex_match(_uri, std::get<std::regex>(tuple)))
      return std::get<VCS::Factory>(tuple)(*this, _uri);
  }
  throw std::runtime_error(std::string("No known vcs can handle uri: ") + _uri);
}

void Context::registerVar(const std::string &_name, const std::string &_value) {
  knownVars_.emplace(_name, _value);
}

void Context::registerVarPrefix(const std::string &_prefix, PrefixCallback _cb) {
  varPrefixes_.emplace(_prefix, _cb);
}

std::string Context::preprocessString(const Repository &_repo,
                                      const Task &_task,
                                      const std::string &_value) {
  // TODO Check for vars in the string
  return _value;
}

std::string Context::lookupVar(const Repository &_repo, const Task &_task, const std::string &_name) {
  auto symbols = _task.symbols_;
  auto symbol = symbols.find(_name.c_str());
  if (symbol != symbols.end())
    return preprocessString(_repo, _task, symbol->second);

  auto constants = _repo.constants_;
  auto constant = constants.find(_name.c_str());
  if (constant != constants.end())
    return preprocessString(_repo, _task, constant->second);

  auto known = knownVars_.find(_name);
  if (known != knownVars_.end())
    return known->second;

  size_t offset = _name.find("::");
  if (offset != std::string::npos) {
    auto prefix = _name.substr(0, offset);
    auto it = varPrefixes_.find(prefix);
    if (it != varPrefixes_.end()) {
      return it->second(_repo, _task, _name.substr(offset + 2));
    }
  }

  throw std::runtime_error("Unknown variable: " + _name);
}

}  // namespace brief
