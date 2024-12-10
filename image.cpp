#include "image.h"

#include <assert.h>

#include <string>

#include "stb_image.h"

// TODO: Virtual File system
extern std::string g_GameDir;

// NOTE: ATM user must check if the image is valid!
CImage::CImage(std::string filename) {

    std::string filePath = g_GameDir + filename; // TODO: File system functions.
    int         x, y, n;
    m_Pixeldata = stbi_load(filePath.c_str(), &x, &y, &n, 4);

    // TODO: We probably should load a checkerboard texture or a
    // very obvous color (pink?) so we see it also in the game
    // when an image could not be loaded.
    if ( !m_Pixeldata ) {
        printf("WARNING (CImage): Failed to load image: %s\n", filename.c_str());

        return;
    }

    m_Width    = x;
    m_Height   = y;
    m_Channels = n;
    m_Valid    = true;
    m_Filename = filename;
}

void CImage::FreePixeldata() {
    if ( !m_Pixeldata ) {
        return;
    }

    stbi_image_free(m_Pixeldata);
}

// Ugh...
unsigned char* CImage::Pixels() {
    return m_Pixeldata;
}

int CImage::Width() {
    return m_Width;
}

int CImage::Height() {
    return m_Height;
}

int CImage::Channels() {
    return m_Channels;
}

bool CImage::Valid() {
    return m_Valid;
}
