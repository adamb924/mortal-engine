cmake_minimum_required(VERSION 3.16)

project(mortal-engine-test LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Sql Xml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Sql Xml)

# Use generator expressions to add the "d" suffix in Debug mode
find_library(MORTALENGINE_LIB_RELEASE NAMES mortalengine)
find_library(MORTALENGINE_LIB_DEBUG NAMES mortalengined)

message(STATUS "MORTALENGINE_LIB_RELEASE = ${MORTALENGINE_LIB_RELEASE}")
message(STATUS "MORTALENGINE_LIB_DEBUG = ${MORTALENGINE_LIB_DEBUG}")

add_executable(mortal-engine-test
    main.cpp
    abstractinputoutputtest.cpp
    abstracttest.cpp
    generationtest.cpp
    harnessxmlreader.cpp
    interlinearglosstest.cpp
    main.cpp
    message.cpp
    parsingtest.cpp
    recognitiontest.cpp
    stemreplacementtest.cpp
    suggestiontest.cpp
    testharness.cpp
    testschema.cpp
    transductiontest.cpp
    abstractinputoutputtest.h
    abstracttest.h
    generationtest.h
    harnessxmlreader.h
    interlinearglosstest.h
    message.h
    parsingtest.h
    recognitiontest.h
    stemreplacementtest.h
    suggestiontest.h
    testharness.h
    testschema.h
    transductiontest.h
)

set(MORTALENGINE_INCLUDE "../mortal-engine")
target_include_directories(mortal-engine-test PUBLIC  ${MORTALENGINE_INCLUDE})

target_link_libraries(mortal-engine-test
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Xml
    $<$<CONFIG:Debug>:${MORTALENGINE_LIB_DEBUG}>
    $<$<NOT:$<CONFIG:Debug>>:${MORTALENGINE_LIB_RELEASE}> )
