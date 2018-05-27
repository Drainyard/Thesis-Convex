#ifndef INCREMENTAL_H
#define INCREMENTAL_H

///forward declare structs
struct IncVertex;
struct IncEdge;
struct IncFace;
struct IncArc;

struct IncArc
{
    union 
    {
        IncVertex *vertexEndpoint;
        IncFace *faceEndpoint;
    };
    size_t indexInEndpoint;
};

struct IncVertex
{
    glm::vec3 vector;
    int vIndex;
    IncEdge *duplicate;
    bool isOnHull;
    bool isProcessed;
    bool isRemoved;
    bool isAlreadyInConflicts;
    IncVertex *next;
    IncVertex *prev;
    List<IncArc> arcs;
};

struct IncEdge
{
    IncFace *adjFace[2];
    IncVertex *endPoints[2];
    IncFace *newFace; //pointer to face about to be added (to remember old structure)
    bool shouldBeRemoved;
    bool isRemoved;
    IncEdge *next;
    IncEdge *prev;
};

struct IncFace
{
    IncEdge *edge[3];
    IncVertex *vertex[3];
    glm::vec3 normal;
    glm::vec3 centerPoint;
    bool isVisible;
    bool isRemoved;
    IncFace *next;
    IncFace *prev;
    List<IncArc> arcs;
};

//Head pointers to each of the three lists
IncVertex *incVertices = nullptr;
IncEdge *incEdges = nullptr;
IncFace *incFaces = nullptr;

//Pointer to current vertex in step context
IncVertex *currentStepVertex = nullptr;

//counters
static int incCreatedFaces = 0;
static int incProcessedVertices = 3;
static unsigned long long  incSidednessQueries = 0;
static int incVerticesOnHull = 0;
static int incFacesOnHull = 0;

struct IncContext
{
    bool initialized;
    
