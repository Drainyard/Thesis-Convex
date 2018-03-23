#ifndef QUICKHULL_H
#define QUICKHULL_H

enum QHIteration
{
    initQH,
    findNextIter,
    findHorizon,
    doIter
};

struct qh_neighbour
{
    int faceHandle;
    int originVertex;
    int endVertex;
    int id;
};

struct qh_vertex
{
    int* faceHandles;
    int numFaceHandles;
    bool assigned;
    int vertexIndex;
    
    union
    {
        vertex_info info;
        struct
        {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec4 color;
        };
    };
};

struct qh_face
{
    int vertices[3];
    int id;
    std::vector<int> outsideSet;
    int furthestPointIndex;
    
    qh_neighbour neighbours[64];
    int neighbourCount;
    
    int indexInMesh;
    bool visited;
    bool visitedV;
    
    glm::vec3 faceNormal;
    glm::vec3 centerPoint;
    glm::vec4 faceColor;
};

struct qh_hull
{
    std::vector<qh_face> faces;
    
    int faceCounter;
    
    bool dirty;
};

struct qh_context
{
    bool initialized;
    
    qh_vertex* vertices;
    int numberOfPoints;
    std::vector<int> faceStack;
    float epsilon;
    QHIteration iter;
    qh_face* currentFace;
    std::vector<int> v;
    int previousIteration;
    qh_hull qHull;
};

static void qhCopyVertices(qh_context& q, vertex* vertices, int numberOfPoints)
{
    q.vertices = (qh_vertex*)malloc(sizeof(qh_vertex) * numberOfPoints);
    for(int i = 0; i < numberOfPoints; i++)
    {
        q.vertices[i].numFaceHandles = 0;
        q.vertices[i].faceHandles = (int*)malloc(sizeof(int) * 1024);
        q.vertices[i].vertexIndex = i;
        q.vertices[i].position = vertices[i].position;
        q.vertices[i].normal = vertices[i].normal;
        q.vertices[i].color = vertices[i].color;
    }
}

static void qhFindNeighbours(int v1Handle, int v2Handle, qh_hull& q, qh_face& f, qh_vertex* vertices)
{
    auto& v1 = vertices[v1Handle];
    auto& v2 = vertices[v2Handle];
    
    Log("Find neighbours\n");
    
    for(int v1FaceIndex = 0; v1FaceIndex < v1.numFaceHandles; v1FaceIndex++)
    {
        auto v1Face = v1.faceHandles[v1FaceIndex];
        
        if(v1Face == f.indexInMesh)
            continue;
        
        bool alreadyAdded = false;
        
        auto& fe = q.faces[v1Face];
        
        for(int n = 0; n < (int)fe.neighbourCount; n++)
        {
            auto& neigh = fe.neighbours[n];
            auto& neighbour = q.faces[neigh.faceHandle];
            if(neighbour.id == f.id)
            {
                alreadyAdded = true;
                break;
            }
        }
        
        if(alreadyAdded)
            continue;
        
        for(int v2FaceIndex = 0; v2FaceIndex < v2.numFaceHandles; v2FaceIndex++)
        {
            auto v2Face = v2.faceHandles[v2FaceIndex];
            
            if(v1Face == v2Face)
            {
                auto& foundNeighbour = q.faces[v1Face];
                
                // Set neighbour on current face
                auto index = f.neighbourCount++;
                
                f.neighbours[index].faceHandle = v1Face;
                f.neighbours[index].originVertex = v1.vertexIndex;
                f.neighbours[index].endVertex = v2.vertexIndex;
                f.neighbours[index].id = foundNeighbour.id;
                
                // Set neighbour on other face
                auto neighbourIndex = foundNeighbour.neighbourCount++;
                foundNeighbour.neighbours[neighbourIndex].faceHandle = f.indexInMesh;
                foundNeighbour.neighbours[neighbourIndex].endVertex = v1.vertexIndex;
                foundNeighbour.neighbours[neighbourIndex].originVertex = v2.vertexIndex;
                foundNeighbour.neighbours[neighbourIndex].id = f.id;
                
                foundNeighbour.visited = false;
                
                // Can we have 3+ neighbours while we're constructing?
                // Meaning: Can we temporarily have malformed triangles?
                //Assert(neighbour.neighbourCount <= 3);
                //Assert(f.neighbourCount <= 3);
            }
        }
    }
}

