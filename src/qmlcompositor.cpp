#include "qmlcompositor.h"
#include <QWaylandSurfaceInterface>

QmlCompositor::QmlCompositor(QQuickView *window)
    : QObject(window)
    , QWaylandQuickCompositor(window, 0, DefaultExtensions | SubSurfaceExtension)
    , m_fullscreenSurface(0)
{
    setScreenOrientation(Qt::LandscapeOrientation);
    //setOutputGeometry(QRect(0,0, 960, 540));
    addDefaultShell();
}

QWaylandQuickSurface *QmlCompositor::fullscreenSurface() const
{
    return m_fullscreenSurface;
}

void QmlCompositor::setSize(int width, int height)
{
    setOutputGeometry(QRect(0,0, width, height));
}

QWaylandSurfaceItem *QmlCompositor::item(QWaylandSurface *surf)
{
    return static_cast<QWaylandSurfaceItem *>(surf->views().first());
}

void QmlCompositor::destroyWindow(QVariant window)
{
    qvariant_cast<QObject *>(window)->deleteLater();
}

void QmlCompositor::setFullscreenSurface(QWaylandQuickSurface *surface)
{
    if (surface == m_fullscreenSurface)
        return;
    m_fullscreenSurface = surface;
    emit fullscreenSurfaceChanged();
}

void QmlCompositor::surfaceMapped()
{
    QWaylandQuickSurface *surface = qobject_cast<QWaylandQuickSurface *>(sender());
    emit windowAdded(QVariant::fromValue(surface));
}
void QmlCompositor::surfaceUnmapped()
{
    QWaylandQuickSurface *surface = qobject_cast<QWaylandQuickSurface *>(sender());
    if (surface == m_fullscreenSurface)
        m_fullscreenSurface = 0;
    emit windowDestroyed(QVariant::fromValue(surface));
}

void QmlCompositor::surfaceDestroyed(QObject *object)
{
    QWaylandQuickSurface *surface = static_cast<QWaylandQuickSurface *>(object);
    if (surface == m_fullscreenSurface)
        m_fullscreenSurface = 0;
    emit windowDestroyed(QVariant::fromValue(surface));
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
    connect(surface, SIGNAL(destroyed(QObject *)), this, SLOT(surfaceDestroyed(QObject *)));
    connect(surface, SIGNAL(mapped()), this, SLOT(surfaceMapped()));
    connect(surface, SIGNAL(unmapped()), this,SLOT(surfaceUnmapped()));
}

