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

#include "brief/context.hpp"

#include <string>

namespace brief {

Context::Context() {
}

void Context::registerToolchain(const std::string &_name, const Toolchain::Factory &_factory) {
  toolchainFactories_.emplace(_name, _factory);
}

void Context::registerVCSHandling(const std::regex &_pattern, const VCS::Factory &_factory) {
  vcsFactories_.push_back(std::make_tuple(_pattern, _factory));
}

void Context::registerVar(const std::string &_name, const std::string &_value) {
  knownVars_.emplace(_name, _value);
}

void Context::registerVarPrefix(const std::string &_prefix, PrefixCallback _cb) {
  varPrefixes_.emplace(_prefix, _cb);
}

std::string Context::preprocessString(const Repository &_activeRepo,
                                      const Task &_activeTask,
                                      const std::string &_value) {
  // TODO Check for vars in the string
  return _value;
}

std::string Context::lookupVar(const Repository &_activeRepo, const Task &_activeTask, const std::string &_name) {
  auto symbols = _activeTask.symbols_;
  auto symbol = symbols.find(_name.c_str());
  if (symbol != symbols.end())
    return preprocessString(_activeRepo, _activeTask, symbol->second);

  auto constants = _activeRepo.constants_;
  auto constant = constants.find(_name.c_str());
  if (constant != constants.end())
    return preprocessString(_activeRepo, _activeTask, constant->second);

  auto known = knownVars_.find(_name);
  if (known != knownVars_.end())
    return known->second;

  size_t offset = _name.find("::");
  if (offset != std::string::npos) {
    auto prefix = _name.substr(0, offset);
    auto it = varPrefixes_.find(prefix);
    if (it != varPrefixes_.end()) {
      return it->second(_activeRepo, _activeTask, _name.substr(offset + 2));
    }
  }

  throw std::runtime_error("Unknown variable: " + _name);
}

}  // namespace brief
