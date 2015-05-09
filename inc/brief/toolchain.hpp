#pragma once

#include <stdexcept>
#include <memory>
#include <functional>

#include "repo_generated.h"

namespace brief {

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
  using Factory = std::function<std::shared_ptr<Toolchain>()>;

  virtual void build(const std::string &task, const std::initializer_list<std::string> &flavors) = 0;
  virtual void test(const std::string &task) = 0;
  virtual void install(const std::string &task) = 0;
};

} // namespace brief
