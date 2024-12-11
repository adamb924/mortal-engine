cmake_minimum_required(VERSION 3.16)

project(mortalengine LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_DEBUG_POSTFIX d)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Sql Xml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Sql Xml)

add_library(mortalengine SHARED
    constraints/ignoreflag.cpp
    constraints/precedingnodeconstraint.cpp
    constraints/satisfiedcondition.cpp
    datatypes/hashseed.cpp
    datatypes/morphemelabel.cpp
    datatypes/morphemesequence.cpp
    datatypes/parsingsummary.cpp
    datatypes/portmanteau.cpp
    debug.cpp
    generation-constraints/abstractgenerationconstraint.cpp
    generation-constraints/stemidentityconstraint.cpp
    datatypes/generation.cpp
    datatypes/lexicalstem.cpp
    constraints/abstractconstraint.cpp
    constraints/abstractlongdistanceconstraint.cpp
    constraints/abstractmatchcondition.cpp
    constraints/abstractnestedconstraint.cpp
    constraints/andcondition.cpp
    constraints/boundcondition.cpp
    constraints/followingnodeconstraint.cpp
    constraints/followingphonologicalcondition.cpp
    constraints/notcondition.cpp
    constraints/orcondition.cpp
    constraints/phonologicalcondition.cpp
    constraints/pointertoconstraint.cpp
    constraints/tagmatchcondition.cpp
    constraints/wordfinalcondition.cpp
    messages.cpp
    nodes/abstractnode.cpp
    datatypes/allomorph.cpp
    datatypes/form.cpp
    nodes/abstractpath.cpp
    nodes/abstractsqlstemlist.cpp
    nodes/abstractstemlist.cpp
    nodes/copynode.cpp
    nodes/fork.cpp
    nodes/jump.cpp
    nodes/morphemenode.cpp
    nodes/path.cpp
    nodes/morphologicalmodel.cpp
    nodes/sequence.cpp
    nodes/sqlserverstemlist.cpp
    nodes/xmlstemlist.cpp
    generation-constraints/morphemesequenceconstraint.cpp
    morphology.cpp
    nodes/mutuallyexclusivemorphemes.cpp
    morphologychecker.cpp
    morphologyxmlreader.cpp
    datatypes/parsing.cpp
    datatypes/parsingstep.cpp
    returns/lexicalsteminsertresult.cpp
    create-allomorphs/createallomorphs.cpp
    create-allomorphs/createallomorphscase.cpp
    create-allomorphs/createallomorphsreplacement.cpp
    datatypes/tag.cpp
    datatypes/writingsystem.cpp
    nodes/sqlitestemlist.cpp
)

target_include_directories(mortalengine PUBLIC ".")

target_link_libraries(mortalengine PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Xml)

target_compile_definitions(mortalengine PRIVATE MORTALENGINE_LIBRARY)

install(TARGETS mortalengine
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)

# this installs some non-public headers, but so be it
install(DIRECTORY "${CMAKE_SOURCE_DIR}/mortal-engine"
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
        FILES_MATCHING
        PATTERN "*.h"
)
