#-------------------------------------------------
#
# Project created by QtCreator 2020-09-17T06:30:22
#tableWidget
#-------------------------------------------------

QT       += core gui
QT += sql 
QT += network
QT += multimedia multimediawidgets
QT += serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# 群华记录仪只取devno
#DEFINES += CONFIG_NAME=qunhua

# 群华车辆段
#DEFINES += CAR_DEPOT

# 使用人脸/指纹
#DEFINES += USE_FACE_FINGER

# 使用锁/灯
#DEFINES += LOCK_LAMP

# 海信插入记录仪需要验证设备编码和人员编码是否关联,不兼容ZHY
#DEFINES += RELEVANCE


# 大容量盘
# DEFINES += USE_EXTERN_DISK

# 使用 /data 数据盘
DEFINES += USE_DATA_DISK


TARGET = Acquisition_Station
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# 添加控制台输出
#CONFIG += console



CONFIG += c++11

#include(QtUsb/src/usb/files.pri)
SOURCES += \
        main.cpp \
        mainwindow.cpp \
    dasbuddy.cpp \
    loginform.cpp \
    searchbytype.cpp \
    mainform.cpp \
    getusbinfo.cpp \
    zfycontrol.cpp \
    mysqllite.cpp \
    config.cpp \
    hotplugthread.cpp \
    mymqproducer.cpp \
    mymqconsumer.cpp \
    writesettingdialog.cpp \
    copytask.cpp \
    networkutility.cpp \
    systemmanagewindow.cpp \
    recordersettingform.cpp \
    workstationsetform.cpp \
    deptmanagesetform.cpp \
    usermanagesetform.cpp \
    loggerqueryform.cpp \
    wssetform.cpp \
    wsrunmodesetform.cpp \
    wsinterfacesetform.cpp \
    wspairdeviceform.cpp \
    wspairdevicethread.cpp \
    deviceportform.cpp \
    userinfodialog.cpp \
    ziangfingerutility.cpp \
    locwdget.cpp \
    lockerutils.cpp \
    autoquerythread.cpp \
    disk.cpp \
    loginformfactory.cpp \
    faceformutility.cpp \
    faceenrollthread.cpp \
    facerecognitionthread.cpp \
    fingerprintform.cpp \
    audioplayer.cpp \
    devicepolicydialog.cpp \
    lampthread.cpp \
    videocompressor.cpp \
    mmapcopy.cpp

HEADERS += \
        mainwindow.h \
    dasbuddy.h \
    loginform.h \
    searchbytype.h \
    mainform.h \
    getusbinfo.h \
    zfycontrol.h \
    mysqllite.h \
    config.h \
    hotplugthread.h \
    mymqproducer.h \
    mymqconsumer.h \
    writesettingdialog.h \
    copytask.h \
    networkutility.h \
    systemmanagewindow.h \
    recordersettingform.h \
    workstationsetform.h \
    deptmanagesetform.h \
    usermanagesetform.h \
    loggerqueryform.h \
    wssetform.h \
    wsrunmodesetform.h \
    wsinterfacesetform.h \
    wspairdeviceform.h \
    wspairdevicethread.h \
    deviceportform.h \
    userinfodialog.h \
    ziangfingerutility.h \
    locwdget.h \
    lockerutils.h \
    autoquerythread.h \
    disk.h \
    loginformfactory.h \
    faceformutility.h \
    faceenrollthread.h \
    facerecognitionthread.h \
    fingerprintform.h \
    audioplayer.h \
    devicepolicydialog.h \
    lampthread.h \
    videocompressor.h \
    hsglobal.h \
    mmapcopy.h

