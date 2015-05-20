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
