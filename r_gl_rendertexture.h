#ifndef _R_GL_RENDERTEXTURE_H_
#define _R_GL_RENDERTEXTURE_H_

#include <glad/glad.h>

class CglRenderTexture {

  public:
    CglRenderTexture();
    CglRenderTexture(int width, int height, GLenum format);
    ~CglRenderTexture();
    void Bind();
    void Unbind();

    GLuint m_gl_Handle;
    //unsigned char* m_Pixeldata;
    int m_Width;
    int m_Height;
};

#endif
