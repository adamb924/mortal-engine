QT -= gui
QT += sql xml

TEMPLATE = lib
DEFINES += MORTALENGINE_LIBRARY

TARGET = mortalengine

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    constraints/precedingnodeconstraint.cpp \
    datatypes/portmanteau.cpp \
    debug.cpp \
    generation-constraints/abstractgenerationconstraint.cpp \
    generation-constraints/stemidentityconstraint.cpp \
    datatypes/generation.cpp \
    datatypes/lexicalstem.cpp \
    constraints/abstractconstraint.cpp \
    constraints/abstractlongdistanceconstraint.cpp \
    constraints/abstractmatchcondition.cpp \
    constraints/abstractnestedconstraint.cpp \
    constraints/andcondition.cpp \
    constraints/boundcondition.cpp \
    constraints/followingnodeconstraint.cpp \
    constraints/followingphonologicalcondition.cpp \
    constraints/notcondition.cpp \
    constraints/orcondition.cpp \
    constraints/phonologicalcondition.cpp \
    constraints/pointertoconstraint.cpp \
    constraints/tagmatchcondition.cpp \
    constraints/wordfinalcondition.cpp \
    messages.cpp \
    nodes/abstractnode.cpp \
    datatypes/allomorph.cpp \
    datatypes/form.cpp \
    nodes/abstractpath.cpp \
    nodes/abstractstemlist.cpp \
    nodes/fork.cpp \
    nodes/jump.cpp \
    nodes/morphemenode.cpp \
    nodes/path.cpp \
    nodes/morphologicalmodel.cpp \
    nodes/sequence.cpp \
    nodes/sqlitestemlist.cpp \
    nodes/xmlstemlist.cpp \
    generation-constraints/morphemesequenceconstraint.cpp \
    morphology.cpp \
    nodes/mutuallyexclusivemorphemes.cpp \
    morphologychecker.cpp \
    morphologyxmlreader.cpp \
    datatypes/parsing.cpp \
    datatypes/parsingstep.cpp \
    returns/lexicalsteminsertresult.cpp \
    create-allomorphs/createallomorphs.cpp \
    create-allomorphs/createallomorphscase.cpp \
    create-allomorphs/createallomorphsreplacement.cpp \
    datatypes/tag.cpp \
    datatypes/writingsystem.cpp

HEADERS += \
    constraints/constraintmatcher.h \
    constraints/precedingnodeconstraint.h \
    datatypes/portmanteau.h \
    debug.h \
    generation-constraints/abstractgenerationconstraint.h \
    generation-constraints/stemidentityconstraint.h \
    datatypes/generation.h \
    datatypes/lexicalstem.h \
    constraints/abstractconstraint.h \
    constraints/abstractlongdistanceconstraint.h \
    constraints/abstractmatchcondition.h \
    constraints/abstractnestedconstraint.h \
    constraints/andcondition.h \
    constraints/boundcondition.h \
    constraints/followingnodeconstraint.h \
    constraints/followingphonologicalcondition.h \
    constraints/notcondition.h \
    constraints/orcondition.h \
    constraints/phonologicalcondition.h \
    constraints/pointertoconstraint.h \
    constraints/tagmatchcondition.h \
    constraints/wordfinalcondition.h \
    messages.h \
    nodes/abstractnode.h \
    datatypes/allomorph.h \
    datatypes/form.h \
    nodes/abstractpath.h \
    nodes/abstractstemlist.h \
    nodes/fork.h \
    nodes/jump.h \
    nodes/morphemenode.h \
    nodes/nodematcher.h \
    nodes/path.h \
    nodes/morphologicalmodel.h \
    nodes/sequence.h \
    nodes/sqlitestemlist.h \
    nodes/xmlstemlist.h \
    generation-constraints/morphemesequenceconstraint.h \
    morphology.h \
    nodes/mutuallyexclusivemorphemes.h \
    morphologychecker.h \
    morphologyxmlreader.h \
    datatypes/parsing.h \
    datatypes/parsingstep.h \
    mortal-engine_global.h \
    returns/lexicalsteminsertresult.h \
    create-allomorphs/createallomorphs.h \
    create-allomorphs/createallomorphscase.h \
    create-allomorphs/createallomorphsreplacement.h \
    datatypes/tag.h \
    datatypes/writingsystem.h

FORMS +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
