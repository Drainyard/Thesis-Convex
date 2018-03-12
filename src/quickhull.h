#ifndef QUICKHULL_H
#define QUICKHULL_H

enum QHIteration
{
    initQH,
    findNextIter,
    findHorizon,
    doIter
};

float DistanceBetweenPoints(vertex& p1, vertex& p2)
{
    return glm::distance(p1.position, p2.position);
}

float SquareDistancePointToSegment(vertex& a, vertex& b, vertex& c)
{
    glm::vec3 ab = b.position - a.position;
    glm::vec3 ac = c.position - a.position;
    glm::vec3 bc = c.position - b.position;
    
    float e = glm::dot(ac, ab);
    
    if(e <= 0.0f) return glm::dot(ac, ac);
    float f = glm::dot(ab, ab);
    if( e >= f) return glm::dot(bc, bc);
    
    return glm::dot(ac, ac) - (e * e) / f;
}

// t=nn·v1−nn·p
// p0=p+t·nn
float DistancePointToFace(face& f, vertex& v)
{
    return glm::abs((glm::dot(f.faceNormal, v.position) - glm::dot(f.faceNormal, f.centerPoint))); 
}

struct vertex_pair
{
    vertex first;
    vertex second;
};

coord_t GenerateInitialSimplex(render_context& renderContext, vertex* vertices, int numVertices, mesh& m)
{
    // First we find all 6 extreme points in the whole point set
    auto extremePoints = FindExtremePoints(vertices, numVertices);
    
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
    for(int i = 0; i < numVertices; i++)
    {
        auto distance = SquareDistancePointToSegment(mostDistantPair.first, mostDistantPair.second, vertices[i]);
        if(distance > extremePointCurrentFurthest)
        {
            extremePointCurrentIndex = i;
            extremePointCurrentFurthest = distance;
        }
    }
    
    auto epsilon = 3 * (extremePoints[1] + extremePoints[3] + extremePoints[5]) * FLT_EPSILON;
    //auto epsilon = 0.0f;
    
    auto* f = AddFace(m, mostDist1, mostDist2, extremePointCurrentIndex, vertices, numVertices);
    if(!f)
    {
        return epsilon;
    }
    
    auto currentFurthest = 0.0f;
    auto currentIndex = 0;
    
    // Now find the points furthest away from the face
    for(int i = 0; i < numVertices; i++)
    {
        if(i == mostDist1 || i == mostDist2 || i == extremePointCurrentIndex)
        {
            continue;
        }
        
        auto distance = DistancePointToFace(*f, vertices[i]);
        if(distance > currentFurthest)
        {
            currentIndex = i;
            currentFurthest = distance;
        }
    }
    
    renderContext.debugContext.currentFaceIndex = f->indexInMesh;
    renderContext.debugContext.currentDistantPoint = currentIndex;
    if(IsPointOnPositiveSide(*f, vertices[currentIndex], epsilon))
    {
        auto t = f->vertices[0];
        f->vertices[0] = f->vertices[1];
        f->vertices[1] = t;
        f->faceNormal = ComputeFaceNormal(*f, vertices);
        AddFace(m, mostDist1, currentIndex, extremePointCurrentIndex, vertices, numVertices);
        AddFace(m, currentIndex, mostDist2, extremePointCurrentIndex, vertices, numVertices);
        AddFace(m, mostDist1, mostDist2, currentIndex, vertices, numVertices);
    }
    else
    {
        AddFace(m, currentIndex, mostDist1, extremePointCurrentIndex, vertices, numVertices);
        AddFace(m, mostDist2, currentIndex, extremePointCurrentIndex, vertices, numVertices);
        AddFace(m, mostDist2, mostDist1, currentIndex, vertices, numVertices);
    }
    
    return epsilon;
}

void AddToOutsideSet(face& f, vertex& v)
{
    f.outsideSet[f.outsideSetCount++] = v.vertexIndex;
    v.assigned = true;
}

