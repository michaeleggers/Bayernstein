#ifndef _R_ITEXTURE_MGR_H_
#define _R_ITEXTURE_MGR_H_

#include "r_font.h"

#include <stdint.h>

#include <string>

class ITextureManager {
  public:
    virtual ITexture* CreateTexture(const std::string& filename)          = 0;
    virtual bool      CreateTextureGetHandle(const std::string& filename, uint64_t* out_handle) = 0;
    virtual ITexture* CreateTexture(CFont* font)                   = 0;
    virtual ITexture* GetTexture(std::string filename)             = 0;
    virtual ITexture* GetTexture(uint64_t handle)                  = 0;
};

#endif // _R_ITEXTURE_MGR_H_
