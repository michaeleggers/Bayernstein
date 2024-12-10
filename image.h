#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <assert.h>

#include <string>

class CImage {

  public:
    CImage(std::string filename);
    ~CImage() {}; // NOTE: User is responsible to unload the image!

    void FreePixeldata();

    unsigned char* Pixels();
    int            Width();
    int            Height();
    int            Channels();
    bool           Valid();

  private:
    unsigned char* m_Pixeldata = nullptr;
    int            m_Width     = 0;
    int            m_Height    = 0;
    int            m_Channels  = 0;
    bool           m_Valid     = false;
    std::string    m_Filename;
};

#endif
