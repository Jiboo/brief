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

void Context::registerToolchain(const std::string &name, const Toolchain::Factory &factory) {
  toolchainFactories.emplace(name, factory);
}

void Context::registerVCSHandling(const std::regex &pattern, const VCS::Factory &factory) {
  vcsFactories.push_back(std::make_tuple(pattern, factory));
}

void Context::registerVar(const std::string &name, const std::string &value) {
  knownVars.emplace(name, value);
}

void Context::registerVarPrefix(const std::string &prefix, PrefixCallback cb) {
  varPrefixes.emplace(prefix, cb);
}

std::string Context::preprocessString(const Repository& activeRepo, const Task& activeTask, const std::string &value) {
  // TODO Check for vars in the string
  return value;
}

std::string Context::lookupVar(const Repository& activeRepo, const Task& activeTask, const std::string &name) {
  auto symbols = activeTask.symbols();
  if (symbols) {
    const Constant *symbol = symbols->LookupByKey(name.c_str());
    if (symbol != nullptr)
      return preprocessString(activeRepo, activeTask,
                              std::string{(const char*)symbol->value()->Data(), symbol->value()->Length()});
  }

  auto constants = activeTask.symbols();
  if (constants) {
    const Constant *constant = activeRepo.constants()->LookupByKey(name.c_str());
    if (constant != nullptr)
      return preprocessString(activeRepo, activeTask,
                              std::string{(const char*)constant->value()->Data(), constant->value()->Length()});
  }

  auto known = knownVars.find(name);
  if (known != knownVars.end())
    return known->second;

  size_t offset = name.find("::");
  if (offset != std::string::npos) {
    auto prefix = name.substr(0, offset - 1);
    auto it = varPrefixes.find(prefix);
    if (it != varPrefixes.end()) {
      return it->second(activeRepo, activeTask, name.substr(offset + 2));
    }
  }

  throw std::runtime_error("Unknown variable: " + name);
}

}  // namespace brief
