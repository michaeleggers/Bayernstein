#include "r_common.h"

#include <assert.h>
#include <math.h>

#define GLM_FORCE_RADIANS
#include "dependencies/glm/ext.hpp"
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/gtx/quaternion.hpp"

#include "collision.h"
#include "hkd_interface.h"
#include "irender.h"
#include "r_itexture_mgr.h"
#include "r_model.h"

void RotateTri(Tri* tri, glm::vec3 axis, float angle)
{
    glm::quat q   = glm::angleAxis(glm::radians(angle), glm::normalize(axis));
    tri->a.pos    = glm::rotate(q, tri->a.pos);
    tri->b.pos    = glm::rotate(q, tri->b.pos);
    tri->c.pos    = glm::rotate(q, tri->c.pos);
    tri->a.normal = glm::rotate(q, tri->a.normal);
    tri->b.normal = glm::rotate(q, tri->b.normal);
    tri->c.normal = glm::rotate(q, tri->c.normal);
}

void TranslateTri(Tri* tri, glm::vec3 t)
{
    tri->a.pos += t;
    tri->b.pos += t;
    tri->c.pos += t;
}

void TransformTri(Tri* tri, glm::mat4 modelMatrix)
{
    tri->a.pos = modelMatrix * glm::vec4(tri->a.pos, 1.0f);
    tri->b.pos = modelMatrix * glm::vec4(tri->b.pos, 1.0f);
    tri->c.pos = modelMatrix * glm::vec4(tri->c.pos, 1.0f);
}

void SetTriColor(Tri* tri, glm::vec4 color)
{
    tri->a.color = color;
    tri->b.color = color;
    tri->c.color = color;
}

void SubdivTri(Tri* tri, Tri out_tris[])
{
    glm::vec3 A = tri->a.pos;
    glm::vec3 B = tri->b.pos;
    glm::vec3 C = tri->c.pos;

    // Vectors from vert to vert.

    glm::vec3 AB = B - A;
    glm::vec3 BC = C - B;
    glm::vec3 CA = A - C;

    // Midpoints between the points on half distance

    glm::vec3 mAB = A + 0.5f * AB;
    glm::vec3 mBC = B + 0.5f * BC;
    glm::vec3 mCA = C + 0.5f * CA;

    // Colors between points

    glm::vec4 cAB = 0.5f * tri->a.color + 0.5f * tri->b.color;
    glm::vec4 cBC = 0.5f * tri->b.color + 0.5f * tri->c.color;
    glm::vec4 cCA = 0.5f * tri->c.color + 0.5f * tri->a.color;

    // New tris

    Tri t1     = { .vertices = { { .pos = A }, { .pos = mAB }, { .pos = mCA } } };
    Tri t2     = { .vertices = { { .pos = mAB }, { .pos = B }, { .pos = mBC } } };
    Tri t3     = { .vertices = { { .pos = mBC }, { .pos = C }, { .pos = mCA } } };
    Tri t4     = { .vertices = { { .pos = mCA }, { .pos = mAB }, { .pos = mBC } } };
    t1.a.color = tri->a.color;
    t1.b.color = cAB;
    t1.c.color = cCA;
    t2.a.color = cAB;
    t2.b.color = tri->b.color;
    t2.c.color = cBC;
    t3.a.color = cBC;
    t3.b.color = tri->c.color;
    t3.c.color = cCA;
    t4.a.color = cCA;
    t4.b.color = cAB;
    t4.c.color = cBC;

    out_tris[ 0 ] = t1;
    out_tris[ 1 ] = t2;
    out_tris[ 2 ] = t3;
    out_tris[ 3 ] = t4;
}

void SubdivTri(Tri* tri, Tri out_tris[], uint32_t numIterations)
{
    // each iteration makes one tri to four tris -> 4 tris -> 16 tris and so on.
    // so each iteration will multiply the current tricount by 4.

    uint32_t maxTris = pow(4, numIterations);

    //SubdivTri(tri, out_tris);

    Tri* tmp         = (Tri*)malloc(maxTris * sizeof(Tri));
    out_tris[ 0 ]    = *tri;
    tmp[ 0 ]         = *tri;
    uint32_t numTris = 1;
    for ( int i = 0; i < numIterations; i++ )
    {
        for ( int j = 0; j < numTris; j++ )
        {
            SubdivTri(&out_tris[ j ], tmp + j * 4);
        }
        numTris <<= 2;
        memcpy(out_tris, tmp, numTris * sizeof(Tri));
    }
    free(tmp);
}

