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

// t=nn·v1−nn·p
// p0=p+t·nn
float DistancePointToFace(face& f, vertex& v, vertex* vertices)
{
    /*auto t = f.faceNormal * f.vertices[0].position - f.faceNormal * v.position;
    auto p0 = v.position + t * f.faceNormal;
    return Distance(p0, v.position);*/
    
    auto d1 = Distance(vertices[f.vertices[0]].position, v.position);
    auto d2 = Distance(vertices[f.vertices[1]].position, v.position);
    auto d3 = Distance(vertices[f.vertices[2]].position, v.position);
    return d1 + d2 + d3;
}

void GenerateInitialSimplex(render_context& renderContext, vertex* vertices, int numberOfPoints, mesh& m)
{
    auto extremePoints = FindExtremePoints(vertices, numberOfPoints);
    
    auto extremePointCurrentFurthest = 0.0f;
    auto extremePointCurrentIndex = 0;
    
    for(int i = 2; i < 6; i++)
    {
        auto distance = SquareDistancePointToSegment(vertices[extremePoints[0]], vertices[extremePoints[1]], vertices[extremePoints[i]]);
        if(distance > extremePointCurrentFurthest)
        {
            extremePointCurrentIndex = i;
        }
    }
    
    auto f = AddFace(renderContext, m, vertices[extremePoints[0]], vertices[extremePoints[1]], vertices[extremePoints[extremePointCurrentIndex]], vertices);
    
    auto currentFurthest = 0.0f;
    auto currentIndex = 0;
    
    for(int i = 0; i < numberOfPoints; i++)
    {
        if(i == extremePoints[0] || i == extremePoints[1] || i == extremePoints[extremePointCurrentIndex])
        {
            continue;
        }
        auto distance = DistancePointToFace(f, vertices[i], vertices);
        if(distance > currentFurthest)
        {
            currentIndex = i;
        }
    }
    
    AddFace(renderContext, m, vertices[currentIndex], vertices[extremePoints[0]], vertices[extremePoints[extremePointCurrentIndex]], vertices);
    AddFace(renderContext, m, vertices[extremePoints[1]], vertices[currentIndex], vertices[extremePoints[extremePointCurrentIndex]], vertices);
    AddFace(renderContext, m, vertices[extremePoints[1]], vertices[extremePoints[0]], vertices[currentIndex], vertices);
    
}

bool IsAbove(vertex& v, face f, vertex* vertices)
{
    auto A = (vertices[f.vertices[0]].position + vertices[f.vertices[1]].position + vertices[f.vertices[2]].position)/3.0f;
    
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
            f.outsideSet = (int*)malloc(sizeof(int) * f.outsideSetSize);
        }
        else
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
            if(IsAbove(unassigned[vertexIndex], f, vertices))
            {
                vertex v = vertices[vertexIndex];
                AddToOutsideSet(f, v);
                unassigned[vertexIndex] = unassigned[unassignedCount - 1];
                //unassigned = (vertex*)memmove(&unassigned[vertexIndex], &unassigned[vertexIndex + 1], sizeof(vertex) * (unassignedCount - vertexIndex + 1));
                unassignedCount--;
            }
        }
    }
    free(unassigned);
    
}

struct edge
{
    int origin;
    int end;
};

struct edge_list
{
    edge* edges;
    int count;
    
    edge& operator[](int index)
    {
        return this->edges[index];
    }
};

edge_list FindConvexHorizon(vertex& viewPoint, face* faces, int numFaces, vertex* vertices, mesh& m)
{
    //TODO: 16 is an arbitrary number/size
    edge_list list = {};
    list.edges = (edge*)malloc(sizeof(edge) * 16);
    list.count = 0;
    for(int faceIndex = 0; faceIndex < numFaces; faceIndex++)
    {
        for(int neighbourIndex = 0; neighbourIndex < 3; neighbourIndex++)
        {
            auto& neighbour = faces[faceIndex].neighbours[neighbourIndex];
            if(!IsAbove(viewPoint, m.faces[neighbour.faceHandle], vertices))
            {
                printf("ECH\n");
                edge newEdge = {};
                newEdge.origin = neighbour.originVertex;
                newEdge.end = neighbour.endVertex;
                list.edges[list.count++] = newEdge;
            }
        }
    }
    return list;
}

void QuickHull(render_context& renderContext, vertex* vertices, int numberOfPoints)
{
    auto& m = InitEmptyMesh(renderContext);
    m.position = glm::vec3(0.0f);
    m.scale = glm::vec3(globalScale);
    m.numFaces = 0;
    m.facesSize = 0;
    GenerateInitialSimplex(renderContext, vertices, numberOfPoints, m);
    AssignToOutsideSets(vertices, numberOfPoints, m.faces,  m.numFaces);
    
    // TODO: Optimize for less faces than going through all
    for(int i = 0; i < m.numFaces; i++)
    {
        auto& f = m.faces[i];
        //f.visited = true;
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
            face v[32];
            int inV = 0;
            
            auto& p = vertices[f.outsideSet[currentFurthest]];
            
            //TODO: for all unvisited neighbours -> What is unvisited exactly in this case?
            for(int neighbourIndex = 0; neighbourIndex < 3; neighbourIndex++)
            {
                auto& neighbour = m.faces[f.neighbours[neighbourIndex].faceHandle];
                if(!neighbour.visited)
                {
                    if(IsAbove(p, neighbour, vertices))
                    {
                        v[inV++] = neighbour;
                    }
                    neighbour.visited = true;
                }
            }
            
            auto numFaces = m.numFaces;
            
            auto edgeList = FindConvexHorizon(p, v, inV, vertices, m);
            
            for(int edgeIndex = 0; edgeIndex < edgeList.count; edgeIndex++)
            {
                auto edge = edgeList[edgeIndex];
                AddFace(renderContext, m, vertices[edge.origin], vertices[edge.end], p, vertices);
            }
            
            // The way we understand this, is that unassigned now means any point that was
            // assigned in the first round, but is part of a face that is about to be
            // removed. Thus about to be unassigned.
            for(int newFaceIndex = numFaces; newFaceIndex < m.numFaces; newFaceIndex++)
            {
                for(int oldFaceIndex = 0; oldFaceIndex < inV; oldFaceIndex++)
                {
                    auto& f = v[oldFaceIndex];
                    for(int osIndex = 0; osIndex < f.outsideSetCount; osIndex++)
                    {
                        auto& q = vertices[f.outsideSet[osIndex]];
                        if(IsAbove(q, m.faces[newFaceIndex], vertices))
                        {
                            AddToOutsideSet(m.faces[newFaceIndex], q);
                        }
                    }
                }
            }
            
            for(int vIndex = 0; vIndex < inV; vIndex++)
            {
                printf("Removing face\n");
                RemoveFace(m, &v[vIndex]);
            }
        }
    }
}




#endif
