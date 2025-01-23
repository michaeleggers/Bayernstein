#include "r_gl_texture.h"

#include <assert.h>
#include <string>

#include "CImageManager.h"
#include "platform.h"
#include "r_font.h"
#include "r_itexture.h"

extern std::string g_GameDir;

// TODO: This is the texture manager at the moment...

GLTexture::~GLTexture() {
    // TODO: Nuke Texture from GPU.
}

GLTexture::GLTexture(const std::string& filename)
{
    m_IsValid                   = false;

    CImageManager* imageManager = CImageManager::Instance();

    std::string           filePath = g_GameDir + "textures/" + filename;
    CImageManager::Image* image    = imageManager->Create(filePath);

    // TODO: Load checkerboard texture if not valid.
    if ( !image->isValid )
    {        
        return;
    }

    GLenum internalFormat;
    GLenum sourceFormat;

    if ( image->channels == 4 )
    {
        internalFormat = GL_RGBA8;
        sourceFormat   = GL_RGBA;
    }
    else if ( image->channels == 3 )
    {
        internalFormat = GL_RGB8;
        sourceFormat   = GL_RGB;
    }
    else if ( image->channels == 1 )
    {
        internalFormat = GL_R8;
        sourceFormat = GL_RED;
    }
    else
    {
        printf("Error: Unsupported pixelformat in image: %s (channels=%d)\n", filename.c_str(), image->channels);
        assert(false && "Bad pixelformat");
    }
        

    GLuint glTextureHandle;
    glGenTextures(1, &glTextureHandle);
    glBindTexture(GL_TEXTURE_2D, glTextureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, (GLsizei)image->width, (GLsizei)image->height);
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    (GLsizei)image->width,
                    (GLsizei)image->height,
                    sourceFormat,
                    GL_UNSIGNED_BYTE,
                    image->pixeldata);

    //glTexImage2D(GL_TEXTURE_2D,
    //             0,
    //             GL_RGBA8,
    //             (GLuint)image->width,
    //             (GLuint)image->height,
    //             0,
    //             GL_RGBA,
    //             GL_UNSIGNED_BYTE,
    //             image->pixeldata);
    
    //glBindTexture(GL_TEXTURE_2D, 0);

    m_Filename  = filename;
    m_gl_Handle = (GLuint)glTextureHandle;
    m_Width     = image->width;
    m_Height    = image->height;
    m_Channels  = image->channels; // must be 4 at the moment.
    m_IsValid   = true;

    m_hGPU = (uint64_t)glTextureHandle;
}

GLTexture::GLTexture(CFont* font)
{
    m_IsValid = false;

    assert(font->m_Bitmap != NULL && "GLTexture: Cannot create GLTexture from font, because the font-bitmap is NULL!");

    int    x = FONT_TEX_SIZE;
    int    y = FONT_TEX_SIZE;
    GLuint glTextureHandle;
    glGenTextures(1, &glTextureHandle);
    glBindTexture(GL_TEXTURE_2D, glTextureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLuint)x, (GLuint)y, 0, GL_RED, GL_UNSIGNED_BYTE, font->m_Bitmap);    
    glBindTexture(GL_TEXTURE_2D, 0);

    m_Filename  = font->m_Filename;
    m_gl_Handle = glTextureHandle;
    m_Width     = x;
    m_Height    = y;
    m_Channels  = 1;
    m_IsValid   = true;

    m_hGPU = (uint64_t)glTextureHandle;
}
