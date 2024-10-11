#include "r_gl_texture.h"

#include "r_itexture.h"
#include "platform.h"
#include "r_font.h"
#include "stb_image.h"

#include <assert.h>
#include <string>

extern std::string g_GameDir;

// TODO: This is the texture manager at the moment...

GLTexture::GLTexture(std::string filename)
{
    m_Pixeldata = NULL; // TODO: (Michael): We could keep the original pixel data in
    // the future?

    std::string filePath = g_GameDir + "textures/" + filename;
    int x, y, n;
    unsigned char* data = stbi_load(filePath.c_str(), &x, &y, &n, 4);

    if (!data) {
        printf("WARNING: Failed to load texture: %s\n", filename.c_str());
        // return {}; // TODO: Load checkerboard texture instead.
    }

    GLuint glTextureHandle;
    glGenTextures(1, &glTextureHandle);
    glBindTexture(GL_TEXTURE_2D, glTextureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLuint)x, (GLuint)y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data); m_Filename = filename;
    m_gl_Handle = glTextureHandle;
    m_Width= x;
    m_Height = y;
    m_Channels = 4;

    m_hGPU = (uint64_t)glTextureHandle;    
}

GLTexture::GLTexture(CFont* font)
{
    m_Pixeldata = NULL; // TODO: (Michael): We could keep the original pixel data in
    // the future?
    
    assert( font->m_Bitmap != NULL && "GLTexture: Cannot create GLTexture from font, because the font-bitmap is NULL!" );

    int x = 512;
    int y = 512;
    GLuint glTextureHandle;
    glGenTextures(1, &glTextureHandle);
    glBindTexture(GL_TEXTURE_2D, glTextureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLuint)x, (GLuint)y, 0, GL_ALPHA, GL_UNSIGNED_BYTE, font->m_Bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_Filename = font->m_Filename;
    m_gl_Handle = glTextureHandle;
    m_Width= x;
    m_Height = y;
    m_Channels = 1;

    m_hGPU = (uint64_t)glTextureHandle;    
}

// For now make this a RGBA 32 bit/pixel Texture by default.
// We can add more params later to make it more configurable.
GLTexture::GLTexture(int width, int height)
{
    m_Pixeldata = (unsigned char*)malloc( 4 * width * height );
    GLuint glTextureHandle;
    glGenTextures(1, &glTextureHandle);
    glBindTexture(GL_TEXTURE_2D, glTextureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLuint)width, (GLuint)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_Pixeldata);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_Filename = "rendertexture";
    m_gl_Handle = glTextureHandle;
    m_Width= width;
    m_Height = height;
    m_Channels = 4;

    m_hGPU = (uint64_t)glTextureHandle;
}

