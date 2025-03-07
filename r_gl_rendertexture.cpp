#include "r_gl_rendertexture.h"

#include <assert.h>

#include <glad/glad.h>

CglRenderTexture::CglRenderTexture()
{
    m_Width     = 0;
    m_Height    = 0;
    m_gl_Handle = 0;
}

CglRenderTexture::CglRenderTexture(int width, int height, GLenum format, GLint nearestOrLinear)
{
    m_Width  = width;
    m_Height = height;

    glGenTextures(1, &m_gl_Handle);
    glBindTexture(GL_TEXTURE_2D, m_gl_Handle);

    if ( format == GL_RGBA8 )
    {
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, nearestOrLinear);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, nearestOrLinear);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else if ( format == GL_DEPTH_COMPONENT32F )
    {
        glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);
    }
    else
    {
        assert(false && "CglRenderTexture: Unsupported format.");
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

CglRenderTexture::~CglRenderTexture()
{
    // TODO: (Michael): Nuke from GPU texture memory.
}

void CglRenderTexture::Bind()
{
    glBindTexture(GL_TEXTURE_2D, m_gl_Handle);
}

void CglRenderTexture::Unbind()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}
