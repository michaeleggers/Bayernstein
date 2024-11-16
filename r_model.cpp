#include "r_model.h"

#include <unordered_map>


#define GLM_FORCE_RADIANS
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext.hpp"
#include "dependencies/glm/gtx/quaternion.hpp"

#include "r_common.h"
#include "collision.h"
#include "map_parser.h"
#include "polysoup.h"

//glm::vec3 pos;
//glm::vec3 uv;
//glm::vec3 bc;
//glm::vec3 normal;
//glm::vec3 color;
//uint32_t  blendindices[4];
//glm::vec4 blendweights;

Vertex IQMVertexToVertex(IQMVertex iqmVert, glm::vec3 bc) {
    Vertex vertex = {
        .pos = glm::vec3(iqmVert.pos[0], iqmVert.pos[1], iqmVert.pos[2]),
        .uv = glm::vec2(iqmVert.texCoord[0], iqmVert.texCoord[1]),
        .bc = bc,
        .normal = glm::vec3(iqmVert.normal[0], iqmVert.normal[1], iqmVert.normal[2]),
        .color = glm::vec4(iqmVert.color[0], iqmVert.color[1], iqmVert.color[2], iqmVert.color[3]),
        .blendweights = glm::vec4(iqmVert.blendweights[0], iqmVert.blendweights[1], iqmVert.blendweights[2], iqmVert.blendweights[3])
    };
    vertex.blendweights /= 255.0f;
    vertex.blendindices[0] = iqmVert.blendindices[0];
    vertex.blendindices[1] = iqmVert.blendindices[1];
    vertex.blendindices[2] = iqmVert.blendindices[2];
    vertex.blendindices[3] = iqmVert.blendindices[3];

    return vertex;
}

