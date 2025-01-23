#include "CImageManager.h"

#include <assert.h>

#include <string>
#include <unordered_map>

#include "stb_image.h"

#include "platform.h"

CImageManager* CImageManager::Instance()
{
    static CImageManager theOneAndOnly;
    return &theOneAndOnly;
}

CImageManager::Image* CImageManager::Create(const std::string& filename)
{
    if ( m_Filename2image.contains(filename) )
    {
        return m_Filename2image.at(filename);
    }

    Image* image = new Image();

    int            x, y, n;
    unsigned char* pixeldata = stbi_load(filename.c_str(), &x, &y, &n, 0);

    // TODO: We probably should load a checkerboard texture or a
    // very obvous color (pink?) so we see it also in the game
    // when an image could not be loaded.
    if ( !pixeldata )
    {
        return image; // returns an image with width/height/channels = 0; pixeldata = nullptr; isValid = false;
    }

    image->isValid   = true;
    image->width     = x;
    image->height    = y;
    image->channels  = n;
    image->pixeldata = pixeldata;

    m_Filename2image.insert({ filename, image });

    return image;
}
