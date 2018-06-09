#ifndef DIVIDE_H
#define DIVIDE_H

struct DacVertex
{
    glm::vec3 position;
    int vIndex;
    DacVertex *next;
    DacVertex *prev;
    void act()
    {
        //insert
        if (prev->next != this)
        {
            prev->next = next->prev = this;
        }
        //delete
        else
        {
            prev->next = next;
            next->prev = prev;
        }
    }
};

struct DacFace
{
    DacVertex *vertex[3];
    glm::vec3 normal;
    glm::vec3 centerPoint;
};

struct DacContext
{
    bool initialized;
    DacVertex *vertices;
    int numberOfPoints;
    std::vector<DacFace *> faces;
    Mesh *m;
    DacVertex *sortedP;
    DacVertex *sortedUpperP;
    DacVertex **A;
    DacVertex **B;
    bool lower;

    struct
    {
        int createdFaces;
        int processedVertices;
        unsigned long long sidednessQueries;
        int facesOnHull;
        int verticesOnHull;
        unsigned long long timeSpent;
    } processingState;

    struct
    {
        int mergesLeft;
        int offset;
        bool swap;
        bool initAB;
    } stepInfo;
};

//no need for such a big inf
const coord_t INF = 1e30f;
static DacVertex nil = {glm::vec3(INF, INF, INF), 0, nullptr, nullptr};
DacVertex *NIL = &nil;

//bottom up merge sort
void merge(DacVertex *A, DacVertex *B, int size, int left, int mid)
{
    int right = mid + mid - left;
    if (right > size)
        right = size;
    int i = left;
    int j = mid;
    int k = left;
    while (i < mid && j < right)
    {
        if (A[i].position.x < A[j].position.x)
            B[k++] = A[i++];
        else
            B[k++] = A[j++];
    }
    while (i < mid)
        B[k++] = A[i++];
    while (j < right)
        B[k++] = A[j++];
    for (i = left; i < right; ++i)
        A[i] = B[i];
}

void sort(DacVertex A[], int n)
{
    int subsize, left, mid;
    DacVertex *B = new DacVertex[n];
    for (subsize = 1; subsize < n; subsize *= 2)
        for (left = 0, mid = subsize; mid < n; left = mid + subsize, mid = left + subsize)
            merge(A, B, n, left, mid);
    delete[] B;
}

glm::vec3 dacComputeFaceNormal(DacFace *f)
{
    // Newell's Method
    // https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
    glm::vec3 normal = glm::vec3(0.0);

    for (int i = 0; i < 3; i++)
    {
        glm::vec3 current = f->vertex[i]->position;
        glm::vec3 next = f->vertex[(i + 1) % 3]->position;

        normal.x = normal.x + (current.y - next.y) * (current.z + next.z);
        normal.y = normal.y + (current.z - next.z) * (current.x + next.x);
        normal.z = normal.z + (current.x - next.x) * (current.y + next.y);
    }

    return glm::normalize(normal);
}

static bool dacIsPointOnPositiveSide(DacFace *f, DacVertex *v, coord_t epsilon = 0.0)
{
    auto d = glm::dot(f->normal, v->position - f->centerPoint);
    return d > epsilon;
}

DacFace *dacCreateFaceFromPoints(DacVertex *u, DacVertex *v, DacVertex *w)
{
    DacFace *f = (DacFace *)malloc(sizeof(DacFace));
    f->vertex[0] = u;
    f->vertex[1] = v;
    f->vertex[2] = w;
    f->centerPoint = (u->position + v->position + w->position) / 3.0f;
    f->normal = dacComputeFaceNormal(f);
    return f;
}

static void dacCopyVertices(DacContext &dac, Vertex *vertices, int numberOfPoints)
{
    dac.vertices = (DacVertex *)malloc(sizeof(DacVertex) * numberOfPoints);
    for (int i = 0; i < numberOfPoints; i++)
    {
        dac.vertices[i].vIndex = i;
        dac.vertices[i].position = vertices[i].position;
        dac.vertices[i].prev = dac.vertices[i].next = NIL;
    }
}

