/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "window_singlethreaded.h"

#include "../../framelesswindowsmanager.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOffscreenSurface>
#include <QScreen>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQuickRenderControl>
#include <QCoreApplication>
#include <QQuickRenderTarget>
#include <QQuickGraphicsDevice>

#define DEPTH_BUFFER_SIZE 16
#define STENCIL_BUFFER_SIZE 16

class RenderControlImp : public QQuickRenderControl
{
public:
    RenderControlImp(QWindow *displayedWindow) : m_displayedWindow(displayedWindow) { }
    QWindow *renderWindow(QPoint *offset) override { return m_displayedWindow; }

private:
    QWindow *m_displayedWindow;
};

WindowSingleThreaded::WindowSingleThreaded()
{
    this->setSurfaceType(QSurface::OpenGLSurface);

    QSurfaceFormat surfaceFormat;
    // Qt Quick may need a depth and stencil buffer. Always make sure these are available.
    surfaceFormat.setDepthBufferSize(DEPTH_BUFFER_SIZE);
    surfaceFormat.setStencilBufferSize(STENCIL_BUFFER_SIZE);
    this->setFormat(surfaceFormat);

    m_glContext = new QOpenGLContext;
    m_glContext->setFormat(surfaceFormat);
    m_glContext->create();

    m_offscreenSurface = new QOffscreenSurface;
    // Pass m_context->format(), not format. Format does not specify and color buffer
    // sizes, while the context, that has just been created, reports a format that has
    // these values filled in. Pass this to the offscreen surface to make sure it will be
    // compatible with the context's configuration.
    m_offscreenSurface->setFormat(m_glContext->format());
    m_offscreenSurface->create();

    m_renderControl = new RenderControlImp(this);

    // Create a QQuickWindow that is associated with out render control. Note that this
    // window never gets created or shown, meaning that it will never get an underlying
    // native (platform) window.
    m_quickWindow = new QQuickWindow(m_renderControl);

    // Create a QML engine.
    m_qmlEngine = new QQmlEngine;
    if (!m_qmlEngine->incubationController())
        m_qmlEngine->setIncubationController(m_quickWindow->incubationController());

    m_qmlEngine->rootContext()->setContextProperty(QStringLiteral("window"), this);

    // When Quick says there is a need to render, we will not render immediately. Instead,
    // a timer with a small interval is used to get better performance.
    m_updateTimer.setSingleShot(true);
    m_updateTimer.setInterval(5);
    connect(&m_updateTimer, &QTimer::timeout, this, &WindowSingleThreaded::render);

    // Now hook up the signals. For simplicy we don't differentiate between
    // renderRequested (only render is needed, no sync) and sceneChanged (polish and sync
    // is needed too).
    connect(m_quickWindow, &QQuickWindow::sceneGraphInitialized, this, &WindowSingleThreaded::createTexture);
    connect(m_quickWindow, &QQuickWindow::sceneGraphInvalidated, this, &WindowSingleThreaded::destroyTexture);
    connect(m_renderControl, &QQuickRenderControl::renderRequested, this, &WindowSingleThreaded::requestUpdate);
    connect(m_renderControl, &QQuickRenderControl::sceneChanged, this, &WindowSingleThreaded::requestUpdate);

    // Just recreating the texture on resize is not sufficient, when moving between screens
    // with different devicePixelRatio the QWindow size may remain the same but the texture
    // dimension is to change regardless.
    connect(this, &QWindow::screenChanged, this, &WindowSingleThreaded::handleScreenChange);
}

WindowSingleThreaded::~WindowSingleThreaded()
{
    // Make sure the context is current while doing cleanup. Note that we use the
    // offscreen surface here because passing 'this' at this point is not safe: the
    // underlying platform window may already be destroyed. To avoid all the trouble, use
    // another surface that is valid for sure.
    m_glContext->makeCurrent(m_offscreenSurface);

    delete m_qmlComponent;
    delete m_qmlEngine;
    delete m_quickWindow;
    delete m_renderControl;

    if (m_textureId)
        m_glContext->functions()->glDeleteTextures(1, &m_textureId);

    delete m_program;
    delete m_vbo;
    delete m_vao;

    m_glContext->doneCurrent();

    delete m_offscreenSurface;
    delete m_glContext;
}

void WindowSingleThreaded::setHitTestVisible(QQuickItem *item, bool value)
{
    __flh_ns::FramelessWindowsManager::setHitTestVisible(this, item, value);
}

void WindowSingleThreaded::createTexture()
{
    // The scene graph has been initialized. It is now time to create an texture and associate
    // it with the QQuickWindow.
    m_dpr = devicePixelRatio();
    m_textureSize = size() * m_dpr;
    QOpenGLFunctions *f = m_glContext->functions();
    f->glGenTextures(1, &m_textureId);
    f->glBindTexture(GL_TEXTURE_2D, m_textureId);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_textureSize.width(), m_textureSize.height(), 0,
                    GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    m_quickWindow->setRenderTarget(QQuickRenderTarget::fromOpenGLTexture(m_textureId, m_textureSize));
}

