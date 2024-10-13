#include "r_font.h"

#include <stdio.h>
#include <string>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

extern std::string  g_GameDir;

// TODO: (Michael): Set bitmap size through ctor
CFont::CFont(std::string fontFile, int size) {
    m_Filename = fontFile;
    m_Cdata = (stbtt_bakedchar*)malloc( 96 * sizeof(stbtt_bakedchar) );
    m_Bitmap = (unsigned char*)malloc( 512 * 512 );
    unsigned char* ttfBuffer = (unsigned char*)malloc( 1<<20 );
    std::string fontFilePath = g_GameDir + fontFile;
    fread( (void*)ttfBuffer, 1, 1<<20, 
	  fopen(fontFilePath.c_str(), "rb") );
    stbtt_InitFont( &m_FontInfo, ttfBuffer, stbtt_GetFontOffsetForIndex(ttfBuffer, 0) );
    stbtt_BakeFontBitmap( ttfBuffer, 0, (float)size, m_Bitmap, 512, 512, 32, 96, m_Cdata );

    //memset( m_Bitmap, 128, 512 * 512 ); // Debug (Check bitmap of texture in Renderdoc)
    
    free(ttfBuffer);
}

CFont::~CFont() {
    free(m_Cdata);
    free(m_Bitmap);
}