void AssignToOutsideSets(vertex* vertices, int numVertices, face* faces, int numFaces, coord_t epsilon)
{
    std::vector<vertex> unassigned(vertices, vertices + numVertices);
    
    for(int faceIndex = 0; faceIndex < numFaces; faceIndex++)
    {
        auto& f = faces[faceIndex];
        coord_t currentDist = 0.0;
        int currentDistIndex = 0;
        
        for(size_t vertexIndex = 0; vertexIndex < unassigned.size(); vertexIndex++)
        {
            auto& vert = vertices[unassigned[vertexIndex].vertexIndex];
            
            if(vert.numFaceHandles > 0)
            {
                continue;
            }
            
            if(IsPointOnPositiveSide(f, unassigned[vertexIndex], epsilon))
            {
                vertex v = unassigned[vertexIndex];
                auto newDist = DistancePointToFace(f, v);
                if(newDist > currentDist)
                {
                    currentDist = newDist;
                    currentDistIndex = v.vertexIndex;
                    f.furthestPointIndex = currentDistIndex;
                }
                
                AddToOutsideSet(f, v);
                unassigned.erase(unassigned.begin() + vertexIndex);
                vertexIndex--;
            }
        }
    }
}

bool EdgeUnique(edge& input, std::vector<edge>& list)
{
    for(const auto& e : list)
    {
        if(e.origin == input.origin && e.end == input.end)
        {
            return false;
        }
    }
    return true;
}

bool HorizonValid(std::vector<edge>& horizon)
{
    int currentIndex = 0;
    auto currentEdge = horizon[currentIndex++];
    
    bool foundEdge = false;
    
    auto firstEdge = horizon[0];
    
    auto cpyH = std::vector<edge>(horizon);
    
    cpyH.erase(cpyH.begin());
    
    while(currentEdge.end != firstEdge.origin)
    {
        for(size_t i = 0; i < cpyH.size(); i++)
        {
            if(currentEdge.end == cpyH[i].origin)
            {
                foundEdge = true;
                Log("%d %d -> %d %d ->", currentEdge.origin, currentEdge.end, cpyH[i].origin, cpyH[i].end);
                currentIndex = (int)i;
                currentEdge = cpyH[currentIndex];
                break;
            }
        }
        
        if(!foundEdge)
        {
            Log("\n");
            return false;
        }
        
        cpyH.erase(cpyH.begin() + currentIndex);
        
        foundEdge = false;
    }
    Log("\n");
    return true;
}

void PrintHorizon(std::vector<edge>& list)
{
    for(size_t i = 0; i < list.size(); i++)
    {
        Log("Edge: origin %d end %d\n", list[i].origin, list[i].end);
    }
}

void FindConvexHorizon(vertex& viewPoint, std::vector<int>& faces, mesh& m, std::vector<edge>& list, coord_t epsilon)
{
    std::vector<int> possibleVisibleFaces(faces);
    for(size_t faceIndex = 0; faceIndex < possibleVisibleFaces.size(); faceIndex++)
    {
        //auto f = GetFaceById(possibleVisibleFaces[faceIndex], m);
        auto f = &m.faces[possibleVisibleFaces[faceIndex]];
        if(f)
        {
            for(int neighbourIndex = 0; neighbourIndex < f->neighbourCount; neighbourIndex++)
            {
                auto& neighbour = f->neighbours[neighbourIndex];
                auto neighbourFace = &m.faces[neighbour.faceHandle];
                //auto neighbourFace = GetFaceById(neighbour.id, m);
                
                if(neighbourFace)
                {
                    if(!IsPointOnPositiveSide(*neighbourFace, viewPoint, epsilon))
                    {
                        edge newEdge = {};
                        newEdge.origin = neighbour.originVertex;
                        newEdge.end = neighbour.endVertex;
                        if(EdgeUnique(newEdge, list))
                        {
                            list.push_back(newEdge);
                        }
                        
                    }
                    else if(!neighbourFace->visited)
                    {
                        //possibleVisibleFaces.push_back(neighbour.id);
                        //faces.push_back(neighbourFace->id);
                        possibleVisibleFaces.push_back(neighbour.faceHandle);
                        faces.push_back(neighbourFace->indexInMesh);
                    }
                    neighbourFace->visited = true;
                }
            }
        }
    }
    
    if(!HorizonValid(list))
    {
        Log("Invalid horizon\n");
    }
}



