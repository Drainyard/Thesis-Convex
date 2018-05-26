#ifndef QUICKHULL_H
#define QUICKHULL_H

enum QHIteration
{
    initQH,
    findNextIter,
    findHorizon,
    doIter
};

struct QhNeighbour
{
    int faceHandle;
    int originVertex;
    int endVertex;
};

struct QhVertex
{
    List<int> faceHandles;
    
    bool assigned;
    int vertexIndex;
    
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec4 color;
};

#define MAX_NEIGHBOURS 16

struct QhFace
{
    List<int> vertices;
    List<int> outsideSet;
    
    int furthestPointIndex;
    
    QhNeighbour neighbours[MAX_NEIGHBOURS];
    size_t neighbourCount;
    
    int indexInHull;
    bool visited;
    bool visitedV;
    
    glm::vec3 faceNormal;
    glm::vec3 centerPoint;
    glm::vec4 faceColor;
};

struct QhHull
{
    List<QhFace> faces;
    Mesh* m;
    
    struct
    {
        int addedFaces;
        int pointsProcessed;
        unsigned long long distanceQueryCount;
        unsigned long long sidednessQueries;
        int verticesInHull;
        unsigned long long timeSpent;
    } processingState;
    
    bool failed;
};

struct QhContext
{
    bool initialized;
    
    QhVertex* vertices;
    int numberOfPoints;
    std::vector<int> faceStack;
    float epsilon;
    QHIteration iter;
    QhFace* currentFace;
    std::vector<int> v;
    size_t previousIteration;
    QhHull qHull;
    
    std::vector<Edge> horizon;
};

static void qhCopyVertices(QhContext& q, Vertex* vertices, int numberOfPoints)
{
    q.vertices = (QhVertex*)malloc(sizeof(QhVertex) * numberOfPoints);
    for(int i = 0; i < numberOfPoints; i++)
    {
        init(q.vertices[i].faceHandles);
        q.vertices[i].vertexIndex = i;
        q.vertices[i].position = vertices[i].position;
        q.vertices[i].normal = vertices[i].normal;
        q.vertices[i].color = vertices[i].color;
    }
}

// t=nn·v1−nn·p
// p0=p+t·nn
float DistancePointToFace(QhHull &q, QhFace f, QhVertex v)
{
    q.processingState.distanceQueryCount++;
    return glm::abs((glm::dot(f.faceNormal, v.position) - glm::dot(f.faceNormal, f.centerPoint))); 
}

static bool IsPointOnPositiveSide(QhHull &q, QhFace f, QhVertex v, coord_t epsilon = 0.0f)
{
    q.processingState.sidednessQueries++;
    auto d = glm::dot(f.faceNormal, v.position - f.centerPoint);
    return d > epsilon;
}

static bool IsPointOnPositiveSide(QhHull &q, QhFace f, glm::vec3 v, coord_t epsilon = 0.0f)
{
    q.processingState.sidednessQueries++;
    auto d = glm::dot(f.faceNormal, v - f.centerPoint);
    return d > epsilon;
}