// out_verts are expect to hold 6 vertices
// out_indices are expected to hold 12 indices
void SubdivIndexedTri(
    Vertex* verts, uint32_t numVerts, uint16_t* indices, uint32_t numIndices, Vertex* out_verts, uint16_t* out_indices)
{
    if ( numVerts < 3 )
    {
        return;
    }

    if ( numIndices < 3 )
    {
        return;
    }

    glm::vec3 A = verts[ 0 ].pos;
    glm::vec3 B = verts[ 1 ].pos;
    glm::vec3 C = verts[ 2 ].pos;

    glm::vec3 mAB = A + 0.5f * (B - A);
    glm::vec3 mBC = B + 0.5f * (C - B);
    glm::vec3 mCA = C + 0.5f * (A - C);

    // find highest index

    uint16_t startIndex = 0;
    for ( int i = 0; i < numIndices; i++ )
    {
        if ( indices[ i ] > startIndex )
        {
            startIndex = indices[ i ];
        }
    }
    startIndex += 1;

    // Put new indices into the buffer

    out_indices[ 0 ] = indices[ 0 ];
    out_indices[ 1 ] = startIndex++;
    out_indices[ 2 ] = startIndex++;

    out_indices[ 3 ] = out_indices[ 1 ];
    out_indices[ 4 ] = indices[ 1 ];
    out_indices[ 5 ] = startIndex++;

    out_indices[ 6 ] = out_indices[ 5 ];
    out_indices[ 7 ] = indices[ 2 ];
    out_indices[ 8 ] = out_indices[ 2 ];

    out_indices[ 9 ]  = out_indices[ 8 ];
    out_indices[ 10 ] = out_indices[ 1 ];
    out_indices[ 11 ] = out_indices[ 5 ];

    // Now the vertices

    out_verts[ 0 ] = verts[ 0 ];
    out_verts[ 1 ] = verts[ 1 ];
    out_verts[ 2 ] = verts[ 2 ];
    out_verts[ 3 ] = { .pos = mAB };
    out_verts[ 4 ] = { .pos = mCA };
    out_verts[ 5 ] = { .pos = mBC };
}

void SubdivIndexedTri(Vertex*   verts,
                      uint32_t  numVerts,
                      uint16_t* indices,
                      uint32_t  numIndices,
                      Vertex*   out_verts,
                      uint16_t* out_indices,
                      uint32_t  numIterations)
{
    // TODO: implement
}