static glm::vec3 ComputeFaceNormal(qh_face f, qh_vertex* vertices)
{
    // Newell's Method
    // https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
    glm::vec3 normal = glm::vec3(0.0f);
    
    for(int i = 0; i < 3; i++)
    {
        auto& current = vertices[f.vertices[i]].position;
        auto& next = vertices[f.vertices[(i + 1) % 3]].position;
        
        normal.x = normal.x + (current.y - next.y) * (current.z + next.z);
        normal.y = normal.y + (current.z - next.z) * (current.x + next.x);
        normal.z = normal.z + (current.x - next.x) * (current.y + next.y);
    }
    
    return glm::normalize(normal);
}

static qh_face* qhAddFace(qh_hull& q, int v1Handle, int v2Handle, int v3Handle, qh_vertex* vertices)
{
    auto& v1 = vertices[v1Handle];
    auto& v2 = vertices[v2Handle];
    auto& v3 = vertices[v3Handle];
    
    if(v1.vertexIndex == v2.vertexIndex || v1.vertexIndex == v3.vertexIndex ||  v2.vertexIndex == v3.vertexIndex)
    {
        return nullptr;
    }
    
    qh_face newFace = {};
    
    newFace.neighbourCount = 0;
    newFace.indexInMesh = (int)q.faces.size();
    newFace.id = q.faceCounter++;
    
    qhFindNeighbours(v1Handle, v2Handle, q, newFace, vertices);
    qhFindNeighbours(v1Handle, v3Handle, q, newFace, vertices);
    qhFindNeighbours(v2Handle, v3Handle, q, newFace, vertices);
    
    v1.faceHandles[v1.numFaceHandles++] = newFace.indexInMesh;
    v2.faceHandles[v2.numFaceHandles++] = newFace.indexInMesh;
    v3.faceHandles[v3.numFaceHandles++] = newFace.indexInMesh;
    
    newFace.vertices[0] = v1.vertexIndex;
    newFace.vertices[1] = v2.vertexIndex;
    newFace.vertices[2] = v3.vertexIndex;
    newFace.faceNormal = ComputeFaceNormal(newFace, vertices);
    
    newFace.faceColor = glm::vec4(0.0f, 1.0f, 1.0f, 0.7f);
    newFace.faceColor.w = 0.5f;
    
    newFace.centerPoint = (vertices[newFace.vertices[0]].position + vertices[newFace.vertices[1]].position + vertices[newFace.vertices[2]].position) / 3.0f;
    
    q.dirty = true;
    q.faces.push_back(newFace);
    
    return &q.faces[q.faces.size() - 1];
}

