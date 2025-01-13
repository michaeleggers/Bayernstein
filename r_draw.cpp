#include "r_draw.h"

#define GLM_FORCE_RADIANS
#include "dependencies/glm/ext.hpp"
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/gtx/quaternion.hpp"

#include "hkd_interface.h"
#include "irender.h"
#include "r_common.h"
#include "utils/quick_math.h"

void r_DrawFrustum(const math::Frustum& frustum)
{
    IRender* renderer = GetRenderer();

    /*
    Line frustumNormalA // right
        = { Vertex(enemy->m_Position + enemy->m_Far * enemy->m_Forward - 50.0f * frustumWorld.planes[ 0 ].normal),
            Vertex(enemy->m_Position + enemy->m_Far * enemy->m_Forward - 50.0f * frustumWorld.planes[ 0 ].normal
                   + 30.0f * frustumWorld.planes[ 0 ].normal) };
    frustumNormalA.a.color = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
    frustumNormalA.b.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    renderer->ImDrawLines(frustumNormalA.vertices, 2, false);

    Line frustumNormalB // top
        = { Vertex(enemy->m_Position + enemy->m_Far * enemy->m_Forward - 50.0f * frustumWorld.planes[ 1 ].normal),
            Vertex(enemy->m_Position + enemy->m_Far * enemy->m_Forward - 50.0f * frustumWorld.planes[ 1 ].normal
                   + 30.0f * frustumWorld.planes[ 1 ].normal) };
    frustumNormalB.a.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    frustumNormalB.b.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    renderer->ImDrawLines(frustumNormalB.vertices, 2, false);

    Line frustumNormalC // left
        = { Vertex(enemy->m_Position + enemy->m_Far * enemy->m_Forward - 50.0f * frustumWorld.planes[ 2 ].normal),
            Vertex(enemy->m_Position + enemy->m_Far * enemy->m_Forward - 50.0f * frustumWorld.planes[ 2 ].normal
                   + 30.0f * frustumWorld.planes[ 2 ].normal) };
    frustumNormalC.a.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
    frustumNormalC.b.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    renderer->ImDrawLines(frustumNormalC.vertices, 2, false);

    Line frustumNormalD
        = { Vertex(enemy->m_Position + enemy->m_Far * enemy->m_Forward - 50.0f * frustumWorld.planes[ 3 ].normal),
            Vertex(enemy->m_Position + enemy->m_Far * enemy->m_Forward - 50.0f * frustumWorld.planes[ 3 ].normal
                   + 30.0f * frustumWorld.planes[ 3 ].normal) };
    frustumNormalD.a.color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    frustumNormalD.b.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    renderer->ImDrawLines(frustumNormalD.vertices, 2, false);
*/

    // Near and far planes
    Line frustumSideA = { frustum.vertices[ 0 ], frustum.vertices[ 4 ] };
    Line frustumSideB = { frustum.vertices[ 1 ], frustum.vertices[ 5 ] };
    Line frustumSideC = { frustum.vertices[ 2 ], frustum.vertices[ 6 ] };
    Line frustumSideD = { frustum.vertices[ 3 ], frustum.vertices[ 7 ] };
    renderer->ImDrawLines(frustumSideA.vertices, 2, false);
    renderer->ImDrawLines(frustumSideB.vertices, 2, false);
    renderer->ImDrawLines(frustumSideC.vertices, 2, false);
    renderer->ImDrawLines(frustumSideD.vertices, 2, false);
    renderer->ImDrawLines(frustum.vertices + 4, 4, true);
}