mesh& InitQuickHull(render_context& renderContext, vertex* vertices, int numVertices, std::vector<int>& faceStack, coord_t* epsilon)
{
    auto& m = InitEmptyMesh(renderContext);
    m.position = glm::vec3(0.0f);
    m.scale = glm::vec3(globalScale);
    m.numFaces = 0;
    m.facesSize = 0;
    *epsilon = GenerateInitialSimplex(renderContext, vertices, numVertices, m);
    AssignToOutsideSets(vertices, numVertices, m.faces,  m.numFaces, *epsilon);
    
    for(int i = 0; i < m.numFaces; i++)
    {
        if(m.faces[i].outsideSetCount > 0)
        {
            faceStack.push_back(m.faces[i].indexInMesh);
        }
    }
    return m;
}

face* QuickHullFindNextIteration(render_context& renderContext, mesh& m, std::vector<int>& faceStack)
{
    if(faceStack.size() <= 0)
        return nullptr;
    
    face* res = nullptr;
    
    //auto f = GetFaceById(faceStack.top(), m);
    auto f = &m.faces[faceStack[faceStack.size() - 1]];
    
    if(f && f->outsideSetCount > 0)
    {
        renderContext.debugContext.currentFaceIndex = f->indexInMesh;
        renderContext.debugContext.currentDistantPoint = f->furthestPointIndex;
        
        res = &m.faces[f->indexInMesh];
    }
    
    faceStack.pop_back();
    return res;
}

void QuickHullHorizon(render_context& renderContext, mesh& m, vertex* vertices, face& f, std::vector<int>& v, int* prevIterationFaces, coord_t epsilon)
{ 
    //v.push_back(f.id);
    v.push_back(f.indexInMesh);
    
    auto& p = vertices[renderContext.debugContext.currentDistantPoint];
    
    for(size_t i = 0; i < v.size(); i++)
    {
        //auto fa = GetFaceById(v[i], m);
        auto fa = &m.faces[v[i]];
        if(!fa || fa->visitedV)
            continue;
        
        fa->visitedV = true;
        
        for(int neighbourIndex = 0; neighbourIndex < fa->neighbourCount; neighbourIndex++)
        {
            auto& neighbour = m.faces[fa->neighbours[neighbourIndex].faceHandle];
            
            auto& newF = m.faces[fa->neighbours[neighbourIndex].faceHandle];
            if(IsPointOnPositiveSide(neighbour, p, epsilon))
            {
                if(!newF.visitedV)
                {
                    //v.push_back(newF.id);
                    v.push_back(newF.indexInMesh);
                }
            }
        }
    }
    
    Log("V size: %zd\n", v.size());
    *prevIterationFaces = m.numFaces;
    
    //std::vector<edge> horizon;
    renderContext.debugContext.horizon.clear();
    FindConvexHorizon(p, v, m, renderContext.debugContext.horizon, epsilon);
    PrintHorizon(renderContext.debugContext.horizon);
}

