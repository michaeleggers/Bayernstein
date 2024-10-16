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
    unsigned char* ttfFileData = (unsigned char*)malloc( 1<<20 );
    std::string fontFilePath = g_GameDir + fontFile;
    fread( (void*)ttfFileData, 1, 1<<20, 
	  fopen(fontFilePath.c_str(), "rb") );
    
    stbtt_InitFont( &m_FontInfo, ttfFileData, stbtt_GetFontOffsetForIndex(ttfFileData, 0) );

    // Create a bitmap containing all the glyphs. A gpu texture may be
    // created from that bitmap. m_Cdata can then be queried to get rectangles for the glyphs.
    stbtt_BakeFontBitmap( ttfFileData, 0, (float)size, m_Bitmap, 512, 512, 32, NUM_GLYPHS, m_Cdata );


    // Better API (apparently)

    stbtt_pack_context pc;
    
    stbtt_PackBegin();

    stbtt_pack_ranges* ranges = (stbtt_pack_ranges*)malloc( NUM_GLYPHS * sizeof(stbtt_pack_ranges) );
    stbtt_PackFontRanges( &pc, ttfFileData, 32, NUM_GLYPHS, ranges, 1 );

    stbtt_PackEnd();
    
    // Cleanup
    free(ttfFileData);
}

CFont::~CFont() {
    free(m_Cdata);
    free(m_Bitmap);
}

