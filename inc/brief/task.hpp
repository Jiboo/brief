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

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "brief/msgpack.hpp"
#include "brief/description.hpp"

namespace brief {

class Dependency {
  BRIEF_MSGPACK_FRIENDS_INTERNAL()
  BRIEF_JSON_FRIENDS_INTERNAL()

 private:
    /** The name of a task */
    std::string name_;

    /** Tag as per defined in dependency repo description, a list of tags either custom
     * or mapped to repo internal tags. */
    std::string tag_;

    /** Preferred linking to this library (embark the code instead of using a system shareable code)
     * Will also link it's dependencies statically, if not overridden somewhere. */
    bool staticLink_;

    /** Add dependency on *optional*, *experimental* or *flavor* tasks from the targeted task
     * For example you could depend on a "debug" flavor, or "filesystem" of boost
     * (Although i'd recommend splitting boost libs in a task for each, instead of a mega-task, but both could work) */
    std::vector<std::string> require_;
};

BRIEF_JSON_START_INTERNAL(Dependency, std::string, name_, "name")
BRIEF_JSON_ARG_INTERNAL(std::string, tag_, "tag")
BRIEF_JSON_ARG_INTERNAL(bool, staticLink_, "staticLink")
BRIEF_JSON_ARG_INTERNAL(std::vector<std::string>, require_, "require")
BRIEF_JSON_STOP_INTERNAL()
BRIEF_MSGPACK_INTERNAL(Dependency, _.name_, _.tag_, _.staticLink_, _.require_)

/** Filters are used to enable a task if only certain criteria are met.
 * If two tasks with the same name collide, they must have different filters. */
class TaskFilters {
  BRIEF_MSGPACK_FRIENDS_INTERNAL()
  BRIEF_JSON_FRIENDS_INTERNAL()

 private:
    /** Whitelist of archs, default to all. */
    std::vector<std::string> archs_;

    /** Whitelist of platforms, defaults to all. */
    std::vector<std::string> platforms_;

    /** Used to limit this task in a specific range of tags, defaults to first tag. */
    std::string minTag_;

    /** Used to limit this task in a specific range of tags, defaults to last tag. */
    std::string maxTag_;
};

BRIEF_JSON_START_INTERNAL(TaskFilters, std::vector<std::string>, archs_, "archs")
BRIEF_JSON_ARG_INTERNAL(std::vector<std::string>, platforms_, "platforms")
BRIEF_JSON_ARG_INTERNAL(std::string, minTag_, "minTag")
BRIEF_JSON_ARG_INTERNAL(std::string, maxTag_, "maxTag")
BRIEF_JSON_STOP_INTERNAL()
BRIEF_MSGPACK_INTERNAL(TaskFilters, _.archs_, _.platforms_, _.minTag_, _.maxTag_)

class Task {
  BRIEF_MSGPACK_FRIENDS_INTERNAL()
  BRIEF_JSON_FRIENDS_INTERNAL()

 private:
  /** Inherits from another task.
   * Maps&Lists fields from *the specified task* will be merged into *this* one.
   * For other fields:
   *   - *this* task values defined are kept intact.
   *   - Fields defined in *the specified task* and not in *this* task are copied. */
  std::string inherits_;

  enum type_t : uint8_t {
    /** Export task that implements a specification like libc, opengl...
     * It exposes nothing but dependencies to implementation.
     * It's the only task that won't fail on install if another task
     * is already exposed with this name, you can set preferred impl in config file. */
    SPECIFICATION,

    /** Export task used by other tasks that depends on it. */
    LIBRARY,

    /** Export task can be run from console or desktop env. */
    APPLICATION,

    /** Export task used for listing tasks available for install.
     * When installed, all it's .repo files are added to the local database of
     * known repos, that is used for search by the repo manager. */
    PACKAGES,

    /** Task that is a set of tasks (can be in exports).
     * It exposes nothing but dependencies on other tasks.
     * Used for bundling tasks together and make groups (like "dev" depends on clang, musl, ..)
     * Bundles can depend on other bundles. */
    BUNDLE,

    /** Task that can build, install and test other tasks.
     * It exposes a plugin (.so/.dll) or a toolchain script,
     * that takes a set of tasks as input to different entry points. */
    TOOLCHAIN
  } type_;

  /** Used by tools to pick the best task between two having the same name, see the associated table. */
  TaskFilters filters_;

