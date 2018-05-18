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
};

template<typename T>
struct qh_list
{
    size_t size = 0;
    size_t capacity = 0;
    T *list = nullptr;
    
    T &operator[](size_t index)
    {
#if DEBUG
        if(index > this->size)
        {
            assert(false);
        }
#endif
        return this->list[index];
    }
    
    
    T &operator[](int index)
    {
#if DEBUG
        if(index > (int)this->size)
        {
            assert(false);
        }
#endif
        return this->list[index];
    }
    
    
    T *begin() {return this->list;}
    T *end() {return this->list + this->size;}
    
};

template<typename T>
static void qhAddToList(qh_list<T> &list, T element)
{
    if (list.capacity == 0)
    {
        list.capacity = 2;
        list.list = (T*)malloc(sizeof(T) * list.capacity);
    }
    if (list.size + 1 > list.capacity)
    {
        list.capacity *= 2;
        list.list = (T*)realloc(list.list, sizeof(T) * list.capacity);
        for(size_t i = list.size; i < list.capacity; i++)
        {
            list.list[i] = {};
        }
    }
    
    list.list[list.size++] = element;
}

template<typename T>
static void clear(qh_list<T> &list)
{
    for(size_t i = 0; i < list.size; i++)
    {
        list[i] = {};
    }
    
    if(list.size > 0)
    {
        free(list.list);
    }
    
    list.size = 0;
    list.capacity = 0;
}

template<typename T>
static void init(qh_list<T> &list)
{
    list.size = 0;
    list.capacity = 0;
    list.list = nullptr;
}

struct qh_vertex
{
    qh_list<int> faceHandles;
    
    bool assigned;
    int vertexIndex;
    
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec4 color;
};

#define MAX_NEIGHBOURS 16

struct qh_face
{
    int vertices[3];
    qh_list<int> outsideSet;
    
    int furthestPointIndex;
    
    qh_neighbour neighbours[MAX_NEIGHBOURS];
    size_t neighbourCount;
    
    int indexInHull;
    bool visited;
    bool visitedV;
    
    glm::vec3 faceNormal;
    glm::vec3 centerPoint;
    glm::vec4 faceColor;
};


struct qh_hull
{
    qh_list<qh_face> faces;
    mesh* m;
    
    struct
    {
        int addedFaces;
        int pointsProcessed;
        int distanceQueryCount;
        int sidednessQueries;
        int verticesInHull;
        unsigned long long timeSpent;
    } processingState;
    
    bool failed;
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
    size_t previousIteration;
    qh_hull qHull;
    
    std::vector<edge> horizon;
};

static void qhCopyVertices(qh_context& q, vertex* vertices, int numberOfPoints)
{
    q.vertices = (qh_vertex*)malloc(sizeof(qh_vertex) * numberOfPoints);
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
float DistancePointToFace(qh_hull &q, qh_face f, qh_vertex v)
{
    q.processingState.distanceQueryCount++;
    return glm::abs((glm::dot(f.faceNormal, v.position) - glm::dot(f.faceNormal, f.centerPoint))); 
}

static bool IsPointOnPositiveSide(qh_hull &q, qh_face f, qh_vertex v, coord_t epsilon = 0.0f)
{
    q.processingState.sidednessQueries++;
    auto d = glm::dot(f.faceNormal, v.position - f.centerPoint);
    return d > epsilon;
}

static bool IsPointOnPositiveSide(qh_hull &q, qh_face f, glm::vec3 v, coord_t epsilon = 0.0f)
{
    q.processingState.sidednessQueries++;
    auto d = glm::dot(f.faceNormal, v - f.centerPoint);
    return d > epsilon;
}

static void qhFindNeighbours(int v1Handle, int v2Handle, qh_hull &q, qh_face& f, qh_vertex* vertices)
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
    
    newFace.outsideSet = {};
    q.processingState.addedFaces++;
    
    newFace.neighbourCount = 0;
    newFace.indexInHull = (int)q.faces.size;
    
    newFace.vertices[0] = v1.vertexIndex;
    newFace.vertices[1] = v2.vertexIndex;
    newFace.vertices[2] = v3.vertexIndex;
    
    newFace.centerPoint = (vertices[newFace.vertices[0]].position + vertices[newFace.vertices[1]].position + vertices[newFace.vertices[2]].position) / 3.0f;
    
    qhFindNeighbours(v1Handle, v2Handle, q, newFace, vertices);
    
    if(q.failed)
        return nullptr;
    qhFindNeighbours(v1Handle, v3Handle, q, newFace, vertices);
    if(q.failed)
        return nullptr;
    qhFindNeighbours(v2Handle, v3Handle, q, newFace, vertices);
    if(q.failed)
        return nullptr;
    
    if(v1.faceHandles.size == 0)
    {
        q.processingState.verticesInHull++;
    }
    
    if(v2.faceHandles.size == 0)
    {
        q.processingState.verticesInHull++;
    }
    
    if(v3.faceHandles.size == 0)
    {
        q.processingState.verticesInHull++;
    }
    
    qhAddToList(v1.faceHandles, newFace.indexInHull);
    qhAddToList(v2.faceHandles, newFace.indexInHull);
    qhAddToList(v3.faceHandles, newFace.indexInHull);
    
    
    newFace.faceNormal = ComputeFaceNormal(newFace, vertices);
    
    newFace.faceColor = glm::vec4(0.0f, 1.0f, 1.0f, 0.7f);
    newFace.faceColor.w = 0.5f;
    
    qhAddToList(q.faces, newFace);
    
    return &q.faces[q.faces.size - 1];
}