void QuickHullIteration(render_context& renderContext, mesh& m, vertex* vertices, std::vector<int>& faceStack, int fHandle, std::vector<int>& v, int prevIterationFaces, int numVertices, coord_t epsilon)
{
    //face* f = GetFaceById(fHandle, m);
    face* f = &m.faces[fHandle];
    
    if(!f)
        return;
    
    Log("Horizon: %zd\n", renderContext.debugContext.horizon.size());
    
    for(const auto& e : renderContext.debugContext.horizon)
    {
        auto* newF = AddFace(m, e.origin, e.end, renderContext.debugContext.currentDistantPoint, vertices, numVertices);
        if(newF)
        {
            if(IsPointOnPositiveSide(*newF, f->centerPoint, epsilon))
            {
                Log("Reversing normals\n");
                auto t = newF->vertices[0];
                newF->vertices[0] = newF->vertices[1];
                newF->vertices[1] = t;
                newF->faceNormal = ComputeFaceNormal(*newF, vertices);
            }
            
            //faceStack.push(newF->id);
            faceStack.push_back(newF->indexInMesh);
        }
    }
    
    for(const auto& handle : v)
    {
        //auto fInV = GetFaceById(handle, m);
        auto fInV = &m.faces[handle];
        if(fInV)
        {
            for(int osIndex = 0; osIndex < fInV->outsideSetCount; osIndex++)
            {
                auto osHandle = fInV->outsideSet[osIndex];
                auto& q = vertices[osHandle];
                q.assigned = false;
            }
        }
    }
    
    
    // The way we understand this, is that unassigned now means any point that was
    // assigned in the first round, but is part of a face that is about to be
    // removed. Thus about to be unassigned.
    for(int newFaceIndex = prevIterationFaces; newFaceIndex < m.numFaces; newFaceIndex++)
    {
        if(m.faces[newFaceIndex].outsideSetCount > 0)
            continue;
        
        coord_t currentDist = 0.0;
        int currentDistIndex = 0;
        
        for(const auto& handle : v)
        {
            //auto fInV = GetFaceById(handle, m);
            auto fInV = &m.faces[handle];
            if(fInV)
            {
                for(int osIndex = 0; osIndex < fInV->outsideSetCount; osIndex++)
                {
                    auto osHandle = fInV->outsideSet[osIndex];
                    auto& q = vertices[osHandle];
                    if(!q.assigned && q.numFaceHandles == 0 && IsPointOnPositiveSide(m.faces[newFaceIndex], q, epsilon))
                    {
                        auto newDist = DistancePointToFace(m.faces[newFaceIndex], q);
                        if(newDist > currentDist)
                        {
                            currentDist = newDist;
                            currentDistIndex = q.vertexIndex;
                            m.faces[newFaceIndex].furthestPointIndex = q.vertexIndex;
                        }
                        
                        Assert(m.faces[newFaceIndex].outsideSetCount <= numVertices);
                        AddToOutsideSet(m.faces[newFaceIndex], q);
                    }
                }
            }
        }
    }
    
    std::vector<int> uniqueInV;
    for(size_t i = 0; i < v.size(); i++)
    {
        bool alreadyAdded = false;
        for(size_t j = 0; j < uniqueInV.size(); j++)
        {
            if(uniqueInV[j] == v[i])
            {
                alreadyAdded = true;
                break;
            }
        }
        
        if(!alreadyAdded)
        {
            uniqueInV.push_back(v[i]);
        }
    }
    
    Log("Faces before: %d\n", m.numFaces);
    //for(int id : v)
    for(size_t vIndex = 0; vIndex < uniqueInV.size(); vIndex++)
    {
        Log("Removing face: %d\n", id);
        
        //auto oldFaceHandle = m.faces[v[vIndex]].indexInMesh;
        auto movedHandle = m.numFaces - 1;
        
        auto newHandle = RemoveFace(m, uniqueInV[vIndex], vertices);
        if(newHandle == -1)
            continue;
        
        for(size_t j = 0; j < uniqueInV.size(); j++)
        {
            if(uniqueInV[j] == movedHandle)
            {
                uniqueInV[j] = newHandle;
            }
        }
        
        auto iToRemove = -1;
        
        for(size_t i = 0; i < faceStack.size(); i++)
        {
            if(newHandle == faceStack[i])
            {
                iToRemove = (int)i;
            }
        }
        
        if(iToRemove != -1)
        {
            faceStack.erase(faceStack.begin() + iToRemove);
        }
        
        for(size_t i = 0; i < faceStack.size(); i++)
        {
            if(movedHandle == faceStack[i])
            {
                faceStack[i] = newHandle;
            }
        }
    }
    
    renderContext.debugContext.currentFaceIndex = -1;
    
    Log("Faces After: %d\n", m.numFaces);
    
    for(int i = 0; i < m.numFaces; i++)
    {
        m.faces[i].visited = false;
    }
    
    Log("Number of faces: %d\n", m.numFaces);
}

