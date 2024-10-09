#include "iqm_loader.h" 

#include <assert.h>

#include <string>
#include <vector>

#define GLM_FORCE_RADIANS
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext.hpp"
#include "dependencies/glm/gtx/quaternion.hpp"

#include "platform.h"

extern std::string  g_GameDir;


IQMModel LoadIQM(const char* file)
{
	IQMModel result = {};
	result.filename = g_GameDir + std::string(file);

	HKD_File iqmFile;
	if (hkd_read_file(result.filename.c_str() , &iqmFile) != HKD_FILE_SUCCESS) {
		printf("Could not read IQM file: %s\n", file);
		exit(-1);
	}

 	uint8_t* iqmData = (uint8_t*)iqmFile.data;

	IQMHeader* pHeader = (IQMHeader*)iqmFile.data;
	char* pText = (char*)(iqmData + pHeader->ofsText);
	IQMMeshData* pMeshes = (IQMMeshData*)(iqmData + pHeader->ofsMeshes);
	IQMVertArray* pVertArrays = (IQMVertArray*)(iqmData + pHeader->ofsVertArrays);
	uint32_t numVertices = pHeader->numVertices;
	IQMTri* pTris = (IQMTri*)(iqmData + pHeader->ofsTris);
	uint8_t* pAdjacency = (uint8_t*)(iqmData + pHeader->ofsAdjacency);
	IQMJoint* pJoints = (IQMJoint*)(iqmData + pHeader->ofsJoints);
	IQMPose* pPoses = (IQMPose*)(iqmData + pHeader->ofsPoses);
	IQMAnim* pAnims = (IQMAnim*)(iqmData + pHeader->ofsAnims);
	uint16_t* pFrames = (uint16_t*)(iqmData + pHeader->ofsFrames);
	IQMBounds* pBounds = (IQMBounds*)(iqmData + pHeader->ofsBounds);
	char* pComments = (char*)(iqmData + pHeader->ofsComment);
	uint8_t* pExt = (uint8_t*)(iqmData + pHeader->ofsExt);

	printf("IQM Header: %s\n", pHeader->magic);

	uint8_t* pPositions = NULL;
	uint32_t positionStride = 0;
	uint8_t* pTexCoords = NULL;
	uint32_t texCoordStride = 0;
	uint8_t* pNormals = NULL;
	uint32_t normalStride = 0;
	uint8_t* pBlendIndices = NULL;
	uint32_t blendIndexStride = 0;
	uint8_t* pBlendWeights = NULL;
	uint32_t blendWeightStride = 0;

	for (int i = 0; i < pHeader->numVertArrays; i++) {
		
		IQMVertArray* pVertArray = pVertArrays + i;
		IQMVertArrayType type = static_cast<IQMVertArrayType>(pVertArray->type);
		IQMVertArrayFormat format = static_cast<IQMVertArrayFormat>(pVertArray->format);
		uint32_t numComponents = pVertArray->size;
		uint32_t offset = pVertArray->offset;

		if (type == IQM_POSITION) {
			pPositions = iqmData + offset;
			uint32_t dataSize = 0;
			if (format == IQM_FLOAT) {
				dataSize = sizeof(float);
			}
			else if (format == IQM_DOUBLE) {
				dataSize = sizeof(double);
			}
			positionStride = numComponents * dataSize;
		}
		else if (type == IQM_TEXCOORD) {
			pTexCoords = iqmData + offset;
			uint32_t dataSize = 0;
			if (format == IQM_FLOAT) {
				dataSize = sizeof(float);
			}
			else if (format == IQM_DOUBLE) {
				dataSize = sizeof(double);
			}
			texCoordStride = numComponents * dataSize;
		}
		else if (type == IQM_NORMAL) {
			pNormals = iqmData + offset;
			uint32_t dataSize = 0;
			if (format == IQM_FLOAT) {
				dataSize = sizeof(float);
			}
			else if (format == IQM_DOUBLE) {
				dataSize = sizeof(double);
			}
			normalStride = numComponents * dataSize;
		}
		else if (type == IQM_BLENDINDIXES) {
			pBlendIndices = iqmData + offset;
			uint32_t dataSize = 0;
			if (format == IQM_UBYTE) {
				dataSize = sizeof(uint8_t);
			}
			blendIndexStride = numComponents * dataSize;
		}
		else if (type == IQM_BLENDWEIGHTS) {
			pBlendWeights = iqmData + offset;
			uint32_t dataSize = 0;
			if (format == IQM_UBYTE) {
				dataSize = sizeof(uint8_t);
			}
			blendWeightStride = numComponents * dataSize;
		}
		
		printf("type: %d, format: %d, numComponents: %d, offset: %d\n", type, format, numComponents, offset);
	}
	
	for (int i = 0; i < pHeader->numMeshes; i++) {
		IQMMeshData* iqmMesh = pMeshes + i;
		uint32_t firstTri = iqmMesh->firstTri;
		uint32_t numTris = iqmMesh->numTris;
		IQMMesh mesh = {};
		//mesh.vertices = (IQMVertex*)malloc(iqmMesh->numVerts * sizeof(IQMVertex));
		mesh.material = std::string(pText + iqmMesh->material);
		mesh.firstTri = firstTri;
		mesh.numTris = numTris;

		for (int j = 0; j < numTris; j++) {
			IQMTri* tri = pTris + firstTri + j;

			IQMVertex vertA = {};
			IQMVertex vertB = {};
			IQMVertex vertC = {};

			memcpy(vertA.pos, pPositions + tri->vertex[0] * positionStride, 3 * sizeof(float));
			memcpy(vertB.pos, pPositions + tri->vertex[1] * positionStride, 3 * sizeof(float));
			memcpy(vertC.pos, pPositions + tri->vertex[2] * positionStride, 3 * sizeof(float));

			memcpy(vertA.texCoord, pTexCoords + tri->vertex[0] * texCoordStride, 2 * sizeof(float));
			memcpy(vertB.texCoord, pTexCoords + tri->vertex[1] * texCoordStride, 2 * sizeof(float));
			memcpy(vertC.texCoord, pTexCoords + tri->vertex[2] * texCoordStride, 2 * sizeof(float));

			memcpy(vertA.normal, pNormals + tri->vertex[0] * normalStride, 3 * sizeof(float));
			memcpy(vertB.normal, pNormals + tri->vertex[1] * normalStride, 3 * sizeof(float));
			memcpy(vertC.normal, pNormals + tri->vertex[2] * normalStride, 3 * sizeof(float));

			if (pHeader->numJoints > 0) {
				memcpy(vertA.blendindices, pBlendIndices + tri->vertex[0] * blendIndexStride, 4);
				memcpy(vertB.blendindices, pBlendIndices + tri->vertex[1] * blendIndexStride, 4);
				memcpy(vertC.blendindices, pBlendIndices + tri->vertex[2] * blendIndexStride, 4);

				memcpy(vertA.blendweights, pBlendWeights + tri->vertex[0] * blendWeightStride, 4);
				memcpy(vertB.blendweights, pBlendWeights + tri->vertex[1] * blendWeightStride, 4);
				memcpy(vertC.blendweights, pBlendWeights + tri->vertex[2] * blendWeightStride, 4);
			}

			mesh.vertices.push_back(vertA);
			mesh.vertices.push_back(vertB);
			mesh.vertices.push_back(vertC);
		}
		result.meshes.push_back(mesh);
	}

	std::vector<glm::mat4> globalBindPoses;
	std::vector<glm::mat4> bindPoses;
	std::vector<glm::mat4> invBindPoses;
	bindPoses.resize(pHeader->numJoints);
	for (int i = 0; i < pHeader->numJoints; i++) {
		IQMJoint* pJoint = pJoints + i;
		printf("Joint %d name: %s\n", i, pText + pJoint->name);
		
		glm::vec3 translation = glm::vec3(pJoint->translate[0], pJoint->translate[1], pJoint->translate[2]);
		// WARNING: GLM STORES W AS ITS ____FIRST____ COMPONENT!!! WHAT THE FUCK IS WRONG WITH THEM???
		glm::quat rotation = glm::quat(pJoint->rotate[3], pJoint->rotate[0], pJoint->rotate[1], pJoint->rotate[2]);
		glm::vec3 scale = glm::vec3(pJoint->scale[0], pJoint->scale[1], pJoint->scale[2]);

		glm::mat4 mTranslate = glm::translate(glm::mat4(1.0f), translation);
		glm::mat4 mRotate = glm::toMat4(rotation);
		glm::mat4 mScale = glm::scale(glm::mat4(1.0f), scale);
		glm::mat4 m = mTranslate * mRotate * mScale;
		bindPoses[i] = m;
		if (pJoint->parent >= 0) {
			bindPoses[i] = bindPoses[pJoint->parent] * bindPoses[i];
		}
	}

	// Invert global bind poses

	invBindPoses.resize(pHeader->numJoints);
	for (int i = 0; i < invBindPoses.size(); i++) {
		invBindPoses[i] = glm::inverse(bindPoses[i]);
	}

	result.bindPoses = bindPoses;
	result.invBindPoses = invBindPoses;

	std::vector<Pose> poses;
	uint16_t* pFramedata = pFrames;
	for (int i = 0; i < pHeader->numFrames; i++) {

		IQMBounds* pBound = pBounds + i;

		Frame frame = {};
		frame.bbmins = glm::vec3(pBound->bbmins[0], pBound->bbmins[1], pBound->bbmins[2]);
		frame.bbmaxs = glm::vec3(pBound->bbmaxs[0], pBound->bbmaxs[1], pBound->bbmaxs[2]);
		frame.xyRadius = pBound->xyradius;
		frame.sphericalRadius = pBound->radius;
		result.frameData.push_back(frame);

		for (int j = 0; j < pHeader->numPoses; j++) {  // A pose is actually the local matrix for a joint
			IQMPose* pPose = pPoses + j;
			glm::vec3 translation = {};
			glm::vec3 scale = {};
			glm::quat rotation = {};

			translation.x = pPose->channeloffset[0];
			if (pPose->channelmask & 0x01) {
				translation.x += (float)*pFramedata * pPose->channelscale[0];
				pFramedata += 1;
			}
			translation.y = pPose->channeloffset[1];
			if (pPose->channelmask & 0x02) {
				translation.y += (float)*pFramedata * pPose->channelscale[1];
				pFramedata += 1;
			}
			translation.z = pPose->channeloffset[2];
			if (pPose->channelmask & 0x04) {
				translation.z += (float)*pFramedata * pPose->channelscale[2];
				pFramedata += 1;
			}

			rotation.x = pPose->channeloffset[3];
			if (pPose->channelmask & 0x08) {
				rotation.x += (float)*pFramedata * pPose->channelscale[3];
				pFramedata += 1;
			}
			rotation.y = pPose->channeloffset[4];
			if (pPose->channelmask & 0x10) {
				rotation.y += (float)*pFramedata * pPose->channelscale[4];
				pFramedata += 1;
			}
			rotation.z = pPose->channeloffset[5];
			if (pPose->channelmask & 0x20) {
				rotation.z += (float)*pFramedata * pPose->channelscale[5];
				pFramedata += 1;
			}
			rotation.w = pPose->channeloffset[6];
			if (pPose->channelmask & 0x40) {
				rotation.w += (float)*pFramedata * pPose->channelscale[6];
				pFramedata += 1;
			}			

			scale.x = pPose->channeloffset[7];
			if (pPose->channelmask & 0x80) {
				scale.x += (float)*pFramedata * pPose->channelscale[7];
				pFramedata += 1;
			}
			scale.y = pPose->channeloffset[8];
			if (pPose->channelmask & 0x100) {
				scale.y += (float)*pFramedata * pPose->channelscale[8];
				pFramedata += 1;
			}
			scale.z = pPose->channeloffset[9];
			if (pPose->channelmask & 0x200) {
				scale.z += (float)*pFramedata * pPose->channelscale[9];
				pFramedata += 1;
			}

			Pose pose = {};
			pose.parent = pPose->parent;
			pose.translations = translation;
			pose.scale = scale;
			pose.rotation = rotation;

			result.poses.push_back(pose);
		}
	}

	result.numJoints = pHeader->numJoints;
	result.numFrames = pHeader->numFrames;

	for (int i = 0; i < pHeader->numAnims; i++) {
		IQMAnim* pAnim = pAnims + i;

		printf("Animation %d, name: %s, first frame: %d, last frame: %d, num frames: %d\n", 
			i, pText + pAnim->name, pAnim->firstFrame, pAnim->firstFrame + pAnim->numFrames, pAnim->numFrames);

		Anim anim = {};
		anim.name = std::string(pText + pAnim->name);
		anim.firstFrame = pAnim->firstFrame;
		anim.numFrames = pAnim->numFrames;
		anim.framerate = pAnim->framerate;

		result.animations.push_back(anim);
	}

	hkd_destroy_file(&iqmFile);

	return result;
}

void UnloadIQM(IQMModel* iqmModel)
{
}
