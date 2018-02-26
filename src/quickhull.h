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

float Distance(glm::vec3 p1, glm::vec3 p2)
{
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2) + pow(p1.z - p2.z, 2));
}

float DistanceBetweenPoints(vertex& p1, vertex& p2)
{
    return Distance(p1.position, p2.position);
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

bool IsPointOnPositiveSide(face& f, vertex& v, vertex* vertices)
{
    auto c = (vertices[f.vertices[0]].position + vertices[f.vertices[1]].position + vertices[f.vertices[2]].position) / 3.0f;
    auto distanceToOrigin = glm::distance(c, glm::vec3(0.0f));
    auto d = glm::dot(f.faceNormal, v.position - c);
    return d >= 0;
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
    
    auto& f = AddFace(renderContext, m, mostDistantPair.first, mostDistantPair.second, vertices[extremePointCurrentIndex], vertices);
    
    auto currentFurthest = 0.0f;
    auto currentIndex = 0;
    
    // Now find the points furthest away from the face
    for(int i = 0; i < numberOfPoints; i++)
    {
        if(i == mostDist1 || i == mostDist2 || i == extremePointCurrentIndex)
        {
            continue;
        }
        auto distance = DistancePointToFace(f, vertices[i], vertices);
        if(distance > currentFurthest)
        {
            currentIndex = i;
            currentFurthest = distance;
        }
    }
    
    renderContext.debugContext.currentFaceIndex = f.indexInMesh;
    renderContext.debugContext.currentDistantPoint = currentIndex;
    if(IsPointOnPositiveSide(f, vertices[currentIndex], vertices))
    {
        auto t = f.vertices[0];
        f.vertices[0] = f.vertices[1];
        f.vertices[1] = t;
        f.faceNormal = ComputeFaceNormal(renderContext, f, vertices);
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
    vertex* unassigned = (vertex*)malloc(sizeof(vertex) * numVertices);
    memcpy(unassigned, vertices, sizeof(vertex) * numVertices);
    int unassignedCount = numVertices;
    
    for(int faceIndex = 0; faceIndex < numFaces; faceIndex++)
    {
        auto& f = faces[faceIndex];
        for(int vertexIndex = 0; vertexIndex < unassignedCount - 1; vertexIndex++)
        {
            if(IsPointOnPositiveSide(f, unassigned[vertexIndex], vertices))
            {
                vertex v = unassigned[vertexIndex];
                AddToOutsideSet(f, v);
                // Swap last into current effectively "deleting"
                // We can do this since we throw away this list afterwards anyway
                unassigned[vertexIndex] = unassigned[unassignedCount - 1];
                unassignedCount--;
            }
        }
    }
    free(unassigned);
}

void FindConvexHorizon(vertex& viewPoint, std::vector<face>& faces, vertex* vertices, mesh& m, std::vector<edge>& list)
{
    for(int faceIndex = 0; faceIndex < faces.size(); faceIndex++)
    {
        auto& f = faces[faceIndex];
        for(int neighbourIndex = 0; neighbourIndex < f.neighbourCount; neighbourIndex++)
        {
            auto& neighbour = f.neighbours[neighbourIndex];
            if(!IsPointOnPositiveSide(m.faces[neighbour.faceHandle], viewPoint, vertices))
            {
                edge newEdge = {};
                newEdge.origin = neighbour.originVertex;
                newEdge.end = neighbour.endVertex;
                list.push_back(newEdge);
            }
        }
    }
}

struct stack
{
    face* begin;
    int size;
};

mesh& InitQuickHull(render_context& renderContext, vertex* vertices, int numberOfPoints, stack& faceStack)
{
    auto& m = InitEmptyMesh(renderContext);
    m.position = glm::vec3(0.0f);
    m.scale = glm::vec3(globalScale);
    m.numFaces = 0;
    m.facesSize = 0;
    GenerateInitialSimplex(renderContext, vertices, numberOfPoints, m);
    AssignToOutsideSets(vertices, numberOfPoints, m.faces,  m.numFaces);
    
    faceStack.begin = (face*)malloc(sizeof(face) * m.numFaces * 2);
    faceStack.size = m.numFaces;
    memcpy(faceStack.begin, m.faces, sizeof(face) * m.numFaces);
    return m;
}

face* QuickHullFindNextIteration(render_context& renderContext, mesh& m, vertex* vertices, stack& faceStack)
{
    if(faceStack.size <= 0)
        return nullptr;
    
    auto& f = faceStack.begin[--faceStack.size];
    
    if(f.outsideSetCount > 0)
    {
        // TODO: Optimize, save distance for each vertex and sort?
        float furthestDistance = 0.0f;
        int currentFurthest = 0;
        for(int vertexIndex = 0; vertexIndex < f.outsideSetCount; vertexIndex++)
        {
            auto dist = DistancePointToFace(f, vertices[f.outsideSet[vertexIndex]], vertices);
            if(dist > furthestDistance)
            {
                currentFurthest = vertexIndex;
                furthestDistance = dist;
            }
        }
        
        auto& p = vertices[f.outsideSet[currentFurthest]];
        
        renderContext.debugContext.currentFaceIndex = f.indexInMesh;
        renderContext.debugContext.currentDistantPoint = p.vertexIndex;
    }
    return &m.faces[f.indexInMesh];
}

void QuickHullHorizon(render_context& renderContext, mesh& m, vertex* vertices, face& f, std::vector<face>& v, stack& faceStack, int* prevIterationFaces)
{
    v.push_back(f);
    
    auto& p = vertices[renderContext.debugContext.currentDistantPoint];
    
    Assert(f.neighbourCount <= 20);
    
    //TODO: for all unvisited neighbours -> What is unvisited exactly in this case?
    for(int neighbourIndex = 0; neighbourIndex < f.neighbourCount; neighbourIndex++)
    {
        auto& neighbour = m.faces[f.neighbours[neighbourIndex].faceHandle];
        if(!neighbour.visited)
        {
            if(IsPointOnPositiveSide(neighbour, p, vertices))
            {
                v.push_back(neighbour);
            }
            neighbour.visited = true;
        }
    }
    
    *prevIterationFaces = m.numFaces;
    
    std::vector<edge> horizon;
    FindConvexHorizon(p, v, vertices, m, horizon);
    
    for(const auto& e : horizon)
    {
        auto newF = AddFace(renderContext, m, vertices[e.origin], vertices[e.end], p, vertices);
        //faceStack.begin[faceStack.size++] = newF;
    }
}

void QuickHullIteration(render_context& renderContext, mesh& m, vertex* vertices, stack& faceStack, face& f, std::vector<face>& v, int prevIterationFaces)
{
    
    // The way we understand this, is that unassigned now means any point that was
    // assigned in the first round, but is part of a face that is about to be
    // removed. Thus about to be unassigned.
    for(int newFaceIndex = prevIterationFaces; newFaceIndex < m.numFaces; newFaceIndex++)
    {
        for(auto& f : v)
        {
            for(int osIndex = 0; osIndex < f.outsideSetCount; osIndex++)
            {
                auto& q = vertices[f.outsideSet[osIndex]];
                if(IsPointOnPositiveSide(m.faces[newFaceIndex], q, vertices))
                {
                    AddToOutsideSet(m.faces[newFaceIndex], q);
                }
            }
        }
    }
    
    for(auto& f : v)
    {
        printf("Removing face\n");
        RemoveFace(m, &f, vertices);
    }
    printf("Number of faces: %d\n", m.numFaces);
}


void QuickHull(render_context& renderContext, vertex* vertices, int numberOfPoints, mesh& m, stack& faceStack)
{
    //face* currentFace = nullptr;
    while(faceStack.size > 0)
    {
        //QuickHullIteration(renderContext, m, vertices, faceStack);
    }
    
    
}


#endif
