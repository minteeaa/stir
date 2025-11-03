cmake_minimum_required(VERSION 3.28...3.30)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

option(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" ON)
option(ENABLE_QT "Use Qt functionality" OFF)

include(compilerconfig)
include(defaults)
include(helpers)

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

if(ENABLE_FRONTEND_API)
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
endif()

if(ENABLE_QT)
  find_package(Qt6 COMPONENTS Widgets Core)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets)
  target_compile_options(
    ${CMAKE_PROJECT_NAME}
    PRIVATE $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header -Wno-comma>
  )
  set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON
  )
endif()

target_sources(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
    src/filters/common.c
    src/filters/common.h
    src/filters/stir-echo.c
    src/filters/stir-echo.h
    src/filters/stir-gain.c
    src/filters/stir-gain.h
    src/filters/stir-lowpass.c
    src/filters/stir-lowpass.h
    src/filters/stir-router.c
    src/filters/stir-router.h
    src/filters/stir-highpass.c
    src/filters/stir-highpass.h
    src/filters/stir-tremolo.c
    src/filters/stir-tremolo.h
    src/init.c
    src/init.h
    src/chain.c
    src/chain.h
    src/stir-context.c
    src/stir-context.h
    src/util.c
    src/util.h
    src/ext/uthash.h
)

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})

get_target_property(target_type ${CMAKE_PROJECT_NAME} TYPE)
message(STATUS "Target type: ${target_type}")
