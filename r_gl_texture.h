#ifndef _R_GL_TEXTURE_H_
#define _R_GL_TEXTURE_H_

#include <glad/glad.h>

#include <string>

#include "r_font.h"
#include "r_itexture.h"

class GLTexture : public ITexture {
  public:

    virtual ~GLTexture() override;

    GLTexture(const std::string& filename);
    GLTexture(CFont* font);

    // TODO: (Michael) Nuke texture from GPU memory

    GLuint         m_gl_Handle;     
};

#endif
