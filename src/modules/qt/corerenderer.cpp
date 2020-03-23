#include "corerenderer.h"
#include "qmlanimationdriver.h"
#include <memory>

#include <QCoreApplication>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QQuickRenderControl>
#include <QOpenGLFramebufferObject>
#include <QThread>
#include <QOpenGLFunctions>

CoreRenderer::CoreRenderer(QObject *parent)
    : QObject(parent),
      m_quit(false)
{

}

CoreRenderer::~CoreRenderer() = default;

//void CoreRenderer::setContext(std::unique_ptr<QOpenGLContext> context)
//{
//    m_context = std::move(context);
//}

void CoreRenderer::requestInit()
{
    QCoreApplication::postEvent(this, new QEvent(INIT));
}

void CoreRenderer::requestRender()
{
    QCoreApplication::postEvent(this, new QEvent(RENDER));
}

void CoreRenderer::requestResize()
{
    QCoreApplication::postEvent(this, new QEvent(RESIZE));
}

void CoreRenderer::requestStop()
{
    QCoreApplication::postEvent(this, new QEvent(STOP));
}


bool CoreRenderer::event(QEvent *e)
{
    QMutexLocker lock(&m_mutex);

    switch (int(e->type())) {
    case INIT:
        init();
        return true;
    case RENDER:
        render(&lock);
        return true;
    case RESIZE:
        // TODO
        return true;
    case STOP:
        cleanup();
        return true;

    default:
        return QObject::event(e);
    }
}

void CoreRenderer::init()
{
    qDebug() << " CORE RENDER INIT!!!! CONTEXT MADE BE CURRENT !! \n \n ";
    m_context->makeCurrent(m_offscreenSurface.get());

    m_renderControl->initialize(m_context.get());

    Q_ASSERT(m_fps>0);
    m_animationDriver = std::make_unique<QmlAnimationDriver>(1000/m_fps);
    m_animationDriver->install();
    qDebug() << " AFTER INIT!!!!!! \n";
}

void CoreRenderer::cleanup()
{
    m_context->makeCurrent(m_offscreenSurface.get());

    m_renderControl->invalidate();

    m_context->doneCurrent();
    m_context->moveToThread(QCoreApplication::instance()->thread());

    m_cond.wakeOne();
}

void CoreRenderer::ensureFbo()
{
    Q_ASSERT(!m_size.isEmpty());
    Q_ASSERT(m_dpr != 0.0);

    if (m_fbo && m_fbo->size() != m_size * m_dpr) {
        // m_fbo.reset();
    }


    if (!m_fbo) {
        m_fbo = std::make_unique<QOpenGLFramebufferObject>(m_size * m_dpr, QOpenGLFramebufferObject::CombinedDepthStencil);
        m_quickWindow->setRenderTarget(m_fbo.get());
        Q_ASSERT(m_quickWindow->isSceneGraphInitialized());
    }
}

void CoreRenderer::render(QMutexLocker *lock)
{
   //  Q_ASSERT(QThread::currentThread() != m_window->thread());
    qDebug() << " IN CORERENDERER RENDER()!!!!! \n \n ";
    if (!m_context->makeCurrent(m_offscreenSurface.get())) {
        qWarning("Failed to make context current on render thread");
        return;
    }

    ensureFbo();

    // Synchronization and rendering happens here on the render thread.
    m_renderControl->sync();



    // Meanwhile on this thread continue with the actual rendering (into the FBO first).
    m_renderControl->render();
    m_context->functions()->glFlush();

    // Get something onto the screen using our custom OpenGL engine.
    QMutexLocker quitLock(&m_quitMutex);

    image = m_fbo->toImage();
    image.convertTo(m_format);
    //bool ok = image.save("/home/akhilkg/otuput2.jpg");
    // The gui thread can now continue.
    m_cond.wakeOne();
    lock->unlock();
    m_animationDriver->advance();

    //QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    qDebug() << " DONE CORERENDERER!!!! VALUEEE = "; //<< ok << "\n \n ";
}


void CoreRenderer::aboutToQuit()
{
    QMutexLocker lock(&m_quitMutex);
    m_quit = true;
}
