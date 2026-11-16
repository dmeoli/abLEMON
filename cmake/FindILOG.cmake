FIND_PATH(ILOG_ROOT_DIR
  NAMES cplex
  DOC "CPLEX STUDIO root directory"
  PATHS /opt/ibm/ILOG /usr/local/ibm/ILOG /usr/local/ILOG /usr/local/ilog
  PATHS "$ENV{HOME}/ILOG" "$ENV{HOME}/.local/ILOG"
  PATHS "$ENV{HOME}/ibm/ILOG" "$ENV{HOME}/.local/ibm/ILOG"
  PATHS "C:/Program Files/IBM/ILOG" 
  PATH_SUFFIXES "CPLEX_Studio126" "CPLEX_Studio125"
  "CPLEX_Studio124" "CPLEX_Studio123" "CPLEX_Studio122"
  NO_DEFAULT_PATH
)

# the directories of the libraries and of the DLLs of every platform that
# CPLEX Studio ships, each looked for in turn
SET(ILOG_LIB_SUFFIXES
  x86_sles10_4.1/static_pic x86-64_sles10_4.1/static_pic
  x86_debian4.0_4.1/static_pic x86-64_debian4.0_4.1/static_pic
  x86_linux/static_pic x86-64_linux/static_pic
  x64_windows_vs2010/stat_mda x64_windows_vs2008/stat_mda
  x64_windows_vs2005/stat_mda x86_windows_vs2010/stat_mda
  x86_windows_vs2008/stat_mda x86_windows_vs2005/stat_mda)
SET(ILOG_BIN_SUFFIXES x64_win32 x86_win32)

FIND_PATH(ILOG_CPLEX_ROOT_DIR
  NAMES include/ilcplex
  HINTS ${ILOG_ROOT_DIR}/cplex ${ILOG_ROOT_DIR}/cplex121
  ${ILOG_ROOT_DIR}/cplex122 ${ILOG_ROOT_DIR}/cplex123
  DOC "CPLEX root directory"
  NO_DEFAULT_PATH
)

FIND_PATH(ILOG_CONCERT_ROOT_DIR
  NAMES include/ilconcert
  HINTS ${ILOG_ROOT_DIR}/concert ${ILOG_ROOT_DIR}/concert29
  DOC "CONCERT root directory"
  NO_DEFAULT_PATH
)

FIND_PATH(ILOG_CPLEX_INCLUDE_DIR
  ilcplex/cplex.h
  HINTS ${ILOG_CPLEX_ROOT_DIR}/include
  NO_DEFAULT_PATH
)

FIND_PATH(ILOG_CONCERT_INCLUDE_DIR
  ilconcert/ilobasic.h
  HINTS ${ILOG_CONCERT_ROOT_DIR}/include
  NO_DEFAULT_PATH
)

FIND_LIBRARY(ILOG_CPLEX_LIBRARY
  cplex cplex121 cplex122 cplex123 cplex124
  HINTS ${ILOG_CPLEX_ROOT_DIR}/lib
  PATH_SUFFIXES ${ILOG_LIB_SUFFIXES}
  NO_DEFAULT_PATH
  )

FIND_LIBRARY(ILOG_CONCERT_LIBRARY
  concert
  HINTS ${ILOG_CONCERT_ROOT_DIR}/lib
  PATH_SUFFIXES ${ILOG_LIB_SUFFIXES}
  NO_DEFAULT_PATH
  )

FIND_FILE(ILOG_CPLEX_DLL
  cplex121.dll cplex122.dll cplex123.dll cplex124.dll
  HINTS ${ILOG_CPLEX_ROOT_DIR}/bin
  PATH_SUFFIXES ${ILOG_BIN_SUFFIXES}
  NO_DEFAULT_PATH
  )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ILOG
  DEFAULT_MSG ILOG_CPLEX_LIBRARY ILOG_CPLEX_INCLUDE_DIR
  )

IF(ILOG_FOUND)
  SET(ILOG_INCLUDE_DIRS ${ILOG_CPLEX_INCLUDE_DIR} ${ILOG_CONCERT_INCLUDE_DIR})
  SET(ILOG_LIBRARIES ${ILOG_CPLEX_LIBRARY} ${ILOG_CONCERT_LIBRARY})
  # the libraries of the system that the static CPLEX calls, where the
  # system has them apart: the mathematical one, the threads and dl
  FIND_LIBRARY(ILOG_M_LIBRARY m)
  IF(ILOG_M_LIBRARY)
    LIST(APPEND ILOG_LIBRARIES ${ILOG_M_LIBRARY})
  ENDIF()
  FIND_PACKAGE(Threads)
  LIST(APPEND ILOG_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
  LIST(APPEND ILOG_LIBRARIES ${CMAKE_DL_LIBS})
ENDIF(ILOG_FOUND)

MARK_AS_ADVANCED(
  ILOG_M_LIBRARY ILOG_CPLEX_LIBRARY ILOG_CPLEX_INCLUDE_DIR ILOG_CPLEX_DLL
  ILOG_CONCERT_LIBRARY ILOG_CONCERT_INCLUDE_DIR ILOG_CONCERT_DLL
  )