// Returns index of moved mesh
static int qhRemoveFace(qh_hull& qHull, int faceId, qh_vertex* vertices)
{
    if(qHull.faces.size() == 0)
    {
        return -1;
    }
    
    auto& f = qHull.faces[faceId];
    
    auto& v1 = vertices[f.vertices[0]];
    auto& v2 = vertices[f.vertices[1]];
    auto& v3 = vertices[f.vertices[2]];
    
    bool contained = false;
    for(int i = 0; i < v1.numFaceHandles; i++)
    {
        contained |= v1.faceHandles[i] == f.indexInMesh;
    }
    Assert(contained);
    contained = false;
    for(int i = 0; i < v2.numFaceHandles; i++)
    {
        contained |= v2.faceHandles[i] == f.indexInMesh;
    }
    Assert(contained);
    contained = false;
    for(int i = 0; i < v3.numFaceHandles; i++)
    {
        contained |= v3.faceHandles[i] == f.indexInMesh;
    }
    Assert(contained);
    
    qHull.dirty = true;
    
    // Go through all of f's neighbours to remove itself
    for(int n = 0; n < (int)f.neighbourCount; n++)
    {
        // Get the neighbour of f and corresponding face
        auto& neighbour = f.neighbours[n];
        auto& neighbourFace = qHull.faces[neighbour.faceHandle];
        
        // go through the neighbours of the neighbour to find f
        for(int i = 0; i < (int)neighbourFace.neighbourCount; i++)
        {
            if(neighbourFace.neighbours[i].faceHandle == f.indexInMesh)
            {
                Log("Removing\n");
                // Found the neighbour
                neighbourFace.neighbours[i] = neighbourFace.neighbours[neighbourFace.neighbourCount - 1];
                neighbourFace.neighbourCount--;
                break;
            }
        }
    }
    
    auto indexInMesh = f.indexInMesh;
    
    // Invalidates the f pointer
    // But we only need to swap two faces to make this work
    qHull.faces[indexInMesh] = qHull.faces[qHull.faces.size() - 1];
    qHull.faces.erase(qHull.faces.begin() + qHull.faces.size() - 1);
    
    auto& newFace = qHull.faces[indexInMesh];
    
    for(int neighbourIndex = 0; neighbourIndex < (int)newFace.neighbourCount; neighbourIndex++)
    {
        auto& neighbour = newFace.neighbours[neighbourIndex];
        auto& neighbourFace = qHull.faces[neighbour.faceHandle];
        
        for(int n = 0; n < (int)neighbourFace.neighbourCount; n++)
        {
            if(neighbourFace.neighbours[n].faceHandle == newFace.indexInMesh)
            {
                // Set it to the new index in the mesh
                neighbourFace.neighbours[n].faceHandle = indexInMesh;
                break;
            }
        }
    }
    
    Log("before faces: %d\n", v1.numFaceHandles);
    Log("before faces: %d\n", v2.numFaceHandles);
    Log("before faces: %d\n", v3.numFaceHandles);
    
    for(int fIndex = 0; fIndex < v1.numFaceHandles; fIndex++)
    {
        if(v1.faceHandles[fIndex] == indexInMesh)
        {
            Log("Removing facehandle for v1\n");
            v1.faceHandles[fIndex] = v1.faceHandles[v1.numFaceHandles - 1];
            v1.numFaceHandles--;
            break;
        }
    }
    
    for(int fIndex = 0; fIndex < v2.numFaceHandles; fIndex++)
    {
        if(v2.faceHandles[fIndex] == indexInMesh)
        {
            Log("Removing facehandle for v2\n");
            v2.faceHandles[fIndex] = v2.faceHandles[v2.numFaceHandles - 1];
            v2.numFaceHandles--;
            break;
        }
    }
    
    for(int fIndex = 0; fIndex < v3.numFaceHandles; fIndex++)
    {
        if(v3.faceHandles[fIndex] == indexInMesh)
        {
            Log("Removing facehandle for v3\n");
            v3.faceHandles[fIndex] = v3.faceHandles[v3.numFaceHandles - 1];
            v3.numFaceHandles--;
            break;
        }
    }
    
    Log("faces: %d\n", v1.numFaceHandles);
    Log("faces: %d\n", v2.numFaceHandles);
    Log("faces: %d\n", v3.numFaceHandles);
    
    auto& v1new = vertices[newFace.vertices[0]];
    for(int fIndex = 0; fIndex < v1new.numFaceHandles; fIndex++)
    {
        if(v1new.faceHandles[fIndex] == newFace.indexInMesh)
        {
            v1new.faceHandles[fIndex] = indexInMesh;
            break;
        }
    }
    
    auto& v2new = vertices[newFace.vertices[1]];
    for(int fIndex = 0; fIndex < v2new.numFaceHandles; fIndex++)
    {
        if(v2new.faceHandles[fIndex] == newFace.indexInMesh)
        {
            v2new.faceHandles[fIndex] = indexInMesh;
            break;
        }
    }
    
    auto& v3new = vertices[newFace.vertices[2]];
    for(int fIndex = 0; fIndex < v3new.numFaceHandles; fIndex++)
    {
        if(v3new.faceHandles[fIndex] == newFace.indexInMesh)
        {
            v3new.faceHandles[fIndex] = indexInMesh;
            break;
        }
    }
    
    newFace.indexInMesh = indexInMesh;
    return indexInMesh;
}


float DistanceBetweenPoints(vertex& p1, vertex& p2)
{
    return glm::distance(p1.position, p2.position);
}

float SquareDistancePointToSegment(glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
    glm::vec3 ab = b - a;
    glm::vec3 ac = c - a;
    glm::vec3 bc = c - b;
    
    float e = glm::dot(ac, ab);
    
    if(e <= 0.0f) return glm::dot(ac, ac);
    float f = glm::dot(ab, ab);
    if( e >= f) return glm::dot(bc, bc);
    
    return glm::dot(ac, ac) - (e * e) / f;
}

