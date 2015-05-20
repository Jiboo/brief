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
#include <memory>
#include <stdexcept>
#include <string>

namespace brief {

class Context;

/**
 * Implements how to treat a set of brief::tasks.
 * Library tasks usually declares sources and headers.
 * Application tasks usually declares sources.
 * Must be thread safe, there might be multiple instances of the same toolchain running in parallel.
 * Task inputed to toolchains are already merged, you should have to work only with:
 *    - toolchain_flags
 *    - standard
 *    - sources
 *    - headers
 *    - includeDirs
 *    - symbols
 *    - optimize
 * Toolchains should document how they react to thus inputs.
 */

class Toolchain {
 public:
  using Factory = std::function<std::shared_ptr<Toolchain>(Context&)>;

  virtual void build(const Task &_task, const std::initializer_list<std::string> &_flavors) = 0;
  virtual void test(const Task &_task) = 0;
  virtual void install(const Task &_task) = 0;
};

}  // namespace brief