static void qhFindNeighbours(int v1Handle, int v2Handle, QhHull &q, QhFace& f, QhVertex* vertices)
{
    auto& v1 = vertices[v1Handle];
    auto& v2 = vertices[v2Handle];
    
    for(size_t v1FaceIndex = 0; v1FaceIndex < v1.faceHandles.size; v1FaceIndex++)
    {
        auto v1Face = v1.faceHandles[v1FaceIndex];
        
        if(v1Face == f.indexInHull)
            continue;
        
        bool alreadyAdded = false;
        
        auto &fe = q.faces[v1Face];
        
        if(fe.visitedV)
        {
            continue;
        }
        
        for(size_t n = 0; n < fe.neighbourCount; n++)
        {
            auto& neigh = fe.neighbours[n];
            if(neigh.faceHandle >= (int)q.faces.size)
            {
                alreadyAdded = true;
                break;
            }
        }
        
        if(alreadyAdded)
            continue;
        
        for(size_t v2FaceIndex = 0; v2FaceIndex < v2.faceHandles.size; v2FaceIndex++)
        {
            auto v2Face = v2.faceHandles[v2FaceIndex];
            
            //  Found an edge that connects two faces
            if(v1Face == v2Face)
            {
                bool duplicateVerts = false;
                for(size_t i = 0; i < f.neighbourCount; i++)
                {
                    if((v1.vertexIndex == f.neighbours[i].originVertex && v2.vertexIndex == f.neighbours[i].endVertex) ||
                       (v2.vertexIndex == f.neighbours[i].originVertex && v1.vertexIndex == f.neighbours[i].endVertex))
                    {
                        if(fe.visitedV)
                        {
                            duplicateVerts = true;
                            break;
                        }
                    }
                }
                
                for(size_t i = 0; i < fe.neighbourCount; i++)
                {
                    if((v1.vertexIndex == fe.neighbours[i].originVertex && v2.vertexIndex == fe.neighbours[i].endVertex) ||
                       (v2.vertexIndex == fe.neighbours[i].originVertex && v1.vertexIndex == fe.neighbours[i].endVertex))
                    {
                        if(fe.visitedV)
                        {
                            duplicateVerts = true;
                            break;
                        }
                    }
                }
                
                if(duplicateVerts)
                    continue;
                
                // Set neighbour on current face
                auto index = f.neighbourCount++;
                
                f.neighbours[index].faceHandle = v1Face;
                f.neighbours[index].originVertex = v1.vertexIndex;
                f.neighbours[index].endVertex = v2.vertexIndex;
                
                // Set neighbour on other face
                auto neighbourIndex = fe.neighbourCount++;
                
                fe.neighbours[neighbourIndex].faceHandle = f.indexInHull;
                fe.neighbours[neighbourIndex].endVertex = v1.vertexIndex;
                fe.neighbours[neighbourIndex].originVertex = v2.vertexIndex;
                
                fe.visited = false;
                
                if(fe.neighbourCount + 1 >= MAX_NEIGHBOURS)
                {
                    q.failed = true;
                    log_a("FAILED QH\n");
                    return;
                }
            }
        }
    }
}

static glm::vec3 ComputeFaceNormal(QhFace f, QhVertex* vertices)
{
    // Newell's Method
    // https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
    glm::vec3 normal = glm::vec3(0.0f);
    
    for(size_t i = 0; i < f.vertices.size; i++)
    {
        auto& current = vertices[f.vertices[i]].position;
        auto& next = vertices[f.vertices[(i + 1) % 3]].position;
        
        normal.x = normal.x + (current.y - next.y) * (current.z + next.z);
        normal.y = normal.y + (current.z - next.z) * (current.x + next.x);
        normal.z = normal.z + (current.x - next.x) * (current.y + next.y);
    }
    
    return glm::normalize(normal);
}

static QhFace* qhAddFace(QhHull& q, List<int> &vertexHandles, QhVertex* vertices)
{
    QhFace newFace = {};
    
    init(newFace.vertices);
    
    for(size_t i = 0; i < vertexHandles.size; i++)
    {
        auto &v1 = vertices[vertexHandles[i]];
        addToList(newFace.vertices, v1.vertexIndex);
        
        for(size_t j = 0; j < vertexHandles.size; j++)
        {
            if(j == i)
                continue;
            
            auto &v2 = vertices[vertexHandles[j]];
            if(v1.vertexIndex == v2.vertexIndex)
                return nullptr;
        }
    }
    
    auto center = glm::vec3();
    
    for(size_t i = 0; i < vertexHandles.size; i++)
    {
        center += vertices[newFace.vertices[i]].position;
    }
    
    newFace.centerPoint = glm::vec3(center.x / vertexHandles.size, center.y / vertexHandles.size, center.z / vertexHandles.size);
    
    newFace.outsideSet = {};
    q.processingState.addedFaces++;
    
    newFace.neighbourCount = 0;
    newFace.indexInHull = (int)q.faces.size;
    
    for(size_t i = 0; i < vertexHandles.size; i++)
    {
        auto &v1 = vertices[vertexHandles[i]];
        
        if(v1.faceHandles.size == 0)
        {
            q.processingState.verticesInHull++;
        }
        
        addToList(v1.faceHandles, newFace.indexInHull);
        
        for(size_t j = 0; j < vertexHandles.size; j++)
        {
            if(j == i)
                continue;
            
            auto &v2 = vertexHandles[j];
            qhFindNeighbours(vertexHandles[i], v2, q, newFace, vertices);
            if(q.failed)
                return nullptr;
        }
    }
    
    newFace.faceNormal = ComputeFaceNormal(newFace, vertices);
    
    newFace.faceColor = glm::vec4(0.0f, 1.0f, 1.0f, 0.7f);
    newFace.faceColor.w = 0.5f;
    
    addToList(q.faces, newFace);
    
    return &q.faces[q.faces.size - 1];
}

