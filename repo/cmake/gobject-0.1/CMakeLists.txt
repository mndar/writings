cmake_minimum_required (VERSION 3.0)

add_compile_options (-fdiagnostics-color=always)
project (gobject-test)
set (CMAKE_C_FLAGS "-g ${CMAKE_C_FLAGS}")
file(GLOB GOBJECTTEST_SRC
    "src/*.h"
    "src/*.c"
)

add_executable(gobjecttest ${GOBJECTTEST_SRC})
find_package(PkgConfig REQUIRED)
SET (PACKAGES glib-2.0 gobject-2.0)

foreach(PKG_NAME IN ITEMS ${PACKAGES} )
  #MESSAGE (${PKG_NAME})
  pkg_search_module(${PKG_NAME} REQUIRED ${PKG_NAME} )
  target_link_libraries(gobjecttest ${${PKG_NAME}_LIBRARIES})
  target_include_directories(gobjecttest PUBLIC ${${PKG_NAME}_INCLUDE_DIRS})
endforeach(PKG_NAME)

#Install
include(GNUInstallDirs)
set (CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
install (TARGETS gobjecttest RUNTIME DESTINATION bin)

#build debian packages
SET(CPACK_GENERATOR "DEB")
SET(CPACK_PACKAGE_VERSION_MAJOR  1)
SET(CPACK_PACKAGE_VERSION_MINOR  0)
SET(CPACK_PACKAGE_DESCRIPTION "A Demonstartion of GObject")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "ABC") #required
INCLUDE(CPack)