// Defined in CCW.
Quad CreateQuad(glm::vec3 pos, float width, float height, glm::vec4 color)
{
    Quad result     = {};
    Tri  upperRight = {};
    Tri  lowerLeft  = {};

    float halfWidth  = width / 2.0f;
    float halfHeight = height / 2.0f;

    upperRight.a.pos    = glm::vec3(1.0f * halfWidth, 0.0f, -1.0f * halfHeight) + pos;
    upperRight.a.color  = color;
    upperRight.a.bc     = glm::vec3(1.0f, 0.0f, 0.0f);
    upperRight.a.normal = glm::vec3(0.0f, -1.0f, 0.0f);
    upperRight.b.pos    = glm::vec3(1.0f * halfWidth, 0.0f, 1.0f * halfHeight) + pos;
    upperRight.b.color  = color;
    upperRight.b.bc     = glm::vec3(0.0f, 1.0f, 0.0f);
    upperRight.b.normal = glm::vec3(0.0f, -1.0f, 0.0f);
    upperRight.c.pos    = glm::vec3(-1.0f * halfWidth, 0.0f, 1.0f * halfHeight) + pos;
    upperRight.c.color  = color;
    upperRight.c.bc     = glm::vec3(0.0f, 0.0f, 1.0f);
    upperRight.c.normal = glm::vec3(0.0f, -1.0f, 0.0f);
    lowerLeft.a.pos     = glm::vec3(1.0f * halfWidth, 0.0f, -1.0f * halfHeight) + pos;
    lowerLeft.a.color   = color;
    lowerLeft.a.bc      = glm::vec3(1.0f, 0.0f, 0.0f);
    lowerLeft.a.normal  = glm::vec3(0.0f, -1.0f, 0.0f);
    lowerLeft.b.pos     = glm::vec3(-1.0f * halfWidth, 0.0f, 1.0f * halfHeight) + pos;
    lowerLeft.b.color   = color;
    lowerLeft.b.bc      = glm::vec3(0.0f, 1.0f, 0.0f);
    lowerLeft.b.normal  = glm::vec3(0.0f, -1.0f, 0.0f);
    lowerLeft.c.pos     = glm::vec3(-1.0f * halfWidth, 0.0f, -1.0f * halfHeight) + pos;
    lowerLeft.c.color   = color;
    lowerLeft.c.bc      = glm::vec3(0.0f, 0.0f, 1.0f);
    lowerLeft.c.normal  = glm::vec3(0.0f, -1.0f, 0.0f);

    result.a = upperRight;
    result.b = lowerLeft;

    return result;
}

void RotateQuad(Quad* quad, glm::vec3 axis, float angle)
{
    RotateTri(&quad->a, axis, angle);
    RotateTri(&quad->b, axis, angle);
}

void TranslateQuad(Quad* quad, glm::vec3 t)
{
    TranslateTri(&quad->a, t);
    TranslateTri(&quad->b, t);
}

FaceQuad QuadToFace(Quad* quad)
{
    return { quad->tl, quad->tr, quad->br, quad->bl };
}

FaceQuad CreateFaceQuadFromVerts(Vertex* vertices)
{
    FaceQuad fq{};
    memcpy(fq.vertices, vertices, 4 * sizeof(Vertex));

    return fq;
}

void SetQuadColor(Quad* quad, glm::vec4 color)
{
    SetTriColor(&quad->a, color);
    SetTriColor(&quad->b, color);
}

//Quad front;
//Quad right;
//Quad back;
//Quad left;
//Quad top;
//Quad bottom;
Box CreateBox(glm::vec3 scale, glm::vec4 color)
{
    Box result = {};

    float halfWidth  = scale.x / 2.0f;
    float halfDepth  = scale.y / 2.0f;
    float halfHeight = scale.z / 2.0f;

    glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);

    Quad front  = CreateQuad(glm::vec3(0.0f), scale.x, scale.z, color);
    Quad right  = CreateQuad(glm::vec3(0.0f), scale.y, scale.z, color);
    Quad back   = CreateQuad(glm::vec3(0.0f), scale.x, scale.z, color);
    Quad left   = CreateQuad(glm::vec3(0.0f), scale.y, scale.z, color);
    Quad top    = CreateQuad(glm::vec3(0.0f), scale.x, scale.y, color);
    Quad bottom = CreateQuad(glm::vec3(0.0f), scale.x, scale.y, color);

    RotateQuad(&right, up, 90.0f);
    RotateQuad(&back, up, 180.0f);
    RotateQuad(&left, up, -90.0f);
    RotateQuad(&top, glm::vec3(1.0f, 0.0f, 0.0f), -90.0f);
    RotateQuad(&bottom, glm::vec3(1.0f, 0.0f, 0.0f), 90.0f);

    TranslateQuad(&front, glm::vec3(0.0f, -1.0f * halfDepth, 0.0f));
    TranslateQuad(&right, glm::vec3(1.0f * halfWidth, 0.0f, 0.0f));
    TranslateQuad(&back, glm::vec3(0.0f, 1.0f * halfDepth, 0.0f));
    TranslateQuad(&left, glm::vec3(-1.0f * halfWidth, 0.0f, 0.0f));
    TranslateQuad(&top, glm::vec3(0.0f, 0.0f, 1.0f * halfHeight));
    TranslateQuad(&bottom, glm::vec3(0.0, 0.0, -1.0f * halfHeight));

    result.front  = front;
    result.right  = right;
    result.back   = back;
    result.left   = left;
    result.top    = top;
    result.bottom = bottom;

    return result;
}

