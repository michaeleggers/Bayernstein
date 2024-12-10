#ifndef _R_GL_TEXTURE_MGR_
#define _R_GL_TEXTURE_MGR_

#include <string>
#include <unordered_map>

#include <stdint.h>

#include "r_font.h"
#include "r_itexture.h"

class GLTextureManager {
  public:
    static GLTextureManager* Instance();

    ITexture* CreateTexture(std::string filename);
    uint64_t  CreateTextureGetHandle(std::string filename);
    ITexture* CreateTexture(CFont* font);
    ITexture* GetTexture(std::string filename);
    ITexture* GetTexture(uint64_t handle);

    // TODO: Shutdown methods

    std::unordered_map<std::string, ITexture*> m_NameToTexture;
    std::unordered_map<uint64_t, ITexture*>    m_HandleToTexture;

  private:
    GLTextureManager();
};

#endif
