#ifndef MOUSETRACKER_H
#define MOUSETRACKER_H

#include <QObject>

class QScreen;
class MouseTracker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject *target READ target WRITE setTarget)
    Q_PROPERTY(int mouseX READ mouseX NOTIFY mouseXChanged)
    Q_PROPERTY(int mouseY READ mouseY NOTIFY mouseYChanged)
    Q_PROPERTY(bool mouseActive READ mouseActive NOTIFY mouseActiveChanged)
    Q_PROPERTY(Qt::ScreenOrientation orientation READ orientation WRITE setOrientation)
public:
    explicit MouseTracker(QObject *parent = nullptr);

    QObject *target() const;
    void setTarget(QObject *target);

    int mouseX() const;
    void setMouseX(int mouseX);

    int mouseY() const;
    void setMouseY(int mouseY);

    bool mouseActive() const;
    void setMouseActive(bool mouseActive);

    Qt::ScreenOrientation orientation() const;
    void setOrientation(const Qt::ScreenOrientation &orientation);

    Q_INVOKABLE void start();

private:
    QObject *m_target;
    QScreen *m_screen;
    int m_mouseX;
    int m_mouseY;
    Qt::ScreenOrientation m_orientation;
    bool m_mouseActive;

private:
    bool eventFilter(QObject *watched, QEvent *e);

signals:
    void mouseXChanged();
    void mouseYChanged();
    void mouseActiveChanged();
};

#endif // MOUSETRACKER_H
