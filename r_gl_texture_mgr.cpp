#include "r_gl_texture_mgr.h"

#include "r_gl_texture.h"
#include "r_itexture.h"

GLTextureManager::GLTextureManager() = default;

GLTextureManager *GLTextureManager::Instance() {
	static GLTextureManager theOneAndOnly;
	return &theOneAndOnly;
}

ITexture *GLTextureManager::CreateTexture(const std::string &filename) {
	if (m_NameToTexture.contains(filename)) {
		return m_NameToTexture.at(filename);
	}

	ITexture *result = new GLTexture(filename);

	m_NameToTexture.insert({filename, result});

	return result;
}
