#include "r_font.h"

#include <stdio.h>
#include <string>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

static const int NUM_GLYPHS = 96;

extern std::string  g_GameDir;

// TODO: (Michael): Set bitmap size through ctor
// or compute it through provided size to fit all glyphs.
CFont::CFont(std::string fontFile, int size) {
    m_Filename = fontFile;
    m_Cdata = (stbtt_bakedchar*)malloc( NUM_GLYPHS * sizeof(stbtt_bakedchar) ); 
    m_Bitmap = (unsigned char*)malloc( 512 * 512 );
    
    // Read TTF file into buffer.
    unsigned char* ttfBuffer = (unsigned char*)malloc( 1<<20 );
    std::string fontFilePath = g_GameDir + fontFile;
    fread( (void*)ttfBuffer, 1, 1<<20, 
	  fopen(fontFilePath.c_str(), "rb") );
    
    stbtt_InitFont( &m_FontInfo, ttfBuffer, stbtt_GetFontOffsetForIndex(ttfBuffer, 0) );

    // Create a bitmap containing all the glyphs. A gpu texture may be
    // created from that bitmap.
    stbtt_BakeFontBitmap( ttfBuffer, 0, (float)size, m_Bitmap, 512, 512, 32, NUM_GLYPHS, m_Cdata );

    
    free(ttfBuffer);
}

CFont::~CFont() {
    free(m_Cdata);
    free(m_Bitmap);
}

