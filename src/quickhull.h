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
    
    auto f = AddFace(renderContext, m, points[extremePoints[0]], points[extremePoints[1]], points[extremePoints[extremePointCurrentIndex]]);
    
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
    
    AddFace(renderContext, m, points[currentIndex], points[extremePoints[0]], points[extremePoints[extremePointCurrentIndex]]);
    AddFace(renderContext, m, points[extremePoints[1]], points[currentIndex], points[extremePoints[extremePointCurrentIndex]]);
    AddFace(renderContext, m, points[extremePoints[1]], points[extremePoints[0]], points[currentIndex]);
    
}

bool IsAbove(vertex v, face f)
{
    auto A = (f.vertices[0].position + f.vertices[1].position + f.vertices[2].position)/3.0f;
    
    auto B = v.position;
    auto AB = B - A;
    auto dot = glm::dot(AB, f.faceNormal);
    
    return dot > 0;
}

void AddToOutsideSet(face& f, vertex& v)
{
    if(f.outsideSetCount + 1 >= f.outsideSetSize)
    {
        if(f.outsideSetSize == 0)
        {
            f.outsideSetSize = 2;
            f.outsideSet = (vertex*)malloc(sizeof(vertex) * f.outsideSetSize);
        }
        else
        {
            f.outsideSetSize *= 2;
            f.outsideSet = (vertex*)realloc(f.outsideSet, f.outsideSetSize * sizeof(vertex));
        }
    }
    f.outsideSet[f.outsideSetCount++] = v;
}

void AssignToOutsideSets(vertex* vertices, int numVertices, face* faces, int numFaces)
{
    vertex* unassigned = (vertex*)malloc(sizeof(vertex) * numVertices);
    memcpy(unassigned, vertices, sizeof(vertex) * numVertices);
    int unassignedCount = numVertices;
    
    
    for(int faceIndex = 0; faceIndex < numFaces; faceIndex++)
    {
        auto& f = faces[faceIndex];
        for(int vertexIndex = 0; vertexIndex < unassignedCount; vertexIndex++)
        {
            if(IsAbove(unassigned[vertexIndex], f))
            {
                vertex v = vertices[vertexIndex];
                AddToOutsideSet(f, v);
                memmove(&unassigned[vertexIndex], &unassigned[vertexIndex + 1], sizeof(vertex) * (unassignedCount - vertexIndex + 1));
                unassignedCount--;
            }
        }
    }
    free(unassigned);
    
}

void QuickHull(render_context& renderContext, vertex* points, int numberOfPoints)
{
    auto& m = InitEmptyMesh(renderContext);
    m.position = glm::vec3(0.0f);;
    m.scale = glm::vec3(globalScale);
    m.numFaces = 0;
    m.facesSize = 0;
    GenerateInitialSimplex(renderContext, points, numberOfPoints, m);
    AssignToOutsideSets(points, numberOfPoints, m.faces,  m.numFaces);
    
    // TODO: Optimize for less faces than going through all
    for(int i = 0; i < 5; i++)
    {
        auto& f = m.faces[i];
        f.visited = true;
        if(f.outsideSetCount > 0)
        {
            // TODO: Optimize, save distance for each vertex and sort?
            float furthestDistance = 0.0f;
            int currentFurthest = 0;
            for(int vertexIndex = 0; vertexIndex < f.outsideSetCount; vertexIndex++)
            {
                auto dist = DistancePointToFace(f, f.outsideSet[vertexIndex]);
                if(dist > furthestDistance)
                {
                    currentFurthest = vertexIndex;
                    furthestDistance = dist;
                }
            }
            face v[32];
            int inV = 0;
            
            auto& p = f.outsideSet[currentFurthest];
            
            //TODO: for all unvisited neighbours -> What is unvisited exactly in this case?
            for(int neighbourIndex = 0; neighbourIndex < 3; neighbourIndex++)
            {
                if(!f.neighbours[neighbourIndex].visited)
                {
                    if(IsAbove(p, f.neighbours[neighbourIndex]))
                    {
                        v[inV++] = f.neighbours[neighbourIndex];
                    }
                }
            }
            
            //TODO: What is the boundary of V?
            for(int vIndex = 0; vIndex < inV; vIndex)
            {
                // Definitely not every edge of faces in V, because that would never really do anything
                auto& f = v[vIndex];
                f.vertices[0];
                f.vertices[1];
                f.vertices[2];
                
                AddFace(renderContext, m, f.vertices[0], f.vertices[1], p);
                AddFace(renderContext, m, f.vertices[1], f.vertices[2], p);
                AddFace(renderContext, m, f.vertices[2], f.vertices[0], p);
            }
            
            for(int vIndex = 0; vIndex < inV; vIndex++)
            {
                RemoveFace(m, &v[vIndex]);
            }
        }
    }
}




#endif
