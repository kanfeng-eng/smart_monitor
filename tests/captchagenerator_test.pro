QT += widgets
CONFIG += c++11 console
CONFIG -= app_bundle

SOURCES += \
    captchagenerator_test.cpp \
    ../src/utils/captchagenerator.cpp

HEADERS += \
    ../src/utils/captchagenerator.h
