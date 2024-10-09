#ifndef _R_GL_TEXTURE_MGR_
#define _R_GL_TEXTURE_MGR_

#include <string>
#include <unordered_map>

#include "r_itexture.h"

class GLTextureManager {
public:
	static GLTextureManager* Instance();
	
	ITexture* CreateTexture(std::string filename);

	// TODO: Shutdown methods

	std::unordered_map<std::string, ITexture*> m_NameToTexture;
private:
	GLTextureManager();
	
};

#endif
