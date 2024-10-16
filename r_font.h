#ifndef _R_FONT_H_
#define _R_FONT_H_

#include "stb_truetype.h"

#include <string>
#include <vector>

class CFont {

public:
    CFont(std::string fontFile, int size);
    ~CFont();

    std::string     m_Filename;
    unsigned char*  m_Bitmap;
    stbtt_bakedchar* m_Cdata; // glyphs
    stbtt_fontinfo  m_FontInfo;
};

#endif