    int numberOfPoints;
    Mesh *m;
    struct
    {
        int createdFaces;
        int processedVertices;
        unsigned long long sidednessQueries;
        int facesOnHull;
        int verticesOnHull;
        unsigned long long timeSpent;
    } processingState;
    bool failed;
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

static void incCopyVertices(Vertex *vertices, int numberOfPoints)
{
    Vertex *shuffledVertices = (Vertex *)malloc(sizeof(Vertex) * numberOfPoints);
    memcpy(shuffledVertices, vertices, sizeof(Vertex) * numberOfPoints);
    Vertex temp;
    int j;
    //Fisher Yates shuffle
    for (int i = numberOfPoints - 1; i > 0; i--)
    {
        j = rand() % (i + 1);
        temp = shuffledVertices[j];
        shuffledVertices[j] = shuffledVertices[i];
        shuffledVertices[i] = temp;
    }
    
    IncVertex *v;
    for (int i = 0; i < numberOfPoints; i++)
    {
        v = (IncVertex *)malloc(sizeof(IncVertex));
        v->duplicate = nullptr;
        v->isOnHull = false;
        v->isProcessed = false;
        v->isRemoved = false;
        v->isAlreadyInConflicts = false;
        v->vIndex = i;
        v->vector = shuffledVertices[i].position;
        init(v->arcs);
        incAddToHead(&incVertices, v);
    }
    free(shuffledVertices);
}

IncEdge *incCreateNullEdge()
{
    IncEdge *e = (IncEdge *)malloc(sizeof(IncEdge));
    e->adjFace[0] = e->adjFace[1] = nullptr;
    e->newFace = nullptr;
    e->endPoints[0] = e->endPoints[1] = nullptr;
    e->shouldBeRemoved = false;
    e->isRemoved = false;
    e->next = nullptr;
    e->prev = nullptr;
    incAddToHead(&incEdges, e);
    
    return e;
}

IncFace *incCreateNullFace()
{
    IncFace *f = (IncFace *)malloc(sizeof(IncFace));
    f->edge[0] = f->edge[1] = f->edge[2] = nullptr;
    f->vertex[0] = f->vertex[1] = f->vertex[2] = nullptr;
    f->isVisible = false;
    f->isRemoved = false;
    f->next = nullptr;
    f->prev = nullptr;
    incAddToHead(&incFaces, f);
    init(f->arcs);
    
    incCreatedFaces++;
    
    return f;
}

/*
(a-b) and (b-c) are direction vectors in the plane
If they are on the same line, the angle between them is 0.
So colinear if (a-b)x(b-c)=0

This is short hand of
return
    ((v1->vector.y - v0->vector.y)(v2->vector.z - v0->vector.z)-(v2->vector.y - v0->vector.y)(v1->vector.z - v0->vector.z) == 0) &&
    ((v2->vector.x - v0->vector.x)(v1->vector.z - v0->vector.z)-(v1->vector.x - v0->vector.x)(v2->vector.z - v0->vector.z) == 0) &&
    ((v1->vector.x - v0->vector.x)(v2->vector.y - v0->vector.y)-(v2->vector.x - v0->vector.x)(v1->vector.y - v0->vector.y) == 0)
    
*/
bool incColinear(IncVertex *v0, IncVertex *v1, IncVertex *v2)
{
    return glm::cross((v0->vector - v1->vector), (v1->vector - v2->vector)) == glm::vec3(0, 0, 0);
}

glm::vec3 incComputeFaceNormal(IncFace *f)
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

static bool incIsPointOnPositiveSide(IncFace *f, IncVertex *v, coord_t epsilon = 0.0f)
{
    incSidednessQueries++;
    auto d = glm::dot(f->normal, v->vector - f->centerPoint);
    return d > epsilon;
}

static bool incIsPointCoplanar(IncFace *f, IncVertex *v, coord_t epsilon = 0.0f)
{
    auto d = glm::dot(f->normal, v->vector - f->centerPoint);
    return d >= -epsilon && d <= epsilon;
}

IncFace *incMakeFace(IncVertex *v0, IncVertex *v1, IncVertex *v2, IncFace *face)
{
    IncEdge *e0, *e1, *e2;
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
    
    IncFace *f = incCreateNullFace();
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

void incInitConflictListForFace(IncFace *newFace, IncFace *oldFace1, IncFace *oldFace2)
{
    for (IncArc &arc : oldFace1->arcs)
    {
        IncVertex *v = arc.vertexEndpoint;
        if (incIsPointOnPositiveSide(newFace, v))
        {
            IncArc arcToFace = {};
            arcToFace.faceEndpoint = newFace;
            arcToFace.indexInEndpoint = newFace->arcs.size;
            addToList(v->arcs, arcToFace);
            
            IncArc arcToVertex = {};
            arcToVertex.vertexEndpoint = v;
            arcToVertex.indexInEndpoint = v->arcs.size - 1;
            addToList(newFace->arcs, arcToVertex);
            v->isAlreadyInConflicts = true;
        }
    }
    
    for (IncArc &arc : oldFace2->arcs)
    {
        IncVertex *v = arc.vertexEndpoint;
        if (!v->isAlreadyInConflicts)
        {
            if (incIsPointOnPositiveSide(newFace, v))
            {
                IncArc arcToFace = {};
                arcToFace.faceEndpoint = newFace;
                arcToFace.indexInEndpoint = newFace->arcs.size;
                addToList(v->arcs, arcToFace);
                
                IncArc arcToVertex = {};
                arcToVertex.vertexEndpoint = v;
                arcToVertex.indexInEndpoint = v->arcs.size - 1;
                addToList(newFace->arcs, arcToVertex);
            }
        }
    }
    
    for (IncArc &arc : newFace->arcs)
    {
        arc.vertexEndpoint->isAlreadyInConflicts = false;
    }
}

void incCleanConflictGraph(std::vector<IncFace *> &facesToRemove)
{
    for (IncFace *face : facesToRemove)
    {
        for (IncArc &arc : face->arcs)
        {
            //We have to remove the arc from f->v and the arc v->f.
            //This is done by swapping with the last element in the arcs list
            //But since every arc knows the index of its duplicate arc, we also have to update the endPoint arcs we swap with
            IncVertex *v = arc.vertexEndpoint;
            
            if(v->arcs.size == 0)
                continue;
            
            IncArc lastArcInV = v->arcs[v->arcs.size - 1];
            lastArcInV.faceEndpoint->arcs[lastArcInV.indexInEndpoint].indexInEndpoint = arc.indexInEndpoint;
            v->arcs[arc.indexInEndpoint] = v->arcs[v->arcs.size - 1];
            v->arcs.size = v->arcs.size - 1;
        }
    }
}

//double sided triangle from points NOT colinear
void incCreateBihedron()
{
    IncVertex *v0 = incVertices;
    while (incColinear(v0, v0->next, v0->next->next))
    {
        v0 = v0->next;
        if (v0 == incVertices)
        {
            //ERROR ONLY COLINEAR POINTS
            printf("incCreateBihedron - colinear points");
            exit(0);
        }
    }
    IncVertex *v1 = v0->next;
    IncVertex *v2 = v1->next;
    
    IncFace *f0, *f1;
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
    
    IncVertex *v3 = v2->next;
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

void incEnforceCounterClockWise(IncFace *newFace, IncEdge *e, IncVertex *v)
{
    //From Computational Geometry in C page 136
    //We are applying the orientation of the face we are replacing. As we are on the horizon edge, one face is visible, while one is not.
    //As we are creating a new face above the old face, they have two points in common, and the orientation will be the same
    IncFace *visibleFace;
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
        IncEdge *temp = newFace->edge[1];
        newFace->edge[1] = newFace->edge[2];
        newFace->edge[2] = temp;
    }
    newFace->vertex[2] = v;
}

IncFace *incMakeConeFace(IncEdge *e, IncVertex *v)
{
    //duplicate is commented in compgeoC book 135
    //since we create faces to v in arbitrary order, we let vertices on hull know about the edge of the new face that it is endpoint for
    //if neighbor face is created we copy edge info from that one. If neighbor face is not created yet, duplicate is null and we create new edges for that
    IncEdge *newEdge1 = e->endPoints[0]->duplicate;
    if (!newEdge1)
    {
        newEdge1 = incCreateNullEdge();
        newEdge1->endPoints[0] = e->endPoints[0];
        newEdge1->endPoints[1] = v;
        e->endPoints[0]->duplicate = newEdge1;
    }
    
    IncEdge *newEdge2 = e->endPoints[1]->duplicate;
    if (!newEdge2)
    {
        newEdge2 = incCreateNullEdge();
        newEdge2->endPoints[0] = e->endPoints[1];
        newEdge2->endPoints[1] = v;
        e->endPoints[1]->duplicate = newEdge2;
    }
    
    IncFace *newFace = incCreateNullFace();
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

std::pair<std::vector<IncFace *>, std::vector<IncEdge *>> incAddToHull(IncVertex *v, IncContext &incContext)
{
    std::vector<IncFace *> facesToRemove;
    std::vector<IncEdge *> horizonEdges;
    bool visible = false;
    std::vector<IncArc> vConflicts;
    std::copy(v->arcs.begin(), v->arcs.end(), std::back_inserter(vConflicts));
    
    for (IncArc &arc : vConflicts)
    {
        arc.faceEndpoint->isVisible = visible = true;
    }
    if (!visible)
    {
        //No faces are visible and we are inside hull. No arcs to update
        v->isOnHull = false;
        v->isRemoved = true;
        //clear(v->arcs);
        incRemoveFromHead(&incVertices, &v);
        std::pair<std::vector<IncFace *>, std::vector<IncEdge *>> cleaningBundle(facesToRemove, horizonEdges);
        return cleaningBundle;
    }
    
    IncEdge *e;
    for (IncArc arc : vConflicts)
    {
        IncFace *face = arc.faceEndpoint;
        //since two faces can share an edge, we could go through all edges twice (although it fails fast). Discussion?
        for (int i = 0; i < 3; i++)
        {
            e = face->edge[i];
            //if shouldBeRemoved is set, it has already been processed and is not important for new faces
            //horizon edges will always only be processed once
            if (!e->shouldBeRemoved)
            {
                if(!e->adjFace[0] || !e->adjFace[1])
                {
                    incContext.failed = true;
                    printf("FAILED INC\n");
                    std::pair<std::vector<IncFace *>, std::vector<IncEdge *>> cleaningBundle(facesToRemove, horizonEdges);
                    return cleaningBundle;
                }
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
    
    std::pair<std::vector<IncFace *>, std::vector<IncEdge *>> cleaningBundle(facesToRemove, horizonEdges);
    return cleaningBundle;
}

void incCleanEdgesAndFaces(std::vector<IncFace *> &facesToRemove, std::vector<IncEdge *> &horizonEdges)
{
    //replace the pointer to the newly created face. no need to go through all edges
    //send horizon edges to this one. Loop over them.
    for (IncEdge *horizonEdge : horizonEdges)
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
    for (IncFace *face : facesToRemove)
    {
        for (i = 0; i < 3; i++)
        {
            IncEdge *e = face->edge[i];
            if (e && !e->isRemoved && e->shouldBeRemoved)
            {
                //no idea why this works
                e->isRemoved = true;
                incRemoveFromHead(&incEdges, &e);
            }
            
            IncVertex *v = face->vertex[i];
            if (v && !v->isRemoved && !v->isOnHull)
            {
                v->isRemoved = true;
                clear(v->arcs);
                incRemoveFromHead(&incVertices, &v);
            }
        }
        clear(face->arcs);
        incRemoveFromHead(&incFaces, &face);
    }
    //reset vertex flags
    for (IncEdge *horizonEdge : horizonEdges)
    {
        horizonEdge->endPoints[0]->isOnHull = horizonEdge->endPoints[1]->isOnHull = false;
        horizonEdge->endPoints[0]->duplicate = horizonEdge->endPoints[1]->duplicate = nullptr;
    }
}

void incCleanStuff(std::pair<std::vector<IncFace *>, std::vector<IncEdge *>> &cleaningBundle)
{
    //    auto timerCleanEdgesAndFaces = startTimer();
    incCleanConflictGraph(cleaningBundle.first);
    incCleanEdgesAndFaces(cleaningBundle.first, cleaningBundle.second);
}

Mesh &incConvertToMesh(IncContext &context, RenderContext &renderContext)
{
    if (!context.m)
    {
        context.m = &InitEmptyMesh(renderContext);
    }
    
    context.m->faces.clear();
    context.m->position = glm::vec3(0.0f);
    context.m->scale = glm::vec3(globalScale);
    context.m->dirty = true;
    
    IncFace *f = incFaces;
    if (f)
    {
        do
        {
            Face newFace = {};
            for (int i = 0; i < 3; i++)
            {
                Vertex newVertex = {};
                newVertex.position = f->vertex[i]->vector;
                newVertex.vertexIndex = f->vertex[i]->vIndex;
                addToList(newFace.vertices, newVertex);
            }
            newFace.faceColor = glm::vec4(0.0f, 1.0f, 1.0f, 0.7f);
            newFace.faceColor.w = 0.5f;
            newFace.faceNormal = f->normal;
            newFace.centerPoint = f->centerPoint;
            context.m->faces.push_back(newFace);
            
            f = f->next;
        } while (f != incFaces);
    }
    
    return *context.m;
}

void incInitConflictLists()
{
    //determine which of the points can see which of the two faces - linear time
    //for each point - positive side of one face, also if coplanar
    IncVertex *v = incVertices;
    IncVertex *nextVertex;
    IncFace *f1 = incFaces;
    IncFace *f2 = incFaces->next;
    int i = 0;
    do
    {
        i++;
        nextVertex = v->next;
        
        IncFace *conflictFace = incIsPointOnPositiveSide(f1, v) ? f1 : f2;
        IncArc arcToFace = {};
        arcToFace.faceEndpoint = conflictFace;
        arcToFace.indexInEndpoint = conflictFace->arcs.size;
        addToList(v->arcs, arcToFace);
        
        IncArc arcToVertex = {};
        arcToVertex.vertexEndpoint = v;
        arcToVertex.indexInEndpoint = v->arcs.size - 1;
        addToList(conflictFace->arcs, arcToVertex);
        
        v = nextVertex;
    } while (v != incVertices);
}

void incConstructFullHull(IncContext &incContext)
{
    incCreateBihedron();
    incInitConflictLists();
    IncVertex *v = incVertices;
    IncVertex *nextVertex;
    do
    {
        nextVertex = v->next;
        if (!v->isProcessed)
        {
            std::pair<std::vector<IncFace *>, std::vector<IncEdge *>> cleaningBundle = incAddToHull(v, incContext);
            if (incContext.failed)
            {
                return;
            }
            v->isProcessed = true;
            incProcessedVertices++;
            incCleanStuff(cleaningBundle);
        }
        v = nextVertex;
    } while (v != incVertices);
    
    // add counters to context
    incContext.processingState.createdFaces = incCreatedFaces;
    incContext.processingState.processedVertices = incProcessedVertices;
    incContext.processingState.sidednessQueries = incSidednessQueries;
    v = incVertices;
    do
    {
        incVerticesOnHull++;
        v = v->next;
    } while (v != incVertices);
    incContext.processingState.verticesOnHull = incVerticesOnHull;
    IncFace *f = incFaces;
    do
    {
        incFacesOnHull++;
        f = f->next;
    } while (f != incFaces);
    incContext.processingState.facesOnHull = incFacesOnHull;
}

void incInitStepHull()
{
    incCreateBihedron();
    incInitConflictLists();
    currentStepVertex = incVertices;
}

void incHullStep(IncContext &incContext)
{
    IncVertex *nextVertex;
    nextVertex = currentStepVertex->next;
    if (!currentStepVertex->isProcessed)
    {
        std::pair<std::vector<IncFace *>, std::vector<IncEdge *>> cleaningBundle = incAddToHull(currentStepVertex, incContext);
        if (incContext.failed)
        {
            return;
        }
        currentStepVertex->isProcessed = true;
        incCleanStuff(cleaningBundle);
    }
    currentStepVertex = nextVertex;
}

void incInitializeContext(IncContext &incContext, Vertex *vertices, int numberOfPoints)
{
    //counter reset
    incCreatedFaces = 0;
    incProcessedVertices = 3;
    incSidednessQueries = 0;
    incVerticesOnHull = 0;
    incFacesOnHull = 0;
    
    if (incVertices)
    {
        IncVertex *v = incVertices->next;
        IncVertex *nextVertex;
        while (incVertices)
        {
            nextVertex = v->next;
            clear(v->arcs);
            incRemoveFromHead(&incVertices, &v);
            v = nextVertex;
        };
    }
    
    if (incFaces)
    {
        IncFace *f = incFaces->next;
        IncFace *nextFace;
        while (incFaces)
        {
            nextFace = f->next;
            clear(f->arcs);
            incRemoveFromHead(&incFaces, &f);
            f = nextFace;
        };
    }
    if (incEdges)
    {
        IncEdge *e = incEdges->next;
        IncEdge *nextEdge;
        while (incEdges)
        {
            nextEdge = e->next;
            incRemoveFromHead(&incEdges, &e);
            e = nextEdge;
        }
    }
    
    incCopyVertices(vertices, numberOfPoints);
    incContext.numberOfPoints = numberOfPoints;
    incContext.initialized = true;
}

#endif
