#ifndef QUICKHULL_H
#define QUICKHULL_H

enum QHIteration
{
    findNextIter,
    findHorizon,
    doIter
};

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

float DistanceBetweenPoints(vertex& p1, vertex& p2)
{
    return glm::distance(p1.position, p2.position);
}

float Dot(glm::vec3 p1, glm::vec3 p2)
{
    return p1.x * p2.x + p1.y * p2.y + p1.z * p2.z;
}

float Dot(vertex& v1, vertex v2)
{
    return Dot(v1.position, v2.position);
}

float SquareDistancePointToSegment(vertex& a, vertex& b, vertex& c)
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
float DistancePointToFace(face& f, vertex& v, vertex* vertices)
{
    auto c = (vertices[f.vertices[0]].position + vertices[f.vertices[1]].position + vertices[f.vertices[2]].position) / 3.0f;
    
    auto distanceToOrigin = glm::distance(c, glm::vec3(0.0f));
    
    //return glm::dot(f.faceNormal, v.position) + distanceToOrigin;
    
    return glm::abs((glm::dot(f.faceNormal, v.position) - distanceToOrigin)); 
}

struct vertex_pair
{
    vertex first;
    vertex second;
};

void GenerateInitialSimplex(render_context& renderContext, vertex* vertices, int numberOfPoints, mesh& m)
{
    // First we find all 6 extreme points in the whole point set
    auto extremePoints = FindExtremePoints(vertices, numberOfPoints);
    
    vertex_pair mostDistantPair = {};
    auto dist = 0.0f;
    auto mostDist1 = -1;
    auto mostDist2 = -1;
    
    // Now look through the extreme points to find the two
    // points furthest away from each other
    for(int i = 0; i < 6; i++)
    {
        for(int j = 0; j < 6; j++)
        {
            auto newDist = glm::distance(vertices[extremePoints[i]].position, vertices[extremePoints[j]].position);
            if(dist < newDist)
            {
                dist = newDist;
                mostDistantPair.first = vertices[extremePoints[i]];
                mostDistantPair.second = vertices[extremePoints[j]];
                
                // Save indices for use later
                mostDist1 = extremePoints[i];
                mostDist2 = extremePoints[j];
            }
        }
    }
    
    auto extremePointCurrentFurthest = 0.0f;
    auto extremePointCurrentIndex = 0;
    
    // Find the poin that is furthest away from the segment 
    // spanned by the two extreme points
    for(int i = 0; i < numberOfPoints; i++)
    {
        auto distance = SquareDistancePointToSegment(mostDistantPair.first, mostDistantPair.second, vertices[i]);
        if(distance > extremePointCurrentFurthest)
        {
            extremePointCurrentIndex = i;
            extremePointCurrentFurthest = distance;
        }
    }
    
    auto f = AddFace(renderContext, m, mostDistantPair.first, mostDistantPair.second, vertices[extremePointCurrentIndex], vertices);
    if(!f)
    {
        return;
    }
    
    auto currentFurthest = 0.0f;
    auto currentIndex = 0;
    
    // Now find the points furthest away from the face
    for(int i = 0; i < numberOfPoints; i++)
    {
        if(i == mostDist1 || i == mostDist2 || i == extremePointCurrentIndex)
        {
            continue;
        }
        auto distance = DistancePointToFace(*f, vertices[i], vertices);
        if(distance > currentFurthest)
        {
            currentIndex = i;
            currentFurthest = distance;
        }
    }
    
    renderContext.debugContext.currentFaceIndex = f->indexInMesh;
    renderContext.debugContext.currentDistantPoint = currentIndex;
    if(IsPointOnPositiveSide(*f, vertices[currentIndex], vertices))
    {
        auto t = f->vertices[0];
        f->vertices[0] = f->vertices[1];
        f->vertices[1] = t;
        f->faceNormal = ComputeFaceNormal(*f, vertices);
        AddFace(renderContext, m, mostDistantPair.first, vertices[currentIndex], vertices[extremePointCurrentIndex], vertices);
        AddFace(renderContext, m, vertices[currentIndex], mostDistantPair.second, vertices[extremePointCurrentIndex], vertices);
        AddFace(renderContext, m, mostDistantPair.first, mostDistantPair.second, vertices[currentIndex], vertices);
    }
    else
    {
        AddFace(renderContext, m, vertices[currentIndex], mostDistantPair.first, vertices[extremePointCurrentIndex], vertices);
        AddFace(renderContext, m, mostDistantPair.second, vertices[currentIndex], vertices[extremePointCurrentIndex], vertices);
        AddFace(renderContext, m, mostDistantPair.second, mostDistantPair.first, vertices[currentIndex], vertices);
    }
}

