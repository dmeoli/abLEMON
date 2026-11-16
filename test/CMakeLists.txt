INCLUDE_DIRECTORIES(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
)

LINK_DIRECTORIES(
  ${PROJECT_BINARY_DIR}/lemon
)

SET(TEST_WITH_VALGRIND "NO" CACHE STRING
  "Run the test with valgrind (YES/NO).")
SET(VALGRIND_FLAGS "" CACHE STRING "Valgrind flags used by the tests.")

SET(TESTS
  adaptors_test
  arc_look_up_test
  bellman_ford_test
  bfs_test
  bipartite_matching_test
  bpgraph_test
  circulation_test
  concept_check_test
  connectivity_test
  counter_test
  dfs_test
  digraph_test
  dijkstra_test
  dim_test
  dimacs_test
  edge_case_test
  edge_set_test
  elevator_test
  error_test
  euler_test
  fractional_matching_test
  gomory_hu_test
  graph_copy_test
  graph_test
  graph_to_eps_test
  graph_utils_test
  hao_orlin_test
  heap_test
  k_best_flows_test
  kruskal_test
  lgf_reader_writer_test
  lgf_test
  lp_skeleton_test
  macro_test
  maps_test
  matching_test
  max_cardinality_search_test
  max_clique_test
  max_flow_test
  min_cost_arborescence_test
  min_cost_flow_test
  min_mean_cycle_test
  nagamochi_ibaraki_test
  nauty_reader_test
  path_test
  point_to_point_test
  planarity_test
  radix_sort_test
  random_test
  suurballe_test
  time_measure_test
  tomita_seki_mc_test
  tsp_test
  unionfind_test
  vf2_test
)

# a unit that imports the module lemon instead of including the headers
IF(LEMON_BUILD_MODULE)
  ADD_EXECUTABLE(module_test module_test.cc)
  TARGET_LINK_LIBRARIES(module_test lemon_module)
  ADD_TEST(NAME module_test COMMAND module_test)
  ADD_DEPENDENCIES(check module_test)
ENDIF()

# the DLLs of GLPK and of CPLEX, where the solvers have them, copied next to
# a test that links them
FUNCTION(LEMON_COPY_SOLVER_DLLS _lemon_test)
  SET(_lemon_dlls "")
  IF(LEMON_HAVE_GLPK)
    FOREACH(_lemon_dll glpk.dll libltdl3.dll zlib1.dll)
      IF(EXISTS ${GLPK_BIN_DIR}/${_lemon_dll})
        LIST(APPEND _lemon_dlls ${GLPK_BIN_DIR}/${_lemon_dll})
      ENDIF()
    ENDFOREACH()
  ENDIF()
  IF(LEMON_HAVE_CPLEX AND ILOG_CPLEX_DLL)
    LIST(APPEND _lemon_dlls ${ILOG_CPLEX_DLL})
  ENDIF()
  IF(_lemon_dlls)
    ADD_CUSTOM_COMMAND(TARGET ${_lemon_test} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${_lemon_dlls}
        $<TARGET_FILE_DIR:${_lemon_test}>)
  ENDIF()
ENDFUNCTION()

IF(LEMON_HAVE_LP)
  IF(${CMAKE_BUILD_TYPE} STREQUAL "Maintainer")
    ADD_EXECUTABLE(lp_test lp_test.cc)
  ELSE()
    ADD_EXECUTABLE(lp_test lp_test.cc)
  ENDIF()

  SET(LP_TEST_LIBS lemon)

  IF(LEMON_HAVE_GLPK)
    SET(LP_TEST_LIBS ${LP_TEST_LIBS} ${GLPK_LIBRARIES})
  ENDIF()
  IF(LEMON_HAVE_CPLEX)
    SET(LP_TEST_LIBS ${LP_TEST_LIBS} ${ILOG_LIBRARIES})
  ENDIF()
  IF(LEMON_HAVE_CLP)
    SET(LP_TEST_LIBS ${LP_TEST_LIBS} ${COIN_CLP_LIBRARIES})
  ENDIF()
  IF(LEMON_HAVE_SOPLEX)
    SET(LP_TEST_LIBS ${LP_TEST_LIBS} ${SOPLEX_LIBRARIES})
  ENDIF()

  TARGET_LINK_LIBRARIES(lp_test ${LP_TEST_LIBS})
  ADD_TEST(NAME lp_test COMMAND lp_test)
  ADD_DEPENDENCIES(check lp_test)

  LEMON_COPY_SOLVER_DLLS(lp_test)
ENDIF()

IF(LEMON_HAVE_MIP)
  IF(${CMAKE_BUILD_TYPE} STREQUAL "Maintainer")
    ADD_EXECUTABLE(mip_test mip_test.cc)
  ELSE()
    ADD_EXECUTABLE(mip_test mip_test.cc)
  ENDIF()

  SET(MIP_TEST_LIBS lemon)

  IF(LEMON_HAVE_GLPK)
    SET(MIP_TEST_LIBS ${MIP_TEST_LIBS} ${GLPK_LIBRARIES})
  ENDIF()
  IF(LEMON_HAVE_CPLEX)
    SET(MIP_TEST_LIBS ${MIP_TEST_LIBS} ${ILOG_LIBRARIES})
  ENDIF()
  IF(LEMON_HAVE_CBC)
    SET(MIP_TEST_LIBS ${MIP_TEST_LIBS} ${COIN_CBC_LIBRARIES})
  ENDIF()

  TARGET_LINK_LIBRARIES(mip_test ${MIP_TEST_LIBS})
  ADD_TEST(NAME mip_test COMMAND mip_test)
  ADD_DEPENDENCIES(check mip_test)

  LEMON_COPY_SOLVER_DLLS(mip_test)
