#ifndef INCREMENTAL_H
#define INCREMENTAL_H

enum class IHIteration
{
    initIH,
    findNextIter,
    doIter
};

struct inc_context
{
    bool initialized;
    vertex *vertices;
    int numberOfPoints;
    IHIteration iter;
    face *currentFace;
    std::vector<int> v;
    mesh m;
    int previousIteration;
};

//edges have faces and faces have edges, so forward declare here
struct incEdge;
struct incFace;

struct incVertex
{
    glm::vec3 vector;
    int vNum; //id?
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
    bool isVisible;
    incFace *next;
    incFace *prev;
};
//Head pointers to each of the three lists
incVertex *incVertices = nullptr;
incEdge *incEdges = nullptr;
incFace *incFaces = nullptr;

template <typename T>
void incAddToHead(T head, T pointer)
{
    if (head)
    {
        pointer->next = head;
        pointer->prev = head->prev;
        head->prev = pointer;
        pointer->prev->next = pointer;
    }
    else
    {
        head = pointer;
        head->next = head->prev = pointer;
    }
};

template <typename T>
void incRemoveFromHead(T head, T pointer)
{
    if (head)
    {
        if (head == head->next)
        {
            head = nullptr;
        }
        else if (pointer == head)
        {
            head = head->next;
        }
        pointer->next->prev = pointer->prev;
        pointer->prev->next = pointer->next;
        free(pointer);
        pointer = nullptr;
    }
};

incVertex *incCreateNullVertex()
{
    incVertex *v = (incVertex *)malloc(sizeof(incVertex));
    v->duplicate = nullptr;
    v->isOnHull = false;
    v->isProcessed = false;
    incAddToHead(incVertices, v);

    return v;
}

incEdge *incCreateNullEdge()
{
    incEdge *e = (incEdge *)malloc(sizeof(incEdge));
    e->adjFace[0] = e->adjFace[1] = e->newFace = nullptr;
    e->endPoints[0] = e->endPoints[1] = nullptr;
    e->shouldBeRemoved = false;
    incAddToHead(incEdges, e);

    return e;
}

incFace *incCreateNullFace()
{
    incFace *f = (incFace *)malloc(sizeof(incFace));
    int i;
    for (i = 0; i < 3; i++)
    {
        f->edge[i] = nullptr;
        f->vertex[i] = nullptr;
    }
    f->isVisible = false;
    incAddToHead(incFaces, f);

    return f;
}

//PREPROCESSING
void incReadVertices()
{ /*
    int vnum = 0;
    while(vertices) 
    {
        v = createNullVertex();
        v->v[0] = x;
        v->v[1] = y;
        v->v[2] = z;
        v->vnum = vnum++;
    }*/
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

    e0->adjFace[0] = e1->adjFace[0] = e2->adjFace[0] = f;

    return f;
}
/*
wiki: For a tetrahedron a, b, c, d, the volume is 
(1/6)|det(a-d,b-d,c-d)| 

The volume is signed; it is positive if (a,b,c) form a counterclockwise circuit
when viewed from the side away from d, so that the face normal determined by the
right-hand rule points toward the outside.

6V(tetra) = 
|a0 a1 a2 1
 b0 b1 b2 1
 c0 c1 c2 1
 d0 d1 d2 1|
 =
-(a2-d2)(b1-d1)(c0-d0)
+(a1-d1)(b2-d2)(c0-d0)
+(a2-d2)(b0-d0)(c1-d1)
-(a0-d0)(b2-d2)(c1-d1)
-(a1-d1)(b0-d0)(c2-d2)
+(a0-d0)(b1-d1)(c2-d2)

TODO optimize with fewer multiplications or use normals?
    dot a vector from face to d with normal a.  
*/
int incVolumeSign(incFace *f, incVertex *d)
{
    incVertex *a = f->vertex[0];
    incVertex *b = f->vertex[1];
    incVertex *c = f->vertex[2];

    double vol = -1 * (a->vector.z - d->vector.z) * (b->vector.y - d->vector.y) * (c->vector.x - d->vector.x) + (a->vector.y - d->vector.y) * (b->vector.z - d->vector.z) * (c->vector.x - d->vector.x) + (a->vector.z - d->vector.z) * (b->vector.x - d->vector.x) * (c->vector.y - d->vector.y) - (a->vector.x - d->vector.x) * (b->vector.z - d->vector.z) * (c->vector.y - d->vector.y) - (a->vector.y - d->vector.y) * (b->vector.x - d->vector.x) * (c->vector.z - d->vector.z) + (a->vector.x - d->vector.x) * (b->vector.y - d->vector.y) * (c->vector.z - d->vector.z);
    //TODO add real epsilon
    if (vol > 0.5)
    {
        return 1;
    }
    if (vol < -0.5)
    {
        return -1;
    }
    return 0;
}

