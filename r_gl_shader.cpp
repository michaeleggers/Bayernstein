#include "r_gl_shader.h"

#include <glad/glad.h>

#include <stdio.h>

#define GLM_FORCE_RADIANS
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext.hpp"

#include "platform.h"
#include "r_common.h"

extern std::string g_GameDir;

// GLOBAL SHADER BUFFERS (USED BY ALL SHADERS)

#define BIND_POINT_VIEW_PROJECTION    0
#define BIND_POINT_SETTINGS			  1
static GLuint g_PaletteBindingPoint = 2;

static GLuint   g_ViewProjUBO;
static GLuint   g_SettingsUBO;
static uint32_t g_SettingsBits;

bool Shader::Load(const std::string& vertName, const std::string& fragName, uint32_t shaderFeatureBits)
{
	if (!CompileShader(vertName, GL_VERTEX_SHADER, m_VertexShader)
		|| !CompileShader(fragName, GL_FRAGMENT_SHADER, m_FragmentShader)) {

		return false;
	}

	m_ShaderProgram = glCreateProgram();
	glAttachShader(m_ShaderProgram, m_VertexShader);
	glAttachShader(m_ShaderProgram, m_FragmentShader);
	glLinkProgram(m_ShaderProgram);

	if (!IsValidProgram()) return false;

	// Uniforms

	// Per frame matrices
	//glBindBuffer(GL_UNIFORM_BUFFER, g_ViewProjUBO);
	GLuint bindingPoint = BIND_POINT_VIEW_PROJECTION;
	m_ViewProjUniformIndex = glGetUniformBlockIndex(m_ShaderProgram, "ViewProjMatrices");
	if (m_ViewProjUniformIndex == GL_INVALID_INDEX) {
		printf("SHADER-WARNING: Not able to get index for UBO in shader program.\nShaders:\n %s\n %s\n", vertName.c_str(), fragName.c_str());
		// TODO: What to do in this case???
	}
	glUniformBlockBinding(m_ShaderProgram, m_ViewProjUniformIndex, bindingPoint);
	glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, g_ViewProjUBO, 0, 2*sizeof(glm::mat4));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Per frame settings
	//glBindBuffer(GL_UNIFORM_BUFFER, g_SettingsUBO);
	GLuint settingsBindingPoint = BIND_POINT_SETTINGS;
	m_SettingsUniformIndex = glGetUniformBlockIndex(m_ShaderProgram, "Settings");
	if (m_SettingsUniformIndex == GL_INVALID_INDEX) {
		printf("SHADER-WARNING: Not able to get index for UBO in shader program.\nShaders:\n %s\n %s\n", vertName.c_str(), fragName.c_str());		
		// TODO: What to do in this case???
	}
	glUniformBlockBinding(m_ShaderProgram, m_SettingsUniformIndex, settingsBindingPoint);
	glBindBufferRange(GL_UNIFORM_BUFFER, settingsBindingPoint, g_SettingsUBO, 0, 4 * sizeof(uint32_t));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// TODO: Animation stuff is not global to all shaders. That is why we check for shader flags here.

	if (shaderFeatureBits & SHADER_FEATURE_MODEL_ANIMATION_BIT) {

		// Per frame per model animation matrix palette
		GLuint paletteBindingPoint = g_PaletteBindingPoint++; // If a UBO is not being shared between shaders we need separate binding points to these buffers
		m_PaletteUniformIndex = glGetUniformBlockIndex(m_ShaderProgram, "Palette");
		if (m_PaletteUniformIndex == GL_INVALID_INDEX) {
			printf("SHADER-WARNING: Not able to get index for UBO in shader program.\nShaders:\n %s\n %s\n", vertName.c_str(), fragName.c_str());
			// TODO: What to do in this case???
		}
		glUniformBlockBinding(m_ShaderProgram, m_PaletteUniformIndex, paletteBindingPoint);

		glGenBuffers(1, &m_PaletteUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, m_PaletteUBO);
		glBufferData(GL_UNIFORM_BUFFER, MAX_BONES * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
		glBindBufferRange(GL_UNIFORM_BUFFER, paletteBindingPoint, m_PaletteUBO, 0, MAX_BONES*sizeof(glm::mat4));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	return true;
}

void Shader::Unload()
{
	glDeleteProgram(m_ShaderProgram);
	glDeleteShader(m_VertexShader);
	glDeleteShader(m_FragmentShader);
}

void Shader::Activate()
{
	glUseProgram(m_ShaderProgram);
}

GLuint Shader::Program() const
{
	return m_ShaderProgram;
}

void Shader::SetViewProjMatrices(glm::mat4 view, glm::mat4 proj)
{	
	glBindBuffer(GL_UNIFORM_BUFFER, g_ViewProjUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(proj));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Shader::SetMatrixPalette(glm::mat4* palette, uint32_t numMatrices)
{
	glBindBuffer(GL_UNIFORM_BUFFER, m_PaletteUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, numMatrices*sizeof(glm::mat4), palette);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Shader::SetMat4(std::string uniformName, glm::mat4 mat4)
{
	GLuint loc = glGetUniformLocation(m_ShaderProgram, uniformName.c_str());
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&mat4);
}

void Shader::SetVec3(std::string uniformName, glm::vec3 vec3)
{
	GLuint loc = glGetUniformLocation(m_ShaderProgram, uniformName.c_str());
	glUniform3fv(loc, 1, (float*)&vec3);
}

void Shader::SetVec4(std::string uniformName, glm::vec4 vec4) {
	GLuint loc = glGetUniformLocation(m_ShaderProgram, uniformName.c_str());
	glUniform4fv(loc, 1, (float*)&vec4);
}

void Shader::DrawWireframe(uint32_t yesOrNo)
{		
	if (yesOrNo) {
		g_SettingsBits |= SHADER_WIREFRAME_ON_MESH;
	}
	glBindBuffer(GL_UNIFORM_BUFFER, g_SettingsUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(uint32_t), (void*)&yesOrNo);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Shader::SetShaderSettingBits(uint32_t bits)
{
	g_SettingsBits |= bits;
	ShaderSettings settings;
	settings.u32bitMasks.x = g_SettingsBits;
	glBindBuffer(GL_UNIFORM_BUFFER, g_SettingsUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, 4 * sizeof(uint32_t), (void*)&settings);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Shader::ResetShaderSettingBits(uint32_t bits)
{
	g_SettingsBits = (g_SettingsBits & (~bits));
	glBindBuffer(GL_UNIFORM_BUFFER, g_SettingsUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, 4*sizeof(uint32_t), (void*)&g_SettingsBits);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Shader::InitGlobalBuffers()
{
	printf("INITIALIZE GLOBAL SHADER BUFFERS...\n");

	// view projection matrices
	
	glGenBuffers(1, &g_ViewProjUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, g_ViewProjUBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// render settings (such as wireframe)

	glGenBuffers(1, &g_SettingsUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, g_SettingsUBO);
	glBufferData(GL_UNIFORM_BUFFER, 4 * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

bool Shader::CompileShader(const std::string& fileName, GLenum shaderType, GLuint& outShader)
{
	std::string shaderFilePath = g_GameDir + fileName;
	HKD_File shaderCode;
	if (hkd_read_file(shaderFilePath.c_str(), &shaderCode) != HKD_FILE_SUCCESS) {
		printf("Could not read file: %s!\n", fileName.c_str());
		return false;
	}

	outShader = glCreateShader(shaderType);
	glShaderSource(outShader, 1, (GLchar**)(&shaderCode.data), nullptr);
	glCompileShader(outShader);

	if (!IsCompiled(outShader)) {
		printf("Failed to compile shader: %s\n", fileName.c_str());
		return false;
	}

	return true;
}

bool Shader::IsCompiled(GLuint shader)
{
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		char buffer[512];
		memset(buffer, 0, 512);
		glGetShaderInfoLog(shader, 511, nullptr, buffer);
		printf("GLSL compile error:\n%s\n", buffer);

		return false;
	}

	return true;
}

bool Shader::IsValidProgram()
{
	GLint status;
	glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		char buffer[512];
		memset(buffer, 0, 512);
		glGetProgramInfoLog(m_ShaderProgram, 512, nullptr, buffer);
		printf("GLSL compile error:\n%s\n", buffer);

		return false;
	}

	return true;
}
