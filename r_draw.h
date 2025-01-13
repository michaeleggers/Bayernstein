#ifndef _R_DRAW_H_
#define _R_DRAW_H_

#define GLM_FORCE_RADIANS
#include "dependencies/glm/ext.hpp"
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/gtx/quaternion.hpp"

#include "Entity/Enemy/g_enemy.h"
#include "r_common.h"
#include "utils/quick_math.h"

void r_DrawFrustum(const math::Frustum& frustum, const Enemy* enemy);

#endif