ENDIF()

FOREACH(TEST_NAME ${TESTS})
  IF(${CMAKE_BUILD_TYPE} STREQUAL "Maintainer")
    ADD_EXECUTABLE(${TEST_NAME} ${TEST_NAME}.cc)
  ELSE()
    ADD_EXECUTABLE(${TEST_NAME} ${TEST_NAME}.cc)
  ENDIF()
  TARGET_LINK_LIBRARIES(${TEST_NAME} lemon)
    IF(TEST_WITH_VALGRIND)
      ADD_TEST(NAME ${TEST_NAME}
        COMMAND valgrind --error-exitcode=1 ${VALGRIND_FLAGS}
        $<TARGET_FILE:${TEST_NAME}>)
    ELSE()
      ADD_TEST(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    ENDIF()
  ADD_DEPENDENCIES(check ${TEST_NAME})
ENDFOREACH()

# the elevators with the checks of the standard library on their vectors,
# where the library has them (a macro that no other library reads)
TARGET_COMPILE_DEFINITIONS(elevator_test PRIVATE _GLIBCXX_ASSERTIONS)

# the battery of the minimum cost flow algorithms, once for each value type
ADD_EXECUTABLE(min_cost_flow_battery_test min_cost_flow_battery_test.cc)
TARGET_LINK_LIBRARIES(min_cost_flow_battery_test lemon)
FOREACH(VALUE_TYPE long double)
  ADD_TEST(NAME min_cost_flow_battery_${VALUE_TYPE}
    COMMAND min_cost_flow_battery_test ${VALUE_TYPE})
ENDFOREACH()
ADD_DEPENDENCIES(check min_cost_flow_battery_test)

# the tools on small DIMACS files written here and on few nodes, their output
# and their report matched by regular expressions
IF(LEMON_BUILD_TOOLS)
  FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/unbounded.min
    "p min 2 2\na 1 2 0 -1 -1\na 2 1 0 -1 -1\n")
  ADD_TEST(NAME dimacs_solver_unbounded
    COMMAND dimacs-solver ${CMAKE_CURRENT_BINARY_DIR}/unbounded.min)
  SET_TESTS_PROPERTIES(dimacs_solver_unbounded PROPERTIES
    PASS_REGULAR_EXPRESSION "s UNBOUNDED" FAIL_REGULAR_EXPRESSION "cost:")
  ADD_DEPENDENCIES(check dimacs-solver)
  FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/unknown.dimacs "p xyz 2 1\na 1 2\n")
  FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/nosource.max "p max 2 1\na 1 2 5\n")
  FOREACH(INPUT unknown.dimacs nosource.max)
    STRING(REPLACE "." "_" NAME dimacs_to_lgf_${INPUT})
    ADD_TEST(NAME ${NAME}
      COMMAND dimacs-to-lgf ${CMAKE_CURRENT_BINARY_DIR}/${INPUT})
    SET_TESTS_PROPERTIES(${NAME} PROPERTIES
      PASS_REGULAR_EXPRESSION "dimacs-to-lgf[^:]*: ")
  ENDFOREACH()
  ADD_DEPENDENCIES(check dimacs-to-lgf)
  ADD_TEST(NAME lgf_gen_tsp2_one
    COMMAND lgf-gen -tsp2 -n 1 ${CMAKE_CURRENT_BINARY_DIR}/lgf_gen_tsp2_one)
  SET_TESTS_PROPERTIES(lgf_gen_tsp2_one PROPERTIES
    PASS_REGULAR_EXPRESSION "need at least 3 nodes" TIMEOUT 60)
  FOREACH(ALG dela tree)
    ADD_TEST(NAME lgf_gen_${ALG}_empty COMMAND lgf-gen -${ALG} -n 0
      ${CMAKE_CURRENT_BINARY_DIR}/lgf_gen_${ALG}_empty)
    SET_TESTS_PROPERTIES(lgf_gen_${ALG}_empty PROPERTIES TIMEOUT 60)
  ENDFOREACH()
  ADD_DEPENDENCIES(check lgf-gen)
ENDIF()

# with LEMON as a DLL, the tests find it in the directory where it is built,
# which the PATH of any other system does not need; the generator expression
# is expanded in the property of a test created by ADD_TEST(NAME ...) only,
# which all of them are
IF(BUILD_SHARED_LIBS)
  GET_DIRECTORY_PROPERTY(_lemon_tests TESTS)
  SET_TESTS_PROPERTIES(${_lemon_tests} PROPERTIES ENVIRONMENT_MODIFICATION
    "PATH=path_list_prepend:$<TARGET_FILE_DIR:lemon>")
ENDIF()

