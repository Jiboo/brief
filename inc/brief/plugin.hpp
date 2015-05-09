#pragma once

#include "context.hpp"

namespace brief {

class Plugin {
  virtual bool load(context &ctx) = 0;
  virtual bool unload(context &ctx) = 0;
};

} // namespace brief
