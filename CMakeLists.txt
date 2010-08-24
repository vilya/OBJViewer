project(OBJViewer)
cmake_minimum_required(VERSION 2.6)

# External libraries that we use.
find_package(GLUT)
find_package(OpenGL)
find_package(OpenMP)

# General build properties.
file(GLOB OBJViewer_SOURCES src/*.cpp)
file(GLOB OBJViewer_HEADERS src/*.h)

# Compilation and linking properties.
set_source_files_properties(${OBJViewer_SOURCES}
  COMPILE_FLAGS "${OpenMP_CXX_FLAGS}")

# The VGL submodule.
add_subdirectory(VGL vgl)

# The main executable.
include_directories(src ${CMAKE_SOURCE_DIR}/VGL/src)
link_directories(${CMAKE_BINARY_DIR}/vgl)
add_executable(objviewer ${OBJViewer_SOURCES})
target_link_libraries(objviewer vgl
  ${OPENGL_LIBRARIES}
  ${GLUT_LIBRARIES})
set_target_properties(objviewer PROPERTIES
  LINK_FLAGS "${OpenMP_CXX_FLAGS}")
add_dependencies(objviewer vgl)