Box CreateBoxFromAABB(glm::vec3 mins, glm::vec3 maxs)
{
    float     width  = abs(maxs.x - mins.x);
    float     depth  = abs(maxs.y - mins.y);
    float     height = abs(maxs.z - mins.z);
    glm::vec3 posFix = mins + glm::vec3(width / 2.0f, depth / 2.0f, height / 2.0f);
    Box       result = CreateBox(glm::vec3(width, depth, height), glm::vec4(1.0, 1.0f, 1.0f, 1.0f));
    TranslateBox(&result, posFix);

    return result;
}

Ellipsoid CreateEllipsoidFromAABB(glm::vec3 mins, glm::vec3 maxs)
{
    float     width  = abs(maxs.x - mins.x);
    float     height = abs(maxs.z - mins.z);
    Ellipsoid result{};
    result.radiusA = width / 2.0f;
    result.radiusB = height / 2.0f;

    float redIncrement = 1.0f / (float)ELLIPSOID_VERT_COUNT;
    float sliceAngle   = 2.0f * HKD_PI / (float)ELLIPSOID_VERT_COUNT;
    for ( int i = 0; i < ELLIPSOID_VERT_COUNT; i++ )
    {
        Vertex v             = {};
        v.pos.y              = 0.0f;
        v.pos.x              = result.radiusA * cosf(i * sliceAngle);
        v.pos.z              = result.radiusB * sinf(i * sliceAngle);
        v.color              = glm::vec4(redIncrement * i, 0.4f, 0.2f, 1.0f);
        result.vertices[ i ] = v;
    }

    return result;
}

MeshEllipsoid CreateUnitEllipsoid(uint32_t numSubdivs)
{
    NBox unitNbox = CreateNBox(glm::vec3(1.0f), numSubdivs);

    // Project box vertices onto unit sphere
    for ( int i = 0; i < unitNbox.tris.size(); i++ )
    {
        Tri* tri   = &unitNbox.tris[ i ];
        tri->a.pos = glm::normalize(tri->a.pos);
        tri->b.pos = glm::normalize(tri->b.pos);
        tri->c.pos = glm::normalize(tri->c.pos);
    }

    MeshEllipsoid result;
    result.radiusA = 1.0f;
    result.radiusB = 1.0f;
    result.tris    = unitNbox.tris;

    return result;
}

void TranslateBox(Box* box, glm::vec3 t)
{
    for ( int i = 0; i < 6; i++ )
    {
        Quad* q = &box->quads[ i ];
        TranslateQuad(q, t);
    }
}

void TransformBox(Box* box, glm::mat4 modelMatrix)
{
    for ( int i = 0; i < 6; i++ )
    {
        Quad* q = &box->quads[ i ];
        for ( int j = 0; j < 2; j++ )
        {
            TransformTri(&q->tris[ j ], modelMatrix);
        }
    }
}

void TransformEllipsoid(Ellipsoid* ellipsoid, glm::mat4 modelMatrix)
{
    for ( int i = 0; i < ELLIPSOID_VERT_COUNT; i++ )
    {
        Vertex* v = &ellipsoid->vertices[ i ];
        v->pos    = modelMatrix * glm::vec4(v->pos, 1.0f);
    }
}

NBox CreateNBox(glm::vec3 scale, uint32_t numSubidvs)
{
    NBox             result;
    Box              unitBox = CreateBox(scale, glm::vec4(1.0f));
    std::vector<Tri> currentTris;
    currentTris.resize(12);
    memcpy(currentTris.data(), unitBox.tris, 12 * sizeof(Tri));

    for ( int i = 0; i < numSubidvs; i++ )
    {
        std::vector<Tri> tmp;
        for ( int j = 0; j < currentTris.size(); j++ )
        {
            Tri out[ 4 ];
            SubdivTri(&currentTris[ j ], out);
            tmp.push_back(out[ 0 ]);
            tmp.push_back(out[ 1 ]);
            tmp.push_back(out[ 2 ]);
            tmp.push_back(out[ 3 ]);
        }
        currentTris = tmp;
    }

    result.tris = currentTris;

    return result;
}