void createFaces(DacContext &dacContext, DacVertex **events)
{
    int i;
    for (i = 0; events[i] != NIL; i++)
    {
        DacFace *newFace = dacCreateFaceFromPoints(events[i]->prev, events[i], events[i]->next);
        dacContext.faces.push_back(newFace);
        events[i]->act();
    }

    //dirty normal check
    for (size_t j = 0; j != dacContext.faces.size(); ++j)
    {
        DacFace *face = dacContext.faces[j];
        DacFace *otherFace = dacContext.faces[(j + 199) % dacContext.faces.size()];
        for (i = 0; i < 3; ++i)
        {
            DacVertex *v = otherFace->vertex[i];
            if (v != face->vertex[0] || v != face->vertex[1] || v != face->vertex[2])
            {
                if (dacIsPointOnPositiveSide(face, v))
                {
                    DacVertex *u = face->vertex[0];
                    DacVertex *w = face->vertex[2];
                    face->vertex[0] = w;
                    face->vertex[2] = u;
                    face->normal = dacComputeFaceNormal(face);
                    break;
                }
            }
        }
    }
}

double orient(DacVertex *p, DacVertex *q, DacVertex *r)
{
    //orient(p,q,r)=det(1  p_x  p_y)
    //                 (1  q_x  q_y)
    //                 (1  r_x  r_y)
    //the determinant gives twice the signed area of the triangle formed by p, q and r
    //SO, this tests if three points do a ccw turn at time -INF

    if (p == NIL || q == NIL || r == NIL)
    {
        return 1.0;
    }
    return (q->position.x - p->position.x) * (r->position.y - p->position.y) - (r->position.x - p->position.x) * (q->position.y - p->position.y);
}

//by dividing with orient, we can determine the time when three points switch from cw to ccw (or the other way)
double time(DacVertex *p, DacVertex *q, DacVertex *r)
{
    if (p == NIL || q == NIL || r == NIL)
    {
        return INF;
    }

    return ((q->position.x - p->position.x) * (r->position.z - p->position.z) - (r->position.x - p->position.x) * (q->position.z - p->position.z)) / orient(p, q, r);
}

