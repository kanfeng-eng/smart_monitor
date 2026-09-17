QT += widgets sql
CONFIG += c++17


# =========================
# 项目源文件
# =========================

SOURCES += \
    main.cpp \
    src/model/dbconn.cpp \
    src/view/mainwindow.cpp

HEADERS += \
    src/model/dbconn.h \
    src/view/mainwindow.h


# =========================
# 项目头文件搜索路径
# =========================

INCLUDEPATH += $$PWD/src/model
INCLUDEPATH += $$PWD/src/view
INCLUDEPATH += $$PWD/src/include


# =========================
# OpenCV 配置
# =========================

OPENCV_ROOT = D:/MyLibs/opencv

# OpenCV 头文件
INCLUDEPATH += $$OPENCV_ROOT/include

# OpenCV 库目录
LIBS += -L$$OPENCV_ROOT/x64/mingw/lib

# OpenCV 核心库
LIBS += -lopencv_core4100
LIBS += -lopencv_imgproc4100
LIBS += -lopencv_videoio4100
LIBS += -lopencv_highgui4100
LIBS += -lopencv_imgcodecs4100

# OpenCV 扩展库
LIBS += -lopencv_features2d4100
LIBS += -lopencv_calib3d4100
LIBS += -lopencv_objdetect4100
LIBS += -lopencv_dnn4100


# =========================
# exe 输出目录
# =========================

DESTDIR = $$PWD/bin


# =========================
# 默认部署规则
# =========================

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target