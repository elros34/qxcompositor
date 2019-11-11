#include "xclipboard.h"
#include <QProcess>
#include <QClipboard>
#include <QGuiApplication>
#include <QTimer>
#include <QDebug>

XClipboard::XClipboard(QObject *parent) : QObject(parent),
    sshProc(NULL),
    clipboard(QGuiApplication::clipboard()),
    clipboardState(STATE_NONE),
    remoteReady(false),
    xwaylandReady(false)
{
    sshProc = new QProcess(this);
    sshProc->setProgram("ssh");

    timeoutTimer = new QTimer(this);
    timeoutTimer->setInterval(3000);
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, timeoutTimer, [this](){
        qDebug() << "terminate from timer";
        sshProc->terminate();
    });
    connect(sshProc, &QProcess::started, sshProc, [this](){
        timeoutTimer->start();
    });
    connect(sshProc, SIGNAL(finished(int)), this, SLOT(onRemoteClipboard(int)));
}

XClipboard::~XClipboard()
{
    sshProc->terminate();
}

QString XClipboard::hostClipboard()
{
    return clipboard->text();
}

void XClipboard::remoteClipboard()
{
    sshProc->kill();
    sshProc->setArguments(sshProcArgs + QStringList("--output"));
    sshProc->start();
}

void XClipboard::onRemoteClipboard(int exitCode)
{
    timeoutTimer->stop();
    QString error = sshProc->readAllStandardError();
    qDebug() << "ssh finished codes: " << exitCode;
    if (!error.isEmpty())
        qDebug() << "error: " << error;

    remoteReady = exitCode != 255;
    xwaylandReady = exitCode == 0;

    if (xwaylandReady) {
        if (clipboardState == STATE_READ_FROM_REMOTE) {
            QString text = sshProc->readAll();
            setHostClipboard(text);
        } else if (clipboardState == STATE_WRITE_TO_REMOTE) {
            qDebug() << "write to remote clipboard finished";
        }
    }

    sshProc->close();
}

void XClipboard::setHostClipboard(const QString &text)
{
    if (text.isEmpty())
        return;
    clipboard->setText(text);
}

void XClipboard::setRemoteClipboard(const QString &text)
{
    sshProc->kill();
    if (text.isEmpty())
        return;
    sshProc->setArguments(sshProcArgs + QStringList("--input"));
    sshProc->start();
    sshProc->write(text.toLocal8Bit());
    sshProc->closeWriteChannel();
}


void XClipboard::setXwaylandWindowReady(bool xwaylandWindowReady)
{
    mXwaylandWindowReady = xwaylandWindowReady;
    if (!xwaylandReady && mCompositorWindowActive) {
        setCompositorWindowActive(true);
    }
}

void XClipboard::setCompositorWindowActive(bool compositorWindowActive)
{
    mCompositorWindowActive = compositorWindowActive;
    if (!mXwaylandWindowReady)
        return;

    if (compositorWindowActive) {
        clipboardState = STATE_WRITE_TO_REMOTE;
        const QString text = hostClipboard();
        setRemoteClipboard(text);
    } else {
        if (remoteReady && xwaylandReady) {
            clipboardState = STATE_READ_FROM_REMOTE;
            remoteClipboard();
        }
    }
}

void XClipboard::setSshUser(const QString &sshUser)
{
    mSshUser = sshUser;
    sshProcArgs = QStringList({"-o", "StrictHostKeyChecking=no", "-p", mSshPort, mSshUser + "@localhost", "xsel", "--clipboard", "--display", ":0"});

}

void XClipboard::setSshPort(const QString &sshPort)
{
    mSshPort = sshPort;
    sshProcArgs = QStringList({"-o", "StrictHostKeyChecking=no", "-p", mSshPort, mSshUser + "@localhost", "xsel", "--clipboard", "--display", ":0"});

}
