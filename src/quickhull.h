#ifndef QUICKHULL_H
#define QUICKHULL_H

int* FindExtremePoints(vertex* points, int numPoints)
{
    if(numPoints > 0)
    {
        auto minX = 0;
        auto maxX = 0;
        auto minY = 0;
        auto maxY = 0;
        auto minZ = 0;
        auto maxZ = 0;
        
        for(int i = 0; i < numPoints; i++)
        {
            auto& p = points[i];
            if(p.position.x > points[maxX].position.x)
            {
                maxX = i;
            }
            if(p.position.x < points[minX].position.x)
            {
                minX = i;
            }
            if(p.position.y > points[maxY].position.y)
            {
                maxY = i;
            }
            if(p.position.y < points[minY].position.y)
            {
                minY = i;
            }
            if(p.position.z > points[maxZ].position.z)
            {
                maxZ = i;
            }
            if(p.position.z < points[minZ].position.z)
            {
                minZ = i;
            }
        }
        int* res = (int*)malloc(sizeof(int) * 6);
        res[0] = minX;
        res[1] = maxX;
        res[2] = minY;
        res[3] = maxY;
        res[4] = minZ;
        res[5] = maxZ;
        return res;
    }
    return nullptr;
}

float Distance(glm::vec3 p1, glm::vec3 p2)
{
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2) + pow(p1.z - p2.z, 2));
}

float DistanceBetweenPoints(vertex p1, vertex p2)
{
    return Distance(p1.position, p2.position);
}

float Dot(glm::vec3 p1, glm::vec3 p2)
{
    return p1.x * p2.x + p1.y * p2.y + p1.z * p2.z;
}

float Dot(vertex v1, vertex v2)
{
    return Dot(v1.position, v2.position);
}

float SquareDistancePointToSegment(vertex a, vertex b, vertex c)
{
    glm::vec3 ab = b.position - a.position;
    glm::vec3 ac = c.position - a.position;
    glm::vec3 bc = c.position - b.position;
    
    float e = Dot(ac, ab);
    
    if(e <= 0.0f) return Dot(ac, ac);
    float f = Dot(ab, ab);
    if( e >= f) return Dot(bc, bc);
    
    return Dot(ac, ac) - e * e / f;
}

// t=nn·v1−nn·p
// p0=p+t·nn
float DistancePointToFace(face f, vertex v)
{
    /*auto t = f.faceNormal * f.vertices[0].position - f.faceNormal * v.position;
    auto p0 = v.position + t * f.faceNormal;
    return Distance(p0, v.position);*/
    
    auto d1 = Distance(f.vertices[0].position, v.position);
    auto d2 = Distance(f.vertices[1].position, v.position);
    auto d3 = Distance(f.vertices[2].position, v.position);
    return d1 + d2 + d3;
}

void GenerateInitialSimplex(render_context& renderContext, vertex* points, int numberOfPoints, mesh& m)
{
    auto extremePoints = FindExtremePoints(points, numberOfPoints);
    
    auto extremePointCurrentFurthest = 0.0f;
    auto extremePointCurrentIndex = 0;
    
    for(int i = 2; i < 6; i++)
    {
        auto distance = SquareDistancePointToSegment(points[extremePoints[0]], points[extremePoints[1]], points[extremePoints[i]]);
        if(distance > extremePointCurrentFurthest)
        {
            extremePointCurrentIndex = i;
        }
    }
    
    auto f = AddFace(renderContext, m, points[extremePoints[0]], points[extremePoints[extremePointCurrentIndex]], points[extremePoints[1]]);
    
    auto currentFurthest = 0.0f;
    auto currentIndex = 0;
    
    for(int i = 0; i < numberOfPoints; i++)
    {
        if(i == extremePoints[0] || i == extremePoints[1] || i == extremePoints[extremePointCurrentIndex])
        {
            continue;
        }
        auto distance = DistancePointToFace(f, points[i]);
        if(distance > currentFurthest)
        {
            currentIndex = i;
        }
    }
    
    AddFace(renderContext, m, points[extremePoints[0]], points[extremePoints[extremePointCurrentIndex]], points[currentIndex]);
    AddFace(renderContext, m, points[extremePoints[extremePointCurrentIndex]], points[extremePoints[1]], points[currentIndex]);
    AddFace(renderContext, m, points[extremePoints[0]], points[currentIndex], points[extremePoints[extremePointCurrentIndex]]);
    
}

void QuickHull(render_context& renderContext, vertex* points, int numberOfPoints)
{
    auto& m = InitEmptyMesh(renderContext);
    m.position = glm::vec3(0.0f);;
    m.scale = glm::vec3(globalScale);
    m.numFaces = 0;
    m.facesSize = 0;
    GenerateInitialSimplex(renderContext, points, numberOfPoints, m);
}




#endif
