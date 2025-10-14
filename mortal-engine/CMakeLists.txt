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
    mortal-engine_global.h
    constraints/ignoreflag.h constraints/ignoreflag.cpp
    constraints/precedingnodeconstraint.h constraints/precedingnodeconstraint.cpp
    constraints/satisfiedcondition.h constraints/satisfiedcondition.cpp
    datatypes/hashseed.h datatypes/hashseed.cpp
    datatypes/morphemelabel.h datatypes/morphemelabel.cpp
    datatypes/morphemesequence.h datatypes/morphemesequence.cpp
    datatypes/parsingsummary.h datatypes/parsingsummary.cpp
    datatypes/portmanteau.h datatypes/portmanteau.cpp
    debug.h debug.cpp
    generation-constraints/abstractgenerationconstraint.h generation-constraints/abstractgenerationconstraint.cpp
    generation-constraints/stemidentityconstraint.h generation-constraints/stemidentityconstraint.cpp
    datatypes/generation.h datatypes/generation.cpp
    datatypes/lexicalstem.h datatypes/lexicalstem.cpp
    constraints/abstractconstraint.h constraints/abstractconstraint.cpp
    constraints/abstractlongdistanceconstraint.h constraints/abstractlongdistanceconstraint.cpp
    constraints/abstractmatchcondition.h constraints/abstractmatchcondition.cpp
    constraints/abstractnestedconstraint.h constraints/abstractnestedconstraint.cpp
    constraints/andcondition.h constraints/andcondition.cpp
    constraints/boundcondition.h constraints/boundcondition.cpp
    constraints/followingnodeconstraint.h constraints/followingnodeconstraint.cpp
    constraints/followingphonologicalcondition.h constraints/followingphonologicalcondition.cpp
    constraints/notcondition.h constraints/notcondition.cpp
    constraints/orcondition.h constraints/orcondition.cpp
    constraints/phonologicalcondition.h constraints/phonologicalcondition.cpp
    constraints/pointertoconstraint.h constraints/pointertoconstraint.cpp
    constraints/tagmatchcondition.h constraints/tagmatchcondition.cpp
    constraints/wordfinalcondition.h constraints/wordfinalcondition.cpp
    messages.h messages.cpp
    nodes/abstractnode.h nodes/abstractnode.cpp
    datatypes/allomorph.h datatypes/allomorph.cpp
    datatypes/form.h datatypes/form.cpp
    nodes/abstractpath.h nodes/abstractpath.cpp
    nodes/abstractsqlstemlist.h nodes/abstractsqlstemlist.cpp
    nodes/abstractstemlist.h nodes/abstractstemlist.cpp
    nodes/copynode.h nodes/copynode.cpp
    nodes/fork.h nodes/fork.cpp
    nodes/jump.h nodes/jump.cpp
    nodes/morphemenode.h nodes/morphemenode.cpp
    nodes/path.h nodes/path.cpp
    nodes/morphologicalmodel.h nodes/morphologicalmodel.cpp
    nodes/sequence.h nodes/sequence.cpp
    nodes/sqlserverstemlist.h nodes/sqlserverstemlist.cpp
    nodes/xmlstemlist.h nodes/xmlstemlist.cpp
    generation-constraints/morphemesequenceconstraint.h generation-constraints/morphemesequenceconstraint.cpp
    morphology.h morphology.cpp
    nodes/mutuallyexclusivemorphemes.h nodes/mutuallyexclusivemorphemes.cpp
    morphologychecker.h morphologychecker.cpp
    morphologyxmlreader.h morphologyxmlreader.cpp
    datatypes/parsing.h datatypes/parsing.cpp
    datatypes/parsingstep.h datatypes/parsingstep.cpp
    returns/lexicalsteminsertresult.h returns/lexicalsteminsertresult.cpp
    create-allomorphs/createallomorphs.h create-allomorphs/createallomorphs.cpp
    create-allomorphs/createallomorphscase.h create-allomorphs/createallomorphscase.cpp
    create-allomorphs/createallomorphsreplacement.h create-allomorphs/createallomorphsreplacement.cpp
    datatypes/tag.h datatypes/tag.cpp
    datatypes/writingsystem.h datatypes/writingsystem.cpp
    nodes/sqlitestemlist.h nodes/sqlitestemlist.cpp
    datatypes/nodeid.h datatypes/nodeid.cpp
    logging/parsinglog.h logging/parsinglog.cpp
    logging/xmlparsinglog.h logging/xmlparsinglog.cpp
    mortal-engine_global.cpp
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