void WindowSingleThreaded::destroyTexture()
{
    m_glContext->functions()->glDeleteTextures(1, &m_textureId);
    m_textureId = 0;
}


void WindowSingleThreaded::init()
{
    m_sharedGLContext = new QOpenGLContext;
    m_sharedGLContext->setShareContext(m_glContext);
    m_sharedGLContext->setFormat(this->requestedFormat());
    m_sharedGLContext->create();

    if (!m_sharedGLContext->makeCurrent(this))
        return;

    QOpenGLFunctions *f = m_sharedGLContext->functions();
    f->glClearColor(0.0f, 0.1f, 0.25f, 1.0f);
    f->glViewport(0, 0, this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());

    static const char *vertexShaderSource =
        "attribute highp vec4 vertexIn;\n"
        "attribute lowp vec2 vertTexCoord;\n"
        "varying lowp vec2 fragTexCoord;\n"
        "void main() {\n"
        "   fragTexCoord = vertTexCoord;\n"
        "   gl_Position = vertexIn;\n"
        "}\n";

    static const char *fragmentShaderSource =
        "uniform lowp sampler2D tex;\n"
        "varying vec2 fragTexCoord;\n"
        "void main(void)\n"
        "{\n"
        "   gl_FragColor = texture2D(tex, fragTexCoord);\n"
        "}\n";

    m_program = new QOpenGLShaderProgram;
    m_program->addCacheableShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program->addCacheableShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program->bindAttributeLocation("vertexIn", 0);
    m_program->bindAttributeLocation("vertTexCoord", 1);
    m_program->link();

    f->glUseProgram(m_program->programId());

    m_program->setUniformValue("tex", 0);

    int t1 = f->glGetUniformLocation(m_program->programId(), "tex");
    f->glUniform1i(t1, 0);

    m_vao = new QOpenGLVertexArrayObject;
    m_vao->create();
    QOpenGLVertexArrayObject::Binder vaoBinder(m_vao);

    m_vbo = new QOpenGLBuffer;
    m_vbo->create();
    m_vbo->bind();

    GLfloat v[] = {-1.0,-1.0, 1.0,-1.0, 1.0,1.0,
                   1.0,1.0, -1.0,1.0, -1.0,-1.0};

    GLfloat texCoords[] = {0.0,0.0, 1.0,0.0, 1.0,1.0,
                           1.0,1.0, 0.0,1.0, 0.0,0.0};

    const int vertexCount = 6;
    m_vbo->allocate(sizeof(GLfloat) * vertexCount * 4);
    m_vbo->write(0, v, sizeof(GLfloat) * vertexCount * 2);
    m_vbo->write(sizeof(GLfloat) * vertexCount * 2, texCoords, sizeof(GLfloat) * vertexCount * 2);
    m_vbo->release();

    if (m_vao->isCreated())
        setupVertexAttribs();
}

void WindowSingleThreaded::setupVertexAttribs()
{
    m_vbo->bind();
    m_program->enableAttributeArray(0);
    m_program->enableAttributeArray(1);
    m_sharedGLContext->functions()->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    m_sharedGLContext->functions()->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0,
                                                  (const void *)(6 * 2 * sizeof(GLfloat)));
    m_vbo->release();
}

