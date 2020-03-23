#ifndef CORERENDERER_H
#define CORERENDERER_H

#include <memory>
#include <QObject>
#include <QMutex>
#include <QQuickWindow>
#include <QWaitCondition>


class QOpenGLContext;
class QOpenGLFramebufferObject;
class QOffscreenSurface;
class QQuickRenderControl;
class QQmlComponent;
class QQuickItem;
class QmlAnimationDriver;

static const QEvent::Type INIT = QEvent::Type(QEvent::User + 1);
static const QEvent::Type RENDER = QEvent::Type(QEvent::User + 2);
static const QEvent::Type RESIZE = QEvent::Type(QEvent::User + 3);
static const QEvent::Type STOP = QEvent::Type(QEvent::User + 4);
static const QEvent::Type UPDATE = QEvent::Type(QEvent::User + 5);

class CoreRenderer : public QObject
{
    Q_OBJECT
public:
    explicit CoreRenderer(QObject *parent = nullptr);
    ~CoreRenderer() override;

    void requestInit();
    void requestRender();
    void requestResize();
    void requestStop();

    QWaitCondition *cond() { return &m_cond; }
    QMutex *mutex() { return &m_mutex; }

    void setContext(std::shared_ptr<QOpenGLContext> context){ m_context = context; }
    void setSurface(std::shared_ptr<QOffscreenSurface> surface) { m_offscreenSurface = surface; }
    void setQuickWindow(std::shared_ptr<QQuickWindow> window) { m_quickWindow = window; }
    void setRenderControl(std::shared_ptr<QQuickRenderControl> control) { m_renderControl = control; }
    void setSize(QSize s){ m_size = s; }
    void setDPR(qreal value){ m_dpr = value; }
    void setFPS(int value){ m_fps = value;}
    void setFormat( QImage::Format f){ m_format = f; }
    void aboutToQuit();

    QImage image;

private:
    bool event(QEvent *e) override;
    void init();
    void cleanup();
    void ensureFbo();
    void render(QMutexLocker *lock);


    QWaitCondition m_cond;
    QMutex m_mutex;

    std::shared_ptr<QOpenGLContext> m_context;
    std::shared_ptr<QOffscreenSurface> m_offscreenSurface;
    std::shared_ptr<QQuickRenderControl> m_renderControl;
    std::shared_ptr<QQuickWindow> m_quickWindow;
    std::unique_ptr<QOpenGLFramebufferObject> m_fbo;

    std::unique_ptr<QmlAnimationDriver> m_animationDriver;

    QImage::Format m_format;
    QSize m_size;
    qreal m_dpr;
    QMutex m_quitMutex;
    int m_fps;
    bool m_quit;
};

#endif // CORERENDERER_H
