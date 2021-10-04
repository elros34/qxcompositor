/****************************************************************************
**
** Copyright (C) 2019 elros34
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QGuiApplication>
#include <QQmlContext>
#include <QQuickView>
#include <QCommandLineParser>
#include <sailfishapp.h>
#include <unistd.h>

#include "qmlcompositor.h"
#include "xclipboard.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));

    QCommandLineParser parser;
    QCommandLineOption displayOption({"d", "display", "wayland-socket-name"},
                                     "Wayland display socket",
                                     "socket", "../../display/wayland-1");
    QCommandLineOption sshUserOption({"u", "user"},
                                     "ssh user used for clipboard sync", "user");
    QCommandLineOption sshPortOption({"p", "port"},
                                     "ssh port used for clipboard sync", "port");
    QCommandLineOption screenOrientationOption({"o", "orientation"},
                                         "screen orientation to use: portrait, landscape, auto. Default is landscape",
                                         "orientation", "landscape");
    QCommandLineOption xwaylandQuirksOption({"x", "xwaylandquirks"},
                                            "Quirks for Xwayland root window mode");
    QCommandLineOption forkOption({"f", "fork"},
                                  "Fork when wayland socket is ready to use");
    parser.addOption(displayOption);
    parser.addOption(sshUserOption);
    parser.addOption(sshPortOption);
    parser.addOption(screenOrientationOption);
    parser.addOption(xwaylandQuirksOption);
    parser.addOption(forkOption);
    parser.addHelpOption();
    parser.process(*app);

    qmlRegisterType<XClipboard>("QXCompositor", 1, 0, "XClipboard");

    QString screenOrientation = parser.value(screenOrientationOption);
    if (screenOrientation.isEmpty())
        screenOrientation = "landscape";

    QLoggingCategory::setFilterRules("qxcompositor.debug=false\n"
                                     "qxcompositor.orientation.debug=false\n"
                                     "qml=false");

    QmlCompositor compositor(app.data(),
                             qPrintable(parser.value(displayOption)),
                             screenOrientation,
                             parser.value(sshUserOption),
                             parser.value(sshPortOption),
                             parser.isSet(xwaylandQuirksOption));

    if (parser.isSet(forkOption)) {
        qDebug() << "forking";
        int pid = fork();
        if (pid < 0) {
            qDebug() << "Cannot fork: " << strerror(errno);
            exit(1);
        }
        if (pid == 0) {
            qDebug() << "in child";
        } else {
            qDebug() << "in parent, child pid: " << pid;
            exit(0);
        }
    }

    return app->exec();
}

