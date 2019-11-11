#ifndef XCLIPBOARD_H
#define XCLIPBOARD_H

#include <QObject>

class QProcess;
class QClipboard;
class QTimer;
class XClipboard : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool xwaylandWindowReady WRITE setXwaylandWindowReady)
    Q_PROPERTY(bool compositorWindowActive WRITE setCompositorWindowActive)
    Q_PROPERTY(QString sshUser WRITE setSshUser)
    Q_PROPERTY(QString sshPort WRITE setSshPort)
public:
    explicit XClipboard(QObject *parent = nullptr);
    ~XClipboard();

    enum clipboardStates {
        STATE_NONE,
        STATE_WRITE_TO_REMOTE,
        STATE_READ_FROM_REMOTE
    };

    void setXwaylandWindowReady(bool xwaylandWindowReady);
    void setCompositorWindowActive(bool compositorWindowActive);
    void setSshUser(const QString &sshUser);
    void setSshPort(const QString &sshPort);

private:
    QProcess *sshProc;
    QClipboard *clipboard;
    QStringList sshProcArgs;
    int clipboardState;
    bool remoteReady;
    bool xwaylandReady;
    bool mXwaylandWindowReady;
    bool mCompositorWindowActive;
    QString mSshUser;
    QString mSshPort;
    QTimer *timeoutTimer;

private:
    QString hostClipboard();
    void remoteClipboard();
    void setHostClipboard(const QString &text);
    void setRemoteClipboard(const QString &text);

signals:

private slots:
    void onRemoteClipboard(int exitCode);

public slots:
};

#endif // XCLIPBOARD_H