void AddToOutsideSet(face& f, vertex& v)
{
    if(f.outsideSetCount + 1 >= f.outsideSetSize)
    {
        if(f.outsideSetSize == 0)
        {
            f.outsideSetSize = 2;
            f.outsideSet = (int*)malloc(sizeof(int) * f.outsideSetSize);
        }
        else if(f.outsideSet)
        {
            f.outsideSetSize *= 2;
            f.outsideSet = (int*)realloc(f.outsideSet, f.outsideSetSize * sizeof(int));
        }
    }
    f.outsideSet[f.outsideSetCount++] = v.vertexIndex;
}

void AssignToOutsideSets(vertex* vertices, int numVertices, face* faces, int numFaces)
{
    std::vector<vertex> unassigned(vertices, vertices + numVertices);
    
    for(int faceIndex = 0; faceIndex < numFaces; faceIndex++)
    {
        auto& f = faces[faceIndex];
        for(size_t vertexIndex = 0; vertexIndex < unassigned.size(); vertexIndex++)
        {
            auto& vert = vertices[unassigned[vertexIndex].vertexIndex];
            
            if(vert.numFaceHandles > 0)
            {
                continue;
            }
            
            if(IsPointOnPositiveSide(f, unassigned[vertexIndex], vertices))
            {
                vertex v = unassigned[vertexIndex];
                AddToOutsideSet(f, v);
                unassigned.erase(unassigned.begin() + vertexIndex);
            }
        }
    }
}

void FindConvexHorizon(vertex& viewPoint, std::vector<int>& faces, vertex* vertices, mesh& m, std::vector<edge>& list)
{
    std::vector<int> possibleVisibleFaces(faces);
    for(size_t faceIndex = 0; faceIndex < possibleVisibleFaces.size(); faceIndex++)
    {
        auto f = GetFaceById(possibleVisibleFaces[faceIndex], m);
        if(f)
        {
            for(int neighbourIndex = 0; neighbourIndex < f->neighbourCount; neighbourIndex++)
            {
                auto& neighbour = f->neighbours[neighbourIndex];
                auto& neighbourFace = m.faces[neighbour.faceHandle];
                if(!IsPointOnPositiveSide(neighbourFace, viewPoint, vertices))
                {
                    edge newEdge = {};
                    newEdge.origin = neighbour.originVertex;
                    newEdge.end = neighbour.endVertex;
                    list.push_back(newEdge);
                }
                else if(!neighbourFace.visited)
                {
                    possibleVisibleFaces.push_back(neighbour.faceHandle);
                }
                neighbourFace.visited = true;
            }
        }
    }
}

mesh& InitQuickHull(render_context& renderContext, vertex* vertices, int numberOfPoints, std::stack<int>& faceStack)
{
    auto& m = InitEmptyMesh(renderContext);
    m.position = glm::vec3(0.0f);
    m.scale = glm::vec3(globalScale);
    m.numFaces = 0;
    m.facesSize = 0;
    GenerateInitialSimplex(renderContext, vertices, numberOfPoints, m);
    AssignToOutsideSets(vertices, numberOfPoints, m.faces,  m.numFaces);
    
    for(int i = 0; i < m.numFaces; i++)
    {
        faceStack.push(m.faces[i].id);
    }
    return m;
}

face* QuickHullFindNextIteration(render_context& renderContext, mesh& m, vertex* vertices, std::stack<int>& faceStack)
{
    if(faceStack.size() <= 0)
        return nullptr;
    
    auto f = GetFaceById(faceStack.top(), m);
    
    if(f && f->outsideSetCount > 0)
    {
        // TODO: Optimize, save distance for each vertex and sort?
        float furthestDistance = 0.0f;
        int currentFurthest = 0;
        for(int vertexIndex = 0; vertexIndex < f->outsideSetCount; vertexIndex++)
        {
            auto dist = DistancePointToFace(*f, vertices[f->outsideSet[vertexIndex]], vertices);
            if(dist > furthestDistance && vertices[f->outsideSet[vertexIndex]].numFaceHandles == 0)
            {
                currentFurthest = vertexIndex;
                furthestDistance = dist;
            }
        }
        
        auto& p = vertices[f->outsideSet[currentFurthest]];
        
        renderContext.debugContext.currentFaceIndex = f->indexInMesh;
        renderContext.debugContext.currentDistantPoint = p.vertexIndex;
        
        auto res = &m.faces[f->indexInMesh];
        
        return res;
    }
    faceStack.pop();
    return nullptr;
}