// Returns index of moved mesh
static int qhRemoveFace(qh_hull& qHull, int faceId, qh_vertex* vertices)
{
    if(qHull.faces.size == 0 || faceId < 0)
    {
        return -1;
    }
    
    auto &f = qHull.faces[faceId];
    
    auto &v1 = vertices[f.vertices[0]];
    auto &v2 = vertices[f.vertices[1]];
    auto &v3 = vertices[f.vertices[2]];
    
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
    
    assert(qHull.faces[indexInHull].neighbourCount < (int)qHull.faces.size);
    // Invalidates the f pointer
    // But we only need to swap two faces to make this work
    qHull.faces[indexInHull] = qHull.faces[qHull.faces.size - 1];
    qHull.faces.size = qHull.faces.size - 1;
    
    assert(qHull.faces[indexInHull].neighbourCount < (int)qHull.faces.size);
    
    auto &newFace = qHull.faces[indexInHull];
    
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
    
    for(size_t fIndex = 0; fIndex < v1.faceHandles.size; fIndex++)
    {
        if(v1.faceHandles[fIndex] == indexInHull)
        {
            v1.faceHandles[fIndex] = v1.faceHandles[v1.faceHandles.size - 1];
            v1.faceHandles.size = v1.faceHandles.size - 1;
            v1.faceHandles[v1.faceHandles.size] = 0;
            break;
        }
    }
    
    if(v1.faceHandles.size == 0)
    {
        qHull.processingState.verticesInHull--;
    }
    
    for(size_t fIndex = 0; fIndex < v2.faceHandles.size; fIndex++)
    {
        if(v2.faceHandles[fIndex] == indexInHull)
        {
            v2.faceHandles[fIndex] = v2.faceHandles[v2.faceHandles.size - 1];
            v2.faceHandles.size = v2.faceHandles.size - 1;
            v2.faceHandles[v2.faceHandles.size] = 0;
            break;
        }
    }
    
    if(v2.faceHandles.size == 0)
    {
        qHull.processingState.verticesInHull--;
    }
    
    for(size_t fIndex = 0; fIndex < v3.faceHandles.size; fIndex++)
    {
        if(v3.faceHandles[fIndex] == indexInHull)
        {
            v3.faceHandles[fIndex] = v3.faceHandles[v3.faceHandles.size - 1];
            v3.faceHandles.size = v3.faceHandles.size - 1;
            v3.faceHandles[v3.faceHandles.size] = 0;
            break;
        }
    }
    
    if(v3.faceHandles.size == 0)
    {
        qHull.processingState.verticesInHull--;
    }
    
    auto& v1new = vertices[newFace.vertices[0]];
    for(size_t fIndex = 0; fIndex < v1new.faceHandles.size; fIndex++)
    {
        if(v1new.faceHandles[fIndex] == newFace.indexInHull)
        {
            v1new.faceHandles[fIndex] = indexInHull;
            break;
        }
    }
    
    auto& v2new = vertices[newFace.vertices[1]];
    for(size_t fIndex = 0; fIndex < v2new.faceHandles.size; fIndex++)
    {
        if(v2new.faceHandles[fIndex] == newFace.indexInHull)
        {
            v2new.faceHandles[fIndex] = indexInHull;
            break;
        }
    }
    
    auto& v3new = vertices[newFace.vertices[2]];
    for(size_t fIndex = 0; fIndex < v3new.faceHandles.size; fIndex++)
    {
        if(v3new.faceHandles[fIndex] == newFace.indexInHull)
        {
            v3new.faceHandles[fIndex] = indexInHull;
            break;
        }
    }
    
    newFace.indexInHull = indexInHull;
    return indexInHull;
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


coord_t qhGenerateInitialSimplex(qh_vertex* vertices, int numVertices, qh_hull& q)
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
        qhAddFace(q, mostDist1, currentIndex, extremePointCurrentIndex, vertices);
        if(q.failed)
            return epsilon;
        qhAddFace(q, currentIndex, mostDist2, extremePointCurrentIndex, vertices);
        if(q.failed)
            return epsilon;
        qhAddFace(q, mostDist1, mostDist2, currentIndex, vertices);
        if(q.failed)
            return epsilon;
    }
    else
    {
        qhAddFace(q, currentIndex, mostDist1, extremePointCurrentIndex, vertices);
        if(q.failed)
            return epsilon;
        qhAddFace(q, mostDist2, currentIndex, extremePointCurrentIndex, vertices);
        if(q.failed)
            return epsilon;
        qhAddFace(q, mostDist2, mostDist1, currentIndex, vertices);
        if(q.failed)
            return epsilon;
    }
    free(extremePoints);
    return epsilon;
}

