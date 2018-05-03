#ifndef INCREMENTAL_H
#define INCREMENTAL_H

enum class IHIteration
{
    initIH,
    findNextIter,
    doIter
};

/*

Når vi skal fjerne en arc:

vi husker index på hvor dup arc ligger i anden liste. Den swappes med det sidste i listen. 
Den arc swapper med skal opdatere sit index, så den igen ved hvor arcen ligger, de to er igen synkroniseret. repeat for første

Så
remove arcs for face:
    for each arc in face.arcs
        dupArc = arc->vertexEndpoint.arcs[arc.index]
        swapped = dupArc.swapWithLast (swapped is now in dupArcs index)
        swapped->faceEndpoint.arcs[swapped.index].index = dupArc.index
        erase dupArc (that is now on last index)
        do the same for the arc.
        
*/

//forward declare structs
struct incVertex;
struct incEdge;
struct incFace;
struct incArc;

struct incArc
{
    union 
    {
        incVertex *vertexEndpoint;
        incFace *faceEndpoint;
    };
    size_t indexInEndpoint;
};

struct incArcList
{
    size_t size = 0;
    size_t capacity = 0;
    incArc* list = nullptr;

    incArc& operator[](size_t index)
    {
        return this->list[index];
    }
};

void incAddToArcList(incArcList &arcList, incArc arc)
{
    if(arcList.capacity == 0)
    {
        arcList.capacity = 2;
        arcList.list = (incArc*)malloc(sizeof(incArc) * arcList.capacity);
    }
    if(arcList.size + 1 > arcList.capacity)
    {
        arcList.capacity *= 2;
        arcList.list = (incArc*)realloc(arcList.list, sizeof(incArc) * arcList.capacity);
    }

    arcList.list[arcList.size++] = arc;
}

struct incVertex
{
    glm::vec3 vector;
    int vIndex;
    incEdge *duplicate;
    bool isOnHull;
    bool isProcessed;
    bool isRemoved;
    bool isAlreadyInConflicts;
    incVertex *next;
    incVertex *prev;
    incArcList arcs;
    // std::vector<incArc> arcs;
};

struct incEdge
{
    incFace *adjFace[2];
    incVertex *endPoints[2];
    incFace *newFace; //pointer to face about to be added (to remember old structure)
    bool shouldBeRemoved;
    bool isRemoved;
    incEdge *next;
    incEdge *prev;
};

struct incFace
{
    incEdge *edge[3];
    incVertex *vertex[3];
    glm::vec3 normal;
    glm::vec3 centerPoint;
    bool isVisible;
    bool isRemoved;
    incFace *next;
    incFace *prev;
    // std::vector<incArc> arcs;
    incArcList arcs;
};


//Head pointers to each of the three lists
incVertex *incVertices = nullptr;
incEdge *incEdges = nullptr;
incFace *incFaces = nullptr;

//Pointer to current vertex in step context
incVertex *currentStepVertex = nullptr;

struct inc_context
{
    bool initialized;
    incVertex *vertices;
    int numberOfPoints;
    IHIteration iter;
    mesh m;
    int previousIteration;
};

template <typename T>
void incAddToHead(T *head, T pointer)
{
    if (*head)
    {
        pointer->next = *head;
        pointer->prev = (*head)->prev;
        (*head)->prev = pointer;
        pointer->prev->next = pointer;
    }
    else
    {
        *head = pointer;
        (*head)->next = (*head)->prev = pointer;
    }
};

template <typename T>
void incRemoveFromHead(T *head, T *pointer)
{
    if (*head)
    {
        if (*head == (*head)->next)
        {
            *head = nullptr;
        }
        else if (*pointer == *head)
        {
            *head = (*head)->next;
        }
        (*pointer)->next->prev = (*pointer)->prev;
        (*pointer)->prev->next = (*pointer)->next;
        free(*pointer);
        *pointer = nullptr;
    }
};