//double sided triangle from points NOT collinear
void incCreateBihedron()
{
    incVertex *v0, *v1, *v2, *v3;
    int vol;

    v0 = incVertices;
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
    v1 = v0->next;
    v2 = v1->next;

    v0->isProcessed = true;
    v1->isProcessed = true;
    v2->isProcessed = true;

    incFace *f0, *f1 = nullptr;

    f0 = incMakeFace(v0, v1, v2, f1);
    f1 = incMakeFace(v2, v1, v0, f0);

    f0->edge[0]->adjFace[1] = f1;
    f0->edge[1]->adjFace[1] = f1;
    f0->edge[2]->adjFace[1] = f1;
    f1->edge[0]->adjFace[1] = f0;
    f1->edge[1]->adjFace[1] = f0;
    f1->edge[2]->adjFace[1] = f0;

    v3 = v2->next;
    vol = incVolumeSign(f0, v3);
    while (!vol)
    {
        v3 = v3->next;
        if (v3 == v0)
        {
            //ERROR ONLY COPLANAR PLANAR
            printf("incCreateBihedron - coplanar points");
            exit(0);
        }
        vol = incVolumeSign(f0, v3);
    }
    incVertices = v3;
}

void incEnforceCounterClockWise(incFace *newFace, incEdge *e, incVertex *v)
{
    incEdge *temp;
    incFace *visibleFace;
    if (e->adjFace[0]->isVisible)
    {
        visibleFace = e->adjFace[0];
    }
    else
    {
        visibleFace = e->adjFace[1];
    }
    for (int i = 0; visibleFace->vertex[i] != e->endPoints[0]; ++i)
    {
        if (visibleFace->vertex[(i + 1) % 3] != e->endPoints[1])
        {
            newFace->vertex[0] = e->endPoints[1];
            newFace->vertex[1] = e->endPoints[0];
        }
        else
        {
            newFace->vertex[0] = e->endPoints[0];
            newFace->vertex[1] = e->endPoints[1];

            temp = newFace->edge[1];
            newFace->edge[1] = newFace->edge[2];
            newFace->edge[2] = temp;
        }
        newFace->vertex[2] = v;
    }
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
    incFace *f = incFaces;
    bool visible = false;

    do
    {
        if (incVolumeSign(f, v))
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

    incEdge *e, *nextEdge;
    e = nextEdge = incEdges;
    do
    {
        nextEdge = e->next;
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
        e = nextEdge;
    } while (e != incEdges);
}

void incCleanEdges()
{
    incEdge *headEdge, *tempEdge;
    headEdge = incEdges;
    do
    {
        if (headEdge->newFace)
        {
            if (headEdge->adjFace[0]->isVisible)
            {
                headEdge->adjFace[0] = headEdge->newFace;
            }
            else
            {
                headEdge->adjFace[1] = headEdge->newFace;
            }
            headEdge->newFace = nullptr;
        }
        headEdge = headEdge->next;
    } while (headEdge != incEdges);

    while (incEdges && incEdges->shouldBeRemoved)
    {
        headEdge = incEdges;
        incRemoveFromHead(incEdges, headEdge);
    }
    headEdge = incEdges->next;
    do
    {
        if (headEdge->shouldBeRemoved)
        {
            tempEdge = headEdge;
            headEdge = headEdge->next;
            incRemoveFromHead(incEdges, tempEdge);
        }
        else
        {
            headEdge = headEdge->next;
        }
    } while (headEdge != incEdges);
}

void incCleanFaces()
{
    incFace *headFace, *tempFace;
    while (incFaces && incFaces->isVisible)
    {
        headFace = incFaces;
        incRemoveFromHead(incFaces, headFace);
    }
    headFace = incFaces->next;
    do
    {
        if (headFace->isVisible)
        {
            tempFace = headFace;
            headFace = headFace->next;
            incRemoveFromHead(incFaces, tempFace);
        }
        else
        {
            headFace = headFace->next;
        }
    } while (headFace != incFaces);
}

void incCleanVertices(incVertex *nextVertex)
{
    incVertex *headVertex, *tempVertex;
    incEdge *headEdge = incEdges;
    do
    {
        headEdge->endPoints[0]->isOnHull = headEdge->endPoints[1]->isOnHull = true;
        headEdge = headEdge->next;
    } while (headEdge != incEdges);

    while (incVertices && incVertices->isProcessed && !incVertices->isOnHull)
    {
        //if we are about to delete nextVertex, go to the one after
        headVertex = incVertices;
        //he dereferences here *nextVertex???
        if (headVertex == nextVertex)
            nextVertex = headVertex->next;
        incRemoveFromHead(incVertices, headVertex);
    }
    headVertex = incVertices->next;
    do
    {
        if (headVertex->isProcessed && !headVertex->isOnHull)
        {
            tempVertex = headVertex;
            headVertex = headVertex->next;
            //he dereferences here *nextVertex???
            if (tempVertex == nextVertex)
            {
                nextVertex = tempVertex->next;
            }
            incAddToHead(incVertices, tempVertex);
        }
        else
        {
            headVertex = headVertex->next;
        }
    } while (headVertex != incVertices);

    headVertex = incVertices;
    do
    {
        headVertex->duplicate = nullptr;
        headVertex->isOnHull = false;
        headVertex = headVertex->next;
    } while (headVertex != incVertices);
}

void incCleanStuff(incVertex *nextVertex)
{
    //cleanedges called before faces, as we need acces to isVisible
    incCleanEdges();
    incCleanFaces();
    incCleanVertices(nextVertex);
}

void incConstructHull()
{
    incVertex *v = incVertices;
    incVertex *nextVertex;
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

    /*
mesh& InitIncHull(render_context &renderContext, vertex* vertices, int numVertices)
{
    auto &m = InitEmptyMesh(renderContext);
    m.position = glm::vec3(0.0f);
    m.scale = glm::vec3(globalScale);
    m.numFaces = 0;
    m.facesSize = 0;

    return m;
}

face* IncHullFindNextIteration(render_context &renderContext, mesh &m)
{
    face* res = nullptr;

    return res;
}

void IncHullIteration(render_context &renderContext, mesh &m, vertex *vertices, int numVertices)
{
    
}

void IncHull(render_context& renderContext, inc_context& incContext)
{
    incContext.currentFace = nullptr;
    incContext.m = InitIncHull(renderContext, incContext.vertices, incContext.numberOfPoints);
    incContext.v.clear();
    incContext.previousIteration = 0;
}

void InitializeIncContext(inc_context &incContext, vertex *vertices, int numberOfPoints)
{
    if (incContext.vertices)
    {
        free(incContext.vertices);
    }

    incContext.v.clear();

    incContext.vertices = CopyVertices(vertices, numberOfPoints);
    incContext.numberOfPoints = numberOfPoints;
    incContext.iter = IHIteration::initIH;
    incContext.currentFace = 0;
    incContext.previousIteration = 0;
    incContext.initialized = true;
}

void IncHullStep(render_context &renderContext, inc_context &context)
{
    switch (context.iter)
    {
    case IHIteration::initIH:
    {
        context.m = InitIncHull(renderContext, context.vertices, context.numberOfPoints);
        context.iter = IHIteration::findNextIter;
    }
    break;
    case IHIteration::findNextIter:
    {
    }
    break;
    case IHIteration::doIter:
    {
        if (context.currentFace)
        {
            Log("Doing iteration\n");
            IncHullIteration(renderContext, context.m, context.vertices, context.numberOfPoints);
            context.iter = IHIteration::findNextIter;
            context.v.clear();
        }
    }
    break;
    }
}*/

#endif
