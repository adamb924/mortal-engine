QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

TARGET = met

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
        abstracttest.cpp \
        harnessxmlreader.cpp \
        main.cpp \
        message.cpp \
        parsingtest.cpp \
        recognitiontest.cpp \
        stemreplacementtest.cpp \
        suggestiontest.cpp \
        testharness.cpp \
        testschema.cpp \
        transductiontest.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    abstracttest.h \
    harnessxmlreader.h \
    message.h \
    parsingtest.h \
    recognitiontest.h \
    stemreplacementtest.h \
    suggestiontest.h \
    testharness.h \
    testschema.h \
    transductiontest.h

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../mortal-engine/release/ -lmortalengine
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../mortal-engine/debug/ -lmortalengine
else:unix: LIBS += -L$$OUT_PWD/../mortal-engine/ -lmortalengine

INCLUDEPATH += $$PWD/../mortal-engine
DEPENDPATH += $$PWD/../mortal-engine
