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

/* Cur dir:
 * TODO Import <cmake script = CMakeLists.txt>
 *   Try to imports targets from a CMake config file to output a repository description.
 *     -i for an interactive
 * TODO Configure <optional features to be enable, prefixed by <task>: as per Dependency.require = [""]> -d <JSON repo description>
 *   Parse repo description file and merges tasks with optional experimental tasks, as well as merging inherited tasks
 *   for faster access when building. Serialize the output in binary to avoid having to unescape strings or parse
 *   fields in any order.
 *   Downloads, builds and install required dependencies.
 *     -i ask for any optional missing dependency if you wanna install it
 *   Checks that any var is preprocessable.
 * TODO Clean <tasks to clean = (cache.all)>
 *   Remove any build system generated temporary file.
 * TODO Build <task to build = (cache.all)>:<flavors to activate = (release)>
 *
 * TODO Test
 *
 * System:
 * TODO List installable exports.
 * TODO List installed exports.
 * TODO List available toolchains.
 * TODO Install/uninstall export.
 */

int main(int, char **) {
  brief::Context ctx;
  return 0;
}
