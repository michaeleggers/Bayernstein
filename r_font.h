#ifndef _R_FONT_H_
#define _R_FONT_H_

#include "stb_truetype.h"

#include <string>
#include <vector>

const int FIRST_CODE_POINT = 32; // Space (ASCII and Unicode overlap)
const int NUM_GLYPHS       = 96;
const int LAST_CODE_POINT  = FIRST_CODE_POINT + NUM_GLYPHS;
const int FONT_TEX_SIZE    = 1024;

class CFont {

  public:
    CFont(std::string fontFile, float size);
    ~CFont();

    float             m_Size;
    std::string       m_Filename;
    unsigned char*    m_Bitmap;
    stbtt_bakedchar*  m_Cdata; // glyphs
    stbtt_fontinfo    m_FontInfo;
    stbtt_packedchar* m_PackedCharData;
    int               m_Ascender;
    int               m_Descender;
};

#endif
