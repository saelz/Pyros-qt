QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 link_pkgconfig

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
    src/MediaViewer/Overlay/overlay.cpp \
    src/MediaViewer/Overlay/overlay_button.cpp \
    src/MediaViewer/Overlay/overlay_combo_box.cpp \
    src/MediaViewer/Overlay/overlay_progress_bar.cpp \
    src/MediaViewer/Overlay/overlay_spacer.cpp \
    src/MediaViewer/Overlay/overlay_text.cpp \
    src/MediaViewer/Overlay/overlay_volume_button.cpp \
    src/MediaViewer/Overlay/overlay_widget.cpp \
    src/MediaViewer/cbz_viewer.cpp \
    src/MediaViewer/image_viewer.cpp \
    src/MediaViewer/mediaviewer.cpp \
    src/MediaViewer/movie_viewer.cpp \
    src/MediaViewer/mpv_widget.cpp \
    src/MediaViewer/playback_controller.cpp \
    src/MediaViewer/slideshowconfdialog.cpp \
    src/MediaViewer/text_viewer.cpp \
    src/MediaViewer/unsupported_viewer.cpp \
    src/MediaViewer/video_viewer.cpp \
    src/MediaViewer/viewer.cpp \
    src/configtab.cpp \
    src/databasecreation.cpp \
    src/duplicate_selector.cpp \
    src/fileimport.cpp \
    src/filemodel.cpp \
    src/fileview.cpp \
    src/fileviewer.cpp \
    src/main.cpp \
    src/pyrosdb.cpp \
    src/pyrosqt.cpp \
    src/searchtab.cpp \
    src/tab.cpp \
    src/tagitem.cpp \
    src/taglineedit.cpp \
    src/tagtreemodel.cpp \
    src/tagview.cpp \
    src/zip_reader.cpp

HEADERS += \
    src/MediaViewer/Overlay/overlay.h \
    src/MediaViewer/Overlay/overlay_button.h \
    src/MediaViewer/Overlay/overlay_combo_box.h \
    src/MediaViewer/Overlay/overlay_progress_bar.h \
    src/MediaViewer/Overlay/overlay_spacer.h \
    src/MediaViewer/Overlay/overlay_text.h \
    src/MediaViewer/Overlay/overlay_volume_button.h \
    src/MediaViewer/Overlay/overlay_widget.h \
    src/MediaViewer/cbz_viewer.h \
    src/MediaViewer/image_viewer.h \
    src/MediaViewer/mediaviewer.h \
    src/MediaViewer/movie_viewer.h \
    src/MediaViewer/mpv_widget.h \
    src/MediaViewer/playback_controller.h \
    src/MediaViewer/slideshowconfdialog.h \
    src/MediaViewer/text_viewer.h \
    src/MediaViewer/unsupported_viewer.h \
    src/MediaViewer/video_viewer.h \
    src/MediaViewer/viewer.h \
    src/configtab.h \
    src/databasecreation.h \
    src/duplicate_selector.h \
    src/fileimport.h \
    src/filemodel.h \
    src/fileview.h \
    src/fileviewer.h \
    src/pyrosdb.h \
    src/pyrosqt.h \
    src/searchtab.h \
    src/tab.h \
    src/tagitem.h \
    src/taglineedit.h \
    src/tagtreemodel.h \
    src/tagview.h \
    src/zip_reader.h

LIBS += -lpyros
PKGCONFIG += mpv

FORMS += \
    src/databasecreation.ui \
    src/duplicate_selector.ui \
    src/fileimport.ui \
    src/fileviewer.ui \
    src/pyrosqt.ui \
    src/searchtab.ui \
    src/slideshowconfdialog.ui

packagesExist(zlib) {
   DEFINES += ENABLE_ZLIB
   LIBS += -lz
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources/resources.qrc