static void incCopyVertices(vertex *vertices, int numberOfPoints)
{
    vertex *shuffledVertices = (vertex *)malloc(sizeof(vertex) * numberOfPoints);
    memcpy(shuffledVertices, vertices, sizeof(vertex) * numberOfPoints);
    vertex temp;
    int i, j;
    //Fisher Yates shuffle
    for (i = numberOfPoints - 1; i > 0; i--)
    {
        j = rand() % (i + 1);
        temp = shuffledVertices[j];
        shuffledVertices[j] = shuffledVertices[i];
        shuffledVertices[i] = temp;
    }

    incVertex *v;
    for (int i = 0; i < numberOfPoints; i++)
    {
        v = (incVertex *)malloc(sizeof(incVertex));
        v->duplicate = nullptr;
        v->isOnHull = false;
        v->isProcessed = false;
        v->isRemoved = false;
        v->isAlreadyInConflicts = false;
        v->vIndex = i;
        v->vector = shuffledVertices[i].position;
        v->arcs = {};
        incAddToHead(&incVertices, v);
    }
     free(shuffledVertices);
}

incEdge *incCreateNullEdge()
{
    incEdge *e = (incEdge *)malloc(sizeof(incEdge));
    e->adjFace[0] = e->adjFace[1] = nullptr;
    e->newFace = nullptr;
    e->endPoints[0] = e->endPoints[1] = nullptr;
    e->shouldBeRemoved = false;
    e->isRemoved = false;
    incAddToHead(&incEdges, e);

    return e;
}

incFace *incCreateNullFace()
{
    incFace *f = (incFace *)malloc(sizeof(incFace));
    f->edge[0] = f->edge[1] = f->edge[2] = nullptr;
    f->vertex[0] = f->vertex[1] = f->vertex[2] = nullptr;
    f->isVisible = false;
    f->isRemoved = false;
    f->arcs = {};
    incAddToHead(&incFaces, f);

    return f;
}

/*
(a-b) and (b-c) are direction vectors in the plane
If they are on the same line, the angle between them is 0.
So collinear if (a-b)x(b-c)=0

This is short hand of
return
    ((v1->vector.y - v0->vector.y)(v2->vector.z - v0->vector.z)-(v2->vector.y - v0->vector.y)(v1->vector.z - v0->vector.z) == 0) &&
    ((v2->vector.x - v0->vector.x)(v1->vector.z - v0->vector.z)-(v1->vector.x - v0->vector.x)(v2->vector.z - v0->vector.z) == 0) &&
    ((v1->vector.x - v0->vector.x)(v2->vector.y - v0->vector.y)-(v2->vector.x - v0->vector.x)(v1->vector.y - v0->vector.y) == 0)

*/
bool incCollinear(incVertex *v0, incVertex *v1, incVertex *v2)
{
    return glm::cross((v0->vector - v1->vector), (v1->vector - v2->vector)) == glm::vec3(0, 0, 0);
}

glm::vec3 incComputeFaceNormal(incFace *f)
{
    // Newell's Method
    // https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
    glm::vec3 normal = glm::vec3(0.0f);

    for (int i = 0; i < 3; i++)
    {
        glm::vec3 current = f->vertex[i]->vector;
        glm::vec3 next = f->vertex[(i + 1) % 3]->vector;

        normal.x = normal.x + (current.y - next.y) * (current.z + next.z);
        normal.y = normal.y + (current.z - next.z) * (current.x + next.x);
        normal.z = normal.z + (current.x - next.x) * (current.y + next.y);
    }

    return glm::normalize(normal);
}

static bool incIsPointOnPositiveSide(incFace *f, incVertex *v, coord_t epsilon = 0.0f)
{
    auto d = glm::dot(f->normal, v->vector - f->centerPoint);
    return d > epsilon;
}

static bool incIsPointCoplanar(incFace *f, incVertex *v, coord_t epsilon = 0.0f)
{
    auto d = glm::dot(f->normal, v->vector - f->centerPoint);
    return d >= -epsilon && d <= epsilon;
}

incFace *incMakeFace(incVertex *v0, incVertex *v1, incVertex *v2, incFace *face)
{
    incEdge *e0, *e1, *e2;
    //initial hedron, no edges to copy from
    if (!face)
    {
        e0 = incCreateNullEdge();
        e1 = incCreateNullEdge();
        e2 = incCreateNullEdge();
    }
    //copy edges in reverse order
    else
    {
        e0 = face->edge[2];
        e1 = face->edge[1];
        e2 = face->edge[0];
    }
    e0->endPoints[0] = v0;
    e0->endPoints[1] = v1;
    e1->endPoints[0] = v1;
    e1->endPoints[1] = v2;
    e2->endPoints[0] = v2;
    e2->endPoints[1] = v0;

    incFace *f = incCreateNullFace();
    f->edge[0] = e0;
    f->edge[1] = e1;
    f->edge[2] = e2;
    f->vertex[0] = v0;
    f->vertex[1] = v1;
    f->vertex[2] = v2;
    f->centerPoint = (f->vertex[0]->vector + f->vertex[1]->vector + f->vertex[2]->vector) / 3.0f;

    e0->adjFace[0] = e1->adjFace[0] = e2->adjFace[0] = f;

    return f;
}

