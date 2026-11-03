SET(SOPLEX_ROOT_DIR "" CACHE PATH "SoPlex root directory")

# SoPlex 5 and later install a CMake configuration, whose target carries the
# libraries that the headers of SoPlex call directly (GMP, MPFR, zlib, TBB
# and BLAS, with PaPILO), and SOPLEX_LIBRARIES is then that target; without
# it, the library and soplex.h are looked for by path; the dependencies of
# the configuration are looked for in SOPLEX_ROOT_DIR too
SET(_soplex_prefix_path ${CMAKE_PREFIX_PATH})
LIST(APPEND CMAKE_PREFIX_PATH ${SOPLEX_ROOT_DIR})
FIND_PACKAGE(soplex CONFIG QUIET)
SET(CMAKE_PREFIX_PATH ${_soplex_prefix_path})
IF(soplex_FOUND)
  FOREACH(_soplex_target libsoplexshared libsoplex-pic libsoplex)
    IF(TARGET ${_soplex_target})
      SET(SOPLEX_LIBRARY ${_soplex_target})
      BREAK()
    ENDIF()
  ENDFOREACH()
  GET_TARGET_PROPERTY(SOPLEX_INCLUDE_DIR ${SOPLEX_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES)
  GET_FILENAME_COMPONENT(SOPLEX_PREFIX "${soplex_DIR}/../../.." ABSOLUTE)
ELSE()
  FIND_PATH(SOPLEX_INCLUDE_DIR
    soplex.h
    HINTS ${SOPLEX_ROOT_DIR}/include ${SOPLEX_ROOT_DIR}/src
  )
  FIND_LIBRARY(SOPLEX_LIBRARY
    soplex
    HINTS ${SOPLEX_ROOT_DIR}/lib
  )
ENDIF()

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SOPLEX DEFAULT_MSG SOPLEX_LIBRARY SOPLEX_INCLUDE_DIR)

IF(SOPLEX_FOUND)
  SET(SOPLEX_INCLUDE_DIRS ${SOPLEX_INCLUDE_DIR})
  SET(SOPLEX_LIBRARIES ${SOPLEX_LIBRARY})
  IF(NOT soplex_FOUND AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
    SET(SOPLEX_LIBRARIES "${SOPLEX_LIBRARIES};z")
  ENDIF()
ENDIF(SOPLEX_FOUND)

MARK_AS_ADVANCED(SOPLEX_LIBRARY SOPLEX_INCLUDE_DIR)
