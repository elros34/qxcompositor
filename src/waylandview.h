#ifndef WAYLANDVIEW_H
#define WAYLANDVIEW_H

#include <QQuickView>
#include <QObject>

class QWaylandQuickOutput;
class QWaylandClient;
class QmlCompositor;
class QWaylandQuickSurface;
class WaylandView : public QQuickView
{
    Q_OBJECT
public:
    WaylandView(QmlCompositor *qmlCompositor);

    QWaylandQuickOutput *output() const;
    void setOutput(QWaylandQuickOutput *output);

    QWaylandClient *client() const;
    void setClient(QWaylandClient *client);

    QVector<QWaylandClient *> *clients() const;
    void setClients(QVector<QWaylandClient *> *clients);

    Q_INVOKABLE void setOrientation(const Qt::ScreenOrientation orientation);

    bool isXwaylandWindow() const;
    void setIsXwaylandWindow(bool isXwaylandWindow);
    void prepareView();
    void reportWindowAdded(QWaylandQuickSurface *quickSurface);

private:
    QmlCompositor *m_qmlCompositor;
    QWaylandQuickOutput *m_output;
    QWaylandClient *m_client;
    QVector<QWaylandClient*> *m_clients;
    bool m_isXwaylandWindow;

protected:
    bool event(QEvent *event);

signals:
    void closeEvent();
    void viewIdChanged();
    void windowAdded(QWaylandQuickSurface *quickSurface, bool isXwaylandWindow = false);

};

#endif // WAYLANDVIEW_H
