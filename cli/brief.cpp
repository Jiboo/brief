#include "brief/context.hpp"

/* Cur dir:
 * TODO Import <cmake script = CMakeLists.txt>
 *   Try to imports targets from a CMake config file to output a repository
 *   description.
 *     -i for an interactive
 * TODO Configure <JSON repo description = (any .json in the current dir, but only if it's the only one)> :: <optional tasks that needs to be enabled, as per model.Tasks.Dependency.require = [""]>
 *   Parse repo description file and merges tasks with optional experimental
 *   tasks, as well as merging inherited tasks for faster access when
 *   building. Serialize the output as brief.cache.
 *   Downloads, builds and install required dependencies.
 *     -i ask for any optional missing dependency if you wanna install it
 * TODO Clean <tasks to clean = (cache.all)>
 *   Remove any build system generated temporary file.
 * TODO Build <tasks to build = (cache.all)> :: <flavors to activate = (release)>
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
