#include "r_gl_fbo.h"

#include <SDL.h>
#include <glad/glad.h>
#include <stdio.h>

#include "r_gl_rendertexture.h"

// Not usable in this state! Call any of the non-default Ctors to
// get the FBO into valid state.
CglFBO::CglFBO() {
    m_Width  = 0;
    m_Height = 0;
    m_hFBO   = 9999; // TODO: (Michael): Can the handle be 0? Check GL docs!
}

CglFBO::CglFBO(int width, int height) {
    m_Width  = width;
    m_Height = height;

    glGenFramebuffers(1, &m_hFBO);

    Bind();

    m_ColorTexture = CglRenderTexture(width, height, GL_RGBA8);
    m_DepthTexture = CglRenderTexture(width, height, GL_DEPTH_COMPONENT32F);

    m_ColorTexture.Bind();
    m_DepthTexture.Bind();

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_ColorTexture.m_gl_Handle, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthTexture.m_gl_Handle, 0);

    static const GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);

    // Check if framebuffer is complete
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if ( status != GL_FRAMEBUFFER_COMPLETE ) {
        printf("Framebuffer not complete: 0x%x\n", status);
    } else {
        printf("Framebuffer is complete\n");
    }

    Unbind();
}

CglFBO::~CglFBO() {
    glDeleteFramebuffers(1, &m_hFBO);
    m_hFBO = 9999;
}

void CglFBO::Bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_hFBO);
}

void CglFBO::Unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
