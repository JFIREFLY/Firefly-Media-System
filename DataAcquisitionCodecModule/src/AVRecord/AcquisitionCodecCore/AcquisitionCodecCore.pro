#-------------------------------------------------
#
# Project created by QtCreator 2024-01-07T18:18:18
#
#-------------------------------------------------

QT       += core network multimediawidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = lib
TARGET = AcquisitionCodecCore
CONFIG(debug,debug|release)
{
#DESTDIR = ../x64/Debug
    CONFIG += debug
}
CONFIG(release,debug|release)
{
#DESTDIR = ../x64/Release
    CONFIG += release
}
DEPENDPATH += .

#gcc -D 条件编译宏
DEFINES += ACC_EXPORT
#依赖编译宏编译选项：
contains(DEFINES, ACC_EXPORT)
{
    #编译临时文件保存路径
    MOC_DIR += ./GeneratedFiles/$(ConfigurationName)   
    CONFIG(debug,debug|release)
    {
        OBJECTS_DIR += debug
    }
    CONFIG(release,debug|release)
    {
        OBJECTS_DIR += release
    }
    UI_DIR += ./GeneratedFiles
    RCC_DIR += ./GeneratedFiles
}
#与编译宏冲突的编译选项：
!contains(DEFINES, ACC_EXPORT)
{
    message(!ACC_EXPORT)
}

DEFINES += DEFINES_ACC_EXPORT
LIBS += -lws2_32 -lwsock32 -liphlpapi
LIBS += -lole32 -loleaut32 -lStrmiids #winapi
INCLUDEPATH += $$PWD/3rd/ffmpeg/FFmpeg-5.1/x64/include


#windwos
win32 {
    contains(QT_ARCH, i386) {
    #32 本地已编译ffmpeg动态库(暂时不兼容)
    }else {
    #64
    LIBS    += $$PWD/3rd/ffmpeg/FFmpeg-5.1/x64/bin/avcodec-59.dll \
                $$PWD/3rd/ffmpeg/FFmpeg-5.1/x64/bin/avdevice-59.dll \
                $$PWD/3rd/ffmpeg/FFmpeg-5.1/x64/bin/avfilter-8.dll \
                $$PWD/3rd/ffmpeg/FFmpeg-5.1/x64/bin/avformat-59.dll \
                $$PWD/3rd/ffmpeg/FFmpeg-5.1/x64/bin/avutil-57.dll \
                $$PWD/3rd/ffmpeg/FFmpeg-5.1/x64/bin/postproc-56.dll \
                $$PWD/3rd/ffmpeg/FFmpeg-5.1/x64/bin/swresample-4.dll \
                $$PWD/3rd/ffmpeg/FFmpeg-5.1/x64/bin/swscale-6.dll
    }
}

HEADERS += ./src/FFmpegHeader.h \
    ./src/FFmpegHelper.h \
    ./src/FileOutputer.h \
    ./src/mux.h \
    ./src/RecordConfig.h \
    ./src/Acquisition.h \
    ./src/singleton.h \
    ./src/VideoCapture.h \
    ./src/VideoEncoder.h \
    ./src/VideoFrameQueue.h \
    ./src/AudioCapture.h \
    ./src/AudioEncoder.h \
    ./src/AudioFrameQueue.h \
    ./common/timer.h \
    ./common/util/util.h \
    include/AcquisitionCodec.h

SOURCES += ./src/FFmpegHelper.cpp \
    ./src/FileOutputer.cpp \
    ./src/mux.cpp \
    ./src/Acquisition.cpp \
    ./src/VideoCapture.cpp \
    ./src/VideoEncoder.cpp \
    ./src/VideoFrameQueue.cpp \
    ./src/AudioCapture.cpp \
    ./src/AudioEncoder.cpp \
    ./src/AudioFrameQueue.cpp \
    ./common/util/util.cpp
