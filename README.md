brief
======

Hobby attempt at build system with dependency management, portable across systems, archs and languages.
I kinda like Make, CMake, Ninja, Biicode and Gradle for Android, inspirations might come from them.

Revolves around a source code repository description:

- General info (licenses, home and bug reporting pages, tags, ...) 
- How to build and test the repo through *tasks*
  - Try to get the less bash possible to be more portable, simplify build files and be more tool friendly.
    - OK, clearly doesn't respect unix philosophy, but might open new ways of doing things in build systems.
    - Projects should specify what to build, and not how to build.
  - Tasks can depend on other tasks (from this build file, it's dependencies, or system-wide tasks: exports).
  - Tasks rely on a toolchain and a set of inputs: sources, include dirs, build system exposed symbols...
    - Toolchains are language specific, but you could have in the same repo multiple tasks with different toolchains (useful for language bindings)
  - Tasks can have other optional subtasks, if their criteria are met, they get merged into their parent.
    - Experimental features, enabled through config file
    - Optional features, enabled if all it's dependencies are installed
    - Flavors, pretty much like Gradle, enabled through cli arguments (useful for debug/release builds or paid/free apps)
- You can define custom toolchain, through plugins or scripts
- Wrote as a library, for easier tool/ide integration, it exposes
  - Repo files parsing and task scheduling
  - Plugins management for cvs and toolchains
  - V8 engine to run toolchain scriptss
  - Some utils to manipulate and merge tasks, dependency management and reusable utils for toolchains

Concepts/Features
-----------------

- *Task* : A job for the build system, an executable, a static library, a file copy... anything that can be built with a toolchain.
- *Export* : A task that is exposed system-wide and could be installed by a package manager.
- *Toolchain* : A plugin that transforms tasks description into an executable or a library, and that can test and install it.
- *Optional* : An optional task that is enabled only if its dependencies are installed.
- *Flavor* : Similar to gradle, a custom build passed to CLI when building (debug/release, free/paid app...)
- *Filters* : When defined in the same map (tasks, exports, optionals...), multiple tasks with same name are allowed,
tools will choose the right task depending on filters, or crash on ambiguous pick.

Folder hierarchy
----------------

    $BRIEF_ROOT_PATH/ /* Defaults to ~/.brief */
      toolchains/ symlinks to installed toolchains scripts
      plugins/ symlinks to installed toolchains & cvs plugins
      repodescs/ symlinks to installed package-repos .repo files
      repos/ installed repos
        <repo name>/
      build/ temporary build files
        <repo name>/
          <build name>/
            build name is a hash of revision, task name, enabled optionals and experimental features

Planned toolchains
------------------

clang:
  Uses libclang to compile c/cpp source files.

gtest:
  Compiles and link a bunch of test files for gtest, the main() function is supplied by toolchain.

bash:
  Run a set of bash commands, useful to wrap another build system.

js?
  Just agglomerate some JS, Optimisation::size could trigger a minify?

java? / android?:
  Clearly possible, could run JDK and Android's SDK cli tools.

Custom toolchain
----------------

Possible to define custom toolchains, used mainly for custom code generation.
JS and V8?