  /** List of tasks on which this one depends. You can depend on:
   *    - exports in this repo_desc and of all installed repo desc
   *    - tasks defined in this repo_desc
   *    - tasks defined in repo_desc of repos on which this task depends
   *    - optionals and features defined in tasks on which this task depends */
  std::vector<Dependency> dependencies_;

  /** Set of tasks describing features enabled only if all of their dependencies are installed. */
  std::map<std::string, Task> optionals_;

  /** Set of tasks describing experimental features disabled by default. */
  std::map<std::string, Task> experimental_;

  /** Set of flavors describing different ways to build this task (debug/release or paid/free for example).
   * Debug/Release flavors are automagically filled by the toolchain but can be overridden. */
  std::map<std::string, Task> flavors_;

  /** List the URI of patches applied to the repo, must point to a "raw diff", not html content.
   *    - repo:<repo local path> for patches on the repo.
   *    - others URI will be fed to curl (http, https, ftp, ..). */
  std::vector<std::string> patches_;

  /** Toolchain used to build this task. */
  std::string toolchain_;

  /** Used as a argc/argv to pass obscure data to toolchain. */
  std::vector<std::string> toolchainFlags_;

  /** What language standard you want to enforce on the toolchain.
   * For C/C++ toolchain could be c11, c++11... Java could be 1.7, 1.8...
   * Refer to toolchain documentation. */
  std::string standard_;

  /** What optimisation strategy you want to enforce on the toolchain.
   * Default values supplied by the toolchain. */
  enum optimisation_t : uint8_t {
    NONE, SIZE, SPEED
  } optimize_;

  /** Used by the most toolchains to build or install this task
   * You can use wildcards, * for any file in this directory, ** for any file in this directory and subdirectories */
  std::vector<std::string> sources_;

  /** Point to a list of directories in which the compiler should search when using
   * include/imports directive in source code. */
  std::vector<std::string> includeDirs_;

  /** Headers that will get installed
   * Default to all files in includePaths */
  std::vector<std::string> headers_;

  /** Symbols from the build system exposed to the source code during compilation
   * (preprocessor stuff, config.h, macros for cpp BuildConfig for Android...)... entirely up to the toolchain. */
  std::unordered_map<std::string, std::string> symbols_;

  /** General info about this task */
  Description description_;
};

using __strmap = std::unordered_map<std::string, std::string>;
using __namedtasks = std::unordered_map<std::string, Task>;
using __namedtasksmap = std::map<std::string, Task>;

BRIEF_JSON_START_INTERNAL(Task, std::string, inherits_, "inherits")
BRIEF_JSON_ARG_INTERNAL(Task::type_t, type_, "type")
BRIEF_JSON_ARG_INTERNAL(TaskFilters, filters_, "filters")
BRIEF_JSON_ARG_INTERNAL(std::vector<Dependency>, dependencies_, "dependencies")
BRIEF_JSON_ARG_INTERNAL(__namedtasksmap, optionals_, "optionals")
BRIEF_JSON_ARG_INTERNAL(__namedtasksmap, experimental_, "experimental")
BRIEF_JSON_ARG_INTERNAL(__namedtasksmap, flavors_, "flavors")
BRIEF_JSON_ARG_INTERNAL(std::vector<std::string>, patches_, "patches")
BRIEF_JSON_ARG_INTERNAL(std::string, toolchain_, "toolchain")
BRIEF_JSON_ARG_INTERNAL(std::vector<std::string>, toolchainFlags_, "toolchainFlags")
BRIEF_JSON_ARG_INTERNAL(std::string, standard_, "standard")
BRIEF_JSON_ARG_INTERNAL(Task::optimisation_t, optimize_, "optimize")
BRIEF_JSON_ARG_INTERNAL(std::vector<std::string>, sources_, "sources")
BRIEF_JSON_ARG_INTERNAL(std::vector<std::string>, includeDirs_, "includeDirs")
BRIEF_JSON_ARG_INTERNAL(std::vector<std::string>, headers_, "headers")
BRIEF_JSON_ARG_INTERNAL(__strmap, symbols_, "symbols")
BRIEF_JSON_ARG_INTERNAL(Description, description_, "description")
BRIEF_JSON_STOP_INTERNAL()
BRIEF_MSGPACK_INTERNAL(Task, _.inherits_, _.type_, _.filters_, _.dependencies_, _.inherits_, _.type_, _.filters_,
    _.dependencies_, _.optionals_, _.experimental_, _.flavors_, _.patches_, _.toolchain_, _.toolchainFlags_,
    _.standard_, _.optimize_, _.sources_, _.includeDirs_, _.headers_, _.symbols_, _.description_)

}  // namespace brief