HKD_Model CreateModelFromIQM(IQMModel* model)
{
    HKD_Model result = {};

    for (int i = 0; i < model->meshes.size(); i++) {
        IQMMesh* iqmMesh = &model->meshes[i];
        HKD_Mesh mesh = {};
        if (iqmMesh->material.empty()) {
            mesh.isTextured = false;
        }
        else {
            mesh.isTextured = true;
        }
        
        mesh.textureFileName = iqmMesh->material;
        mesh.firstTri = iqmMesh->firstTri;
        mesh.numTris = iqmMesh->numTris;
        for (int v = 0; v < iqmMesh->vertices.size(); v += 3) {

            IQMVertex iqmVertA = iqmMesh->vertices[v + 0];
            IQMVertex iqmVertB = iqmMesh->vertices[v + 1];
            IQMVertex iqmVertC = iqmMesh->vertices[v + 2];

            Vertex vertA = IQMVertexToVertex(iqmVertA, glm::vec3(1.0, 0.0, 0.0));
            Vertex vertB = IQMVertexToVertex(iqmVertB, glm::vec3(0.0, 1.0, 0.0));
            Vertex vertC = IQMVertexToVertex(iqmVertC, glm::vec3(0.0, 0.0, 1.0));

            // NOTE: IQM stores its vertices in +z out of screen, y up, r right.
            // But we use a Blender coordinate system! Note that by doing this
            // Triangles will appear in CW order in RenderDoc (because it uses
            // the same "OpenGL" coordinate system.
            Tri tri = { vertA, vertC, vertB };

            result.tris.push_back(tri);
        }
        result.meshes.push_back(mesh);
    }

    // We take the aabb of the first frame of an animation and ignore the others.
    // Might be changed later. Hopefully good enough for the start.

    int i = 0;
    for (; i < model->animations.size(); i++) {
        Anim a = model->animations[i];
        Frame f = model->frameData[a.firstFrame];
        result.aabbs.push_back({ f.bbmins, f.bbmins });
        Box aabbBox = CreateBoxFromAABB(f.bbmins, f.bbmaxs);        
        result.aabbBoxes.push_back(aabbBox);
        EllipsoidCollider ec = CreateEllipsoidColliderFromAABB(f.bbmins, f.bbmaxs);
        result.ellipsoidColliders.push_back(ec);
    }

    if (i > 0) {
        result.type = HKD_MODEL_TYPE_ANIMATED;
		// TODO: This is just for testing the collision detection.
		// Later we actually want to use dedicated colliders for each animation!
		EllipsoidCollider ec = result.ellipsoidColliders[0];
		for (int i = 0; i < result.ellipsoidColliders.size(); i++) {
			result.ellipsoidColliders[i] = ec;
		}
    } else {
        result.type = HKD_MODEL_TYPE_STATIC;
    }

    result.position     = glm::vec3(0.0f);
    result.orientation  = glm::angleAxis(glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    result.scale        = glm::vec3(1.0f);
    result.filename     = model->filename;
    result.poses        = model->poses;
    result.invBindPoses = model->invBindPoses;
    result.bindPoses    = model->bindPoses;
    result.numJoints    = model->numJoints;
    result.animations   = model->animations;
    result.numFrames    = model->numFrames;
    result.currentAnimIdx = 0;
    result.pctFrameDone = 0.0f;
    result.palette.resize(model->numJoints);
    result.gpuModelHandle = -1;    

    return result;
}

HKD_Model CreateModelFromBrushes(std::vector<Brush>& brushes) {

    // Convert brushes to tris and sort them according to their texture name.
    std::unordered_map<std::string, std::vector<MapPolygon> > texName2polygons{};
    size_t totalTris = 0;
    for (int i = 0; i < brushes.size(); i++) {
        Brush& brush = brushes[i];
        std::vector<MapPolygon> polys = createPolysoup(brush);
        std::vector<MapPolygon> mapTris = triangulate(polys);
        totalTris += mapTris.size();
        for (int j = 0; j < mapTris.size(); j++) {
            MapPolygon& mapTri = mapTris[j];
            const auto& entry = texName2polygons.find(mapTri.textureName);
            if ( entry == texName2polygons.end() ) {
                texName2polygons.insert({ mapTri.textureName, {mapTri} });
            }
            else {
                entry->second.push_back(mapTri);
            }
        }
    }

    // Assign sorted MapPolygons to model as meshes.
    /*
    HKD_Model result{};
    result.type = HKD_MODEL_TYPE_STATIC;
    result.tris.resize(totalTris);
    size_t triOffset = 0;
    for (auto & [ textureName, mapPolys ] : texName2polygons) {
        HKD_Mesh mesh{};
        mesh.isTextured = true;
        mesh.textureFileName = textureName;
        mesh.firstTri = triOffset;
        size_t numTris = mapPolys.size();
        mesh.numTris = numTris;
        for (int i = 0; i < numTris; i++) {
            MapPolygon& mapPoly = mapPolys[i];
            result.tris[triOffset + i] = mapPoly.tri;
        }
        triOffset += numTris;
        result.meshes.push_back(mesh);
    }
    */
    
    return {};
}

static glm::mat4 PoseToMatrix(Pose pose) 
{
    glm::vec3 t = glm::vec3(pose.translations.x, pose.translations.y, pose.translations.z);
    glm::mat4 tMat = glm::translate(glm::mat4(1.0f), t); // TODO: Change to translation
    glm::vec3 s = glm::vec3(pose.scale.x, pose.scale.y, pose.scale.z);
    glm::mat4 sMat = glm::scale(glm::mat4(1.0f), s);
    //glm::quat r = glm::quat(pose.rotation.w, pose.rotation.x, pose.rotation.y, pose.rotation.z);
    glm::mat4 rMat = glm::toMat4(pose.rotation);

    return tMat * rMat * sMat;
}

static glm::mat4 InterpolatePoses(Pose a, Pose b, float pct) 
{
    glm::vec3 interpTrans = (1.0f - pct) * a.translations + pct * b.translations;
    glm::mat4 transMat = glm::translate(glm::mat4(1.0f), interpTrans);

    glm::vec3 interpScale = (1.0f - pct) * a.scale + pct * b.scale;
    glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), interpScale);

    glm::quat interpRot = glm::slerp(a.rotation, b.rotation, pct);
    glm::mat4 rotMat = glm::toMat4(interpRot);

    return transMat * rotMat * scaleMat;    
}

