#include "r_gl_fbo.h"

#include <SDL.h>
#include <glad/glad.h>

CglFBO::CglFBO() {
    m_Width = 0;
    m_Height = 0;
    m_hFBO = 0; // TODO: (Michael): Can the handle be 0? Check GL docs!
}

CglFBO::CglFBO(int width, int height) {
    m_Width = width;
    m_Height = height;
    
    glCreateFramebuffers(1, &m_hFBO);
}

CglFBO::~CglFBO() {
    glDeleteFramebuffers(1, &m_hFBO);
}

void CglFBO::Bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_hFBO);
}

void CglFBO::Unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