FORMS += \
        mainwindow.ui \
    loginform.ui \
    searchbytype.ui \
    mainform.ui \
    writesettingdialog.ui \
    dasbuddy.ui \
    systemmanagewindow.ui \
    recordersetting.ui \
    workstationsetform.ui \
    deptmanagesetform.ui \
    usermanagesetform.ui \
    loggerqueryform.ui \
    wssetform.ui \
    wsrunmodesetform.ui \
    wsinterfacesetform.ui \
    wspairdeviceform.ui \
    deviceportform.ui \
    userinfodialog.ui \
    locwdget.ui \
    faceformutility.ui \
    fingerprintform.ui \
    devicepolicydialog.ui

include(3rdparty/qtxlsx/src/xlsx/qtxlsx.pri)

#language
TRANSLATIONS = lang_cn.ts\
               lang_en.ts\
               lang_ru.ts
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resourse.qrc

DISTFILES +=


message("Compiler: $$QMAKE_CXX")


currentCompiler = $$QMAKE_CXX




unix {
    CONFIG += link_pkgconfig




    contains(currentCompiler,/opt/aarch64-linux-gnu-8.3.0/bin/aarch64-linux-gnu-g++) {
        INCLUDEPATH += /usr/local/arm64HSlib/libusb_aarch64-1.0/include/
        LIBS += -L/usr/local/arm64HSlib/libusb_aarch64-1.0/lib -lusb-1.0
        LIBS += -L/usr/local/arm64HSlib/arm64ActiveMQ-CPP/lib -lactivemq-cpp
        DEPENDPATH += /usr/local/arm64HSlib/arm64ActiveMQ-CPP/lib
        INCLUDEPATH += /usr/local/arm64HSlib/arm64ActiveMQ-CPP/include/activemq-cpp-3.9.5
        LIBS += -L/usr/local/arm64HSlib/arm64ActiveMQ-CPP/lib -lapr-1


        INCLUDEPATH += /usr/local/arm64HSlib/libzhiangfinger/include
        LIBS += -L/usr/local/arm64HSlib/libzhiangfinger/lib -lzaz_aarch64

        
        INCLUDEPATH += /usr/local/arm64HSlib/libzhiangmatch/sdk/include

        LIBS += -L/usr/local/arm64HSlib/libzhiangmatch/sdk/outlib -lzamatch_aarch64

#        INCLUDEPATH += /usr/local/libzhiangfinger_aarch64/include
#        LIBS += -L/usr/local/libzhiangfinger_aarch64/lib \
#                -ltestfinger \
#                -lzamatch_aarch64
#        LIBS += -Wl,-rpath,/usr/local/libzhiangfinger_aarch64/lib



        INCLUDEPATH += /usr/local/arm64HSlib/aarch64opencv3.4/include

        LIBS += -L/usr/local/arm64HSlib/aarch64opencv3.4/lib \
                 -lopencv_core \
                 -lopencv_imgproc \
                 -lopencv_highgui \
                 -lopencv_videoio \
                 -lopencv_objdetect \
                 -lopencv_calib3d \
                 -lopencv_imgcodecs \
                 -lopencv_video \
                 -lopencv_face \
                 -lopencv_flann

         

        INCLUDEPATH += /usr/local/arm64HSlib/aarch64Facesdk
        LIBS += -L/usr/local/arm64HSlib/aarch64Facesdk -lIdFaceSdk \
                 -lTHFaceLive \
                 -lTHFeature \
                 -lTHFaceImage \
                 -leface_quality \
                 -leface_landmark \
                 -fopenmp


    } else:contains(currentCompiler, g++) {
        !packagesExist(libusb-1.0):error("Could not find libusb-1.0 using pkg-config")
        PKGCONFIG += libusb-1.0
        LIBS += -L/data/home/hskj/Desktop/x86libHS/ActiveMQ-CPP/lib -lactivemq-cpp
        DEPENDPATH += /data/home/hskj/Desktop/x86libHS/ActiveMQ-CPP/lib
        INCLUDEPATH += /data/home/hskj/Desktop/x86libHS/ActiveMQ-CPP/include/activemq-cpp-3.9.5

        INCLUDEPATH += /data/home/hskj/Desktop/x86libHS/libzhiangfinger/include
        LIBS += -L/data/home/hskj/Desktop/x86libHS/libzhiangfinger/lib \
                -ltestfinger \
                -lzamatch_x86_64
 
        INCLUDEPATH += /data/home/hskj/Desktop/x86libHS/libzhiangmatch/sdk/include

        LIBS += -L/data/home/hskj/Desktop/x86libHS/libzhiangmatch/sdk/outlib -lzamatch_x86_64



#        CONFIG += link_pkgconfig
#        PKGCONFIG += libavformat libavcodec libavutil libswscale
 
        INCLUDEPATH += /data/home/hskj/Desktop/x86libHS/OpenCV/install/include
 
        LIBS += -L/data/home/hskj/Desktop/x86libHS/OpenCV/install/lib \
                 -lopencv_core \
                 -lopencv_imgproc \
                 -lopencv_highgui \
                 -lopencv_objdetect \
                 -lopencv_face \
                 -lopencv_videoio \
                 -lopencv_imgcodecs
 
        LIBS += -L/opt/Qt5.9.0/5.9/gcc_64/lib \
                 -lQt5Widgets \
                 -lQt5Gui \
                 -lQt5Core \
                 -lGL \
                 -lpthread

       

        INCLUDEPATH += /data/home/hskj/Desktop/x86libHS/face/sdk
        LIBS += -L/data/home/hskj/Desktop/x86libHS/face/sdk \
                 -lIdFaceSdk \
                 -lTHFaceProperty \
                 -lTHFaceLive \
                 -lTHFaceMask \
                 -lTHFaceQuality \
                 -lTHLandMark \
                 -lTHFeature \
                 -lTHFaceImage \
                 -lTHFacialPos

       
#        INCLUDEPATH += /media/admin-hskj/829c89a5-a761-472f-81fc-c312a6b74c30/tools/install/include

 
#        LIBS += -L/media/admin-hskj/829c89a5-a761-472f-81fc-c312a6b74c30/tools/install/lib \
#                 -lopencv_core \
#                 -lopencv_imgproc \
#                 -lopencv_highgui \ 
#                 -lopencv_objdetect \
#                 -lopencv_face \
#                 -lopencv_videoio \
#                 -lopencv_imgcodecs

#        LIBS += -Wl,-rpath,/media/admin-hskj/829c89a5-a761-472f-81fc-c312a6b74c30/tools/install/lib

 
#        LIBS += -L/home/admin-hskj/Qt5.9.0/5.9/gcc_64/lib \
#                 -lQt5Widgets \
#                 -lQt5Gui \
#                 -lQt5Core \
#                 -lGL \
#                 -lpthread

 
#        INCLUDEPATH += /media/admin-hskj/829c89a5-a761-472f-81fc-c312a6b74c30/tools/sdk
#        LIBS += -L/media/admin-hskj/829c89a5-a761-472f-81fc-c312a6b74c30/tools/sdk \
#                 -lIdFaceSdk \
#                 -lTHFaceProperty \
#                 -lTHFaceLive \
#                 -lTHFaceMask \
#                 -lTHFaceQuality \
#                 -lTHLandMark \
#                 -lTHFeature \
#                 -lTHFaceImage \
#                 -lTHFacialPos

 
#        INCLUDEPATH += /media/admin-hskj/829c89a5-a761-472f-81fc-c312a6b74c30/tools/sdk_fingermatch
#        LIBS += -L/media/admin-hskj/829c89a5-a761-472f-81fc-c312a6b74c30/tools/sdk_fingermatch/outlib \
#                 -lzamatch_x86_64
#        LIBS += -Wl,-rpath,/media/admin-hskj/829c89a5-a761-472f-81fc-c312a6b74c30/tools/sdk_fingermatch/outlib

    } else {

        error("Unsupported compiler: $$currentCompiler")
    }







}
