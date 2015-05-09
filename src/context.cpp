#include <fstream>

#include "brief/context.hpp"

namespace brief {
  void Context::registerToolchain(const std::string &name, const Toolchain::Factory &factory) {

  }

  void Context::registerVCSHandling(const std::regex &pattern, const VCS::Factory &factory) {

  }

  std::string Context::registerVar(const std::string &name, const std::string &value) {

  }

  std::string Context::registerVarPrefix(const std::string &prefix, PrefixCallback cb) {

  }

  std::string Context::lookupVar(const std::string &name) {

  }

  void Context::loadDescription(boost::filesystem::path descPath) {

  }

  bool Context::check(Dependency &dep) {

  }

  void Context::install(Dependency &dep) {

  }

  void Context::build(const std::string &task) {

  }

  void Context::test(const std::string &task) {

  }

  void Context::install(const std::string &task) {

  }
}