// t=nn·v1−nn·p
// p0=p+t·nn
float DistancePointToFace(qh_face& f, qh_vertex& v)
{
    return glm::abs((glm::dot(f.faceNormal, v.position) - glm::dot(f.faceNormal, f.centerPoint))); 
}


static bool IsPointOnPositiveSide(qh_face& f, qh_vertex& v, coord_t epsilon = 0.0f)
{
    auto d = glm::dot(f.faceNormal, v.position - f.centerPoint);
    return d > epsilon;
}

static bool IsPointOnPositiveSide(qh_face& f, glm::vec3 v, coord_t epsilon = 0.0f)
{
    auto d = glm::dot(f.faceNormal, v - f.centerPoint);
    return d > epsilon;
}


struct vertex_pair
{
    qh_vertex first;
    qh_vertex second;
};


int* qhFindExtremePoints(qh_vertex* points, int numPoints)
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


coord_t qhGenerateInitialSimplex(render_context& renderContext, qh_vertex* vertices, int numVertices, qh_hull& q)
{
    // First we find all 6 extreme points in the whole point set
    auto extremePoints = qhFindExtremePoints(vertices, numVertices);
    
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
        auto distance = SquareDistancePointToSegment(mostDistantPair.first.position, mostDistantPair.second.position, vertices[i].position);
        if(distance > extremePointCurrentFurthest)
        {
            extremePointCurrentIndex = i;
            extremePointCurrentFurthest = distance;
        }
    }
    
    //auto epsilon = 3 * (extremePoints[1] + extremePoints[3] + extremePoints[5]) * FLT_EPSILON;
    auto epsilon = 0.0f;
    
    auto* f = qhAddFace(q, mostDist1, mostDist2, extremePointCurrentIndex, vertices);
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
        qhAddFace(q, mostDist1, currentIndex, extremePointCurrentIndex, vertices);
        qhAddFace(q, currentIndex, mostDist2, extremePointCurrentIndex, vertices);
        qhAddFace(q, mostDist1, mostDist2, currentIndex, vertices);
    }
    else
    {
        qhAddFace(q, currentIndex, mostDist1, extremePointCurrentIndex, vertices);
        qhAddFace(q, mostDist2, currentIndex, extremePointCurrentIndex, vertices);
        qhAddFace(q, mostDist2, mostDist1, currentIndex, vertices);
    }
    
    return epsilon;
}

void qhAddToOutsideSet(qh_face& f, qh_vertex& v)
{
    f.outsideSet.push_back(v.vertexIndex);
    v.assigned = true;
}

void qhAssignToOutsideSets(qh_vertex* vertices, int numVertices, qh_face* faces, int numFaces, coord_t epsilon)
{
    std::vector<qh_vertex> unassigned(vertices, vertices + numVertices);
    
    for(size_t vertexIndex = 0; vertexIndex < unassigned.size(); vertexIndex++)
    {
        auto& vert = vertices[unassigned[vertexIndex].vertexIndex];
        
        if(vert.numFaceHandles > 0)
        {
            continue;
        }
        
        for(int faceIndex = 0; faceIndex < numFaces; faceIndex++)
        {
            auto& f = faces[faceIndex];
            coord_t currentDist = 0.0;
            int currentDistIndex = 0;
            
            if(IsPointOnPositiveSide(f, unassigned[vertexIndex], epsilon))
            {
                auto v = unassigned[vertexIndex];
                auto newDist = DistancePointToFace(f, v);
                if(newDist > currentDistIndex)
                {
                    currentDist = newDist;
                    currentDistIndex = v.vertexIndex;
                    f.furthestPointIndex = currentDistIndex;
                }
                
                qhAddToOutsideSet(f, v);
                break;
            }
        }
    }
    
    
}

bool qhEdgeUnique(edge& input, std::vector<edge>& list)
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

bool qhHorizonValid(std::vector<edge>& horizon)
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

void qhPrintHorizon(std::vector<edge>& list)
{
    for(size_t i = 0; i < list.size(); i++)
    {
        Log("Edge: origin %d end %d\n", list[i].origin, list[i].end);
    }
}

