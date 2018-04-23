#ifndef INCREMENTAL_H
#define INCREMENTAL_H

enum class IHIteration
{
    initIH,
    findNextIter,
    doIter
};

//edges have faces and faces have edges, so forward declare here
struct incEdge;
struct incFace;

struct incVertex
{
    glm::vec3 vector;
    int vIndex;
    incEdge *duplicate;
    bool isOnHull;
    bool isProcessed;
    incVertex *next;
    incVertex *prev;
};

struct incEdge
{
    incFace *adjFace[2];
    incVertex *endPoints[2];
    incFace *newFace; //pointer to face about to be added (to remember old structure)
    bool shouldBeRemoved;
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
    incFace *next;
    incFace *prev;
};

struct inc_hull
{
    //Could move head pointers up here...
};

struct inc_context
{
    bool initialized;
    incVertex *vertices;
    int numberOfPoints;
    IHIteration iter;
    mesh m;
    int previousIteration;
};

//Head pointers to each of the three lists
incVertex *incVertices = nullptr;
incEdge *incEdges = nullptr;
incFace *incFaces = nullptr;

//Conflict graph
std::unordered_map<incVertex *, std::unordered_set<incFace *>> vertexConflicts;
std::unordered_map<incFace *, std::unordered_set<incVertex *>> faceConflicts;

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
void incRemoveFromHead(T *head, T pointer)
{
    if (*head)
    {
        if (*head == (*head)->next)
        {
            *head = nullptr;
        }
        else if (pointer == *head)
        {
            *head = (*head)->next;
        }
        pointer->next->prev = pointer->prev;
        pointer->prev->next = pointer->next;
        free(pointer);
        pointer = nullptr;
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
    for (i = 0; i < numberOfPoints; i++)
    {
        v = (incVertex *)malloc(sizeof(incVertex));
        v->duplicate = nullptr;
        v->isOnHull = false;
        v->isProcessed = false;
        v->vIndex = i;
        v->vector = shuffledVertices[i].position;
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
    incAddToHead(&incEdges, e);

    return e;
}

incFace *incCreateNullFace()
{
    incFace *f = (incFace *)malloc(sizeof(incFace));
    f->edge[0] = f->edge[1] = f->edge[2] = nullptr;
    f->vertex[0] = f->vertex[1] = f->vertex[2] = nullptr;
    f->isVisible = false;
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

static bool incIsPointOnPositiveSide(incFace *f, incVertex *v, coord_t epsilon = 0.0000001f)
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
    assert(faceConflicts.emplace(newFace, std::unordered_set<incVertex *>{}).second);
    auto &newConf = faceConflicts.find(newFace)->second;
    auto &oldConf1 = faceConflicts.find(oldFace1)->second;
    auto &oldConf2 = faceConflicts.find(oldFace2)->second;

    for (incVertex *v : oldConf1)
    {
        if (incIsPointOnPositiveSide(newFace, v))
        {
            vertexConflicts.find(v)->second.insert(newFace);
            newConf.insert(v);
        }
    }
    for (incVertex *v : oldConf2)
    {
        if (incIsPointOnPositiveSide(newFace, v))
        {
            vertexConflicts.find(v)->second.insert(newFace);
            newConf.insert(v);
        }
    }
}

void incCleanConflictGraph(std::vector<incFace *> &facesToRemove)
{
    for (incFace *face : facesToRemove)
    {
        //for face, look up vertices, and remove this face from their list
        auto &vertices = faceConflicts.find(face)->second;
        for (incVertex *vertex : vertices)
        {
            vertexConflicts.find(vertex)->second.erase(face);
        }
        //benchmark with and without
        faceConflicts.erase(face);
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

    v0->isProcessed = true;
    v1->isProcessed = true;
    v2->isProcessed = true;

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

incFace *incEnforceCounterClockWise(incFace *newFace, incEdge *e, incVertex *v)
{
    //From Computational Geometry in C page 136
    //We are applying the orientation of the face we are replacing. As we are on the horizon edge, one face is visible, while one is not.
    //As we are creating a new face above the old face, they have to points in common, and the orientation will be the same
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
    //the visible face has right corner as endPoint[0], left corner as endPoint[1], but
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
    return newFace;
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
    newFace = incEnforceCounterClockWise(newFace, e, v);
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

std::vector<incFace *> incAddToHull(incVertex *v)
{
    std::vector<incFace *> facesToRemove;
    bool visible = false;
    auto &vConf = vertexConflicts.find(v)->second;

    for (incFace *face : vConf)
    {
        face->isVisible = visible = true;
    }
    if (!visible)
    {
        //No faces are are visible and we are inside hull. Do nothing for this v
        v->isOnHull = false;
        return facesToRemove;
    }
    incEdge *e;
    incFace *newFace;
    for (incFace *face : vConf)
    {
        //since two faces can share an edge, we could go through all edges twice (although it fails fast). Discussion?
        //can we optimize? should we just walk through all edges again?
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
                    newFace = incMakeConeFace(e, v);
                    e->newFace = newFace;
                    incInitConflictListForFace(newFace, e->adjFace[0], e->adjFace[1]);
                }
            }
        }
        facesToRemove.push_back(face);
    }
    incCleanConflictGraph(facesToRemove);
    return facesToRemove;
}

void incCleanEdges()
{
    //2 loops through all edges could be optimized. Maybe give the conflict faces or the point, and run over edges in the faces? duplicate edges would be removed
    incEdge *e, *tempEdge;
    e = incEdges;
    //replace the pointer to the newly created face. no need to go through all edges
    do
    {
        if (e->newFace)
        {
            if (e->adjFace[0]->isVisible)
            {
                e->adjFace[0] = e->newFace;
            }
            else
            {
                e->adjFace[1] = e->newFace;
            }
            e->newFace = nullptr;
        }
        e = e->next;
    } while (e != incEdges);

    //if the first edge of incEdges should be removed, removeFromHead makes next the new head.
    //Therefore, we cannot use the regular do-while loop. We have to remove until we find "a good starting point", then we can loop as usual.
    while (incEdges && incEdges->shouldBeRemoved)
    {
        e = incEdges;
        incRemoveFromHead(&incEdges, e);
    }
    //we know this e should not be deleted (then it would have been in above for loop), so we can go to next
    e = incEdges->next;
    do
    {
        if (e->shouldBeRemoved)
        {
            tempEdge = e;
            e = e->next;
            incRemoveFromHead(&incEdges, tempEdge);
        }
        else
        {
            e = e->next;
        }
    } while (e != incEdges);
}

void incCleanFaces(std::vector<incFace *> facesToRemove)
{
    for (incFace *face : facesToRemove)
    {
        incRemoveFromHead(&incFaces, face);
    }
}

void incCleanVertices(incVertex **nextVertex)
{
    incVertex *v, *tempVertex;
    incEdge *e = incEdges;
    //no clear indicator on vertices on which to delete.
    //Therefore we go through all current edges in the hull, vertices in those must be on the hull.
    //Optimize to go though the edges of the conflict faces?? adjacent faces have duplicate edges.
    //TODO: actually, every time we create an edge, we should set the two endPoints to on hull. Then we have no need for this while loop
    do
    {
        e->endPoints[0]->isOnHull = e->endPoints[1]->isOnHull = true;
        e = e->next;
    } while (e != incEdges);

    //does it make sense to aggregate ALL visible vertices from the conflict faces, union them and loop over those instead?
    //double while loop again to ensure good starting point.
    while (incVertices && incVertices->isProcessed && !incVertices->isOnHull)
    {
        //if we are about to delete nextVertex (in the full hull, pointer to the next to add (is it Nilly?)), make it the one after
        v = incVertices;
        if (v == *nextVertex)
        {
            *nextVertex = v->next;
        }
        incRemoveFromHead(&incVertices, v);
    }
    //we know this v should not be deleted (then it would have been in above for loop), so we can go to next
    v = incVertices->next;
    do
    {
        if (v->isProcessed && !v->isOnHull)
        {
            tempVertex = v;
            v = v->next;
            if (tempVertex == *nextVertex)
            {
                *nextVertex = tempVertex->next;
            }
            incRemoveFromHead(&incVertices, tempVertex);
        }
        else
        {
            //reset flags
            v->duplicate = nullptr;
            v->isOnHull = false;
            v = v->next;
        }
    } while (v != incVertices);
}

void incCleanStuff(incVertex *nextVertex, std::vector<incFace *> &facesToRemove)
{
    //cleanEdges called before faces, as we need access to isVisible
    incCleanEdges();
    incCleanFaces(facesToRemove);
    incCleanVertices(&nextVertex);
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
    assert(faceConflicts.emplace(f1, std::unordered_set<incVertex *>{}).second);
    assert(faceConflicts.emplace(f2, std::unordered_set<incVertex *>{}).second);
    while (incVertices && incIsPointCoplanar(f1, incVertices))
    {
        incRemoveFromHead(&incVertices, incVertices);
    }
    do
    {
        nextVertex = v->next;

        if (incIsPointCoplanar(f1, v))
        {
            //if point is coplanar, it cannot be on hull
            incRemoveFromHead(&incVertices, v);
        }
        else
        {
            incFace *conflictFace = incIsPointOnPositiveSide(f1, v) ? f1 : f2;
            assert(vertexConflicts.emplace(v, std::unordered_set<incFace *>{}).second);
            auto vConf = vertexConflicts.find(v);
            vConf->second.insert(conflictFace);

            auto fConf = faceConflicts.find(conflictFace);
            fConf->second.insert(v);
        }
        v = nextVertex;
    } while (v != incVertices);
}

void incConstructFullHull()
{
    incCreateBihedron();
    incVertex *v = incVertices;
    incVertex *nextVertex;
    incInitConflictLists();
    do
    {
        nextVertex = v->next;
        if (!v->isProcessed)
        {
            std::vector<incFace *> facesToRemove = incAddToHull(v);
            v->isProcessed = true;
            incCleanStuff(nextVertex, facesToRemove);
        }
        v = nextVertex;
    } while (v != incVertices);
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
