#ifndef _MODEL_H_
#define _MODEL_H_

#include <stdint.h>

#include <string>
#include <vector>

#define GLM_FORCE_RADIANS
#include "dependencies/glm/ext.hpp"
#include "dependencies/glm/glm.hpp"

#include "Body.h"
#include "collision.h"
#include "iqm_loader.h"
#include "r_common.h"
#include "map_parser.h"
#include "Entity/base_game_entity.h"

enum AnimState {
    ANIM_STATE_IDLE,
    ANIM_STATE_WALK,
    ANIM_STATE_RUN,
    ANIM_STATE_NONE
};

struct HKD_Mesh {
    uint32_t firstTri, numTris;
    bool isTextured;
    std::string textureFileName;
};

struct AABB {
    glm::vec3 mins;
    glm::vec3 maxs;
};

enum HKD_ModelType {
    HKD_MODEL_TYPE_STATIC,
    HKD_MODEL_TYPE_ANIMATED
};

#define MODEL_RENDER_FLAG_NONE      (0x00000001 << 0)
#define MODEL_RENDER_FLAG_IGNORE    (0x00000001 << 1)

struct HKD_Model {
    BaseGameEntity* pOwner;
    HKD_ModelType type;
    std::string filename;
    std::vector<Tri> tris;
    std::vector<HKD_Mesh> meshes;
    glm::vec3 position;    // Relative to its entity owner.
    glm::quat orientation; // Relative to its entity owner.
    glm::vec3 scale;
    int gpuModelHandle; // -1: Data not yet on GPU
    std::vector<Pose>
        poses; // A POSE IS JUST A LOCAL TRANSFORM FOR A SINGLE JOINT!!! IT IS NOT THE SKELETON STATE AT A CERTAIN FRAME!
    uint32_t currentFrame;
    uint32_t numFrames;
    float pctFrameDone;
    std::vector<glm::mat4> invBindPoses;
    std::vector<glm::mat4> bindPoses;
    std::vector<glm::mat4> palette;
    uint32_t numJoints;
    std::vector<Anim> animations;
    std::vector<AABB> aabbs;    // one AABB for each animation (first frame of anim used).
    std::vector<Box> aabbBoxes; // Actual vertex geometry for each aabb ready to render
    std::vector<EllipsoidCollider> ellipsoidColliders;
    uint32_t currentAnimIdx;
    uint32_t prevAnimIdx;

    // For drawing debugging colors. TODO: Remove later?
    glm::vec4 debugColor;

    // Rigid Body Physics
    bool isRigidBody;
    Body body;
    uint32_t renderFlags;
};

HKD_Model CreateModelFromIQM(IQMModel* model);
HKD_Model CreateModelFromBrushes(const std::vector<Brush>& brushes);
void UpdateModel(HKD_Model* model, float dt);
void ApplyPhysicsToModel(HKD_Model* model);
void UpdateRigidBodyTransform(HKD_Model* model);
glm::mat4 CreateModelMatrix(HKD_Model* model);
glm::mat4 CreateModelMatrix(glm::vec3 pos, glm::quat orientation, glm::vec3 scale);
void SetAnimState(HKD_Model* model, AnimState animState);

#endif