void dacHull(DacContext &dacContext, DacVertex *list, DacVertex **A, DacVertex **B, int offset, int mergeIteration, bool lower)
{
    int leftSideIndex = mergeIteration * offset;
    int rightSideIndex = (leftSideIndex + ((mergeIteration + 1) * offset)) / 2;
    int eventOffset = leftSideIndex * 2;

    //Chan base case
    if (offset == 1)
    {
        A[eventOffset]->next = A[eventOffset]->prev = A[eventOffset] = NIL;
        return;
    }

    DacVertex *u, *v, *mid;
    int i, j, k, l, minl;

    // find last u in L
    for (u = &list[leftSideIndex]; u->next != NIL; u = u->next)
        ;
    mid = v = &list[rightSideIndex];

    double oldTime;
    double newTime;
    double events[6];
    //suppress warning
    minl = -1;

    // find initial bridge
    for (;;)
    {
        if (orient(u, v, v->next) < 0.0)
        {
            v = v->next;
        }
        else if (orient(u->prev, u, v) < 0.0)
        {
            u = u->prev;
        }
        else
            break;
    }

    // merge by tracking bridge uv over time
    // infinite loop until no insertion/deletion events occur
    for (i = leftSideIndex * 2, j = rightSideIndex * 2, k = eventOffset, oldTime = -INF;; oldTime = newTime)
    {
        if (lower)
        {
            events[0] = time(B[i]->prev, B[i], B[i]->next);
            events[1] = time(B[j]->prev, B[j], B[j]->next);
            events[2] = time(u->prev, u, v);
            events[3] = time(u, u->next, v);
            events[4] = time(u, v, v->next);
            events[5] = time(u, v->prev, v);
        }
        else
        {
            events[0] = -time(B[i]->prev, B[i], B[i]->next);
            events[1] = -time(B[j]->prev, B[j], B[j]->next);
            events[2] = -time(u->prev, u, v);
            events[3] = -time(u, u->next, v);
            events[4] = -time(u, v, v->next);
            events[5] = -time(u, v->prev, v);
        }
        //we find the movies in chronological time
        for (newTime = INF, l = 0; l < 6; l++)
        {
            if (events[l] > oldTime && events[l] < newTime)
            {
                minl = l;
                newTime = events[l];
            }
        }

        //if newTime==INF, no insertion/deletion events occured, and we break for loop
        if (newTime == INF)
        {
            break;
        }

        //act on the smallest time value
        switch (minl)
        {
        case 0:
        {
            //insert or delete of w in L. If w is to the left of u, insert or delete w in A.
            if (B[i]->position.x < u->position.x)
            {
                A[k++] = B[i];
            }
            B[i++]->act();
            break;
        }
        case 1:
        {
            //insert or delete of w in R. If w is to the right of v, insert or delete w in A.
            if (B[j]->position.x > v->position.x)
            {
                A[k++] = B[j];
            }
            B[j++]->act();
            break;
        }
        case 2:
        {
            //u->prev, u, v was ccw and has turned cw, so u->prev and v is the new bridge, and we delete u in A.
            A[k++] = u;
            u = u->prev;
            break;
        }
        case 3:
        {
            //u, u->next, v, was cw and turned ccw, so u->next and v is the new bridge, and we insert u->next between u and v.
            A[k++] = u = u->next;
            break;
        }
        case 4:
        {
            //u, v, v->next was ccw and turned cw, so u and v->next is the new bridge, and we delete v in A.
            A[k++] = v;
            v = v->next;
            break;
        }
        case 5:
        {
            //u, v->prev, v, was cw and turned ccw, so u and v->prev is the new bridge, and we insert v->prev between u and v.
            A[k++] = v = v->prev;
            break;
        }
        }
    }
    A[k] = NIL;

    //connect the bridge uv
    u->next = v;
    v->prev = u;

    dacContext.processingState.createdFaces += k;
    // now go back in time to update pointers
    // during insertion of q between p and r, we cannot store p and r in the prev and next fields, as they are still in use in L and R
    for (k--; k >= eventOffset; k--)
    {
        if (A[k]->position.x <= u->position.x || A[k]->position.x >= v->position.x)
        {
            A[k]->act();
            if (A[k] == u)
            {
                u = u->prev;
            }
            else if (A[k] == v)
            {
                v = v->next;
            }
        }
        else
        {
            u->next = A[k];
            A[k]->prev = u;
            v->prev = A[k];
            A[k]->next = v;
            if (A[k]->position.x < mid->position.x)
            {
                u = A[k];
            }
            else
            {
                v = A[k];
            }
        }
    }
}

void dacConstructFullHull(DacContext &dacContext)
{
    int n = dacContext.numberOfPoints;
    DacVertex *P = dacContext.sortedP;

    int i;
    bool swap;
    int offset, mergesLeft;

    for (int m = 0; m < 2; m++)
    {
        dacContext.A = (DacVertex **)malloc(2 * n * sizeof(DacVertex));
        dacContext.B = (DacVertex **)malloc(2 * n * sizeof(DacVertex));

        offset = 1;
        swap = true;
        mergesLeft = n;
        while (mergesLeft > 0)
        {
            for (i = 0; i < mergesLeft; i++)
            {
                if (swap)
                {
                    dacHull(dacContext, P, dacContext.A, dacContext.B, offset, i, dacContext.lower);
                }
                else
                {
                    dacHull(dacContext, P, dacContext.B, dacContext.A, offset, i, dacContext.lower);
                }
            }
            swap = !swap;
            offset *= 2;
            mergesLeft /= 2;
        }

        swap ? createFaces(dacContext, dacContext.B) : createFaces(dacContext, dacContext.A);
        free(dacContext.A);
        free(dacContext.B);

        P = dacContext.sortedUpperP;
        dacContext.lower = false;
    }

    dacContext.processingState.facesOnHull = (int)dacContext.faces.size();
}