void qhFindConvexHorizon(qh_vertex& viewPoint, std::vector<int>& faces, qh_hull& qHull, std::vector<edge>& list, coord_t epsilon)
{
    std::vector<int> possibleVisibleFaces(faces);
    for(size_t faceIndex = 0; faceIndex < possibleVisibleFaces.size(); faceIndex++)
    {
        auto& f = qHull.faces[possibleVisibleFaces[faceIndex]];
        
        for(int neighbourIndex = 0; neighbourIndex < (int)f.neighbourCount; neighbourIndex++)
        {
            auto& neighbour = f.neighbours[neighbourIndex];
            auto& neighbourFace = qHull.faces[neighbour.faceHandle];
            
            if(!IsPointOnPositiveSide(neighbourFace, viewPoint, epsilon))
            {
                edge newEdge = {};
                newEdge.origin = neighbour.originVertex;
                newEdge.end = neighbour.endVertex;
                if(qhEdgeUnique(newEdge, list))
                {
                    list.push_back(newEdge);
                }
                
            }
            else if(!neighbourFace.visited)
            {
                possibleVisibleFaces.push_back(neighbour.faceHandle);
                faces.push_back(neighbourFace.indexInMesh);
            }
            neighbourFace.visited = true;
        }
    }
    
    /*if(!HorizonValid(list))
    {
        Log("Invalid horizon\n");
    }*/
}

mesh& qhConvertToMesh(render_context& renderContext, qh_hull& qHull)
{
    auto& m = InitEmptyMesh(renderContext);
    m.position = glm::vec3(0.0f);
    m.scale = glm::vec3(globalScale);
    m.dirty = true;
    
    for(const auto& f : qHull.faces)
    {
        face newFace = {};
        newFace.vertices[0] = f.vertices[0];
        newFace.vertices[1] = f.vertices[1];
        newFace.vertices[2] = f.vertices[2];
        newFace.faceNormal = f.faceNormal;
        newFace.faceColor = f.faceColor;
        newFace.centerPoint = f.centerPoint;
        m.faces.push_back(newFace);
    }
    
    return m;
}

qh_hull qhInit(render_context& renderContext, qh_vertex* vertices, int numVertices, std::vector<int>& faceStack, coord_t* epsilon)
{
    qh_hull qHull = {};
    qHull.faceCounter = 0;
    qHull.dirty = true;
    
    *epsilon = qhGenerateInitialSimplex(renderContext, vertices, numVertices, qHull);
    qhAssignToOutsideSets(vertices, numVertices, qHull.faces.data(),  (int)qHull.faces.size(), *epsilon);
    
    for(int i = 0; i < (int)qHull.faces.size(); i++)
    {
        if(qHull.faces[i].outsideSet.size() > 0)
        {
            faceStack.push_back(qHull.faces[i].indexInMesh);
        }
    }
    return qHull;
}

qh_face* qhFindNextIteration(render_context& renderContext, qh_hull& qHull, std::vector<int>& faceStack)
{
    if(faceStack.size() == 0)
        return nullptr;
    
    qh_face* res = nullptr;
    
    auto f = &qHull.faces[faceStack[faceStack.size() - 1]];
    
    if(f && f->outsideSet.size() > 0)
    {
        renderContext.debugContext.currentFaceIndex = f->indexInMesh;
        renderContext.debugContext.currentDistantPoint = f->furthestPointIndex;
        
        res = f;
    }
    else
    {
        res = nullptr;
    }
    
    faceStack.pop_back();
    return res;
}

void qhHorizonStep(render_context& renderContext, qh_hull& qHull, qh_vertex* vertices, qh_face& f, std::vector<int>& v, int* prevIterationFaces, coord_t epsilon)
{ 
    v.push_back(f.indexInMesh);
    
    auto& p = vertices[f.furthestPointIndex];
    
    for(size_t i = 0; i < v.size(); i++)
    {
        auto& fa = qHull.faces[v[i]];
        if(fa.visitedV)
            continue;
        
        fa.visitedV = true;
        
        for(int neighbourIndex = 0; neighbourIndex < (int)fa.neighbourCount; neighbourIndex++)
        {
            auto& neighbour = qHull.faces[fa.neighbours[neighbourIndex].faceHandle];
            
            auto& newF = qHull.faces[fa.neighbours[neighbourIndex].faceHandle];
            if(!newF.visitedV && IsPointOnPositiveSide(neighbour, p, epsilon))
            {
                v.push_back(newF.indexInMesh);
            }
        }
    }
    
    Log("V size: %zd\n", v.size());
    //*prevIterationFaces = m.numFaces;
    *prevIterationFaces = (int)qHull.faces.size();
    
    //std::vector<edge> horizon;
    renderContext.debugContext.horizon.clear();
    qhFindConvexHorizon(p, v, qHull, renderContext.debugContext.horizon, epsilon);
    qhPrintHorizon(renderContext.debugContext.horizon);
}