void qhAddToOutsideSet(qh_face& f, qh_vertex& v)
{
    //f.outsideSet.push_back(v.vertexIndex);
    qhAddToList(f.outsideSet, v.vertexIndex);
    v.assigned = true;
}

void qhAssignToOutsideSets(qh_hull& q, qh_vertex* vertices, int numVertices, qh_list<qh_face> faces, coord_t epsilon)
{
    std::vector<qh_vertex> unassigned(vertices, vertices + numVertices);
    
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
    size_t currentIndex = 0;
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

void qhFindConvexHorizon(qh_vertex& viewPoint, std::vector<int>& faces, qh_hull& qHull, std::vector<edge>& list, coord_t epsilon)
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
                faces.push_back(neighbourFace.indexInHull);
            }
            neighbourFace.visited = true;
        }
    }
}

mesh& qhConvertToMesh(render_context& renderContext, qh_hull& qHull, vertex* vertices)
{
    if(!qHull.m)
    {
        qHull.m = &InitEmptyMesh(renderContext);
    }
    
    qHull.m->position = glm::vec3(0.0f);
    qHull.m->scale = glm::vec3(globalScale);
    qHull.m->dirty = true;
    
    qHull.m->faces.clear();
    
    for(const auto& f : qHull.faces)
    {
        face newFace = {};
        newFace.vertices[0] = vertices[f.vertices[0]];
        newFace.vertices[1] = vertices[f.vertices[1]];
        newFace.vertices[2] = vertices[f.vertices[2]];
        newFace.faceNormal = f.faceNormal;
        newFace.faceColor = f.faceColor;
        newFace.centerPoint = f.centerPoint;
        qHull.m->faces.push_back(newFace);
    }
    
    return *qHull.m;
}

qh_hull qhInit(qh_vertex* vertices, int numVertices, std::vector<int>& faceStack, coord_t* epsilon, qh_hull *oldHull = nullptr)
{
    qh_hull qHull = {};
    
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

qh_face* qhFindNextIteration(qh_hull& qHull, std::vector<int>& faceStack)
{
    if(faceStack.size() == 0)
        return nullptr;
    
    qh_face* res = nullptr;
    
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

void qhHorizonStep(qh_hull& qHull, qh_vertex* vertices, qh_face& f, std::vector<int>& v, size_t* prevIterationFaces, coord_t epsilon, std::vector<edge>& horizon)
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

bool qhCheckEdgeConvex(qh_hull &hull, qh_face leftFace, qh_face rightFace)
{
    auto leftBelow = !IsPointOnPositiveSide(hull, leftFace, rightFace.centerPoint);
    auto rightBelow = !IsPointOnPositiveSide(hull, rightFace, leftFace.centerPoint);
    
    return leftBelow && rightBelow;
}

void qhIteration(qh_hull& qHull, qh_vertex* vertices, std::vector<int>& faceStack, int fHandle, std::vector<int>& v, size_t prevIterationFaces, coord_t epsilon, std::vector<edge>& horizon)
{
    for(const auto& e : horizon)
    {
        auto f = qHull.faces[fHandle];
        
        auto* newF = qhAddFace(qHull, e.origin, e.end, f.furthestPointIndex, vertices);
        
        if(qHull.failed)
            return;
        
        if(newF)
        {
            for(size_t i = 0; i < newF->neighbourCount; i++)
            {
                auto neighbourHandle = newF->neighbours[i].faceHandle;
                auto edgeConvex = qhCheckEdgeConvex(qHull, *newF, qHull.faces[neighbourHandle]);
                
                if(!edgeConvex)
                {
                }
                
            }
            
            if(IsPointOnPositiveSide(qHull, *newF, f.centerPoint, epsilon))
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

void qhFullHull(qh_context& qhContext)
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

qh_hull qhFullHull(qh_vertex* vertices, int numVertices)
{
    qh_face* currentFace = nullptr;
    std::vector<int> faceStack;
    coord_t epsilon;
    
    auto qHull = qhInit(vertices, numVertices, faceStack, &epsilon);
    if(qHull.failed)
        return qHull;
    
    std::vector<int> v;
    
    std::vector<edge> horizon;
    
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

void qhInitializeContext(qh_context& qhContext, vertex* vertices, int numberOfPoints)
{
    if(qhContext.vertices)
    {
        for(int i = 0; i < qhContext.numberOfPoints; i++)
        {
            free(qhContext.vertices[i].faceHandles.list);
        }
        free(qhContext.vertices);
    }
    
    for(const auto &f : qhContext.qHull.faces)
    {
        free(f.outsideSet.list);
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

void qhStep(qh_context& context)
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
