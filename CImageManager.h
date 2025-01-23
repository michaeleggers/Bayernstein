#ifndef _IMAGE_MANAGER_H_
#define _IMAGE_MANAGER_H_

#include <string>
#include <unordered_map>

#include "stb_image.h"

class CImageManager
{
  public:
    struct Image
    {
        int            width;
        int            height;
        int            channels;
        unsigned char* pixeldata;
        bool           isValid;

        Image()
        {
            width     = 0;
            height    = 0;
            channels  = 0;
            pixeldata = nullptr;
            isValid   = false;
        }

        ~Image()
        {
            if ( pixeldata )
            {
                stbi_image_free(pixeldata);
            }
        };
    };

    static CImageManager* Instance();
    Image*                Create(const std::string& filename);

  private:
    CImageManager()  = default;
    ~CImageManager() = default;

    std::unordered_map<std::string, Image*> m_Filename2image;
};

#endif
