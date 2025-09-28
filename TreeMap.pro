QT       += core gui
QT       += xml
QT       += core gui sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    diskscan.cpp \
    diskwindow.cpp \
    enroll.cpp \
    logwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    showknowledge.cpp \
    sjf.cpp

HEADERS += \
    diskscan.h \
    diskwindow.h \
    enroll.h \
    job.h \
    logwidget.h \
    mainwindow.h \
    showknowledge.h \
    sjf.h

FORMS += \
    diskwindow.ui \
    enroll.ui \
    logwidget.ui \
    mainwindow.ui \
    showknowledge.ui \
    sjf.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    TreeMap.pro.user

RESOURCES += \
    images.qrc
