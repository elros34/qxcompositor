/****************************************************************************
**
** Copyright (C) 2021 elros34
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

#include "qmlcompositor.h"
#include "waylandview.h"

#include <QWaylandSurfaceInterface>
#include <QWaylandKeymap>
#include <QQuickView>
#include <QScreen>
#include <sailfishapp.h>
#include <mlite5/MDConfGroup>
#include <QQmlPropertyMap>
#include <QGuiApplication>

Q_LOGGING_CATEGORY(logComp, "qxcompositor")
Q_LOGGING_CATEGORY(logCompOrient, "qxcompositor.orientation")

QmlCompositor::QmlCompositor(QObject *parent, const char *socketName,
                             const QString &screenOrientationOption,
                             const QString &sshUserOption,
                             const QString &sshPortOption,
                             const bool xwaylandQuirksOption) :
    QObject(parent),
    //QWaylandQuickCompositor(socketName, DefaultExtensions | SubSurfaceExtension),
    QWaylandQuickCompositor(socketName, SurfaceExtension | QtKeyExtension | TouchExtension | HardwareIntegrationExtension),
    // WindowManagerExtension breaks popup lifetime
    m_xwaylandQuirks(xwaylandQuirksOption),
    m_fullscreenSurface(nullptr),
    m_sshUserOption(sshUserOption),
    m_sshPortOption(sshPortOption),
    m_screenOrientationOption(screenOrientationOption),
    m_waylandView(nullptr)
{
    QSize size = qApp->primaryScreen()->size();
    m_screenWidth = size.width();
    m_screenHeight = size.height();

    qmlRegisterUncreatableType<QWaylandSurface>("QXCompositor", 1, 0, "WaylandSurface", "Can not create instance of WaylandSurface");

    MDConfGroup lipstickConf("/desktop/lipstick-jolla-home");
    QWaylandKeymap keymap(lipstickConf.value("layout", "us").toString(),
                          lipstickConf.value("variant", "").toString(),
                          lipstickConf.value("options", "").toString(),
                          lipstickConf.value("model", "jollasbj").toString(),
                          lipstickConf.value("rules", "evdev").toString());
    defaultInputDevice()->setKeymap(keymap);

    qCDebug(logCompOrient()) << "screenOrientationOption: " << screenOrientationOption;

    setClientFullScreenHint(true);
    addDefaultShell();
    // Required for clients
    m_waylandView = createQuickView();
}

QmlCompositor::~QmlCompositor()
{
    cleanupGraphicsResources();
}

WaylandView* QmlCompositor::createQuickView()
{
    qDebug(logComp()) << __func__;

    WaylandView *view = new WaylandView(this);
    QWaylandQuickOutput *output = static_cast<QWaylandQuickOutput*>(createOutput(view, "", ""));
    view->setOutput(output);
    connect(view, &WaylandView::afterRendering, this, &QmlCompositor::sendCallbacks);

    return view;
}

QString QmlCompositor::pidToCmd(const int pid)
{
    QFile file(QString("/proc/%1/cmdline").arg(pid));
    if (file.open(QIODevice::ReadOnly)) {
        QString cmd = file.readAll().replace('\0', ' ');
        if (cmd.endsWith(" "))
            cmd = cmd.remove(cmd.length()-1, 1);
        return cmd;
    } else {
        qDebug() << "Can not read /proc/" << pid << "cmdline";
    }
    return QString();
}

QWaylandSurfaceItem *QmlCompositor::getSurfaceItem(QWaylandQuickSurface *quickSurface)
{
    if (quickSurface->views().length())
        return static_cast<QWaylandSurfaceItem *>(quickSurface->views().first());
    return nullptr;
}

void QmlCompositor::setPos(QWaylandQuickSurface *quickSurface, float x, float y)
{
    if (quickSurface && quickSurface->views().length()) {
        QWaylandSurfaceView *view = quickSurface->views().first();
        view->setPos(QPointF(x, y));
    }
}

void QmlCompositor::destroyWindow(QWaylandQuickSurface *quickSurface)
{
    quickSurface->deleteLater();
}

void QmlCompositor::setFullscreenSurface(QWaylandQuickSurface *quickSurface)
{
    qDebug(logComp()) << __func__ << ": " << quickSurface;
    if (quickSurface == m_fullscreenSurface)
        return;
    m_fullscreenSurface = quickSurface;
    emit fullscreenSurfaceChanged();
}

void QmlCompositor::surfaceMapped(QWaylandQuickSurface *quickSurface)
{
    qDebug(logComp()) << "quickSurface: " << quickSurface;

    // It's important to enable resize here to get correct surface size on initial view
    QWaylandSurfaceItem *surfaceItem = getSurfaceItem(quickSurface);
    surfaceItem->setResizeSurfaceToItem(true);
    surfaceItem->setTouchEventsEnabled(true);
    surfaceItem->takeFocus();

    if (quickSurface->windowType() != QWaylandSurface::Toplevel) {
        // needs private api to set QWaylandOutput

        qDebug() << "SubWindow doesn't really work correctly!";
        emit subWindowAdded(quickSurface);
        return;
    }

    qDebug(logComp()) << "view: " << m_waylandView;
    QWaylandClient *client = quickSurface->client();
    m_waylandView->setClient(client);
    QWaylandQuickOutput *output = m_waylandView->output();
    qDebug(logComp()) << "output: " << output;

    QString cmd = pidToCmd(client->processId());
    if (m_xwaylandQuirks && cmd.startsWith("Xwayland"))
        m_waylandView->setIsXwaylandWindow(true);
    m_waylandView->prepareView();

    // Revert WaylandOutput size back to screen width x height when Xwayland surface is mapped
    if (m_xwaylandQuirks)
        output->setGeometry(QRect(0, 0, m_screenWidth, m_screenHeight));

    m_waylandView->setVisible(true);
    m_waylandView->show();

#ifdef QT_DEBUG
    showSurfaceProps(quickSurface);
#endif

    m_waylandView->reportWindowAdded(quickSurface);
}
void QmlCompositor::surfaceUnmapped(QWaylandQuickSurface *quickSurface)
{
    qDebug(logComp()) << __func__ << ": " << quickSurface;
    if (quickSurface == m_fullscreenSurface)
        m_fullscreenSurface = nullptr;
    emit windowDestroyed(quickSurface);
}

void QmlCompositor::surfaceDestroyed(QWaylandQuickSurface *quickSurface)
{
    qDebug(logComp()) << __func__ << ": " << quickSurface;
    if (quickSurface == m_fullscreenSurface)
        m_fullscreenSurface = nullptr;

    if (quickSurface->windowType() != QWaylandSurface::Toplevel) {
        qDebug(logComp()) << "not top level window : " << quickSurface->windowType();
        emit subWindowDestroyed(quickSurface);
    } else {
        emit windowDestroyed(quickSurface);
    }
}
void QmlCompositor::sendCallbacks()
{
    if (m_fullscreenSurface)
        sendFrameCallbacks(QList<QWaylandSurface *>() << m_fullscreenSurface);
    else
        sendFrameCallbacks(surfaces());
}

void QmlCompositor::surfaceCreated(QWaylandSurface *surface)
{
    QWaylandQuickSurface *quickSurface = static_cast<QWaylandQuickSurface *>(surface);
    qDebug(logComp()) << "quickSurface: " << quickSurface;

    QWaylandClient *client = surface->client();
    QWaylandQuickOutput *output = static_cast<QWaylandQuickOutput*>(quickSurface->mainOutput());
    qDebug(logComp()) << "output: " << output;

    QString cmd = pidToCmd(client->processId());
    qDebug(logComp()) << "pid: " << client->processId() << ", cmd: " << cmd;

    connect(quickSurface, &QWaylandQuickSurface::surfaceDestroyed,
            quickSurface, [this, quickSurface](){ this->surfaceDestroyed(quickSurface); });
    connect(quickSurface, &QWaylandQuickSurface::mapped,
            this, [this, quickSurface]() { this->surfaceMapped(quickSurface); });
    connect(quickSurface, &QWaylandQuickSurface::unmapped,
            this, [this, quickSurface]() { this->surfaceUnmapped(quickSurface); });
}

void QmlCompositor::surfaceAboutToBeDestroyed(QWaylandSurface *surface)
{
    QWaylandQuickSurface *quickSurface = static_cast<QWaylandQuickSurface *>(surface);
    qDebug(logComp()) << __func__ << ": " << quickSurface;
}

void QmlCompositor::showSurfaceProps(QWaylandQuickSurface *quickSurface)
{
    QQmlPropertyMap *propMap = static_cast<QQmlPropertyMap*>(quickSurface->windowPropertyMap());
    // these props are available when surface is mapped
    qDebug() << "\nquickSurface: " << quickSurface
             << "\n  parentSurface: " << quickSurface->parentSurface()
             << "\n  windowFlags: " << quickSurface->windowFlags()
             << "\n  windowType: " << quickSurface->windowType()
             << "\n  orientationUpdateMask: " << quickSurface->orientationUpdateMask()
             << "\n  windowPropertyMap: " << propMap
             << "\n  windowPropertyMap keys: " << propMap->keys()
             << "\n  className: " << quickSurface->className()
             << "\n  title: " << quickSurface->title()
             << "\n  transientParent: " << quickSurface->transientParent()
             << "\n  transientOffset: " << quickSurface->transientOffset()
             << "\n  windowProperties: " << quickSurface->windowProperties()
             << "\n  quickSurface type: " << quickSurface->type();
}