void incInitConflictListForFace(incFace *newFace, incFace *oldFace1, incFace *oldFace2)
{
    // for (incArc arc : oldFace1->arcs)
    for(int i = 0; i < oldFace1->arcs.size; i++)
    {
        auto &arc = oldFace1->arcs[i];
        incVertex *v = arc.vertexEndpoint;
        if (incIsPointOnPositiveSide(newFace, v))
        {
            incArc arcToFace = {};
            arcToFace.faceEndpoint = newFace;
            // arcToFace.indexInEndpoint = newFace->arcs.size();
            arcToFace.indexInEndpoint = newFace->arcs.size;
            incAddToArcList(v->arcs, arcToFace);
            // v->arcs.push_back(arcToFace);

            incArc arcToVertex = {};
            arcToVertex.vertexEndpoint = v;
            // arcToVertex.indexInEndpoint = v->arcs.size();
            arcToVertex.indexInEndpoint = v->arcs.size - 1;            
            // newFace->arcs.push_back(arcToVertex);
            incAddToArcList(newFace->arcs, arcToVertex);

            v->isAlreadyInConflicts = true;
        }
    }

    // for (incArc arc : oldFace2->arcs)
    for(int i = 0; i < oldFace2->arcs.size; i++)
    {
        auto &arc = oldFace2->arcs[i];
        incVertex *v = arc.vertexEndpoint;
        if (!v->isAlreadyInConflicts)
        {
            if (incIsPointOnPositiveSide(newFace, v))
            {
                incArc arcToFace = {};
                arcToFace.faceEndpoint = newFace;
                // arcToFace.indexInEndpoint = newFace->arcs.size();
                // v->arcs.push_back(arcToFace);
                arcToFace.indexInEndpoint = newFace->arcs.size;
                incAddToArcList(v->arcs, arcToFace);

                incArc arcToVertex = {};
                arcToVertex.vertexEndpoint = v;
                // arcToVertex.indexInEndpoint = v->arcs.size();
                arcToVertex.indexInEndpoint = v->arcs.size - 1;
                // newFace->arcs.push_back(arcToVertex);
                incAddToArcList(newFace->arcs, arcToVertex);
            }
        }
    }
    // for (incArc arc : newFace->arcs)
    for(int i = 0; i < newFace->arcs.size; i++)
    {
        auto &arc = newFace->arcs[i];
        arc.vertexEndpoint->isAlreadyInConflicts = false;
    }
}

void incCleanConflictGraph(std::vector<incFace *> &facesToRemove)
{
    for (incFace *face : facesToRemove)
    {
        // for (incArc arc : face->arcs)
        for(size_t i = 0; i < face->arcs.size; i++)
        {
            auto &arc = face->arcs[i];
            //We have to remove the arc from f->v and the arc v->f.
            //This is done by swapping with the last element in the arcs list
            //But since every arc knows the index of its duplicate arc, we also have to update the endPoint arcs we swap with
            incVertex *v = arc.vertexEndpoint;
            incArc dupArc = v->arcs[arc.indexInEndpoint];
            // incArc lastArcInV = v->arcs[v->arcs.size() - 1];
            incArc lastArcInV = v->arcs[v->arcs.size - 1];
            lastArcInV.faceEndpoint->arcs[lastArcInV.indexInEndpoint].indexInEndpoint = dupArc.indexInEndpoint;
            // v->arcs[arc.indexInEndpoint] = v->arcs[v->arcs.size() - 1];
            v->arcs[arc.indexInEndpoint] = v->arcs[v->arcs.size - 1];
            v->arcs.size--;
            // v->arcs.erase(v->arcs.begin() + v->arcs.size() - 1);
        }
        //should we clear the arcs in f?
    }
}

