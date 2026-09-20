QT += core sql
CONFIG += c++11 console
CONFIG -= app_bundle

SOURCES += \
    loginctl_test.cpp \
    ../src/controller/loginctl.cpp \
    ../src/model/usermodel.cpp \
    ../src/model/dbconn.cpp

HEADERS += \
    ../src/controller/loginctl.h \
    ../src/model/usermodel.h \
    ../src/model/dbconn.h
