cmake_minimum_required(VERSION 3.5)

project(launcher VERSION 0.1 LANGUAGES CXX)

include(cmake/Variables.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS 5Widgets 5Sql Network)
find_package(Qt5 REQUIRED COMPONENTS Widgets Sql Network)

set(PRJ_RESOURCES)
set(PRJ_DEFINES)
set(PRJ_SRCS)


file(GLOB_RECURSE PRJ_SRCS0 "*.h" "*.cpp" "*.ui")
list(APPEND PRJ_SRCS ${PRJ_SRCS0})

add_executable(launcher
    ${PROJECT_SOURCES}
)

target_sources(${PROJECT_NAME} PRIVATE ${PRJ_SRCS} ${PRJ_UIS} ${PRJ_RESOURCES} ${QM_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR})


set_target_properties(launcher PROPERTIES
    ${BUNDLE_ID_OPTION}
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(launcher
    PRIVATE
    Qt5::Widgets
    Qt5::Sql
    Qt5::Network
)

include(GNUInstallDirs)
install(TARGETS launcher
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
