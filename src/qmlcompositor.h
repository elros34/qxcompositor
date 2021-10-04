#ifndef QMLCOMPOSITOR_H
#define QMLCOMPOSITOR_H

#include <QObject>
#include <QtCompositor>
#include <QHash>

Q_DECLARE_LOGGING_CATEGORY(logComp)
Q_DECLARE_LOGGING_CATEGORY(logCompOrient)

class WaylandView;
class QOrientationSensor;
class QmlCompositor : public QObject, public QWaylandQuickCompositor
{
    Q_OBJECT
    Q_PROPERTY(QWaylandQuickSurface* fullscreenSurface MEMBER m_fullscreenSurface WRITE setFullscreenSurface NOTIFY fullscreenSurfaceChanged)
    Q_PROPERTY(bool xwaylandQuirks MEMBER m_xwaylandQuirks CONSTANT)
    Q_PROPERTY(QString screenOrientationOption MEMBER m_screenOrientationOption CONSTANT)
    Q_PROPERTY(QString sshUserOption MEMBER m_sshUserOption CONSTANT)
    Q_PROPERTY(QString sshPortOption MEMBER m_sshPortOption CONSTANT)
public:
    QmlCompositor(QObject *parent = nullptr,
                  const char *socketName = 0,
                  const QString &screenOrientationOption = "landscape",
                  const QString &sshUserOption = "",
                  const QString &sshPortOption = "",
                  const bool xwaylandQuirksOption = false);

    ~QmlCompositor();

    Q_INVOKABLE QWaylandSurfaceItem *getSurfaceItem(QWaylandQuickSurface *quickSurface);
    Q_INVOKABLE void setPos(QWaylandQuickSurface *quickSurface, float x, float y);

public:
    int m_screenWidth;
    int m_screenHeight;
    bool m_xwaylandQuirks;

signals:
    void windowAdded(QWaylandQuickSurface *quickSurface, bool isXwaylandWindow = false);
    void subWindowAdded(QWaylandQuickSurface *quickSurface);
    void windowDestroyed(QWaylandQuickSurface *quickSurface);
    void subWindowDestroyed(QWaylandQuickSurface *quickSurface);
    void windowResized(QWaylandQuickSurface *quickSurface);
    void fullscreenSurfaceChanged();
    void screenOrientationChanged();

public slots:
    void destroyWindow(QWaylandQuickSurface *quickSurface);

    void setFullscreenSurface(QWaylandQuickSurface *quickSurface);
    void sendCallbacks();

private slots:
    void surfaceMapped(QWaylandQuickSurface *quickSurface);
    void surfaceUnmapped(QWaylandQuickSurface *quickSurface);
    void surfaceDestroyed(QWaylandQuickSurface *quickSurface);
    WaylandView *createQuickView();

private:
    QString pidToCmd(const int pid);
    void onOrientationReadingChanged();
    void onScreenOrientationChanged();
    void showSurfaceProps(QWaylandQuickSurface *quickSurface);

protected:
    void surfaceCreated(QWaylandSurface *surface);
    void surfaceAboutToBeDestroyed(QWaylandSurface *surface);

private:
    QWaylandQuickSurface *m_fullscreenSurface;
    QString m_sshUserOption;
    QString m_sshPortOption;
    QString m_screenOrientationOption;
    WaylandView *m_waylandView;
};

#endif // QMLCOMPOSITOR_H
