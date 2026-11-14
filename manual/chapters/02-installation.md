# 2. Installation and first program {#ch-2}

We now describe how abLEMON is obtained, either as a package or from the
sources, how a program finds it once it is installed, and which of its parts
are optional; the first program, which only needs a compiler and the headers,
is the subject of [chapter 3](03-mental-model.md#ch-3). Most of abLEMON is
made of templates, and therefore lives in its headers. The compiled library
(`libemon`, a name kept from LEMON) holds the few parts that are not
templates, i.e., the random number generator, the parser of the command line,
the colors of `graph_to_eps.h`, the locking on Windows and, when they are
enabled, the interfaces to the LP and MIP solvers of [chapter
20](20-lp.md#ch-20).

## 2.1 Packages {#sec-2-1}

The recipes of the packages are in `packaging/` of the repository, and each
release brings them to the archive of its tag (the workflow `release.yml` does
so, after taking the notes of the release from NEWS). The Homebrew tap of the
project works on macOS and Linux:

```sh
brew install dmeoli/ablemon/ablemon
```

Also, the recipes for conda-forge (package `ablemon`), vcpkg (port `ablemon`)
and Conan have been submitted to the respective indices; at the time of
writing (abLEMON 1.9.0) they are still under review. Until they are accepted,
the recipes in `packaging/` can be used locally, e.g., as an overlay of vcpkg
(`vcpkg install ablemon --overlay-ports=packaging/vcpkg`). Each package
installs the headers, the library and the CMake configuration of
[§2.3](#sec-2-3). None of them enables the LP and MIP backends of
[§2.4](#sec-2-4), which one builds from the sources when they are needed.

## 2.2 Building from the sources {#sec-2-2}

Building needs CMake 3.19 or later and a compiler that supports C++11. In
fact, the library compiles without warnings with GCC, Clang, AppleClang and
MSVC, from C++11 to C++23 (the continuous integration builds each change on
Linux, macOS and Windows with these compilers, and also under the address and
undefined behavior sanitizers). The usual sequence is

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build
cmake --install build --prefix /usr/local
```

where the third line runs the tests of `test/` and the programs of this manual
(those of `manual/examples/`, which ctest names `manual_*`), and the last one
installs the library under the given prefix, following `GNUInstallDirs`.

Among the options, `CMAKE_CXX_STANDARD` is honored when it is set, and is 11
otherwise; `BUILD_SHARED_LIBS` builds a shared library (on Windows, a DLL).
`LEMON_BUILD_TESTS`, `LEMON_BUILD_DEMOS`, `LEMON_BUILD_TOOLS` and
`LEMON_BUILD_DOC` are `ON` by default, and each of them leaves out a directory
when it is `OFF`; `LEMON_BUILD_MODULE` is `OFF` by default
(cf. [§2.3](#sec-2-3)), and the four options of the LP and MIP backends are
described in [§2.4](#sec-2-4). The version is usually taken from the tag of
git, and from the variable `LEMON_VERSION` when the sources are not a clone
(as in the archive of a release). The build types `Debug` and `Maintainer`
define `LEMON_ENABLE_DEBUG`, which turns on the checks of `assert.h`
(cf. [chapter 8](08-auxiliary.md#ch-8)); furthermore, `Maintainer` runs the
tests at each build. A project may also take the library in by
`add_subdirectory()` or by `FetchContent`, in which case only the library is
configured, whatever the options say.

For a project built with GNU make, the makefiles of the repository build the
library and the tests without a configuration step:

```sh
make -f makefile-lib            # lib/libemon.a
make -C test check              # the tests, built and run
```

Also, `makefile-c`, which the makefile of a project includes, defines the
objects of the library (`LEMONOBJ`), the external libraries it needs
(`LEMONLIB`), its headers (`LEMONH`) and the `-I` directive (`LEMONINC`); it
takes as input the compiler, its options and the directory of the sources.

## 2.3 Using the installed library {#sec-2-3}

A CMake project finds the installed library by its configuration, which
`find_package()` accepts both as `LEMON` and as `lemon`:

```cmake
find_package(LEMON CONFIG REQUIRED)
target_link_libraries(main PRIVATE LEMON::lemon)
```

Note that the target `LEMON::lemon` carries the include directories, the
compile definitions and, when the library was built with them, the LP and MIP
backends. Also, the installation may be moved to another prefix, since the
configuration only uses paths relative to it, and other build systems use
pkg-config (`pkg-config --cflags --libs lemon`). Thus, the headers are
included as `<lemon/...>`, and the library is in the namespace `lemon`, as in
LEMON 1.3.1.

With `-DLEMON_BUILD_MODULE=ON`, the library is also built and installed as the
C++20 module `lemon`, whose interface is `lemon/lemon.cppm`; a translation
unit linked to `LEMON::lemon_module` may then write `import lemon;` in place
of the includes. This needs CMake 3.28 or later, the Ninja generator (or one
of Visual Studio), and Clang 16 or MSVC 19.34 or later. When one of these
conditions does not hold, the option is turned off with a warning that names
it. GCC (up to version 15, at least) builds the module, but a unit that
imports it does not find the operators of the standard library that the
templates use; hence, GCC is not accepted for it.

## 2.4 Optional backends {#sec-2-4}

The LP and MIP interface of [chapter 20](20-lp.md#ch-20) needs at least one
external solver: GLPK (`LEMON_ENABLE_GLPK`), CPLEX (`LEMON_ENABLE_ILOG`), CLP
and CBC of COIN-OR (`LEMON_ENABLE_COIN`), or SoPlex (`LEMON_ENABLE_SOPLEX`).
Each option is `YES` by default, i.e., the solver is used if CMake finds it,
through its own CMake configuration where it has one. Those found are named by
their file names in the installed configuration, so that a project that links
`LEMON::lemon` also links them. The variables `LEMON_DEFAULT_LP` and
`LEMON_DEFAULT_MIP` choose which of the solvers found the type aliases `Lp`
and `Mip` of `lemon/lp.h` refer to. This choice is recorded in
`lemon/config.h`, which CMake writes in the build directory, and whose macros
`LEMON_HAVE_LP` and `LEMON_HAVE_MIP` tell a program whether there is a solver
at all; when there is none, the two aliases are not defined, while the rest of
the library is unaffected.

## 2.5 Exercises {#sec-2-5}

<span id="ex-2-1"></span>**Exercise 2.1.** Build the library with
`-DCMAKE_CXX_STANDARD=20`, run `ctest --test-dir build -R manual_`, and check
that the programs of this manual pass. Then install the library to a temporary
prefix, and build the program of [chapter 3](03-mental-model.md#ch-3) against
the installation, once with `find_package(LEMON)` and once with pkg-config.

<span id="ex-2-2"></span>**Exercise 2.2.** Configure the library on a machine
where GLPK is installed, and check in the output of CMake and in
`lemon/config.h` of the build directory which solver `Lp` and `Mip` refer to;
the test `lp_test` exercises them.
