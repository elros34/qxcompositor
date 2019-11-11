# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = qxcompositor

CONFIG += sailfishapp

DEFINES += QT_COMPOSITOR_QUICK

LIBS += -L ../../lib

QT += quick qml
QT += quick-private

QT += compositor

#  if you want to compile QtCompositor as part of the application
#  instead of linking to it, remove the QT += compositor and uncomment
#  the following line
#include (../../src/compositor/compositor.pri)

SOURCES += \
    src/main.cpp \
    src/qmlcompositor.cpp \
    src/xclipboard.cpp

OTHER_FILES += \
    qml/cover/CoverPage.qml \
    qml/pages/FirstPage.qml \

DISTFILES += \
    qml/compositor/XWaylandContainer.qml \
    rpm/qxcompositor.spec \
    rpm/qxcompositor.changes.in \
    qml/qxcompositor.qml

HEADERS += \
    src/qmlcompositor.h \
    src/xclipboard.h