void UpdateModel(HKD_Model* model, float dt)
{
    if (model->type == HKD_MODEL_TYPE_ANIMATED) {
        uint32_t currentFrame = model->currentFrame;
        uint32_t animIdx = model->currentAnimIdx;

        Anim anim = model->animations[animIdx];
        float msPerFrame = 1000.0f / anim.framerate;

        // If the frame took really long then we need to catch up

        while (dt > msPerFrame) {
            dt -= msPerFrame;
            currentFrame++;
            model->pctFrameDone -= msPerFrame;
        }

        if (model->pctFrameDone < 0.0) {
            model->pctFrameDone = 0.0;
        }

        if (dt < 0.0) {
            dt = 0.0;
        }

        model->pctFrameDone += dt;

        if (model->pctFrameDone > msPerFrame) {
            currentFrame++;
            model->pctFrameDone -= msPerFrame;
        }

        // For now, we just cylce through all animations. If the current animations has reached its
        // end, we jump to the next animation.

        if (currentFrame >= anim.firstFrame + anim.numFrames-1) {
            //model->currentAnimIdx = (model->currentAnimIdx + 1) % model->animations.size();
            anim = model->animations[model->currentAnimIdx];
            currentFrame = anim.firstFrame;
        }
        model->currentFrame = currentFrame;
        uint32_t nextFrame = (currentFrame + 1) % (anim.firstFrame + anim.numFrames);
        if (nextFrame < anim.firstFrame) {
            nextFrame = anim.firstFrame;
        }

        //printf("currentFrame: %d\n", currentFrame);


        // Build the matrix palette


        // Build the global transform for each bone for the current pose

        for (int i = 0; i < model->numJoints; i++) {
            Pose currentPoseTransform = model->poses[currentFrame * model->numJoints + i];
            Pose nextPoseTransform = model->poses[nextFrame * model->numJoints + i];
            glm::mat4 poseMat = InterpolatePoses(currentPoseTransform, nextPoseTransform, model->pctFrameDone/msPerFrame);
            if (currentPoseTransform.parent >= 0) {
                model->palette[i] = model->palette[currentPoseTransform.parent] * poseMat;
            }
            else {
                model->palette[i] = poseMat;
            }
        }

        // Post multiply the global transforms with the global inverse bind transform to get
        // the vertex from bindspace to local bonespace first and then transform the
        // vertex to the currents pose global bone space.

        for (int i = 0; i < model->numJoints; i++) {
            glm::mat4 invGlobalMat = model->invBindPoses[i];
            model->palette[i] = model->palette[i] * invGlobalMat;
        }

        // Update Ellipsoid Collider center
        // TODO: Also entities that aren't animated should have this.
		/*
        EllipsoidCollider* ec = &model->ellipsoidColliders[model->currentAnimIdx];
        ec->center = model->position + glm::vec3(
            0.0f,
            0.0f,
            ec->radiusB);
		*/
    } // DONE WITH ANIMATION

    // Update the rigid body

    if (model->isRigidBody) {
        UpdateRigidBodyTransform(model);
    }
}

void ApplyPhysicsToModel(HKD_Model* model)
{

}

void UpdateRigidBodyTransform(HKD_Model* model)
{
    model->position = model->body.m_Position;
}

glm::mat4 CreateModelMatrix(HKD_Model* model)
{
    glm::mat4 transMat = glm::translate(glm::mat4(1.0f), model->position);
    glm::mat4 rotMat = glm::toMat4(model->orientation);
    glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), model->scale);
    
    return transMat * rotMat * scaleMat;
}

glm::mat4 CreateModelMatrix(glm::vec3 pos, glm::quat orientation, glm::vec3 scale) 
{
    glm::mat4 T = glm::translate(glm::mat4(1.0f), pos);
    glm::mat4 R = glm::toMat4(orientation);
    glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

    return T * R * S;
}

void SetAnimState(HKD_Model* model, AnimState animState)
{
    AnimState currentState = (AnimState)model->currentAnimIdx;

    if (currentState == animState) {
        return;
    }
    
    model->currentAnimIdx = (uint32_t)animState;
    Anim anim = model->animations[model->currentAnimIdx];
    uint32_t firstFrame = anim.firstFrame;
    model->currentFrame = firstFrame;
    
    //printf("Current anim idx: %d\n", model->currentAnimIdx);
    //printf("Current frame: %d\n", model->currentFrame);
}