//double sided triangle from points NOT collinear
void incCreateBihedron()
{
    incVertex *v0 = incVertices;
    while (incCollinear(v0, v0->next, v0->next->next))
    {
        v0 = v0->next;
        if (v0 == incVertices)
        {
            //ERROR ONLY COLLINEAR POINTS
            printf("incCreateBihedron - collinear points");
            exit(0);
        }
    }
    incVertex *v1 = v0->next;
    incVertex *v2 = v1->next;

    incFace *f0, *f1;
    f0 = f1 = nullptr;

    f0 = incMakeFace(v0, v1, v2, f1);
    f1 = incMakeFace(v2, v1, v0, f0);

    f0->edge[0]->adjFace[1] = f1;
    f0->edge[1]->adjFace[1] = f1;
    f0->edge[2]->adjFace[1] = f1;
    f1->edge[0]->adjFace[1] = f0;
    f1->edge[1]->adjFace[1] = f0;
    f1->edge[2]->adjFace[1] = f0;

    f0->normal = incComputeFaceNormal(f0);
    f1->normal = incComputeFaceNormal(f1);

    v0->isProcessed = true;
    v1->isProcessed = true;
    v2->isProcessed = true;

    incVertex *v3 = v2->next;
    bool coplanar = incIsPointCoplanar(f0, v3);
    while (coplanar)
    {
        v3 = v3->next;
        if (v3 == v0)
        {
            //ERROR ONLY COPLANAR PLANAR
            printf("incCreateBihedron - coplanar points");
            exit(0);
        }
        coplanar = incIsPointCoplanar(f0, v3);
    }
    incVertices = v3;
}

void incEnforceCounterClockWise(incFace *newFace, incEdge *e, incVertex *v)
{
    //From Computational Geometry in C page 136
    //We are applying the orientation of the face we are replacing. As we are on the horizon edge, one face is visible, while one is not.
    //As we are creating a new face above the old face, they have two points in common, and the orientation will be the same
    incFace *visibleFace;
    if (e->adjFace[0]->isVisible)
    {
        visibleFace = e->adjFace[0];
    }
    else
    {
        visibleFace = e->adjFace[1];
    }
    int i = 0;
    //Find a vertex the edge and old face share
    while (visibleFace->vertex[i] != e->endPoints[0])
    {
        i++;
    }
    //if the old face and edge disagree on the following vertex, that means endPoint[0] is on the "left corner" of old face triangle and endPoint[1] is the "right"
    //thus to enforce counter clockwise, flip the two for the base of the newFace
    if (visibleFace->vertex[(i + 1) % 3] != e->endPoints[1])
    {
        newFace->vertex[0] = e->endPoints[1];
        newFace->vertex[1] = e->endPoints[0];
    }
    //the visible face has right corner as endPoint[0], left corner as endPoint[1]
    else
    {
        newFace->vertex[0] = e->endPoints[0];
        newFace->vertex[1] = e->endPoints[1];

        //this swap is just for consistency, it is not exactly necessary to enforce counter clockwise orientation for edges.
        //in incMakeConeFace, we set e as edge[0], edge[1] is based on endPoint[0], and edge[2] on endPoint[1].
        //if e goes from left to right, then edge[1] should be based on endPoint[1] and edge[2] on endPoint[0] to ensure counter clockwise of edges as well
        incEdge *temp = newFace->edge[1];
        newFace->edge[1] = newFace->edge[2];
        newFace->edge[2] = temp;
    }
    newFace->vertex[2] = v;
}

incFace *incMakeConeFace(incEdge *e, incVertex *v)
{
    //duplicate is commented in compgeoC book 135
    //since we create faces to v in arbitrary order, we let vertices on hull know about the edge of the new face that it is endpoint for
    //if neighbor face is created we copy edge info from that one. If neighbor face is not created yet, duplicate is null and we create new edges for that
    incEdge *newEdge1 = e->endPoints[0]->duplicate;
    if (!newEdge1)
    {
        newEdge1 = incCreateNullEdge();
        newEdge1->endPoints[0] = e->endPoints[0];
        newEdge1->endPoints[1] = v;
        e->endPoints[0]->duplicate = newEdge1;
    }

    incEdge *newEdge2 = e->endPoints[1]->duplicate;
    if (!newEdge2)
    {
        newEdge2 = incCreateNullEdge();
        newEdge2->endPoints[0] = e->endPoints[1];
        newEdge2->endPoints[1] = v;
        e->endPoints[1]->duplicate = newEdge2;
    }

    incFace *newFace = incCreateNullFace();
    newFace->edge[0] = e;
    newFace->edge[1] = newEdge1;
    newFace->edge[2] = newEdge2;
    incEnforceCounterClockWise(newFace, e, v);
    newFace->centerPoint = (newFace->vertex[0]->vector + newFace->vertex[1]->vector + newFace->vertex[2]->vector) / 3.0f;
    newFace->normal = incComputeFaceNormal(newFace);

    if (!newEdge1->adjFace[0])
    {
        newEdge1->adjFace[0] = newFace;
    }
    else if (!newEdge1->adjFace[1])
    {
        newEdge1->adjFace[1] = newFace;
    }
    if (!newEdge2->adjFace[0])
    {
        newEdge2->adjFace[0] = newFace;
    }
    else if (!newEdge2->adjFace[1])
    {
        newEdge2->adjFace[1] = newFace;
    }
    return newFace;
}