mesh& QuickHull(render_context& renderContext, vertex* vertices, int numVertices)
{
    face* currentFace = nullptr;
    std::vector<int> faceStack;
    coord_t epsilon;
    auto& m = InitQuickHull(renderContext, vertices, numVertices, faceStack, &epsilon);
    std::vector<int> v;
    int previousIteration = 0;
    while(faceStack.size() > 0)
    {
        currentFace = QuickHullFindNextIteration(renderContext, m, faceStack);
        if(currentFace)
        {
            QuickHullHorizon(renderContext, m, vertices, *currentFace, v, &previousIteration, epsilon);
            //QuickHullIteration(renderContext, m, vertices, faceStack, currentFace->id, v, previousIteration, numVertices, epsilon);
            QuickHullIteration(renderContext, m, vertices, faceStack, currentFace->indexInMesh, v, previousIteration, numVertices, epsilon);
            v.clear();
        }
    }
    return m;
}

struct qh_context
{
    vertex* vertices;
    int numberOfPoints;
    std::vector<int> faceStack;
    float epsilon;
    QHIteration iter;
    face* currentFace;
    std::vector<int> v;
    mesh m;
    int previousIteration;
};

qh_context InitializeQHContext(vertex* vertices, int numberOfPoints)
{
    qh_context qhContext = {};
    qhContext.vertices = CopyVertices(vertices, numberOfPoints);
    qhContext.numberOfPoints = numberOfPoints;
    qhContext.epsilon = 0.0f;
    qhContext.iter = QHIteration::initQH;
    qhContext.currentFace = 0;
    qhContext.previousIteration = 0;
    
    return qhContext;
}


void InitializeQHContext(qh_context& qhContext, vertex* vertices, int numberOfPoints)
{
    if(qhContext.vertices)
    {
        free(qhContext.vertices);
    }
    
    qhContext.faceStack.clear();
    qhContext.v.clear();
    
    qhContext.vertices = CopyVertices(vertices, numberOfPoints);
    qhContext.numberOfPoints = numberOfPoints;
    qhContext.epsilon = 0.0f;
    qhContext.iter = QHIteration::initQH;
    qhContext.currentFace = 0;
    qhContext.previousIteration = 0;
}

void QuickHullStep(render_context& renderContext, qh_context& context)
{
    auto before = glfwGetTime();
    switch(context.iter)
    {
        case QHIteration::initQH:
        {
            context.m = InitQuickHull(renderContext, context.vertices, context.numberOfPoints, context.faceStack, &context.epsilon);
            context.iter = QHIteration::findNextIter;
            auto after = glfwGetTime();
            auto total = after - before;
            Log_A("Init took: %fs\n", total);
        }
        break;
        case QHIteration::findNextIter:
        {
            if(context.faceStack.size() > 0)
            {
                Log("Finding next iteration\n");
                context.currentFace = QuickHullFindNextIteration(renderContext, context.m, context.faceStack);
                if(context.currentFace)
                {
                    context.iter = QHIteration::findHorizon;
                }
                auto after = glfwGetTime();
                auto total = after - before;
                Log_A("Finding iter took: %fs\n", total);
            }
        }
        break;
        case QHIteration::findHorizon:
        {
            if(context.currentFace)
            {
                Log("Finding horizon\n");
                QuickHullHorizon(renderContext, context.m, context.vertices, *context.currentFace, context.v, &context.previousIteration, context.epsilon);
                context.iter = QHIteration::doIter;
                auto after = glfwGetTime();
                auto total = after - before;
                Log_A("Horizon took: %fs\n", total);
            }
        }
        break;
        case QHIteration::doIter:
        {
            if(context.currentFace)
            {
                Log("Doing iteration\n");
                //QuickHullIteration(renderContext, context.m, context.vertices, context.faceStack, context.currentFace->id, context.v, context.previousIteration, context.numberOfPoints, context.epsilon);
                QuickHullIteration(renderContext, context.m, context.vertices, context.faceStack, context.currentFace->indexInMesh, context.v, context.previousIteration, context.numberOfPoints, context.epsilon);
                context.iter = QHIteration::findNextIter;
                context.v.clear();
                auto after = glfwGetTime();
                auto total = after - before;
                Log_A("Iteration took: %fs\n", total);
            }
        }
        break;
    }
}


#endif
