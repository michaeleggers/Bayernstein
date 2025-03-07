#ifndef _R_ITEXTURE_H_
#define _R_ITEXTURE_H_

#include <stdint.h>
#include <string>

class ITexture {
  public:
    virtual ~ITexture(){};

    std::string m_Filename;
    int         m_Width, m_Height, m_Channels;
    uint64_t    m_hGPU; // GPU side handle. Assigned by Renderbackend.
    bool        m_IsValid;
};

#endif