void QuickHullHorizon(render_context& renderContext, mesh& m, vertex* vertices, face& f, std::vector<int>& v, int* prevIterationFaces)
{
    v.push_back(f.id);
    
    auto& p = vertices[renderContext.debugContext.currentDistantPoint];
    
    Assert(f.neighbourCount <= 20);
    
    //TODO: for all unvisited neighbours -> What is unvisited exactly in this case?
    for(int neighbourIndex = 0; neighbourIndex < f.neighbourCount; neighbourIndex++)
    {
        auto& neighbour = m.faces[f.neighbours[neighbourIndex].faceHandle];
        
        if(IsPointOnPositiveSide(neighbour, p, vertices))
        {
            v.push_back(m.faces[f.neighbours[neighbourIndex].faceHandle].id);
        }
    }
    
    printf("V size: %zd\n", v.size());
    *prevIterationFaces = m.numFaces;
    
    std::vector<edge> horizon;
    renderContext.debugContext.horizon.clear();
    FindConvexHorizon(p, v, vertices, m, renderContext.debugContext.horizon);
}

void QuickHullIteration(render_context& renderContext, mesh& m, vertex* vertices, std::stack<int>& faceStack, face& f, std::vector<int>& v, int prevIterationFaces)
{
    auto& p = vertices[renderContext.debugContext.currentDistantPoint];
    for(const auto& e : renderContext.debugContext.horizon)
    {
        auto newF = AddFace(renderContext, m, vertices[e.origin], vertices[e.end], p, vertices);
        if(newF)
        {
            auto dot = glm::dot(f.faceNormal, newF->faceNormal);
            printf("Dot: %f\n", dot);
            
            if(std::isnan(dot))
            {
                printf("New normal: (%f, %f, %f)\n", newF->faceNormal.x, newF->faceNormal.y, newF->faceNormal.z);
                printf("Original normal: (%f, %f, %f)\n", f.faceNormal.x, f.faceNormal.y, f.faceNormal.z);
            }
            if(dot < 0)
            {
                printf("Reversing normals\n");
                auto t = newF->vertices[0];
                newF->vertices[0] = newF->vertices[1];
                newF->vertices[1] = t;
                newF->faceNormal = ComputeFaceNormal(*newF, vertices);
            }
            
            faceStack.push(newF->id);
        }
    }
    
    // The way we understand this, is that unassigned now means any point that was
    // assigned in the first round, but is part of a face that is about to be
    // removed. Thus about to be unassigned.
    for(int newFaceIndex = prevIterationFaces; newFaceIndex < m.numFaces; newFaceIndex++)
    {
        for(int handle : v)
        {
            auto fInV = GetFaceById(handle, m);;
            if(fInV)
            {
                for(int osIndex = 0; osIndex < fInV->outsideSetCount; osIndex++)
                {
                    auto& q = vertices[fInV->outsideSet[osIndex]];
                    if(q.numFaceHandles == 0 && IsPointOnPositiveSide(m.faces[newFaceIndex], q, vertices))
                    {
                        AddToOutsideSet(m.faces[newFaceIndex], q);
                    }
                }
            }
        }
    }
    
    for(int id : v)
    {
        printf("Removing face\n");
        RemoveFace(m, id, vertices);
    }
    printf("Number of faces: %d\n", m.numFaces);
}

void QuickHull(render_context& renderContext, vertex* vertices, int numberOfPoints, mesh& m, std::stack<int>& faceStack)
{
    (void)numberOfPoints;
    (void)vertices;
    (void)renderContext;
    (void)m;
    //face* currentFace = nullptr;
    while(faceStack.size() > 0)
    {
        //QuickHullIteration(renderContext, m, vertices, faceStack);
    }
}


#endif
