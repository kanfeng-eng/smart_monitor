QT += core gui
CONFIG += c++11 console
CONFIG -= app_bundle

SOURCES += \
    cameraworker_test.cpp \
    ../src/view/cameraworker.cpp

HEADERS += ../src/view/cameraworker.h

INCLUDEPATH += ../src/view D:/MyLibs/opencv/include
LIBS += -LD:/MyLibs/opencv/x64/mingw/lib \
    -lopencv_core4100 -lopencv_imgproc4100 -lopencv_videoio4100
