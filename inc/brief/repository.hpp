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

#include "brief/description.hpp"
#include "brief/task.hpp"

namespace brief {

/** Used to point to a state of the repo (a combination of revision/branch/tag)
 * If you don't provide custom tags, we'll try to use the ones on the repo. */
class Tag {
  BRIEF_MSGPACK_FRIENDS_INTERNAL()
  BRIEF_JSON_FRIENDS_INTERNAL()

 private:
  /** For git, that would be commit hash. */
  std::string id_;

  /** If a revision might be in multiple branch, you can pair an id/tag to a branch.
   * If pointing to a branch only (id/tag omitted) then last commit is assumed. */
  std::string branch_;

  /** Use a repo internal tag instead of a revision id
   * This is useful to override some crazy tag names or bad cvs migration/mirror. */
  std::string tag_;
};
BRIEF_JSON_START_INTERNAL(Tag, std::string, id_, "id")
BRIEF_JSON_ARG_INTERNAL(std::string, branch_, "branch")
BRIEF_JSON_ARG_INTERNAL(std::string, tag_, "tag")
BRIEF_JSON_STOP_INTERNAL()
BRIEF_MSGPACK_INTERNAL(Tag, _.id_, _.branch_, _.tag_)

class Repository {
  BRIEF_MSGPACK_FRIENDS_INTERNAL()
  BRIEF_JSON_FRIENDS_INTERNAL()

 private:
  /**Abbreviated name (like gtest and not Google Test)
   * Max size = 64 */
  std::string name_;

  /** Repository URL, will be used as input to the CVS clone operation */
  std::string url_;

  /** Constants referenced in this repo, you can use thus values in any strings of this file. */
  std::unordered_map<std::string, std::string> constants_;

  /** If this field is not provided, it will be filled with pointers from the cvs.
   * Useful if the repo tags aren't reliable (a bad mirroring or something). */
  std::unordered_map<std::string, Tag> tags_;

  /** Estimated in KiB, on a 4k sector disk */
  uint32_t repoSize_;

  /** Estimated in KiB, on a 4k sector disk */
  uint32_t buildSize_;

  /** Estimated in seconds, relative to musl compile time on same machine */
  float buildTime_;

  /** Repo local path to other description files, tasks and exports will get merged into this one.
   * Thus files are optional, will not even warn if missing (except in verbose mode). */
  // No need for msgpack serialization, they will get merged in configure step
  std::vector<std::string> imports_;

  /** List of tasks needed to build everything */
  std::vector<std::string> all_;

  /** List of tasks with application type, that are needed to test the repo. */
  std::vector<std::string> test_;

  /** Tasks defined locally for this file and it's dependencies */
  std::unordered_map<std::string, Task> tasks_;

  /** Tasks that are visible system wide and that can be installed */
  std::unordered_map<std::string, Task> exports_;

  /** General info about the repo */
  Description description_;
};

using __namedtag = std::unordered_map<std::string, Tag>;

BRIEF_JSON_START_INTERNAL(Repository, std::string, name_, "name")
BRIEF_JSON_ARG_INTERNAL(std::string, url_, "url")
BRIEF_JSON_ARG_INTERNAL(__strmap, constants_, "constants")
BRIEF_JSON_ARG_INTERNAL(__namedtag, tags_, "tags")
BRIEF_JSON_ARG_INTERNAL(uint32_t, repoSize_, "repoSize")
BRIEF_JSON_ARG_INTERNAL(uint32_t, buildSize_, "buildSize")
BRIEF_JSON_ARG_INTERNAL(float, buildTime_, "buildTime")
BRIEF_JSON_ARG_INTERNAL(std::vector<std::string>, imports_, "imports")
BRIEF_JSON_ARG_INTERNAL(std::vector<std::string>, all_, "all")
BRIEF_JSON_ARG_INTERNAL(std::vector<std::string>, test_, "test")
BRIEF_JSON_ARG_INTERNAL(__namedtasks, tasks_, "tasks")
BRIEF_JSON_ARG_INTERNAL(__namedtasks, exports_, "exports")
BRIEF_JSON_ARG_INTERNAL(Description, description_, "description")
BRIEF_JSON_STOP_INTERNAL()
BRIEF_MSGPACK_INTERNAL(Repository, _.name_, _.url_, _.constants_, _.tags_, _.repoSize_, _.buildSize_, _.buildTime_,
    // imports ignored, it's eliminated during configure
    _.all_, _.test_, _.tasks_, _.exports_, _.description_)

}  // namespace brief