void dacHullStep(DacContext &dacContext)
{
    if (!dacContext.initialized)
    {
        return;
    }
    int n = dacContext.numberOfPoints;
    DacVertex *P;
    if (dacContext.lower)
    {
        P = dacContext.sortedP;
    }
    else
    {
        P = dacContext.sortedUpperP;
    }
    if (dacContext.stepInfo.initAB)
    {
        dacContext.A = (DacVertex **)malloc(2 * n * sizeof(DacVertex));
        dacContext.B = (DacVertex **)malloc(2 * n * sizeof(DacVertex));

        dacContext.stepInfo.offset = 1;
        dacContext.stepInfo.swap = true;
        dacContext.stepInfo.mergesLeft = n;

        dacContext.stepInfo.initAB = false;
    }
    else
    {
        dacContext.stepInfo.swap = !dacContext.stepInfo.swap;
        dacContext.stepInfo.offset *= 2;
        dacContext.stepInfo.mergesLeft /= 2;
    }

    if (dacContext.stepInfo.mergesLeft > 0)
    {
        for (int i = 0; i < dacContext.stepInfo.mergesLeft; i++)
        {
            if (dacContext.stepInfo.swap)
            {
                dacHull(dacContext, P, dacContext.A, dacContext.B, dacContext.stepInfo.offset, i, dacContext.lower);
            }
            else
            {
                dacHull(dacContext, P, dacContext.B, dacContext.A, dacContext.stepInfo.offset, i, dacContext.lower);
            }
        }
        //TODO need to copy all the objects or else we can't act() on the pointers!!!
        dacContext.stepInfo.swap ? createFaces(dacContext, dacContext.A) : createFaces(dacContext, dacContext.B);
    }
    else
    {
        dacContext.stepInfo.swap ? createFaces(dacContext, dacContext.B) : createFaces(dacContext, dacContext.A);
        free(dacContext.A);
        free(dacContext.B);
        if (!dacContext.lower)
        {
            dacContext.initialized = false;
            return;
        }
        dacContext.lower = false;
        dacContext.stepInfo.initAB = true;
    }
}

void dacInitializeContext(DacContext &dacContext, Vertex *vertices, int n)
{
    if (dacContext.vertices)
    {
        free(dacContext.vertices);
    }
    if (dacContext.sortedP)
    {
        free(dacContext.sortedP);
    }
    if (dacContext.sortedUpperP)
    {
        free(dacContext.sortedUpperP);
    }
    for (DacFace *f : dacContext.faces)
    {
        free(f);
    }
    dacContext.faces.clear();

    dacContext.numberOfPoints = n;
    dacContext.initialized = true;
    nil.position = glm::vec3(INF, INF, INF);
    nil.vIndex = 0;
    nil.next = nullptr;
    nil.prev = nullptr;
    dacCopyVertices(dacContext, vertices, n);

    dacContext.sortedP = (DacVertex *)malloc(sizeof(DacVertex) * n);
    memcpy(dacContext.sortedP, dacContext.vertices, sizeof(DacVertex) * n);

    sort(dacContext.sortedP, n);
    dacContext.sortedUpperP = (DacVertex *)malloc(sizeof(DacVertex) * n);
    memcpy(dacContext.sortedUpperP, dacContext.sortedP, sizeof(DacVertex) * n);
    dacContext.lower = true;
    dacContext.stepInfo.initAB = true;
}

Mesh &dacConvertToMesh(DacContext &context, RenderContext &renderContext)
{
    if (!context.m)
    {
        context.m = &InitEmptyMesh(renderContext);
    }

    context.m->faces.clear();
    context.m->position = glm::vec3(0.0);
    context.m->scale = glm::vec3(globalScale);
    context.m->dirty = true;

    for (const auto &f : context.faces)
    {
        Face newFace = {};
        for (int i = 0; i < 3; i++)
        {
            Vertex newVertex = {};
            newVertex.position = f->vertex[i]->position;
            newVertex.vertexIndex = f->vertex[i]->vIndex;
            addToList(newFace.vertices, newVertex);
        }
        newFace.faceColor = rgb(251, 255, 135);
        newFace.faceColor.w = 0.5f;
        newFace.faceNormal = f->normal;
        newFace.centerPoint = f->centerPoint;
        context.m->faces.push_back(newFace);
    }

    return *context.m;
}

#endif
