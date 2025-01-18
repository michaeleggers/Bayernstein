//
// Created by me on 04/01/25.
//

#include <SDL.h>

#define GLM_FORCE_RADIANS
#include "../../dependencies/glm/ext.hpp"
#include "../../dependencies/glm/glm.hpp"
#include "../../dependencies/glm/gtx/quaternion.hpp"

#include "../../Audio/Audio.h"
#include "../../globals.h"
#include "../../utils/utils.h"
#include "g_weapon.h"

Weapon::Weapon(const std::vector<Property>& properties)
    : BaseGameEntity(ET_WEAPON)
{
    BaseGameEntity::GetProperty<glm::vec3>(properties, "origin", &m_Position);

    // TODO: Load the type of the model from properties.
    LoadModel("models/double_barrel_shotgun/db_shotgun.iqm", m_Position);

    m_HUDSprite = CreateSprite("shotgun_shell_icon.png", glm::vec2(0.0f), glm::vec2(64.0f));

    m_SfxGunshot
        = Audio::LoadSource("sfx/TriuneFilms/Hollywood_Guns_SFX/mossberg590-12gauge-single-shot-processed-C.wav");
    m_SfxReload = Audio::LoadSource("sfx/TriuneFilms/Gun_Foley_SFX/mossberg590-shotgun-foley-charge-5.wav");

    m_MagSize         = 2;
    m_FireRate        = 800.0;
    m_ReloadTime      = 800.0;
    m_RoundsRemaining = m_MagSize;
}

void Weapon::UpdatePosition(glm::vec3 newPosition)
{
    m_Position = newPosition;
    m_TimeElapsed += GetDeltaTime();
    CheckReload();
}

void Weapon::LoadModel(const char* path, glm::vec3 initialPosition)
{
    // Load IQM Model
    IQMModel iqmModel = LoadIQM(path);

    // Convert the model to our internal format
    m_Model        = CreateModelFromIQM(&iqmModel);
    m_Model.pOwner = this;
    //m_Model.renderFlags |= MODEL_RENDER_FLAG_IGNORE;
    m_Model.isRigidBody = false;
    m_Model.scale       = glm::vec3(1.0f);

    for ( int i = 0; i < m_Model.animations.size(); i++ )
    {
        EllipsoidCollider* ec = &m_Model.ellipsoidColliders[ i ];
        ec->radiusA *= m_Model.scale.x;
        ec->radiusB *= m_Model.scale.z;

        // Add the vertical radius of the collider so we make sure we start
        // *over* the floor.
        ec->center      = initialPosition + glm::vec3(0.0f, 0.0f, ec->radiusB);
        glm::vec3 scale = glm::vec3(1.0f / ec->radiusA, 1.0f / ec->radiusA, 1.0f / ec->radiusB);
        ec->toESpace    = glm::scale(glm::mat4(1.0f), scale);
    }

    // Model is defined with origin at its feet. Move it down to be at the ground.
    //m_Model.position.z -= GetEllipsoidColliderPtr()->radiusB;

    // The model was modeled with -y = forward, but we use +y = forward.
    // So, we rotate the model initially by 180 degrees.
    glm::quat modelForwardFix = glm::angleAxis(glm::radians(180.0f), DOD_WORLD_UP);
    m_Model.orientation       = modelForwardFix;
}

EllipsoidCollider* Weapon::GetEllipsoidColliderPtr()
{
    //return m_Model.ellipsoidColliders[ m_Model.currentAnimIdx ];
    return &m_Model.ellipsoidColliders[ 0 ];
}

HKD_Model* Weapon::GetModel()
{
    return &m_Model;
}

void Weapon::CheckReload()
{ // TODO: separate 'check' from actual reload, to allow manual reloading by user?
    if ( m_RoundsRemaining == 0 && m_TimeElapsed > -0.3 * m_ReloadTime ) // automatic reload
    {
        m_RoundsRemaining = m_MagSize;
        Audio::m_SfxBus.play(*m_SfxReload);
    }
}

bool Weapon::Fire()
{
    if ( m_TimeElapsed >= m_FireRate && m_RoundsRemaining > 0 )
    {
        --m_RoundsRemaining;
        m_TimeElapsed = 0.0;
        Audio::m_SfxBus.play(*m_SfxGunshot);
        if ( m_RoundsRemaining == 0 )
        {
            m_TimeElapsed -= m_ReloadTime;
        }
        return true;
    }
    return false;
}

int Weapon::GetRemainingRounds() const
{
    return m_RoundsRemaining;
}

Sprite Weapon::GetHUDSprite()
{
    return m_HUDSprite;
}

bool Weapon::HandleMessage(const Telegram& telegram)
{
    return true;
}