// Returns index of moved mesh
static int qhRemoveFace(QhHull& qHull, int faceId, QhVertex* vertices)
{
    if(qHull.faces.size == 0 || faceId < 0)
    {
        return -1;
    }
    
    auto &f = qHull.faces[faceId];
    
    // Go through all of f's neighbours to remove itself
    for(size_t n = 0; n < f.neighbourCount; n++)
    {
        // Get the neighbour of f and corresponding face
        auto& neighbour = f.neighbours[n];
        auto& neighbourFace = qHull.faces[neighbour.faceHandle];
        
        // go through the neighbours of the neighbour to find f
        for(size_t i = 0; i < neighbourFace.neighbourCount; i++)
        {
            if(neighbourFace.neighbours[i].faceHandle == f.indexInHull)
            {
                // Found the neighbour
                neighbourFace.neighbours[i] = neighbourFace.neighbours[neighbourFace.neighbourCount - 1];
                neighbourFace.neighbourCount = neighbourFace.neighbourCount - 1;
                neighbourFace.neighbours[neighbourFace.neighbourCount] = {};
                break;
            }
        }
    }
    
    auto indexInHull = f.indexInHull;
    
    for(size_t i = 0; i < f.vertices.size; i++)
    {
        auto &v1 = vertices[f.vertices[i]];
        for(size_t fIndex = 0; fIndex < v1.faceHandles.size; fIndex++)
        {
            if(v1.faceHandles[fIndex] == indexInHull)
            {
                v1.faceHandles[fIndex] = v1.faceHandles[v1.faceHandles.size - 1];
                v1.faceHandles[v1.faceHandles.size - 1] = 0;
                v1.faceHandles.size = v1.faceHandles.size - 1;
                break;
            }
        }
        
        if(v1.faceHandles.size == 0)
        {
            clear(v1.faceHandles);
            qHull.processingState.verticesInHull--;
        }
    }
    
    clear(f.vertices);
    clear(f.outsideSet);
    
    // Invalidates the f pointer
    // But we only need to swap two faces to make this work
    qHull.faces[indexInHull] = qHull.faces[qHull.faces.size - 1];
    auto &newFace = qHull.faces[indexInHull];
    
    qHull.faces.size = qHull.faces.size - 1;
    
    assert(newFace.neighbourCount < (int)qHull.faces.size);
    
    for(size_t neighbourIndex = 0; neighbourIndex < newFace.neighbourCount; neighbourIndex++)
    {
        auto& neighbour = newFace.neighbours[neighbourIndex];
        auto& neighbourFace = qHull.faces[neighbour.faceHandle];
        
        for(size_t n = 0; n < neighbourFace.neighbourCount; n++)
        {
            if(neighbourFace.neighbours[n].faceHandle == newFace.indexInHull)
            {
                // Set it to the new index in the mesh
                neighbourFace.neighbours[n].faceHandle = indexInHull;
                break;
            }
        }
    }
    
    for(size_t i = 0; i < newFace.vertices.size; i++)
    {
        auto& v1new = vertices[newFace.vertices[i]];
        for(size_t fIndex = 0; fIndex < v1new.faceHandles.size; fIndex++)
        {
            if(v1new.faceHandles[fIndex] == newFace.indexInHull)
            {
                v1new.faceHandles[fIndex] = indexInHull;
                break;
            }
        }
        
    }
    
    newFace.indexInHull = indexInHull;
    return indexInHull;
}


float DistanceBetweenPoints(Vertex& p1, Vertex& p2)
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

struct vertex_pair
{
    QhVertex first;
    QhVertex second;
};

int* qhFindExtremePoints(QhVertex* points, int numPoints)
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

