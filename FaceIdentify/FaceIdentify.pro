#-------------------------------------------------
#
# Project created by QtCreator 2019-07-24T10:57:52
#
#-------------------------------------------------

QT       += core gui xml sql widgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FaceIdentify
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

unix {
    LIBS += -L$$PWD/../FaceIdentifySDK/linux/ -lFaceIdentifySDK -llogtrace \
-lHTTPServer -lFCDetectOptimum -lISFaceSDK -ljsoncpp -lULKCheck -lImgQuaEval \
-lFaceTracker -lPocoNet -lPocoFoundation -lhasp_linux_x86_64_108780 -lvideodecode\
-lopencv_highgui -lone_to_n -lCore -lvino -lfaceKeyPoint -lopencv_video\
-lopencv_calib3d -linference_engine -lopencv_features2d -lopencv_flann\
-ltbb -L/usr/local/lib -llog4cpp
}

INCLUDEPATH += $$PWD/../FaceIdentifySDK/include

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    Source/functions.cpp \
    Source/dataanalysis.cpp \
    querydialog.cpp \
    login.cpp \
    messagebox.cpp \
    messageboxfx.cpp \
    Source/singleton.cpp \
    subscreen.cpp

HEADERS += \
    mainwindow.h \
    Include/functions.h \
    Include/paramdef.h \
    Include/dataanalysis.h \
    querydialog.h \
    login.h \
    messagebox.h \
    messageboxfx.h \
    Include/singleton.h \
    subscreen.h

FORMS += \
    mainwindow.ui \
    login.ui\
    querydialog.ui \
    messagebox.ui \
    messageboxfx.ui \
    subscreen.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#QMAKE_LFLAGS+=-Wl,-rpath-link=\'/home/liuyong/FaceIdentify/FaceIdentifySDK/linux\'

RESOURCES += \
    images.qrc

INCLUDEPATH += $$PWD/../FaceIdentifySDK/linux
DEPENDPATH += $$PWD/../FaceIdentifySDK/linux
