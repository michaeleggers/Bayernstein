#include "r_gl_texture.h"

#include <assert.h>
#include <string>

#include "CImageManager.h"
#include "platform.h"
#include "r_font.h"
#include "r_itexture.h"

extern std::string g_GameDir;

// TODO: This is the texture manager at the moment...

GLTexture::GLTexture(std::string filename)
{
    CImageManager* imageManager = CImageManager::Instance();

    std::string           filePath = g_GameDir + "textures/" + filename;
    CImageManager::Image* image    = imageManager->Create(filePath);

    // TODO: Load checkerboard texture if not valid.
    assert(image->isValid && "GLTexture must have a valid image to be built!");

    GLuint glTextureHandle;
    glGenTextures(1, &glTextureHandle);
    glBindTexture(GL_TEXTURE_2D, glTextureHandle);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 (GLuint)image->width,
                 (GLuint)image->height,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 image->pixeldata);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_Filename  = filename;
    m_gl_Handle = glTextureHandle;
    m_Width     = image->width;
    m_Height    = image->height;
    m_Channels  = image->channels; // must be 4 at the moment.

    m_hGPU = (uint64_t)glTextureHandle;
}

GLTexture::GLTexture(CFont* font)
{
    m_Pixeldata = NULL; // TODO: (Michael): We could keep the original pixel data in
    // the future?

    assert(font->m_Bitmap != NULL && "GLTexture: Cannot create GLTexture from font, because the font-bitmap is NULL!");

    int    x = FONT_TEX_SIZE;
    int    y = FONT_TEX_SIZE;
    GLuint glTextureHandle;
    glGenTextures(1, &glTextureHandle);
    glBindTexture(GL_TEXTURE_2D, glTextureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLuint)x, (GLuint)y, 0, GL_RED, GL_UNSIGNED_BYTE, font->m_Bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_Filename  = font->m_Filename;
    m_gl_Handle = glTextureHandle;
    m_Width     = x;
    m_Height    = y;
    m_Channels  = 1;

    m_hGPU = (uint64_t)glTextureHandle;
}
