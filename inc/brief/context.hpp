#pragma once

#include <regex>
#include <unordered_map>

#include <boost/filesystem/path.hpp>

#include "repo_generated.h"
#include "toolchain.hpp"
#include "vcs.hpp"

namespace brief {

class Context {
 public:
  using PrefixCallback = std::function<std::string(const std::string &)>;

  void registerToolchain(const std::string &name, const Toolchain::Factory &factory);
  void registerVCSHandling(const std::regex &pattern, const VCS::Factory &factory);

  std::string registerVar(const std::string &name, const std::string &value);
  std::string registerVarPrefix(const std::string &prefix, PrefixCallback cb);
  std::string lookupVar(const std::string &name);

  void loadDescription(boost::filesystem::path descPath);
  bool check(Dependency &dep);
  void install(Dependency &dep);

  void build(const std::string &task);
  void test(const std::string &task);
  void install(const std::string &task);

private:
  std::unordered_map<std::string, Toolchain::Factory> toolchainFactories;
  std::unordered_map<std::string, PrefixCallback> varPrefixes;
  std::vector<std::tuple<std::regex, VCS::Factory>> vcsFactories;
};

} // namespace brief
