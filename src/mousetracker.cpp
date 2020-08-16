#include "mousetracker.h"
#include <QMouseEvent>
#include <QGuiApplication>
#include <QDebug>
#include <QScreen>
#include <QWindow>

MouseTracker::MouseTracker(QObject *parent) :
    QObject(parent),
    m_target(nullptr),
    m_screen(qApp->primaryScreen()),
    m_mouseX(-1),
    m_mouseY(-1),
    m_mouseActive(false)
{
}

int MouseTracker::mouseX() const
{
    return m_mouseX;
}

void MouseTracker::setMouseX(int mouseX)
{
    m_mouseX = mouseX;
    emit mouseXChanged();
}

int MouseTracker::mouseY() const
{
    return m_mouseY;
}

void MouseTracker::setMouseY(int mouseY)
{
    m_mouseY = mouseY;
    emit mouseYChanged();
}

bool MouseTracker::mouseActive() const
{
    return m_mouseActive;
}

void MouseTracker::setMouseActive(bool mouseActive)
{
    if (m_mouseActive == mouseActive)
        return;
    m_mouseActive = mouseActive;
    emit mouseActiveChanged();
}

Qt::ScreenOrientation MouseTracker::orientation() const
{
    return m_orientation;
}

void MouseTracker::setOrientation(const Qt::ScreenOrientation &orientation)
{
    m_orientation = orientation;
}

QObject *MouseTracker::target() const
{
    return m_target;
}

void MouseTracker::setTarget(QObject *target)
{
    if (m_target == target)
        return;
    if (m_target)
        m_target->removeEventFilter(this);
    m_target = target;
    if (target) {
        m_target->installEventFilter(this);
        qDebug() << "install event filter: " << m_target;
    }
}

void MouseTracker::start()
{
    auto windows = qApp->topLevelWindows();
    if (windows.length()) {
        QWindow *window = windows.first();
        setTarget(window);
    }
}

bool MouseTracker::eventFilter(QObject *watched, QEvent *e)
{
    if (watched != m_target)
        return false;

    switch (e->type()) {
    case QMouseEvent::MouseMove: {
        if (!m_mouseActive)
            setMouseActive(true);

        QMouseEvent *event = static_cast<QMouseEvent*>(e);
        //qDebug() << "MouseMove: pos " << event->pos();

        switch (m_orientation) {
        case Qt::LandscapeOrientation:
            setMouseX(event->y());
            setMouseY(m_screen->geometry().width() - event->x());
            break;
        case Qt::InvertedLandscapeOrientation:
            setMouseX(m_screen->geometry().height() - event->y());
            setMouseY(event->x());
            break;
        default:
            setMouseX(event->x());
            setMouseY(event->y());
            break;
        }
        break;
    }
    case QMouseEvent::TouchBegin: {
        if (m_mouseActive)
            setMouseActive(false);
        break;
    }
    case QMouseEvent::HoverMove: {
        qDebug() << "report as a bug/feature 1234";
        break;
    }
    default:
        return false;
    }
    return false;
}
