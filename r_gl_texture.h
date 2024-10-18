#ifndef _R_GL_TEXTURE_H_
#define _R_GL_TEXTURE_H_

#include <glad/glad.h>

#include <string>
#include <unordered_map>

#include "r_itexture.h"
#include "r_font.h"

class GLTexture : public ITexture {
public:
    GLTexture(std::string filename); 
    GLTexture(CFont* font);

    // TODO: (Michael) Nuke texture from GPU memory

    GLuint         m_gl_Handle;
    unsigned char* m_Pixeldata;
};



#endif
