#include "r_gl_texture_mgr.h"

#include "r_itexture.h"
#include "r_font.h"
#include "r_gl_texture.h"

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

	return result;
}

ITexture* GLTextureManager::CreateTexture(CFont* font)
{
	if (m_NameToTexture.contains(font->m_Filename)) {
		return m_NameToTexture.at(font->m_Filename);
	}

	ITexture* result = new GLTexture(font);

	m_NameToTexture.insert({ font->m_Filename, result });

	return result;
}

