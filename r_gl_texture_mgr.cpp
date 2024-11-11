#include "r_gl_texture_mgr.h"

#include "r_itexture.h"
#include "r_font.h"
#include "r_gl_texture.h"

#include <assert.h>

GLTextureManager::GLTextureManager() {

}

GLTextureManager* GLTextureManager::Instance()
{
	static GLTextureManager theOneAndOnly;
	return &theOneAndOnly;
}

ITexture* GLTextureManager::CreateTexture(std::string filename)
{
	if (m_NameToTexture.contains(filename)) {
		return m_NameToTexture.at(filename);
	}

	ITexture* result = new GLTexture(filename);

	m_NameToTexture.insert({ filename, result });
	m_HandleToTexture.insert({ result->m_hGPU, result });

	return result;
}

uint64_t GLTextureManager::CreateTextureGetHandle(std::string filename) {
	ITexture* texture = CreateTexture(filename);
	assert( texture != NULL && "Return of CreateTexture is NULL!" );

	return texture->m_hGPU;
}

ITexture* GLTextureManager::CreateTexture(CFont* font)
{
	if (m_NameToTexture.contains(font->m_Filename)) {
		return m_NameToTexture.at(font->m_Filename);
	}

	ITexture* result = new GLTexture(font);

	m_NameToTexture.insert({ font->m_Filename, result });
	m_HandleToTexture.insert({ result->m_hGPU, result });

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

