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

#include "brief/msgpack.hpp"
#include "brief/json.hpp"

namespace brief {

class License {
  BRIEF_MSGPACK_FRIENDS_INTERNAL()
  BRIEF_JSON_FRIENDS_INTERNAL()

 private:
  /** License name */
  std::string name_;

  /** Repo local path to the full license string */
  std::string path_;
};
BRIEF_JSON_START_INTERNAL(License, std::string, name_, "name")
BRIEF_JSON_ARG_INTERNAL(std::string, path_, "path")
BRIEF_JSON_STOP_INTERNAL()
BRIEF_MSGPACK_INTERNAL(License, _.name_, _.path_)

class Description {
  BRIEF_MSGPACK_FRIENDS_INTERNAL()
  BRIEF_JSON_FRIENDS_INTERNAL()

 private:
  /** Displayed name (like Google Test and not gtest)
   * Defaults to "name", being either from Task's maps keys or Repository */
  std::string title_;

  /** One line TLDR of the purpose of the code in this repo
   * Max size = 120 - sizeof(title) - sizeof(name) + 5
   * (name being either from Task's maps keys or Repository) */
  std::string summary_;

  /** Homepage URL. */
  std::string home_;

  /** Bug reporting URL. */
  std::string bugs_;

  /** A repo local path to an icon image file (svg preferred). */
  std::string icon_;

  /** URL to icon image files (preferably PNG)
   * You can add extra descriptors matching component 4. of https://html.spec.whatwg.org/#image-candidate-string */
  std::vector<std::string> publicIcons_;

  /// License applicable for this repo (usage of applications and/or libraries)
  std::vector<License> licenses_;
};
BRIEF_JSON_START_INTERNAL(Description, std::string, title_, "path")
BRIEF_JSON_ARG_INTERNAL(std::string, summary_, "summary")
BRIEF_JSON_ARG_INTERNAL(std::string, home_, "home")
BRIEF_JSON_ARG_INTERNAL(std::string, bugs_, "bugs")
BRIEF_JSON_ARG_INTERNAL(std::string, icon_, "icon")
BRIEF_JSON_ARG_INTERNAL(std::vector<std::string>, publicIcons_, "publicIcons")
BRIEF_JSON_ARG_INTERNAL(std::vector<License>, licenses_, "licenses")
BRIEF_JSON_STOP_INTERNAL()
BRIEF_MSGPACK_INTERNAL(Description, _.title_, _.summary_, _.home_, _.bugs_, _.icon_, _.publicIcons_, _.licenses_)

}  // namespace brief