std::pair<std::vector<incFace *>, std::vector<incEdge *>> incAddToHull(incVertex *v)
{
    std::vector<incFace *> facesToRemove;
    std::vector<incEdge *> horizonEdges;
    bool visible = false;
    std::vector<incArc> vConflicts;
    // std::copy(v->arcs.begin(), v->arcs.end(), std::back_inserter(vConflicts));
    for(size_t i = 0; i < v->arcs.size; i++)
    {
        vConflicts.push_back(v->arcs[i]);
    }

    for (incArc arc : vConflicts)
    {
        arc.faceEndpoint->isVisible = visible = true;
    }
    if (!visible)
    {
        //No faces are visible and we are inside hull. No arcs to update
        v->isOnHull = false;
        v->isRemoved = true;
        incRemoveFromHead(&incVertices, &v);
        std::pair<std::vector<incFace *>, std::vector<incEdge *>> cleaningBundle(facesToRemove, horizonEdges);
        return cleaningBundle;
    }
    incEdge *e;
    for (incArc arc : vConflicts)
    {
        incFace *face = arc.faceEndpoint;
        //since two faces can share an edge, we could go through all edges twice (although it fails fast). Discussion?
        for (int i = 0; i < 3; i++)
        {
            e = face->edge[i];
            //if shouldBeRemoved is set, it has already been processed and is not important for new faces
            //horizon edges will always only be processed once
            if (!e->shouldBeRemoved)
            {
                if (e->adjFace[0]->isVisible && e->adjFace[1]->isVisible)
                {
                    //both are visible: inside cone and should be removed
                    e->shouldBeRemoved = true;
                }
                else if (e->adjFace[0]->isVisible || e->adjFace[1]->isVisible)
                {
                    //only one is visible: border edge, erect face for cone
                    e->newFace = incMakeConeFace(e, v);

                    //OPTIMIZE THIS!!!
                    incInitConflictListForFace(e->newFace, e->adjFace[0], e->adjFace[1]);

                    horizonEdges.push_back(e);
                }
            }
        }
        facesToRemove.push_back(face);
    }

    incCleanConflictGraph(facesToRemove);

    std::pair<std::vector<incFace *>, std::vector<incEdge *>> cleaningBundle(facesToRemove, horizonEdges);
    return cleaningBundle;
}

void incCleanEdgesAndFaces(std::vector<incFace *> &facesToRemove, std::vector<incEdge *> &horizonEdges)
{
    //replace the pointer to the newly created face. no need to go through all edges
    //send horizon edges to this one. Loop over them.
    for (incEdge *horizonEdge : horizonEdges)
    {
        if (horizonEdge->newFace)
        {
            if (horizonEdge->adjFace[0]->isVisible)
            {
                horizonEdge->adjFace[0] = horizonEdge->newFace;
            }
            else
            {
                horizonEdge->adjFace[1] = horizonEdge->newFace;
            }
            horizonEdge->newFace = nullptr;
        }
        horizonEdge->endPoints[0]->isOnHull = horizonEdge->endPoints[1]->isOnHull = true;
    }
    int i;
    for (incFace *face : facesToRemove)
    {
        for (i = 0; i < 3; i++)
        {
            incEdge *e = face->edge[i];
            if (e && !e->isRemoved && e->shouldBeRemoved)
            {
                //no idea why this works
                e->isRemoved = true;
                incRemoveFromHead(&incEdges, &e);
            }

            incVertex *v = face->vertex[i];
            if (v && !v->isRemoved && !v->isOnHull)
            {
                v->isRemoved = true;
                incRemoveFromHead(&incVertices, &v);
            }
        }
        incRemoveFromHead(&incFaces, &face);
    }
    //reset vertex flags
    for (incEdge *horizonEdge : horizonEdges)
    {
        horizonEdge->endPoints[0]->isOnHull = horizonEdge->endPoints[1]->isOnHull = false;
        horizonEdge->endPoints[0]->duplicate = horizonEdge->endPoints[1]->duplicate = nullptr;
    }
}

