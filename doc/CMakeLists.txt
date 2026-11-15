SET(PACKAGE_NAME ${PROJECT_NAME})
SET(PACKAGE_VERSION ${PROJECT_VERSION})
SET(abs_top_srcdir ${PROJECT_SOURCE_DIR})
SET(abs_top_builddir ${PROJECT_BINARY_DIR})

SET(LEMON_DOC_SOURCE_BROWSER "NO" CACHE STRING
  "Include source into the doc (YES/NO).")
# the formulae drawn by MathJax in the browser, which needs no LaTeX where
# the documentation is built
SET(LEMON_DOC_USE_MATHJAX "YES" CACHE STRING
  "Use MathJax to display math formulae (YES/NO).")
SET(LEMON_DOC_MATHJAX_RELPATH "https://cdn.jsdelivr.net/npm/mathjax@3"
  CACHE STRING "MathJax library location.")

CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/doc/Doxyfile.in
  ${PROJECT_BINARY_DIR}/doc/Doxyfile
  @ONLY
)

# Copy doc from source (if exists)
IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/html AND 
    NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/html/index.html)
  MESSAGE(STATUS "Copy doc from source tree")
  EXECUTE_PROCESS(
    COMMAND cmake -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/html
            ${CMAKE_CURRENT_BINARY_DIR}/html
    )
ENDIF()

IF(DOXYGEN_EXECUTABLE AND PYTHONINTERP_FOUND AND GHOSTSCRIPT_EXECUTABLE)
  FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/)
  SET(GHOSTSCRIPT_OPTIONS -dNOPAUSE -dBATCH -q -dEPSCrop -dTextAlphaBits=4
    -dGraphicsAlphaBits=4 -sDEVICE=pngalpha)
  # the images of the pages, as name:resolution of the PNG made from the EPS
  SET(LEMON_DOC_IMAGES
    grid_graph:20
    adaptors2:32
    connected_components:32
    strongly_connected_components:32
    node_biconnected_components:32
    edge_biconnected_components:32
    bipartite_partitions:32
    matching:24
    bipartite_matching:24
    planar:40
    tsp:24
    nodeshape_0:8
    nodeshape_1:8
    nodeshape_2:8
    nodeshape_3:8
    nodeshape_4:8)
  SET(GS_COMMANDS)
  FOREACH(IMG ${LEMON_DOC_IMAGES})
    STRING(REPLACE ":" ";" IMG ${IMG})
    LIST(GET IMG 0 NAME)
    LIST(GET IMG 1 RES)
    LIST(APPEND GS_COMMANDS COMMAND ${GHOSTSCRIPT_EXECUTABLE}
      ${GHOSTSCRIPT_OPTIONS} -r${RES} -sOutputFile=gen-images/${NAME}.png
      ${CMAKE_CURRENT_SOURCE_DIR}/images/${NAME}.eps)
  ENDFOREACH()

  ADD_CUSTOM_TARGET(html
    COMMAND ${CMAKE_COMMAND} -E remove_directory gen-images
    COMMAND ${CMAKE_COMMAND} -E make_directory gen-images
    ${GS_COMMANDS}
    COMMAND ${CMAKE_COMMAND} -E remove_directory html
    COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/codestyle_html.py html
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )

  SET_TARGET_PROPERTIES(html PROPERTIES PROJECT_LABEL BUILD_DOC)

  IF(UNIX)
    INSTALL(
      DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/
      DESTINATION share/doc/lemon/html
      COMPONENT html_documentation
    )
  ELSEIF(WIN32)
    INSTALL(
      DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/
      DESTINATION doc
      COMPONENT html_documentation
    )
  ENDIF()

ENDIF()