void qhIteration(render_context& renderContext, qh_hull& qHull, qh_vertex* vertices, std::vector<int>& faceStack, int fHandle, std::vector<int>& v, int prevIterationFaces, int numVertices, coord_t epsilon)
{
    Log("Horizon: %zd\n", renderContext.debugContext.horizon.size());
    
    for(const auto& e : renderContext.debugContext.horizon)
    {
        auto* newF = qhAddFace(qHull, e.origin, e.end, renderContext.debugContext.currentDistantPoint, vertices);
        
        auto& f = qHull.faces[fHandle];
        
        if(newF)
        {
            if(IsPointOnPositiveSide(*newF, f.centerPoint, epsilon))
            {
                Log("Reversing normals\n");
                auto t = newF->vertices[0];
                newF->vertices[0] = newF->vertices[1];
                newF->vertices[1] = t;
                newF->faceNormal = ComputeFaceNormal(*newF, vertices);
            }
            
            faceStack.push_back(newF->indexInMesh);
        }
    }
    
    for(const auto& handle : v)
    {
        auto& fInV = qHull.faces[handle];
        
        for(int osIndex = 0; osIndex < (int)fInV.outsideSet.size(); osIndex++)
        {
            auto osHandle = fInV.outsideSet[osIndex];
            auto& q = vertices[osHandle];
            q.assigned = false;
        }
    }
    
    // The way we understand this, is that unassigned now means any point that was
    // assigned in the first round, but is part of a face that is about to be
    // removed. Thus about to be unassigned.
    for(int newFaceIndex = prevIterationFaces; newFaceIndex < (int)qHull.faces.size(); newFaceIndex++)
    {
        if(qHull.faces[newFaceIndex].outsideSet.size() > 0)
            continue;
        
        coord_t currentDist = 0.0;
        int currentDistIndex = 0;
        auto& newFace = qHull.faces[newFaceIndex];
        
        for(const auto& handle : v)
        {
            auto& fInV = qHull.faces[handle];
            for(int osIndex = 0; osIndex < (int)fInV.outsideSet.size(); osIndex++)
            {
                auto osHandle = fInV.outsideSet[osIndex];
                auto& q = vertices[osHandle];
                if(!q.assigned && q.numFaceHandles == 0 && IsPointOnPositiveSide(newFace, q, epsilon))
                {
                    auto newDist = DistancePointToFace(newFace, q);
                    if(newDist > currentDist)
                    {
                        currentDist = newDist;
                        currentDistIndex = q.vertexIndex;
                        newFace.furthestPointIndex = q.vertexIndex;
                    }
                    
                    Assert((int)newFace.outsideSet.size() <= numVertices);
                    qhAddToOutsideSet(newFace, q);
                }
            }
        }
    }
    
    std::vector<int> uniqueInV;
    uniqueInV.reserve(v.size());
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
    
    //Log("Faces before: %d\n", m.numFaces);
    Log("Faces before: %zu\n", qHull.faces.size());
    
    for(size_t vIndex = 0; vIndex < uniqueInV.size(); vIndex++)
    {
        Log("Removing face: %d\n", uniqueInV[vIndex]);
        
        auto movedHandle = qHull.faces.size() - 1;//m.numFaces - 1;
        
        auto newHandle = qhRemoveFace(qHull, uniqueInV[vIndex], vertices);
        if(newHandle == -1)
            continue;
        
        for(size_t j = 0; j < uniqueInV.size(); j++)
        {
            if(uniqueInV[j] == (int)movedHandle)
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
            if((int)movedHandle == faceStack[i])
            {
                faceStack[i] = newHandle;
            }
        }
    }
    renderContext.debugContext.currentFaceIndex = -1;
    
    Log("Faces After: %zu\n", qHull.faces.size());
    
    for(int i = 0; i < (int)qHull.faces.size(); i++)
    {
        qHull.faces[i].visited = false;
    }
}

