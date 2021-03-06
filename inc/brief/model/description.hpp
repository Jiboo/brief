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

#include <cstdint>

#include <string>
#include <unordered_map>
#include <vector>

#include "brief/serial.hpp"
#include "brief/msgpack.hpp"
#include "brief/json.hpp"

namespace brief {

class Description {
  BRIEF_EQUALS_FRIENDS_INTERNAL(Description)

 public:
  /** Displayed name (like Google Test and not gtest)
   * Defaults to "name", being either from Task's maps keys or Repository */
  std::string title_;

  /** One line TLDR of the purpose of the code in this repo
   * Max size = 120 - sizeof(title) - sizeof(name) + 5
   * (name being either from Task's maps keys or Repository) */
  std::string summary_;

  /// License applicable for this repo (usage of applications and/or libraries)
  std::unordered_map<std::string, std::string> licenses_;

  /** Homepage URL. */
  std::string home_;

  /** Bug reporting URL. */
  std::string bugs_;

  /** A repo local path to an icon image file (svg preferred). */
  std::string icon_;

  /** URL to icon image files (preferably PNG)
   * You can add extra descriptors matching component 4. of https://html.spec.whatwg.org/#image-candidate-string */
  std::vector<std::string> publicIcons_;
};

#define Description_PROPERTIES \
  (7, ( \
    (std::string, title_, "title"), \
    (std::string, summary_, "summary"), \
    (__strmap, licenses_, "licenses"), \
    (std::string, home_, "home"), \
    (std::string, bugs_, "bugs"), \
    (std::string, icon_, "icon"), \
    (std::vector<std::string>, publicIcons_, "publicIcons")) \
  )

BRIEF_MSGPACK_INTERNAL(Description, Description_PROPERTIES)
BRIEF_JSON_INTERNAL(Description, Description_PROPERTIES)
BRIEF_EQUALS_INTERNAL(Description, Description_PROPERTIES)

}  // namespace brief
