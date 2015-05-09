#pragma once

#include <boost/filesystem.hpp>

#include "repo_generated.h"

namespace brief {

/**
 * Implements how to drive a version control system.
 */
class VCS {
public:
  using Factory = std::function<std::shared_ptr<VCS>(std::string)>;

  virtual void reset() = 0;
  virtual void reset(const std::string &tag) = 0;
  virtual std::string first_tag() = 0;
  virtual std::string last_tag() = 0;
  virtual int compareTags(const std::string &tag1, const std::string &tag2) = 0;

  virtual std::vector<boost::filesystem::path> diff() = 0;
};

} // namespace brief