coord_t qhGenerateInitialSimplex(QhVertex* vertices, int numVertices, QhHull& q)
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
    
    // Find the point that is furthest away from the segment 
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
    
    coord_t maxX = 0.0f;
    coord_t maxY = 0.0f;
    coord_t maxZ = 0.0f;
    
    for(int i = 0; i < numVertices; i++)
    {
        auto v = vertices[i].position;
        auto curX = Abs(v.x);
        auto curY = Abs(v.y);
        auto curZ = Abs(v.z);
        
        if(curX > maxX)
            maxX = curX;
        if(curY > maxY)
            maxY = curY;
        if(curZ > maxZ)
            maxZ = curZ;
    }
    
    auto epsilon = 3 * (maxX + maxY + maxZ) * FLT_EPSILON;
    //printf("Epsilon: %f\n", epsilon);
    //auto epsilon = 0.0f;
    
    List<int> mostDistList = {};
    init(mostDistList);
    
    addToList(mostDistList, mostDist1);
    addToList(mostDistList, mostDist2);
    addToList(mostDistList, extremePointCurrentIndex);
    
    auto* f = qhAddFace(q, mostDistList, vertices);
    
    clear(mostDistList);
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
        
        auto distance = DistancePointToFace(q, *f, vertices[i]);
        if(distance > currentFurthest)
        {
            currentIndex = i;
            currentFurthest = distance;
        }
    }
    
    if(IsPointOnPositiveSide(q, *f, vertices[currentIndex], epsilon))
    {
        auto t = f->vertices[0];
        f->vertices[0] = f->vertices[1];
        f->vertices[1] = t;
        f->faceNormal = ComputeFaceNormal(*f, vertices);
        List<int> list = {};
        init(list);
        
        addToList(list, mostDist1);
        addToList(list, currentIndex);
        addToList(list, extremePointCurrentIndex);
        qhAddFace(q, list, vertices);
        clear(list);
        if(q.failed)
            return epsilon;
        
        addToList(list, currentIndex);
        addToList(list, mostDist2);
        addToList(list, extremePointCurrentIndex);
        qhAddFace(q, list, vertices);
        clear(list);
        
        if(q.failed)
            return epsilon;
        
        addToList(list, mostDist1);
        addToList(list, mostDist2);
        addToList(list, currentIndex);
        qhAddFace(q, list, vertices);
        clear(list);
        if(q.failed)
            return epsilon;
    }
    else
    {
        List<int> list = {};
        init(list);
        
        addToList(list, currentIndex);
        addToList(list, mostDist1);
        addToList(list, extremePointCurrentIndex);
        qhAddFace(q, list, vertices);
        clear(list);
        if(q.failed)
            return epsilon;
        
        addToList(list, mostDist2);
        addToList(list, currentIndex);
        addToList(list, extremePointCurrentIndex);
        qhAddFace(q, list, vertices);
        clear(list);
        
        if(q.failed)
            return epsilon;
        
        addToList(list, mostDist2);
        addToList(list, mostDist1);
        addToList(list, currentIndex);
        qhAddFace(q, list, vertices);
        clear(list);
        
        if(q.failed)
            return epsilon;
        
    }
    free(extremePoints);
    return epsilon;
}

void qhAddToOutsideSet(QhFace& f, QhVertex& v)
{
    //f.outsideSet.push_back(v.vertexIndex);
    addToList(f.outsideSet, v.vertexIndex);
    v.assigned = true;
}

