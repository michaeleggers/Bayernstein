#include "r_gl_texture_mgr.h"

#include "r_font.h"
#include "r_gl_texture.h"
#include "r_itexture.h"

#include <assert.h>

GLTextureManager::GLTextureManager() {}

GLTextureManager* GLTextureManager::Instance() {
    static GLTextureManager theOneAndOnly;
    return &theOneAndOnly;
}

ITexture* GLTextureManager::CreateTexture(const std::string& filename) {
    if ( m_NameToTexture.contains(filename) ) {
        return m_NameToTexture.at(filename);
    }

    ITexture* result = new GLTexture(filename);

    if ( result->m_IsValid )
    {
        m_NameToTexture.insert({ filename, result });
        m_HandleToTexture.insert({ result->m_hGPU, result });
    }

    return result;
}

bool GLTextureManager::CreateTextureGetHandle(const std::string& filename, uint64_t* out_handle) {
    ITexture* texture = CreateTexture(filename);
    
    if ( !texture->m_IsValid )
    {
        delete texture;
        return false;
    }

    *out_handle = texture->m_hGPU;

    return true;
}

ITexture* GLTextureManager::CreateTexture(CFont* font) {
    if ( m_NameToTexture.contains(font->m_Filename) ) {
        return m_NameToTexture.at(font->m_Filename);
    }

    ITexture* result = new GLTexture(font);

    if ( result->m_IsValid )
    {
        m_NameToTexture.insert({ font->m_Filename, result });
        m_HandleToTexture.insert({ result->m_hGPU, result });
    }

    return result;
}

ITexture* GLTextureManager::GetTexture(std::string filename) {

    if ( m_NameToTexture.contains(filename) ) {
        return m_NameToTexture.at(filename);
    }

    printf("WARNING: GetTexture(): tried to get texture '%s', but not available!\n", filename.c_str());

    return NULL; // TODO: return checkerboard texture.
}

ITexture* GLTextureManager::GetTexture(uint64_t handle) {
    if ( m_HandleToTexture.contains(handle) ) {
        return m_HandleToTexture.at(handle);
    }

    printf("WARNING: GetTexture(): tried to get texture by handle: '%lu', but not available!\n", handle);

    return NULL; // TODO: return checkerboard texture.
}
