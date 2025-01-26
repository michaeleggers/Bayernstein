#include "CImageManager.h"

#include <assert.h>

#include <string>
#include <unordered_map>

#include "stb_image.h"
#include "globals.h"

#include "platform.h"

CImageManager* CImageManager::Instance()
{
    static CImageManager theOneAndOnly;
    return &theOneAndOnly;
}

CImageManager::CImageManager() {
    size_t numBytes           = DOD_FALLBACK_IMG_WIDTH * DOD_FALLBACK_IMG_HEIGHT * DOD_FALLBACK_IMG_CHANNELS;
    m_FallbackImage.pixeldata
        = (unsigned char*)malloc(numBytes);
    // NOTE: Byte order is: ABGR
    uint32_t uglyPixels      = 0xFFFF00FF;
    for ( int i = 0; i < numBytes; i += 4 )
    {
        *(uint32_t*)(m_FallbackImage.pixeldata + i) = uglyPixels;
    }
    m_FallbackImage.width = DOD_FALLBACK_IMG_WIDTH;
    m_FallbackImage.height = DOD_FALLBACK_IMG_HEIGHT;
    m_FallbackImage.channels = DOD_FALLBACK_IMG_CHANNELS;
    m_FallbackImage.isValid  = true;
}

const CImageManager::Image* CImageManager::Create(const std::string& filename)
{
    // Check if image has been loaded before. If so return cached image.

    if ( m_Filename2image.contains(filename) )
    {
        return m_Filename2image.at(filename);
    }

    // Image not yet loaded

    int            x, y, n;
    unsigned char* pixeldata = nullptr;
         
    for ( int i = 0; i < DOD_SUPPORTED_IMAGE_EXTENSION_COUNT; i++ )
    {
        if ( pixeldata = stbi_load( (filename + DOD_IMAGE_EXTENSION_NAMES[i]).c_str(), &x, &y, &n, 0) )
        {
            break;
        }
    }

    if ( !pixeldata )
    {
        printf("Could not find image: %s\n", filename.c_str());    
        return &m_FallbackImage; // returns an image with width/height/channels = 0; pixeldata = nullptr; isValid = false;
    }

    Image* image     = new Image();
    image->isValid   = true;
    image->width     = x;
    image->height    = y;
    image->channels  = n;
    image->pixeldata = pixeldata;

    m_Filename2image.insert({ filename, image });

    return image;
}