void qhAssignToOutsideSets(QhHull& q, QhVertex* vertices, int numVertices, List<QhFace> faces, coord_t epsilon)
{
    std::vector<QhVertex> unassigned(vertices, vertices + numVertices);
    
    for(size_t vertexIndex = 0; vertexIndex < unassigned.size(); vertexIndex++)
    {
        auto& vert = vertices[unassigned[vertexIndex].vertexIndex];
        
        if(vert.faceHandles.size > 0)
        {
            continue;
        }
        
        for(auto &f : faces)
        {
            coord_t currentDist = 0.0;
            int currentDistIndex = 0;
            
            if(IsPointOnPositiveSide(q, f, unassigned[vertexIndex], epsilon))
            {
                auto v = unassigned[vertexIndex];
                auto newDist = DistancePointToFace(q, f, v);
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

bool qhEdgeUnique(Edge& input, std::vector<Edge>& list)
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

bool qhHorizonValid(std::vector<Edge>& horizon)
{
    size_t currentIndex = 0;
    auto currentEdge = horizon[currentIndex++];
    
    bool foundEdge = false;
    
    auto firstEdge = horizon[0];
    
    auto cpyH = std::vector<Edge>(horizon);
    
    cpyH.erase(cpyH.begin());
    
    while(currentEdge.end != firstEdge.origin)
    {
        for(size_t i = 0; i < cpyH.size(); i++)
        {
            if(currentEdge.end == cpyH[i].origin)
            {
                foundEdge = true;
                currentIndex = i;
                currentEdge = cpyH[currentIndex];
                break;
            }
        }
        
        if(!foundEdge)
        {
            return false;
        }
        
        cpyH.erase(cpyH.begin() + currentIndex);
        
        foundEdge = false;
    }
    return true;
}

void qhFindConvexHorizon(QhVertex& viewPoint, std::vector<int>& faces, QhHull& qHull, std::vector<Edge>& list, coord_t epsilon)
{
    std::vector<int> possibleVisibleFaces(faces);
    for(size_t faceIndex = 0; faceIndex < possibleVisibleFaces.size(); faceIndex++)
    {
        auto& f = qHull.faces[possibleVisibleFaces[faceIndex]];
        
        for(size_t neighbourIndex = 0; neighbourIndex < f.neighbourCount; neighbourIndex++)
        {
            auto& neighbour = f.neighbours[neighbourIndex];
            auto& neighbourFace = qHull.faces[neighbour.faceHandle];
            
            if(!IsPointOnPositiveSide(qHull, neighbourFace, viewPoint, epsilon))
            {
                Edge newEdge = {};
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
                faces.push_back(neighbourFace.indexInHull);
            }
            neighbourFace.visited = true;
        }
    }
}

Mesh& qhConvertToMesh(RenderContext& renderContext, QhHull& qHull, Vertex* vertices)
{
    if(!qHull.m)
    {
        qHull.m = &InitEmptyMesh(renderContext);
    }
    
    qHull.m->position = glm::vec3(0.0f);
    qHull.m->scale = glm::vec3(globalScale);
    qHull.m->dirty = true;
    
    for(auto &f : qHull.m->faces)
    {
        clear(f.vertices);
    }
    
    qHull.m->faces.clear();
    
    for(const auto& f : qHull.faces)
    {
        Face newFace = {};
        init(newFace.vertices);
        for(size_t i = 0; i < f.vertices.size; i++)
        {
            auto v = vertices[f.vertices.list[i]];
            addToList(newFace.vertices, v);
        }
        
        newFace.faceNormal = f.faceNormal;
        newFace.faceColor = f.faceColor;
        newFace.centerPoint = f.centerPoint;
        qHull.m->faces.push_back(newFace);
    }
    
    for(auto &f : qHull.faces)
    {
        clear(f.vertices);
        clear(f.outsideSet);
    }
    clear(qHull.faces);
    
    return *qHull.m;
}

QhHull qhInit(QhVertex* vertices, int numVertices, std::vector<int>& faceStack, coord_t* epsilon, QhHull *oldHull = nullptr)
{
    QhHull qHull = {};
    
    if(oldHull)
    {
        qHull.m = oldHull->m;
    }
    
    qHull.processingState.addedFaces = 0;
    qHull.processingState.pointsProcessed = 4;
    qHull.processingState.distanceQueryCount = 0;
    qHull.processingState.sidednessQueries = 0;
    qHull.processingState.verticesInHull = 0;
    qHull.processingState.timeSpent = 0;
    
    *epsilon = qhGenerateInitialSimplex(vertices, numVertices, qHull);
    if(qHull.failed)
        return qHull;
    
    qhAssignToOutsideSets(qHull, vertices, numVertices, qHull.faces, *epsilon);
    if(qHull.failed)
        return qHull;
    
    for(int i = 0; i < (int)qHull.faces.size; i++)
    {
        if(qHull.faces[i].outsideSet.size > 0)
        {
            faceStack.push_back(qHull.faces[i].indexInHull);
        }
    }
    return qHull;
}

QhFace* qhFindNextIteration(QhHull& qHull, std::vector<int>& faceStack)
{
    if(faceStack.size() == 0)
        return nullptr;
    
    QhFace* res = nullptr;
    
    auto f = &qHull.faces[faceStack[faceStack.size() - 1]];
    
    if(f && f->outsideSet.size > 0)
    {
        res = f;
    }
    else
    {
        res = nullptr;
    }
    
    faceStack.pop_back();
    return res;
}

void qhHorizonStep(QhHull& qHull, QhVertex* vertices, QhFace& f, std::vector<int>& v, size_t* prevIterationFaces, coord_t epsilon, std::vector<Edge>& horizon)
{ 
    v.push_back(f.indexInHull);
    
    auto& p = vertices[f.furthestPointIndex];
    
    qHull.processingState.pointsProcessed++;
    
    for(size_t i = 0; i < v.size(); i++)
    {
        auto& fa = qHull.faces[v[i]];
        if(fa.visitedV)
            continue;
        
        fa.visitedV = true;
        
        for(size_t neighbourIndex = 0; neighbourIndex < fa.neighbourCount; neighbourIndex++)
        {
            auto& neighbour = qHull.faces[fa.neighbours[neighbourIndex].faceHandle];
            
            auto& newF = qHull.faces[fa.neighbours[neighbourIndex].faceHandle];
            if(!newF.visitedV && IsPointOnPositiveSide(qHull, neighbour, p, epsilon))
            {
                v.push_back(newF.indexInHull);
            }
        }
    }
    
    *prevIterationFaces = qHull.faces.size;
    
    horizon.clear();
    qhFindConvexHorizon(p, v, qHull, horizon, epsilon);
}

bool qhCheckEdgeConvex(QhHull &hull, QhFace leftFace, QhFace rightFace, coord_t epsilon = 0.0f)
{
    auto leftBelow = !IsPointOnPositiveSide(hull, leftFace, rightFace.centerPoint, epsilon);
    auto rightBelow = !IsPointOnPositiveSide(hull, rightFace, leftFace.centerPoint, epsilon);
    
    return leftBelow && rightBelow;
}

void qhIteration(QhHull& qHull, QhVertex* vertices, std::vector<int>& faceStack, int fHandle, std::vector<int>& v, size_t prevIterationFaces, coord_t epsilon, std::vector<Edge>& horizon)
{
    for(const auto& e : horizon)
    {
        auto f = qHull.faces[fHandle];
        
        List<int> list = {};
        init(list);
        
        addToList(list, e.origin);
        addToList(list, e.end);
        addToList(list, f.furthestPointIndex);
        
        auto* newF = qhAddFace(qHull, list, vertices);
        clear(list);
        
        if(qHull.failed)
            return;
        
        if(newF)
        {
            auto otherFace = qHull.faces[(newF->indexInHull + 5) % qHull.faces.size];
            
            if(otherFace.indexInHull == newF->indexInHull)
            {
                otherFace = qHull.faces[(newF->indexInHull + 8) % qHull.faces.size];
            }
            
            
            if(IsPointOnPositiveSide(qHull, *newF, otherFace.centerPoint, epsilon))
            {
                auto t = newF->vertices[0];
                newF->vertices[0] = newF->vertices[1];
                newF->vertices[1] = t;
                newF->faceNormal = ComputeFaceNormal(*newF, vertices);
            }
            
            faceStack.push_back(newF->indexInHull);
        }
    }
    
    for(const auto& handle : v)
    {
        auto &fInV = qHull.faces[handle];
        
        for(size_t osIndex = 0; osIndex < fInV.outsideSet.size; osIndex++)
        {
            auto osHandle = fInV.outsideSet[osIndex];
            auto &q = vertices[osHandle];
            q.assigned = false;
        }
    }
    
    // The way we understand this, is that unassigned now means any point that was
    // assigned in the first round, but is part of a face that is about to be
    // removed. Thus about to be unassigned.
    for(size_t newFaceIndex = prevIterationFaces; newFaceIndex < qHull.faces.size; newFaceIndex++)
    {
        if(qHull.faces[newFaceIndex].outsideSet.size > 0)
            continue;
        
        coord_t currentDist = 0.0;
        int currentDistIndex = 0;
        auto& newFace = qHull.faces[newFaceIndex];
        
        for(const auto& handle : v)
        {
            auto& fInV = qHull.faces[handle];
            for(size_t osIndex = 0; osIndex < fInV.outsideSet.size; osIndex++)
            {
                auto osHandle = fInV.outsideSet[osIndex];
                auto& q = vertices[osHandle];
                if(!q.assigned && q.faceHandles.size == 0 && IsPointOnPositiveSide(qHull, newFace, q, epsilon))
                {
                    auto newDist = DistancePointToFace(qHull, newFace, q);
                    if(newDist > currentDist)
                    {
                        currentDist = newDist;
                        currentDistIndex = q.vertexIndex;
                        newFace.furthestPointIndex = q.vertexIndex;
                    }
                    
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
    
    for(size_t vIndex = 0; vIndex < uniqueInV.size(); vIndex++)
    {
        auto movedHandle = qHull.faces.size - 1;
        
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
    
    for(size_t i = 0; i < qHull.faces.size; i++)
    {
        qHull.faces[i].visited = false;
    }
}

void qhFullHull(QhContext& qhContext)
{
    qhContext.currentFace = nullptr;
    qhContext.faceStack.clear();
    qhContext.epsilon = 0.0;
    qhContext.qHull = qhInit(qhContext.vertices, qhContext.numberOfPoints, qhContext.faceStack, &qhContext.epsilon, &qhContext.qHull);
    if(qhContext.qHull.failed)
        return;
    
    qhContext.v.clear();
    qhContext.previousIteration = 0;
    while(qhContext.faceStack.size() > 0)
    {
        qhContext.currentFace = qhFindNextIteration(qhContext.qHull, qhContext.faceStack);
        if(qhContext.qHull.failed)
            return;
        
        if(qhContext.currentFace)
        {
            qhHorizonStep(qhContext.qHull, qhContext.vertices, *qhContext.currentFace, qhContext.v, &qhContext.previousIteration, qhContext.epsilon, qhContext.horizon);
            if(qhContext.qHull.failed)
                return;
            
            qhIteration(qhContext.qHull, qhContext.vertices, qhContext.faceStack, qhContext.currentFace->indexInHull, qhContext.v, 
                        qhContext.previousIteration, qhContext.epsilon, qhContext.horizon);
            if(qhContext.qHull.failed)
                return;
            
            qhContext.v.clear();
        }
    }
}

QhHull qhFullHull(QhVertex* vertices, int numVertices)
{
    QhFace* currentFace = nullptr;
    std::vector<int> faceStack;
    coord_t epsilon;
    
    auto qHull = qhInit(vertices, numVertices, faceStack, &epsilon);
    if(qHull.failed)
        return qHull;
    
    std::vector<int> v;
    
    std::vector<Edge> horizon;
    
    size_t previousIteration = 0;
    while(faceStack.size() > 0)
    {
        currentFace = qhFindNextIteration(qHull, faceStack);
        if(qHull.failed)
            return qHull;
        if(currentFace)
        {
            qhHorizonStep(qHull, vertices, *currentFace, v, &previousIteration,
                          epsilon, horizon);
            if(qHull.failed)
                return qHull;
            qhIteration(qHull, vertices, faceStack, currentFace->indexInHull, v,
                        previousIteration, epsilon, horizon);
            if(qHull.failed)
                return qHull;
            v.clear();
        }
    }
    
    return qHull;
}

void qhInitializeContext(QhContext& qhContext, Vertex* vertices, int numberOfPoints)
{
    if(qhContext.vertices)
    {
        free(qhContext.vertices);
    }
    
    for(auto &f : qhContext.qHull.faces)
    {
        clear(f.outsideSet);
        clear(f.vertices);
    }
    
    clear(qhContext.qHull.faces);
    qhContext.qHull.processingState = {};
    
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

void qhStep(QhContext& context)
{
    switch(context.iter)
    {
        case QHIteration::initQH:
        {
            context.qHull = qhInit(context.vertices, context.numberOfPoints, context.faceStack, &context.epsilon);
            context.iter = QHIteration::findNextIter;
        }
        break;
        case QHIteration::findNextIter:
        {
            if(context.faceStack.size() > 0)
            {
                context.currentFace = qhFindNextIteration(context.qHull, context.faceStack);
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
                qhHorizonStep(context.qHull, context.vertices, *context.currentFace, context.v, &context.previousIteration, context.epsilon, context.horizon);
                context.iter = QHIteration::doIter;
            }
        }
        break;
        case QHIteration::doIter:
        {
            if(context.currentFace)
            {
                qhIteration(context.qHull, context.vertices, context.faceStack, context.currentFace->indexInHull, context.v, context.previousIteration, context.epsilon, context.horizon);
                context.iter = QHIteration::findNextIter;
                context.v.clear();
            }
        }
        break;
    }
}


#endif
