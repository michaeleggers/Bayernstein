#ifndef _R_GL_SHADER_H_
#define _R_GL_SHADER_H_

#include <glad/glad.h>

#define GLM_FORCE_RADIANS
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext.hpp"

#include <string>

#include "r_model.h"

#define MAX_BONES 96


#define SHADER_FEATURE_MODEL_ANIMATION_BIT (0x00000001)
#define SHADER_FEATURE_MAX				   (0x00000001 << 1)

class Shader {
public:
	
	bool Load(const std::string& vertName, const std::string& fragName, uint32_t shaderFeatureBits = 0x0);
	void Unload();
	void Activate();
	GLuint Program() const;

	void SetViewProjMatrices(glm::mat4 view, glm::mat4 proj);
	void SetMatrixPalette(glm::mat4* palette, uint32_t numMatrices);
	void SetMat4(std::string uniformName, glm::mat4 mat4);
	void SetVec3(std::string uniformName, glm::vec3 vec3);
	void SetVec4(std::string uniformName, glm::vec4 vec4);
	void DrawWireframe(uint32_t yesOrNo);
	void SetShaderSettingBits(uint32_t bits);
	void ResetShaderSettingBits(uint32_t bits);

	static void InitGlobalBuffers();

private:
	bool CompileShader(const std::string& fileName, GLenum shaderType, GLuint& outShader);
	bool IsCompiled(GLuint shader);
	bool IsValidProgram();

	GLuint m_VertexShader;
	GLuint m_FragmentShader;
	GLuint m_ShaderProgram;

	GLuint m_ViewProjUniformIndex;
	GLuint m_SettingsUniformIndex;
	GLuint m_PaletteUniformIndex;
	GLuint m_ViewProjUBO;
	GLuint m_SettingsUBO;
	GLuint m_PaletteUBO;
};


#endif