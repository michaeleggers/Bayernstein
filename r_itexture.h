#ifndef _R_ITEXTURE_H_
#define _R_ITEXTURE_H_

#include <cstdint>
#include <string>

class ITexture {
  public:
	std::string m_Filename;
	int m_Width{}, m_Height{}, m_Channels{};
	uint64_t m_hGPU{};
};

#endif