void WindowSingleThreaded::renderGL()
{
    if (!this->m_sharedGLContext) {
        this->init();
    }

    if (!this->m_sharedGLContext->makeCurrent(this)) {
        return;
    }

    QOpenGLFunctions *f = m_sharedGLContext->functions();
    f->glViewport(0, 0, this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    uint texture = m_quickReady ? m_textureId : 0;
    if (texture) {
        f->glEnable(GL_CULL_FACE);
        f->glEnable(GL_DEPTH_TEST);

        m_program->bind();

        QOpenGLVertexArrayObject::Binder vaoBinder(m_vao);

        // If VAOs are not supported, set the vertex attributes every time.
        if (!m_vao->isCreated()) {
            qDebug() << "not created";
            setupVertexAttribs();
        }

        f->glActiveTexture(GL_TEXTURE0 + 0);
        f->glBindTexture(GL_TEXTURE_2D, texture);

        // Draw the verticies.
        f->glDrawArrays(GL_TRIANGLES, 0, 6);

        f->glBindTexture(GL_TEXTURE_2D, 0);

        m_sharedGLContext->swapBuffers(this);
    }
}

void WindowSingleThreaded::render()
{
    if (!m_glContext->makeCurrent(m_offscreenSurface))
        return;

    // Polish, synchronize and render the next frame (into our texture).  In this example
    // everything happens on the same thread and therefore all three steps are performed
    // in succession from here. In a threaded setup the render() call would happen on a
    // separate thread.
    m_renderControl->beginFrame();
    m_renderControl->polishItems();
    m_renderControl->sync();
    m_renderControl->render();
    m_renderControl->endFrame();

    QOpenGLFramebufferObject::bindDefault();
    m_glContext->functions()->glFlush();

    m_quickReady = true;
    this->renderGL();
}

void WindowSingleThreaded::requestUpdate()
{
    if (!m_updateTimer.isActive())
        m_updateTimer.start();
}

void WindowSingleThreaded::run()
{
    disconnect(m_qmlComponent, &QQmlComponent::statusChanged, this, &WindowSingleThreaded::run);

    if (m_qmlComponent->isError()) {
        const QList<QQmlError> errorList = m_qmlComponent->errors();
        for (const QQmlError &error : errorList)
            qWarning() << error.url() << error.line() << error;
        return;
    }

    QObject *rootObject = m_qmlComponent->create();
    if (m_qmlComponent->isError()) {
        const QList<QQmlError> errorList = m_qmlComponent->errors();
        for (const QQmlError &error : errorList)
            qWarning() << error.url() << error.line() << error;
        return;
    }

    m_rootItem = qobject_cast<QQuickItem *>(rootObject);
    if (!m_rootItem) {
        qWarning("run: Not a QQuickItem");
        delete rootObject;
        return;
    }

    // The root item is ready. Associate it with the window.
    m_rootItem->setParentItem(m_quickWindow->contentItem());

    // Required otherwise frameless helper will not be able to determine the
    // actual displayed window
    m_rootItem->setParent(m_quickWindow->contentItem());

    // Update item and rendering related geometries.
    updateSizes();

    // Ensure key events are received by the root Rectangle.
    m_rootItem->forceActiveFocus();

    // Initialize the render control and our OpenGL resources.
    m_glContext->makeCurrent(m_offscreenSurface);
    m_quickWindow->setGraphicsDevice(QQuickGraphicsDevice::fromOpenGLContext(m_glContext));
    m_renderControl->initialize();
}

void WindowSingleThreaded::updateSizes()
{
    // Behave like SizeRootObjectToView.
    m_rootItem->setWidth(width());
    m_rootItem->setHeight(height());

    m_quickWindow->setGeometry(0, 0, width(), height());
}

void WindowSingleThreaded::startQuick(const QString &filename)
{
    m_qmlComponent = new QQmlComponent(m_qmlEngine, QUrl(filename));
    if (m_qmlComponent->isLoading())
        connect(m_qmlComponent, &QQmlComponent::statusChanged, this, &WindowSingleThreaded::run);
    else
        run();
}

void WindowSingleThreaded::exposeEvent(QExposeEvent *)
{
    if (isExposed()) {
        if (!m_qmlComponent) {
            this->renderGL();
            startQuick(QStringLiteral("qrc:/rendercontrol/qml/main.qml"));
        }
    }
}

void WindowSingleThreaded::showEvent(QShowEvent *e)
{
#ifndef Q_OS_WASM
    if(!m_isFWMInitalised)
    {
        this->m_isFWMInitalised = true;
        __flh_ns::FramelessWindowsManager::addWindow(this);
        __flh_ns::FramelessWindowsManager::setTitleBarHeight(this, this->m_titleBarHeight);
    }
#endif
}

void WindowSingleThreaded::resizeTexture()
{
    if (m_rootItem && m_glContext->makeCurrent(m_offscreenSurface)) {
        this->destroyTexture();
        createTexture();
        m_glContext->doneCurrent();
        updateSizes();
        render();
    }
}

void WindowSingleThreaded::resizeEvent(QResizeEvent *)
{
    // If this is a resize after the scene is up and running, recreate the texture and the
    // Quick item and scene.
    if (m_textureId && m_textureSize != size() * devicePixelRatio()) {
        resizeTexture();
    }
}

void WindowSingleThreaded::handleScreenChange()
{
    if (m_dpr != devicePixelRatio())
        resizeTexture();
}

void WindowSingleThreaded::mouseMoveEvent(QMouseEvent *e)
{
    QMouseEvent mappedEvent(e->type(), e->position(), e->globalPosition(), e->button(), e->buttons(), e->modifiers());
    QCoreApplication::sendEvent(m_quickWindow, &mappedEvent);
}

void WindowSingleThreaded::mousePressEvent(QMouseEvent *e)
{
    // Use the constructor taking position and globalPosition. That puts position into the
    // event's position and scenePosition, and globalPosition into the event's globalPosition. This way
    // the scenePosition in e is ignored and is replaced by position. This is necessary
    // because QQuickWindow thinks of itself as a top-level window always.
    QMouseEvent mappedEvent(e->type(), e->position(), e->globalPosition(), e->button(), e->buttons(), e->modifiers());
    QCoreApplication::sendEvent(m_quickWindow, &mappedEvent);
}

void WindowSingleThreaded::mouseReleaseEvent(QMouseEvent *e)
{
    QMouseEvent mappedEvent(e->type(), e->position(), e->globalPosition(), e->button(), e->buttons(), e->modifiers());
    QCoreApplication::sendEvent(m_quickWindow, &mappedEvent);
}

void WindowSingleThreaded::keyPressEvent(QKeyEvent *e)
{
    QCoreApplication::sendEvent(m_quickWindow, e);
}

void WindowSingleThreaded::keyReleaseEvent(QKeyEvent *e)
{
    QCoreApplication::sendEvent(m_quickWindow, e);
}

int WindowSingleThreaded::getTitleBarHeight() const
{
    return this->m_titleBarHeight;
}
