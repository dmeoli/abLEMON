CMAKE_MINIMUM_REQUIRED(VERSION 3.19)

SET(PROJECT_NAME "LEMON")
PROJECT(${PROJECT_NAME})
# PROJECT() sets an empty LEMON_VERSION, which would hide the one in the
# cache when CMake runs again on the same build directory
UNSET(LEMON_VERSION)

# whether LEMON is the project being built or one that another project takes
# in with add_subdirectory() or FetchContent: only in the first case does it
# set the build type, the configurations and the target check, which belong
# to the project being built (PROJECT_IS_TOP_LEVEL needs CMake 3.21)
IF(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  SET(LEMON_TOP_LEVEL TRUE)
ELSE()
  SET(LEMON_TOP_LEVEL FALSE)
ENDIF()

FIND_PACKAGE(Python3 COMPONENTS Interpreter QUIET)
SET(PYTHONINTERP_FOUND ${Python3_Interpreter_FOUND})
INCLUDE(FindWget)

IF(EXISTS ${PROJECT_SOURCE_DIR}/cmake/version.cmake)
  INCLUDE(${PROJECT_SOURCE_DIR}/cmake/version.cmake)
ELSEIF(DEFINED ENV{LEMON_VERSION})
  SET(LEMON_VERSION $ENV{LEMON_VERSION} CACHE STRING "LEMON version string.")
ELSE()
  EXECUTE_PROCESS(
    COMMAND
    hg log -r. --template "{latesttag}"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE HG_REVISION_TAG
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  EXECUTE_PROCESS(
    COMMAND
    hg log -r. --template "{latesttagdistance}"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE HG_REVISION_DIST
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  EXECUTE_PROCESS(
    COMMAND
    hg log -r. --template "{node|short}"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE HG_REVISION_ID
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  IF(HG_REVISION_TAG STREQUAL "")
    # no Mercurial checkout: the tag of git, else the version is unknown
    # (a package passes it with -DLEMON_VERSION=...)
    EXECUTE_PROCESS(
      COMMAND git describe --tags --always
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      OUTPUT_VARIABLE HG_REVISION
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    STRING(REGEX REPLACE "^[rv]([0-9])" "\\1" HG_REVISION "${HG_REVISION}")
    IF(HG_REVISION STREQUAL "")
      SET(HG_REVISION "unknown")
    ENDIF()
  ELSE()
    IF(HG_REVISION_TAG STREQUAL "null")
      SET(HG_REVISION_TAG "trunk")
    ELSEIF(HG_REVISION_TAG MATCHES "^r")
      STRING(SUBSTRING ${HG_REVISION_TAG} 1 -1 HG_REVISION_TAG)
    ENDIF()
    IF(HG_REVISION_DIST STREQUAL "0")
      SET(HG_REVISION ${HG_REVISION_TAG})
    ELSE()
      SET(HG_REVISION
	"${HG_REVISION_TAG}+${HG_REVISION_DIST}-${HG_REVISION_ID}")
    ENDIF()
  ENDIF()

  SET(LEMON_VERSION ${HG_REVISION} CACHE STRING "LEMON version string.")
ENDIF()

SET(PROJECT_VERSION ${LEMON_VERSION})

# the numeric part of the version (1.4.0 out of 1.4.0-5-g63754b6), which the
# shared library and the version file of the CMake configuration carry, and
# its first two numbers, the version of the interface of the shared library;
# empty when the version is no number (e.g., a commit, or unknown)
STRING(REGEX MATCH "^[0-9]+\\.[0-9]+(\\.[0-9]+)?" LEMON_NUMERIC_VERSION
  "${LEMON_VERSION}")
STRING(REGEX MATCH "^[0-9]+\\.[0-9]+" LEMON_SOVERSION "${LEMON_VERSION}")

# the directories of the installation as the system wants them (e.g., lib64
# or lib/x86_64-linux-gnu for the libraries)
INCLUDE(GNUInstallDirs)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

FIND_PACKAGE(Doxygen)
FIND_PACKAGE(Ghostscript)

SET(LEMON_ENABLE_GLPK YES CACHE STRING "Enable GLPK solver backend.")
SET(LEMON_ENABLE_ILOG YES CACHE STRING "Enable ILOG (CPLEX) solver backend.")
SET(LEMON_ENABLE_COIN YES CACHE STRING "Enable COIN solver backend.")
SET(LEMON_ENABLE_SOPLEX YES CACHE STRING "Enable SoPlex solver backend.")

IF(LEMON_ENABLE_GLPK) 
  FIND_PACKAGE(GLPK 4.33)
  IF(GLPK_FOUND)
    SET(LEMON_HAVE_LP TRUE)
    SET(LEMON_HAVE_MIP TRUE)
    SET(LEMON_HAVE_GLPK TRUE)
  ENDIF(GLPK_FOUND)
ENDIF(LEMON_ENABLE_GLPK)
IF(LEMON_ENABLE_ILOG)
  FIND_PACKAGE(ILOG)
  IF(ILOG_FOUND)
    SET(LEMON_HAVE_LP TRUE)
    SET(LEMON_HAVE_MIP TRUE)
    SET(LEMON_HAVE_CPLEX TRUE)
  ENDIF(ILOG_FOUND)
ENDIF(LEMON_ENABLE_ILOG)
IF(LEMON_ENABLE_COIN)
  FIND_PACKAGE(COIN)
  IF(COIN_FOUND)
    SET(LEMON_HAVE_LP TRUE)
    SET(LEMON_HAVE_MIP TRUE)
    SET(LEMON_HAVE_CLP TRUE)
    SET(LEMON_HAVE_CBC TRUE)
  ENDIF(COIN_FOUND)
ENDIF(LEMON_ENABLE_COIN)
IF(LEMON_ENABLE_SOPLEX)
  FIND_PACKAGE(SOPLEX)
  IF(SOPLEX_FOUND)
    SET(LEMON_HAVE_LP TRUE)
    SET(LEMON_HAVE_SOPLEX TRUE)
  ENDIF(SOPLEX_FOUND)
ENDIF(LEMON_ENABLE_SOPLEX)

IF(ILOG_FOUND)
  SET(DEFAULT_LP "CPLEX")
  SET(DEFAULT_MIP "CPLEX")
ELSEIF(COIN_FOUND)
  SET(DEFAULT_LP "CLP")
  SET(DEFAULT_MIP "CBC")
ELSEIF(GLPK_FOUND)
  SET(DEFAULT_LP "GLPK")
  SET(DEFAULT_MIP "GLPK")
ELSEIF(SOPLEX_FOUND)
  SET(DEFAULT_LP "SOPLEX")
ENDIF()

IF(NOT LEMON_DEFAULT_LP OR
    (NOT ILOG_FOUND AND (LEMON_DEFAULT_LP STREQUAL "CPLEX")) OR
    (NOT COIN_FOUND AND (LEMON_DEFAULT_LP STREQUAL "CLP")) OR
    (NOT GLPK_FOUND AND (LEMON_DEFAULT_LP STREQUAL "GLPK")) OR
    (NOT SOPLEX_FOUND AND (LEMON_DEFAULT_LP STREQUAL "SOPLEX")))
  SET(LEMON_DEFAULT_LP ${DEFAULT_LP} CACHE STRING
    "Default LP solver backend (GLPK, CPLEX, CLP or SOPLEX)" FORCE)
ELSE()
  SET(LEMON_DEFAULT_LP ${DEFAULT_LP} CACHE STRING
    "Default LP solver backend (GLPK, CPLEX, CLP or SOPLEX)")
ENDIF()
IF(NOT LEMON_DEFAULT_MIP OR
    (NOT ILOG_FOUND AND (LEMON_DEFAULT_MIP STREQUAL "CPLEX")) OR
    (NOT COIN_FOUND AND (LEMON_DEFAULT_MIP STREQUAL "CBC")) OR
    (NOT GLPK_FOUND AND (LEMON_DEFAULT_MIP STREQUAL "GLPK")))
  SET(LEMON_DEFAULT_MIP ${DEFAULT_MIP} CACHE STRING
    "Default MIP solver backend (GLPK, CPLEX or CBC)" FORCE)
ELSE()
  SET(LEMON_DEFAULT_MIP ${DEFAULT_MIP} CACHE STRING
    "Default MIP solver backend (GLPK, CPLEX or CBC)")
ENDIF()

# The warning flags, which depend on the compiler, are not chosen here: they
# are those of LEMON_CXX_WARNING_FLAGS, by default the environment variable
# LEMON_CXX_WARNING, and the presets give those of GCC and Clang and those
# of MSVC [see CMakePresets.json]
SET(LEMON_CXX_WARNING_FLAGS "$ENV{LEMON_CXX_WARNING}" CACHE STRING
  "LEMON warning flags.")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LEMON_CXX_WARNING_FLAGS}")

# The build type Maintainer compiles as Debug; its presets also turn the
# warnings into errors [see CMakePresets.json]
SET( CMAKE_CXX_FLAGS_MAINTAINER "${CMAKE_CXX_FLAGS_DEBUG}" CACHE STRING
  "Flags used by the C++ compiler during maintainer builds." FORCE
  )
SET( CMAKE_C_FLAGS_MAINTAINER "${CMAKE_C_FLAGS_DEBUG}" CACHE STRING
  "Flags used by the C compiler during maintainer builds." FORCE
  )
SET( CMAKE_EXE_LINKER_FLAGS_MAINTAINER
  "${CMAKE_EXE_LINKER_FLAGS_DEBUG}" CACHE STRING
  "Flags used for linking binaries during maintainer builds." FORCE
  )
SET( CMAKE_SHARED_LINKER_FLAGS_MAINTAINER
  "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}" CACHE STRING
  "Flags used by the shared libraries linker during maintainer builds." FORCE
  )

MARK_AS_ADVANCED(
    CMAKE_CXX_FLAGS_MAINTAINER
    CMAKE_C_FLAGS_MAINTAINER
    CMAKE_EXE_LINKER_FLAGS_MAINTAINER
    CMAKE_SHARED_LINKER_FLAGS_MAINTAINER )

IF(LEMON_TOP_LEVEL AND CMAKE_CONFIGURATION_TYPES)
  LIST(APPEND CMAKE_CONFIGURATION_TYPES Maintainer)
  LIST(REMOVE_DUPLICATES CMAKE_CONFIGURATION_TYPES)
  SET(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
      "Add the configurations that we need"
      FORCE)
 endif()

IF(LEMON_TOP_LEVEL)
  IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "Release")
  ENDIF()

  SET( CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
      "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel Maintainer."
      FORCE )
ENDIF()

SET_DIRECTORY_PROPERTIES(PROPERTIES
  COMPILE_DEFINITIONS
  $<$<OR:$<CONFIG:Debug>,$<CONFIG:Maintainer>>:LEMON_ENABLE_DEBUG>
)

INCLUDE(CheckTypeSize)
CHECK_TYPE_SIZE("long long" LONG_LONG)
SET(LEMON_HAVE_LONG_LONG ${HAVE_LONG_LONG})

ENABLE_TESTING()


IF(NOT DEFINED CMAKE_CXX_STANDARD)
  SET(CMAKE_CXX_STANDARD 11)
ENDIF()
SET(CMAKE_CXX_STANDARD_REQUIRED ON)


IF(LEMON_TOP_LEVEL)
  IF("${CMAKE_BUILD_TYPE}" STREQUAL "Maintainer")
    ADD_CUSTOM_TARGET(check ALL COMMAND ${CMAKE_CTEST_COMMAND})
  ELSE()
    ADD_CUSTOM_TARGET(check COMMAND ${CMAKE_CTEST_COMMAND})
  ENDIF()
ENDIF()

OPTION(LEMON_BUILD_TESTS "Build the tests." ON)
OPTION(LEMON_BUILD_DEMOS "Build the demos and the contributed programs." ON)
OPTION(LEMON_BUILD_TOOLS "Build and install the tools." ON)
OPTION(LEMON_BUILD_DOC "Build the documentation." ON)

# the C++20 module lemon [see lemon/lemon.cppm], which needs CMake 3.28 or
# later, a generator and a compiler that build modules, and a compiler whose
# importers find the operators of the standard library that the templates of
# the module use (GCC, up to 15 at least, builds the module, but a unit that
# imports it does not find them): a small module and a unit that imports it
# are built to tell [see cmake/module-check], and where they are not, the
# option is turned off with a warning
OPTION(LEMON_BUILD_MODULE "Build the C++20 module lemon." OFF)
IF(LEMON_BUILD_MODULE)
  IF(CMAKE_VERSION VERSION_LESS 3.28)
    SET(_lemon_why "CMake ${CMAKE_VERSION} is older than 3.28")
  ELSE()
    SET(_lemon_scan "${CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS}")
    TRY_COMPILE(_lemon_module_ok PROJECT lemon_module_check
      SOURCE_DIR ${PROJECT_SOURCE_DIR}/cmake/module-check
      BINARY_DIR ${PROJECT_BINARY_DIR}/module-check
      CMAKE_FLAGS -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_CXX_COMPILER_CLANG_SCAN_DEPS=${_lemon_scan}
      OUTPUT_VARIABLE _lemon_module_out)
    IF(NOT _lemon_module_ok)
      SET(_lemon_why "the generator ${CMAKE_GENERATOR} and the compiler \
${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} do not build a module \
whose templates a unit that imports it can use [see cmake/module-check]")
    ENDIF()
  ENDIF()
  IF(_lemon_why)
    MESSAGE(WARNING "LEMON_BUILD_MODULE is turned OFF: ${_lemon_why}")
    SET(LEMON_BUILD_MODULE OFF)
  ELSE()
    # the scan of the dependencies of modules, for the library and the tests
    CMAKE_POLICY(SET CMP0155 NEW)
  ENDIF()
ENDIF()

# with LEMON as a DLL, the tests and the programs of the manual find it in
# the directory where it is built, which the PATH of any other system does
# not need: each directory of tests calls this at its end; the generator
# expression is expanded in the property of a test created by ADD_TEST(NAME
# ...) only, which all of them are
FUNCTION(LEMON_TESTS_FIND_LIBRARY)
  IF(BUILD_SHARED_LIBS)
    GET_DIRECTORY_PROPERTY(_lemon_tests TESTS)
    SET_TESTS_PROPERTIES(${_lemon_tests} PROPERTIES ENVIRONMENT_MODIFICATION
      "PATH=path_list_prepend:$<TARGET_FILE_DIR:lemon>")
  ENDIF()
ENDFUNCTION()

ADD_SUBDIRECTORY(lemon)
IF(${CMAKE_SOURCE_DIR} STREQUAL ${PROJECT_SOURCE_DIR})
  IF(LEMON_BUILD_DEMOS)
    ADD_SUBDIRECTORY(contrib)
    ADD_SUBDIRECTORY(demo)
  ENDIF()
  IF(LEMON_BUILD_TOOLS)
    ADD_SUBDIRECTORY(tools)
  ENDIF()
  IF(LEMON_BUILD_DOC)
    ADD_SUBDIRECTORY(doc)
  ENDIF()
  IF(LEMON_BUILD_TESTS)
    ADD_SUBDIRECTORY(test)
    ADD_SUBDIRECTORY(manual/examples)
  ENDIF()

ENDIF()

# the CMake configuration, as lemon-config.cmake, which find_package() finds
# both as LEMON and as lemon (trac #628), with the version file that
# find_package(LEMON 1.4) needs, when the version is a number
SET(LEMON_CMAKE_DIR ${CMAKE_INSTALL_DATADIR}/lemon/cmake)
CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/cmake/LEMONConfig.cmake.in
  ${PROJECT_BINARY_DIR}/cmake/lemon-config.cmake
  @ONLY
)
INSTALL(
  FILES ${PROJECT_BINARY_DIR}/cmake/lemon-config.cmake
  DESTINATION ${LEMON_CMAKE_DIR}
)
IF(LEMON_NUMERIC_VERSION)
  INCLUDE(CMakePackageConfigHelpers)
  WRITE_BASIC_PACKAGE_VERSION_FILE(
    ${PROJECT_BINARY_DIR}/cmake/lemon-config-version.cmake
    VERSION ${LEMON_NUMERIC_VERSION}
    COMPATIBILITY SameMajorVersion
  )
  INSTALL(
    FILES ${PROJECT_BINARY_DIR}/cmake/lemon-config-version.cmake
    DESTINATION ${LEMON_CMAKE_DIR}
  )
ENDIF()
INSTALL(
  EXPORT LEMONTargets
  NAMESPACE LEMON::
  DESTINATION ${LEMON_CMAKE_DIR}
)

CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/cmake/version.cmake.in
  ${PROJECT_BINARY_DIR}/cmake/version.cmake
  @ONLY
)

SET(ARCHIVE_BASE_NAME ${CMAKE_PROJECT_NAME})
STRING(TOLOWER ${ARCHIVE_BASE_NAME} ARCHIVE_BASE_NAME)
SET(ARCHIVE_NAME ${ARCHIVE_BASE_NAME}-${PROJECT_VERSION})
ADD_CUSTOM_TARGET(dist
  COMMAND cmake -E remove_directory ${ARCHIVE_NAME}
  COMMAND hg archive ${ARCHIVE_NAME}
  COMMAND cmake -E copy cmake/version.cmake ${ARCHIVE_NAME}/cmake/version.cmake
  COMMAND tar -czf ${ARCHIVE_BASE_NAME}-nodoc-${PROJECT_VERSION}.tar.gz ${ARCHIVE_NAME}
  COMMAND zip -r ${ARCHIVE_BASE_NAME}-nodoc-${PROJECT_VERSION}.zip ${ARCHIVE_NAME}
  COMMAND cmake -E copy_directory doc/html ${ARCHIVE_NAME}/doc/html
  COMMAND tar -czf ${ARCHIVE_NAME}.tar.gz ${ARCHIVE_NAME}
  COMMAND zip -r ${ARCHIVE_NAME}.zip ${ARCHIVE_NAME}
  COMMAND cmake -E copy_directory doc/html ${ARCHIVE_BASE_NAME}-doc-${PROJECT_VERSION}
  COMMAND tar -czf ${ARCHIVE_BASE_NAME}-doc-${PROJECT_VERSION}.tar.gz ${ARCHIVE_BASE_NAME}-doc-${PROJECT_VERSION}
  COMMAND zip -r ${ARCHIVE_BASE_NAME}-doc-${PROJECT_VERSION}.zip ${ARCHIVE_BASE_NAME}-doc-${PROJECT_VERSION}
  COMMAND cmake -E remove_directory ${ARCHIVE_NAME}
  COMMAND cmake -E remove_directory ${ARCHIVE_BASE_NAME}-doc-${PROJECT_VERSION}
  DEPENDS html
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

# CPACK config (Basically for NSIS)
IF(${CMAKE_SOURCE_DIR} STREQUAL ${PROJECT_SOURCE_DIR})
  SET(CPACK_PACKAGE_NAME ${PROJECT_NAME})
  SET(CPACK_PACKAGE_VENDOR "EGRES")
  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "LEMON - Library for Efficient Modeling and Optimization in Networks")
  SET(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")

  SET(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

  SET(CPACK_PACKAGE_INSTALL_DIRECTORY
    "${PROJECT_NAME} ${PROJECT_VERSION}")
  SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY
    "${PROJECT_NAME} ${PROJECT_VERSION}")

  SET(CPACK_COMPONENTS_ALL headers library html_documentation bin)

  SET(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C++ headers")
  SET(CPACK_COMPONENT_LIBRARY_DISPLAY_NAME "Dynamic-link library")
  SET(CPACK_COMPONENT_BIN_DISPLAY_NAME "Command line utilities")
  SET(CPACK_COMPONENT_HTML_DOCUMENTATION_DISPLAY_NAME "HTML documentation")

  SET(CPACK_COMPONENT_HEADERS_DESCRIPTION
    "C++ header files")
  SET(CPACK_COMPONENT_LIBRARY_DESCRIPTION
    "DLL and import library")
  SET(CPACK_COMPONENT_BIN_DESCRIPTION
    "Command line utilities")
  SET(CPACK_COMPONENT_HTML_DOCUMENTATION_DESCRIPTION
    "Doxygen generated documentation")

  SET(CPACK_COMPONENT_HEADERS_DEPENDS library)

  SET(CPACK_COMPONENT_HEADERS_GROUP "Development")
  SET(CPACK_COMPONENT_LIBRARY_GROUP "Development")
  SET(CPACK_COMPONENT_HTML_DOCUMENTATION_GROUP "Documentation")

  SET(CPACK_COMPONENT_GROUP_DEVELOPMENT_DESCRIPTION
    "Components needed to develop software using LEMON")
  SET(CPACK_COMPONENT_GROUP_DOCUMENTATION_DESCRIPTION
    "Documentation of LEMON")

  SET(CPACK_ALL_INSTALL_TYPES Full Developer)

  SET(CPACK_COMPONENT_HEADERS_INSTALL_TYPES Developer Full)
  SET(CPACK_COMPONENT_LIBRARY_INSTALL_TYPES Developer Full)
  SET(CPACK_COMPONENT_HTML_DOCUMENTATION_INSTALL_TYPES Full)

  SET(CPACK_GENERATOR "NSIS")
  SET(CPACK_NSIS_MUI_ICON "${PROJECT_SOURCE_DIR}/cmake/nsis/lemon.ico")
  SET(CPACK_NSIS_MUI_UNIICON "${PROJECT_SOURCE_DIR}/cmake/nsis/uninstall.ico")
  #SET(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/cmake/nsis\\\\installer.bmp")
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\lemon.ico")
  SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} ${PROJECT_NAME}")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\lemon.cs.elte.hu")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\lemon.cs.elte.hu")
  SET(CPACK_NSIS_CONTACT "lemon-user@lemon.cs.elte.hu")
  SET(CPACK_NSIS_CREATE_ICONS_EXTRA "
    CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documentation.lnk\\\" \\\"$INSTDIR\\\\share\\\\doc\\\\index.html\\\"
    ")
  SET(CPACK_NSIS_DELETE_ICONS_EXTRA "
    !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP
    Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documentation.lnk\\\"
    ")

  INCLUDE(CPack)
ENDIF()
