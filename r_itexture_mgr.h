#ifndef _R_ITEXTURE_MGR_H_
#define _R_ITEXTURE_MGR_H_

#include "r_font.h"

#include <stdint.h>

#include <string>

class ITextureManager {
  public:
    virtual ITexture* CreateTexture(std::string filename)          = 0;
    virtual uint64_t  CreateTextureGetHandle(std::string filename) = 0;
    virtual ITexture* CreateTexture(CFont* font)                   = 0;
    virtual ITexture* GetTexture(std::string filename)             = 0;
    virtual ITexture* GetTexture(uint64_t handle)                  = 0;
};

#endif // _R_ITEXTURE_MGR_H_
