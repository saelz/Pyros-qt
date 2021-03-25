QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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
    MediaViewer/cbz_viewer.cpp \
    MediaViewer/image_viewer.cpp \
    MediaViewer/mediaviewer.cpp \
    MediaViewer/movie_viewer.cpp \
    MediaViewer/mpv_widget.cpp \
    MediaViewer/text_viewer.cpp \
    MediaViewer/unsupported_viewer.cpp \
    MediaViewer/video_viewer.cpp \
    MediaViewer/viewer.cpp \
    configtab.cpp \
    databasecreation.cpp \
    duplicate_selector.cpp \
    fileimport.cpp \
    filemodel.cpp \
    fileview.cpp \
    fileviewer.cpp \
    main.cpp \
    pyrosdb.cpp \
    pyrosqt.cpp \
    searchtab.cpp \
    tab.cpp \
    tagitem.cpp \
    taglineedit.cpp \
    tagtreemodel.cpp \
    tagview.cpp \
    zip_reader.cpp

HEADERS += \
    MediaViewer/cbz_viewer.h \
    MediaViewer/image_viewer.h \
    MediaViewer/mediaviewer.h \
    MediaViewer/movie_viewer.h \
    MediaViewer/mpv_widget.h \
    MediaViewer/text_viewer.h \
    MediaViewer/unsupported_viewer.h \
    MediaViewer/video_viewer.h \
    MediaViewer/viewer.h \
    configtab.h \
    databasecreation.h \
    duplicate_selector.h \
    fileimport.h \
    filemodel.h \
    fileview.h \
    fileviewer.h \
    pyrosdb.h \
    pyrosqt.h \
    searchtab.h \
    tab.h \
    tagitem.h \
    taglineedit.h \
    tagtreemodel.h \
    tagview.h \
    zip_reader.h

LIBS += -lpyros -lmpv

FORMS += \
    databasecreation.ui \
    duplicate_selector.ui \
    fileimport.ui \
    fileviewer.ui \
    pyrosqt.ui \
    searchtab.ui

packagesExist(zlib) {
   DEFINES += ENABLE_ZLIB
   LIBS += -lz
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
