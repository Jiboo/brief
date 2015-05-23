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

#include "brief/repository.hpp"
#include "brief/logger.hpp"

namespace brief {

#define BRIEF_MERGE_VALUE(VALUE) \
  if (_task.VALUE != defaults.VALUE) \
    result.VALUE = _task.VALUE;

#define BRIEF_MERGE_SET(CONTAINER) \
  result.CONTAINER.insert(result.CONTAINER.end(), \
    _task.CONTAINER.begin(), \
    _task.CONTAINER.end());

#define BRIEF_MERGE_MAP(CONTAINER) \
  result.CONTAINER.insert(_task.CONTAINER.begin(), \
    _task.CONTAINER.end());

Task Task::merge(const Task &_task) const {
  Task defaults;
  Task result = *this;

  BRIEF_MERGE_VALUE(type_)

  BRIEF_MERGE_SET(filters_.archs_)
  BRIEF_MERGE_SET(filters_.platforms_)
  BRIEF_MERGE_VALUE(filters_.minTag_)
  BRIEF_MERGE_VALUE(filters_.maxTag_)

  BRIEF_MERGE_SET(dependencies_)

  BRIEF_MERGE_MAP(optionals_)

  BRIEF_MERGE_MAP(experimental_)

  BRIEF_MERGE_MAP(flavors_)

  BRIEF_MERGE_SET(patches_)

  BRIEF_MERGE_VALUE(toolchain_)

  BRIEF_MERGE_SET(toolchainFlags_)

  BRIEF_MERGE_VALUE(standard_)

  BRIEF_MERGE_VALUE(optimize_)

  BRIEF_MERGE_SET(sources_)

  BRIEF_MERGE_SET(includeDirs_)

  BRIEF_MERGE_SET(headers_)

  BRIEF_MERGE_MAP(symbols_)

  BRIEF_MERGE_VALUE(description_.title_)
  BRIEF_MERGE_VALUE(description_.summary_)
  BRIEF_MERGE_VALUE(description_.home_)
  BRIEF_MERGE_VALUE(description_.bugs_)
  BRIEF_MERGE_VALUE(description_.icon_)
  BRIEF_MERGE_SET(description_.publicIcons_)
  BRIEF_MERGE_MAP(description_.licenses_)

  return result;
}

}  // namespace brief
