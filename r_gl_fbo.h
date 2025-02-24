#ifndef _R_GL_FBO_H_
#define _R_GL_FBO_H_

#include <SDL.h>
#include <glad/glad.h>

#include "r_gl_rendertexture.h"

class CglFBO
{

  public:
    CglFBO();
    CglFBO(int width, int height, GLint nearestOrLinear = GL_LINEAR);
    ~CglFBO();

    void Bind();
    void Unbind();

    int              m_Width;
    int              m_Height;
    GLuint           m_hFBO;
    CglRenderTexture m_ColorTexture;
    CglRenderTexture m_DepthTexture;
};

#endif
