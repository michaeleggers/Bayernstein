#include "r_texture_mgr.h"

#include "r_itexture.h"

TextureManager* TextureManager::Instance()
{
	static TextureManager theOneAndOnly;
	return &theOneAndOnly;
}

ITexture* TextureManager::CreateTexture(std::string filename)
{
	if (m_NameToTexture.contains(filename)) {
		return m_NameToTexture.at(filename);
	}

	
}
