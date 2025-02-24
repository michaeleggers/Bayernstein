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

    // Normals
    Line frustumNormalA // right
        = { Vertex{ frustum.vertices[ 4 ].pos },
            Vertex{ frustum.vertices[ 4 ].pos + 50.0f * frustum.planes[ 0 ].normal } };
    frustumNormalA.a.color = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
    frustumNormalA.b.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    renderer->ImDrawLines(frustumNormalA.vertices, 2, false);

    /*
     * NOTE: This code would draw the normal at its actual computed plane. Sometimes
     * usefule for debugging if the normal is correct but the plane's location is not.
     *
    Line frustumNormalB // top
        = { Vertex(frustum.planes[ 1 ].d * frustum.planes[ 1 ].normal),
            Vertex(frustum.planes[ 1 ].d * frustum.planes[ 1 ].normal + 50.0f * frustum.planes[ 1 ].normal) };
    frustumNormalB.a.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    frustumNormalB.b.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    renderer->ImDrawLines(frustumNormalB.vertices, 2, false);
    */

    Line frustumNormalB // top
        = { Vertex{ frustum.vertices[ 4 ].pos },
            Vertex{ frustum.vertices[ 4 ].pos + 50.0f * frustum.planes[ 1 ].normal } };
    frustumNormalB.a.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    frustumNormalB.b.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    renderer->ImDrawLines(frustumNormalB.vertices, 2, false);

    Line frustumNormalC // left
        = { Vertex{ frustum.vertices[ 7 ].pos },
            Vertex{ frustum.vertices[ 7 ].pos + 50.0f * frustum.planes[ 2 ].normal } };
    frustumNormalC.a.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
    frustumNormalC.b.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    renderer->ImDrawLines(frustumNormalC.vertices, 2, false);

    Line frustumNormalD // bottom
        = { Vertex{ frustum.vertices[ 6 ].pos },
            Vertex{ frustum.vertices[ 6 ].pos + 50.0f * frustum.planes[ 3 ].normal } };
    frustumNormalD.a.color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    frustumNormalD.b.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    renderer->ImDrawLines(frustumNormalD.vertices, 2, false);

    Line frustumNormalE // near
        = { Vertex{ frustum.vertices[ 0 ].pos },
            Vertex{ frustum.vertices[ 0 ].pos + 50.0f * frustum.planes[ 4 ].normal } };
    frustumNormalE.a.color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    frustumNormalE.b.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    renderer->ImDrawLines(frustumNormalE.vertices, 2, false);

    Line frustumNormalF // far
        = { Vertex{ frustum.vertices[ 4 ].pos },
            Vertex{ frustum.vertices[ 4 ].pos + 50.0f * frustum.planes[ 5 ].normal } };
    frustumNormalF.a.color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    frustumNormalF.b.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    renderer->ImDrawLines(frustumNormalF.vertices, 2, false);

    // Planes
    Line frustumSideA = { frustum.vertices[ 0 ], frustum.vertices[ 4 ] };
    Line frustumSideB = { frustum.vertices[ 1 ], frustum.vertices[ 5 ] };
    Line frustumSideC = { frustum.vertices[ 2 ], frustum.vertices[ 6 ] };
    Line frustumSideD = { frustum.vertices[ 3 ], frustum.vertices[ 7 ] };
    renderer->ImDrawLines(frustumSideA.vertices, 2, false);
    renderer->ImDrawLines(frustumSideB.vertices, 2, false);
    renderer->ImDrawLines(frustumSideC.vertices, 2, false);
    renderer->ImDrawLines(frustumSideD.vertices, 2, false);
    renderer->ImDrawLines(frustum.vertices, 4, true);
    renderer->ImDrawLines(frustum.vertices + 4, 4, true);
}