void incCleanStuff(std::pair<std::vector<incFace *>, std::vector<incEdge *>> &cleaningBundle)
{
    //    auto timerCleanEdgesAndFaces = startTimer();
    incCleanEdgesAndFaces(cleaningBundle.first, cleaningBundle.second);
    //    TIME_END(timerCleanEdgesAndFaces, "incCleanEdgesAndFaces");
}

mesh &incConvertToMesh(render_context &renderContext)
{
    auto &m = InitEmptyMesh(renderContext);
    m.position = glm::vec3(0.0f);
    m.scale = glm::vec3(globalScale);
    m.dirty = true;

    incFace *f = incFaces;
    do
    {
        face newFace = {};
        for (int i = 0; i < 3; i++)
        {
            vertex newVertex = {};
            newVertex.position = f->vertex[i]->vector;
            newVertex.vertexIndex = f->vertex[i]->vIndex;
            newFace.vertices[i] = newVertex;
        }
        newFace.faceColor = glm::vec4(0.0f, 1.0f, 1.0f, 0.7f);
        newFace.faceColor.w = 0.5f;
        newFace.faceNormal = f->normal;
        newFace.centerPoint = f->centerPoint;
        m.faces.push_back(newFace);

        f = f->next;
    } while (f != incFaces);

    return m;
}

void incInitConflictLists()
{
    //determine which of the points can see which of the two faces - linear time
    //for each point - positive side of one face, also if coplanar
    incVertex *v = incVertices;
    incVertex *nextVertex;
    incFace *f1 = incFaces;
    incFace *f2 = incFaces->next;
    int i = 0;
    do
    {
        i++;
        nextVertex = v->next;

        incFace *conflictFace = incIsPointOnPositiveSide(f1, v) ? f1 : f2;
        incArc arcToFace = {};
        arcToFace.faceEndpoint = conflictFace;
        // arcToFace.indexInEndpoint = conflictFace->arcs.size();
        arcToFace.indexInEndpoint = conflictFace->arcs.size;
        incAddToArcList(v->arcs, arcToFace);
        // v->arcs.push_back(arcToFace);

        incArc arcToVertex = {};
        arcToVertex.vertexEndpoint = v;
        // arcToVertex.indexInEndpoint = v->arcs.size() - 1;
        arcToVertex.indexInEndpoint = v->arcs.size - 1;
        // conflictFace->arcs.push_back(arcToVertex);
        incAddToArcList(conflictFace->arcs, arcToVertex);

        v = nextVertex;
    } while (v != incVertices);
}

void incConstructFullHull()
{
    //    auto timerCreateBihedron = startTimer();
    incCreateBihedron();
    //    TIME_END(timerCreateBihedron, "incCreateBihedron");
    //    auto timerInitConflictLists = startTimer();
    incInitConflictLists();
    //    TIME_END(timerInitConflictLists, "incInitConflictLists");
    incVertex *v = incVertices;
    incVertex *nextVertex;
    do
    {
        nextVertex = v->next;
        if (!v->isProcessed)
        {
            std::pair<std::vector<incFace *>, std::vector<incEdge *>> cleaningBundle = incAddToHull(v);
            v->isProcessed = true;
            incCleanStuff(cleaningBundle);
        }
        v = nextVertex;
    } while (v != incVertices);
}

void incInitStepHull()
{
    incCreateBihedron();
    incInitConflictLists();
    currentStepVertex = incVertices;
}

void incHullStep()
{
    incVertex *nextVertex;
    nextVertex = currentStepVertex->next;
    if (!currentStepVertex->isProcessed)
    {
        std::pair<std::vector<incFace *>, std::vector<incEdge *>> cleaningBundle = incAddToHull(currentStepVertex);
        currentStepVertex->isProcessed = true;
        incCleanStuff(cleaningBundle);
    }
    currentStepVertex = nextVertex;
}

void incInitializeContext(inc_context &incContext, vertex *vertices, int numberOfPoints)
{
    if (incContext.vertices)
    {
        free(incContext.vertices);
    }

    incCopyVertices(vertices, numberOfPoints);
    incContext.numberOfPoints = numberOfPoints;
    // incContext.epsilon = 0.0f;
    incContext.iter = IHIteration::initIH;
    incContext.previousIteration = 0;
    incContext.initialized = true;
}

#endif
