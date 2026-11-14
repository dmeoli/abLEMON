# The programs of the manual, which the chapters name: each is built and run
# as the test manual_<name>, so that what the manual states of a class is
# checked by a program. A program checks its own results and returns nonzero
# if one is wrong.

INCLUDE_DIRECTORIES(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
)

FILE(GLOB MANUAL_EXAMPLES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
     ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
LIST(SORT MANUAL_EXAMPLES)

FOREACH(SRC ${MANUAL_EXAMPLES})
  GET_FILENAME_COMPONENT(NAME ${SRC} NAME_WE)
  STRING(REPLACE "-" "_" TARGET manual_${NAME})
  ADD_EXECUTABLE(${TARGET} ${SRC})
  TARGET_LINK_LIBRARIES(${TARGET} lemon)
  ADD_TEST(NAME ${TARGET} COMMAND ${TARGET}
           WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
ENDFOREACH()