Plane CreatePlaneFromTri(Tri tri)
{
    glm::vec3 AB = tri.b.pos - tri.a.pos;
    glm::vec3 AC = tri.c.pos - tri.a.pos;

    glm::vec3 normal = glm::normalize(glm::cross(AB, AC));
    glm::vec3 A      = tri.a.pos;

    // Project A onto the normal. That will yield the distance
    // of the plane from the origin..
    float d = glm::dot(A, normal);

    return { normal, d };
}

// TODO: Maybe pass a CImage instead of textureFilename and store ref to that image
// in case we need data like image size, etc. again.
Sprite CreateSprite(const std::string& textureFilename, const glm::vec2& topLeft, const glm::vec2& bottomRight)
{
    ITextureManager* textureManager = GetRenderer()->GetTextureManager();
    ITexture*        spriteTexture  = textureManager->CreateTexture(textureFilename);

    float textureWidth  = (float)spriteTexture->m_Width;
    float textureHeight = (float)spriteTexture->m_Height;
    float spriteWidth   = bottomRight.x - topLeft.x;
    float spriteHeight  = bottomRight.y - topLeft.y;

    // Convert texture window to uv coordinates [0, 1]
    float s1 = topLeft.x / textureWidth;
    float t1 = topLeft.y / textureHeight;
    float s2 = bottomRight.x / textureWidth;
    float t2 = bottomRight.y / textureHeight;

    Sprite sprite{};
    sprite.size          = glm::vec2(spriteWidth, spriteHeight);
    sprite.uvTopLeft     = glm::vec2(s1, t1);
    sprite.uvBottomRight = glm::vec2(s2, t2);
    sprite.hTexture      = spriteTexture->m_hGPU;

    return sprite;
}

Plane operator*(const glm::mat4& M, const Plane& plane)
{
    glm::vec3 n    = glm::normalize(plane.normal);
    float     w    = plane.d;
    glm::vec3 q    = w * n;
    glm::mat4 invM = glm::inverse(M);

    Plane     transformedPlane = Plane(invM[ 0 ][ 0 ] * n.x + invM[ 0 ][ 1 ] * n.y + invM[ 0 ][ 2 ] * n.z,
                                   invM[ 1 ][ 0 ] * n.x + invM[ 1 ][ 1 ] * n.y + invM[ 1 ][ 2 ] * n.z,
                                   invM[ 2 ][ 0 ] * n.x + invM[ 2 ][ 1 ] * n.y + invM[ 2 ][ 2 ] * n.z,
                                   invM[ 3 ][ 0 ] * n.x + invM[ 3 ][ 1 ] * n.y + invM[ 3 ][ 2 ] * n.z + w);
    glm::vec4 transformedQ     = M * glm::vec4(q, 1.0f);
    transformedPlane.d         = glm::dot(transformedPlane.normal, glm::vec3(transformedQ));
    //transformedPlane.normal = glm::normalize(transformedPlane.normal);

    //glm::vec3 translation(M[ 3 ][ 0 ], M[ 3 ][ 1 ], M[ 3 ][ 2 ]);
    //float     transformedD = w - glm::dot(translation, transformedPlane.normal);

    //transformedPlane.d = 0.0f;

    return Plane(transformedPlane);

    /*
    glm::vec3 n = glm::normalize(plane.normal); // Ensure the input normal is normalized
    float     w = plane.d;

    // Transform the normal using the transpose of the inverse of M
    glm::mat3 inverseTranspose  = glm::transpose(glm::inverse(glm::mat3(M)));
    glm::vec3 transformedNormal = glm::normalize(inverseTranspose * n);

    // Extract the translation from the matrix
    glm::vec3 translation(M[ 3 ][ 0 ], M[ 3 ][ 1 ], M[ 3 ][ 2 ]);

    // Compute the new distance
    float transformedD = w - glm::dot(translation, transformedNormal);
    return Plane(transformedNormal, transformedD);
*/
}
