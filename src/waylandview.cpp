#include "waylandview.h"
#include <QWaylandQuickOutput>
#include <QWaylandClient>
#include <QDebug>
#include <sailfishapp.h>
#include "qmlcompositor.h"

WaylandView::WaylandView(QmlCompositor *qmlCompositor) :
    QQuickView(),
    m_qmlCompositor(qmlCompositor),
    m_output(nullptr),
    m_client(nullptr),
    m_clients(new QVector<QWaylandClient*>),
    m_isXwaylandWindow(false)
{
    setVisible(false);
}

QWaylandQuickOutput *WaylandView::output() const
{
    return m_output;
}

void WaylandView::setOutput(QWaylandQuickOutput *output)
{
    m_output = output;
    if (m_output) {
        // quirk for Xwayland window resize. Screen height x width cause Xwayland crash
        if (m_qmlCompositor->m_xwaylandQuirks)
            m_output->setGeometry(QRect(0, 0, m_qmlCompositor->m_screenHeight, m_qmlCompositor->m_screenHeight));
        else
            m_output->setGeometry(QRect(0, 0, m_qmlCompositor->m_screenWidth, m_qmlCompositor->m_screenHeight));
    }
}

QWaylandClient *WaylandView::client() const
{
    return m_client;
}

void WaylandView::setClient(QWaylandClient *client)
{
    if (m_client && (client->processId() == m_client->processId())) {
        qDebug(logComp()) << "same pid, adding client to list only";
    } else {
        if (m_client && m_client != client)
            qDebug() << "override client!";

        m_client = client;
        connect(client, &QWaylandClient::destroyed,
                client, [this] () {
            qDebug(logComp()) << "QWaylandClient::destroyed" << m_client;
            m_output->deleteLater();
            this->deleteLater();
            qGuiApp->quit();
        });
    }
    m_clients->append(client);
}

QVector<QWaylandClient *> *WaylandView::clients() const
{
    return m_clients;
}

void WaylandView::setClients(QVector<QWaylandClient *> *clients)
{
    m_clients = clients;
}

bool WaylandView::event(QEvent *event)
{
    if (event->type() == QEvent::Close) {
        qDebug(logComp()) << "closeEvent";
        if (m_client)
            m_client->close();
        emit closeEvent();
    }
    return QQuickView::event(event);
}

void WaylandView::setOrientation(const Qt::ScreenOrientation orientation)
{
    if (!output())
        return;

    if (orientation & (Qt::PortraitOrientation | Qt::InvertedPortraitOrientation)) {
        if (orientation == Qt::PortraitOrientation)
            output()->setTransform(QWaylandOutput::TransformNormal);
        else
            output()->setTransform(QWaylandOutput::Transform180);
    } else {
        if (orientation == Qt::LandscapeOrientation)
            output()->setTransform(QWaylandOutput::Transform90);
        else
            output()->setTransform(QWaylandOutput::Transform270);
    }
}

bool WaylandView::isXwaylandWindow() const
{
    return m_isXwaylandWindow;
}

void WaylandView::setIsXwaylandWindow(bool isXwaylandWindow)
{
    m_isXwaylandWindow = isXwaylandWindow;
}

void WaylandView::prepareView()
{
    rootContext()->setContextProperty("compositor", m_qmlCompositor);
    rootContext()->setContextProperty("view", this);
    setSource(SailfishApp::pathTo("qml/qxcompositor.qml"));
    // Required here to prevent some Ambience related transparency issues
    setColor(Qt::black);
}

void WaylandView::reportWindowAdded(QWaylandQuickSurface *quickSurface)
{
    emit windowAdded(quickSurface, isXwaylandWindow());
}

