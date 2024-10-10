#include "r_font.h"

#include <stdio.h>
#include <string>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

extern std::string  g_GameDir;

CFont::CFont(std::string fontFile, int size) {
    m_Bitmap = NULL;
    m_Filename = fontFile;
    unsigned char* ttfBuffer = (unsigned char*)malloc( 1<<25 );
    std::string fontFilePath = g_GameDir + fontFile;
    fread( (void*)ttfBuffer, 1, 1<<25, 
	  fopen(fontFilePath.c_str(), "rb") );
    stbtt_InitFont( &m_FontInfo, ttfBuffer, stbtt_GetFontOffsetForIndex(ttfBuffer, 0) );
    m_Bitmap = (unsigned char*)malloc( 512 * 512 );
    stbtt_BakeFontBitmap(ttfBuffer, 0, (float)size, m_Bitmap, 512, 512, size, 96, m_Cdata);
    
    free(ttfBuffer);
}

CFont::~CFont() {
    free(m_Bitmap);
}

