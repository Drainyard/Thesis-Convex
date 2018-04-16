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
    incFace *conflicts;
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
    incVertex *conflicts;
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
     for (i = numberOfPoints - 1; i > 0; i--) { 
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
        v->conflicts = nullptr;
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
    f->conflicts = nullptr;
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

glm::vec3 incComputeFaceNormal(incFace *f)
{
    // Newell's Method
    // https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
    glm::vec3 normal = glm::vec3(0.0f);
    
    for(int i = 0; i < 3; i++)
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

void incEnforceCounterClockWise(incFace *newFace, incEdge *e, incVertex *v)
{
    //From Computational Geometry in C page 136
    incFace *visibleFace;
    if (e->adjFace[0]->isVisible)
    {
        visibleFace = e->adjFace[0];
    }
    else
    {
        visibleFace = e->adjFace[1];
    }
    int i = 0; // Index of e->endPoint[0] in visibleFace
    while (visibleFace->vertex[i] != e->endPoints[0])
    {
        i++;
    } 
    if (visibleFace->vertex[(i + 1) % 3] != e->endPoints[1])
    {
        newFace->vertex[0] = e->endPoints[1];
        newFace->vertex[1] = e->endPoints[0];
    }
    else
    {
        newFace->vertex[0] = e->endPoints[0];
        newFace->vertex[1] = e->endPoints[1];

        incEdge *temp = newFace->edge[1];
        newFace->edge[1] = newFace->edge[2];
        newFace->edge[2] = temp;
    }
    /* This swap is tricky. e is edge[0]. edge[1] is based on endPoint[0],
        edge[2] on endPoint[1].  So if e is oriented "forwards," we
        need to move edge[1] to follow [0], because it precedes. */
    newFace->vertex[2] = v;
    newFace->centerPoint = (newFace->vertex[0]->vector + newFace->vertex[1]->vector + newFace->vertex[2]->vector) / 3.0f;
}

incFace *incMakeConeFace(incEdge *e, incVertex *v)
{
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

void incAddToHull(incVertex *v)
{
    //use this page 247 in compgeo book
    //https://people.csail.mit.edu/indyk/6.838-old/handouts/lec10.pdf
    //https://cw.fel.cvut.cz/wiki/_media/misc/projects/oppa_oi_english/courses/ae4m39vg/lectures/05-convexhull-3d.pdf
    //https://members.loria.fr/MPouget/files/enseignement/webimpa/mpri-randomized.pdf
    incFace *f = incFaces;
    bool visible = false;
    //check sidedness another way???
    do
    {
        if (incIsPointOnPositiveSide(f, v))
        {
            f->isVisible = visible = true;
        }
        f = f->next;
    } while (f != incFaces);

    if (!visible)
    {
        //No faces are visible and we are inside hull. Do nothing
        v->isOnHull = false;
        return;
    }

    incEdge *e, *tempEdge;
    e = incEdges;
    do
    {
        tempEdge = e->next;
        if (e->adjFace[0]->isVisible && e->adjFace[1]->isVisible)
        {
            //both are visible: inside cone and should be removed
            e->shouldBeRemoved = true;
        }
        else if (e->adjFace[0]->isVisible || e->adjFace[1]->isVisible)
        {
            //only one is visible: border edge, erect face for cone
            e->newFace = incMakeConeFace(e, v);
        }
        e = tempEdge;
    } while (e != incEdges);
}

void incCleanEdges()
{
    incEdge *e, *tempEdge;
    e = incEdges;
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

    while (incEdges && incEdges->shouldBeRemoved)
    {
        e = incEdges;
        incRemoveFromHead(&incEdges, e);
    }
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

void incCleanFaces()
{
    incFace *f, *tempFace;
    while (incFaces && incFaces->isVisible)
    {
        f = incFaces;
        incRemoveFromHead(&incFaces, f);
    }
    f = incFaces->next;
    do
    {
        if (f->isVisible)
        {
            tempFace = f;
            f = f->next;
            incRemoveFromHead(&incFaces, tempFace);
        }
        else
        {
            f = f->next;
        }
    } while (f != incFaces);
}

void incCleanVertices(incVertex **nextVertex)
{
    incVertex *v, *tempVertex;
    incEdge *e = incEdges;
    do
    {
        e->endPoints[0]->isOnHull = e->endPoints[1]->isOnHull = true;
        e = e->next;
    } while (e != incEdges);

    while (incVertices && incVertices->isProcessed && !incVertices->isOnHull)
    {
        //if we are about to delete nextVertex, go to the one after
        v = incVertices;
        if (v == *nextVertex)
        {
            *nextVertex = v->next;
        }
        incRemoveFromHead(&incVertices, v);
    }
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
            v = v->next;
        }
    } while (v != incVertices);

    //reset flags
    v = incVertices;
    do
    {
        v->duplicate = nullptr;
        v->isOnHull = false;
        v = v->next;
    } while (v != incVertices);
}

void incCleanStuff(incVertex *nextVertex)
{
    //cleanEdges called before faces, as we need access to isVisible
    incCleanEdges();
    incCleanFaces();
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

void incConstructFullHull()
{
    incCreateBihedron();
    incVertex *v = incVertices;
    incVertex *nextVertex;
    //incInitConflictGraph();
    do
    {
        nextVertex = v->next;
        if (!v->isProcessed)
        {
            incAddToHull(v);
            v->isProcessed = true;
            incCleanStuff(nextVertex);
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
