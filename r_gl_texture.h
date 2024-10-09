#ifndef _R_GL_TEXTURE_H_
#define _R_GL_TEXTURE_H_

#include <glad/glad.h>

#include <string>
#include <unordered_map>

#include "r_itexture.h"

class GLTexture : public ITexture {
public:
    GLTexture(std::string filename); 

    // TODO: Kill texture

    GLuint      m_gl_Handle;
};



#endif