void qhFullHull(render_context& renderContext, qh_context& qhContext)
{
    qhContext.currentFace = nullptr;
    qhContext.faceStack.clear();
    qhContext.epsilon = 0.0;
    qhContext.qHull = qhInit(renderContext, qhContext.vertices, qhContext.numberOfPoints, qhContext.faceStack, &qhContext.epsilon);
    qhContext.v.clear();
    qhContext.previousIteration = 0;
    while(qhContext.faceStack.size() > 0)
    {
        qhContext.currentFace = qhFindNextIteration(renderContext, qhContext.qHull, qhContext.faceStack);
        if(qhContext.currentFace)
        {
            qhHorizonStep(renderContext, qhContext.qHull, qhContext.vertices, *qhContext.currentFace, qhContext.v, &qhContext.previousIteration, qhContext.epsilon);
            qhIteration(renderContext, qhContext.qHull, qhContext.vertices, qhContext.faceStack, qhContext.currentFace->indexInMesh, qhContext.v, 
                        qhContext.previousIteration, qhContext.numberOfPoints, qhContext.epsilon);
            qhContext.v.clear();
        }
    }
}

qh_hull qhFullHull(render_context& renderContext, qh_vertex* vertices, int numVertices)
{
    qh_face* currentFace = nullptr;
    std::vector<int> faceStack;
    coord_t epsilon;
    
    auto qHull = qhInit(renderContext, vertices, numVertices, faceStack, &epsilon);
    std::vector<int> v;
    
    int previousIteration = 0;
    while(faceStack.size() > 0)
    {
        currentFace = qhFindNextIteration(renderContext, qHull, faceStack);
        if(currentFace)
        {
            qhHorizonStep(renderContext, qHull, vertices, *currentFace, v, &previousIteration, 
                          epsilon);
            qhIteration(renderContext, qHull, vertices, faceStack, currentFace->indexInMesh, v, 
                        previousIteration, numVertices, epsilon);
            v.clear();
        }
    }
    
    //Log_A("Number of triangles: %d\n", m.numFaces);
    return qHull;
}

void qhInitializeContext(qh_context& qhContext, vertex* vertices, int numberOfPoints)
{
    if(qhContext.vertices)
    {
        free(qhContext.vertices);
    }
    
    qhContext.faceStack.clear();
    qhContext.v.clear();
    
    qhCopyVertices(qhContext, vertices, numberOfPoints);
    qhContext.numberOfPoints = numberOfPoints;
    qhContext.epsilon = 0.0f;
    qhContext.iter = QHIteration::initQH;
    qhContext.currentFace = nullptr;
    qhContext.previousIteration = 0;
    qhContext.initialized = true;
}

void qhStep(render_context& renderContext, qh_context& context)
{
    switch(context.iter)
    {
        case QHIteration::initQH:
        {
            context.qHull = qhInit(renderContext, context.vertices, context.numberOfPoints, context.faceStack, &context.epsilon);
            context.iter = QHIteration::findNextIter;
        }
        break;
        case QHIteration::findNextIter:
        {
            if(context.faceStack.size() > 0)
            {
                Log("Finding next iteration\n");
                context.currentFace = qhFindNextIteration(renderContext, context.qHull, context.faceStack);
                if(context.currentFace)
                {
                    context.iter = QHIteration::findHorizon;
                }
            }
        }
        break;
        case QHIteration::findHorizon:
        {
            if(context.currentFace)
            {
                Log("Finding horizon\n");
                qhHorizonStep(renderContext, context.qHull, context.vertices, *context.currentFace, context.v, &context.previousIteration, context.epsilon);
                context.iter = QHIteration::doIter;
            }
        }
        break;
        case QHIteration::doIter:
        {
            if(context.currentFace)
            {
                Log("Doing iteration\n");
                qhIteration(renderContext, context.qHull, context.vertices, context.faceStack, context.currentFace->indexInMesh, context.v, context.previousIteration, context.numberOfPoints, context.epsilon);
                context.iter = QHIteration::findNextIter;
                context.v.clear();
            }
        }
        break;
    }
}


#endif
