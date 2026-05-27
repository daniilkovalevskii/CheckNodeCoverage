cmake_minimum_required(VERSION 3.19)
project(CheckNodeCoverage LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core)

qt_standard_project_setup()

qt_add_executable(CheckNodeCoverage
    main.cpp
    structures.h
)

target_link_libraries(CheckNodeCoverage
    PRIVATE
        Qt::Core
)

include(GNUInstallDirs)

install(TARGETS CheckNodeCoverage
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
