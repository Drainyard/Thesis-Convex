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

struct DacEvent
{
    int eIndex;
    coord_t timeValue;
};

struct DacContext
{
    bool initialized;
    DacVertex *vertices;
    int numberOfPoints;
    std::vector<DacFace *> faces;
    Mesh *m;
    DacVertex *upperP;
    
    struct
    {
        int createdFaces;
        int processedVertices;
        unsigned long long sidednessQueries;
        int facesOnHull;
        int verticesOnHull;
        unsigned long long timeSpent;
    } processingState;
};

//no need for such a big inf
const coord_t INF = 1e30f;
static DacVertex nil = {glm::vec3(INF, INF, INF), 0, nullptr, nullptr};
DacVertex *NIL = &nil;

DacVertex *sort(DacVertex points[], int numberOfPoints)
{
    DacVertex *l, *r, *c, head;
    
    if (numberOfPoints == 1)
    {
        points[0].next = NIL;
        return points;
    }
    l = sort(points, numberOfPoints / 2);
    r = sort(points + (numberOfPoints / 2), numberOfPoints - (numberOfPoints / 2));
    c = &head;
    do
    {
        if (l->position.x < r->position.x)
        {
            c = c->next = l;
            l = l->next;
        }
        else
        {
            c = c->next = r;
            r = r->next;
        }
    } while (c != NIL);
    return head.next;
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
    }
}

void printHull(DacContext &dacContext, DacVertex **events)
{
    for (int i = 0; events[i] != NIL; i++)
    {
        DacFace *newFace = dacCreateFaceFromPoints(events[i]->prev, events[i], events[i]->next);
        dacContext.faces.push_back(newFace);
        events[i]->act();
    }
}

coord_t orient(DacVertex *p, DacVertex *q, DacVertex *r)
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
coord_t time(DacVertex *p, DacVertex *q, DacVertex *r)
{
    if (p == NIL || q == NIL || r == NIL)
    {
        return INF;
    }
    
    return ((q->position.x - p->position.x) * (r->position.z - p->position.z) - (r->position.x - p->position.x) * (q->position.z - p->position.z)) / orient(p, q, r);
}

void dacHull(DacContext &dacContext, DacVertex *list, int n, DacVertex **A, DacVertex **B, bool lower)
{
    DacVertex *u, *v, *mid;
    int i, j, k, l, minl;
    
    DacEvent oldTime;
    DacEvent newTime;
    DacEvent events[6];
    //suppress warning
    minl = -1;
    
    if (n == 1)
    {
        A[0] = list->prev = list->next = NIL;
        return;
    }
    
    //find middle
    for (u = list, i = 0; i < n / 2 - 1; u = u->next, i++)
        ;
    mid = v = u->next;
    // recurse
    dacHull(dacContext, list, n / 2, B, A, lower);
    dacHull(dacContext, mid, n - n / 2, &B[n / 2 * 2], &A[n / 2 * 2], lower);
    
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
    
    // possible to overflow this
    int eIndex = -1;
    
    // merge by tracking bridge uv over time
    // infinite loop until no insertion/deletion events occur
    for (i = k = 0, j = n / 2 * 2, oldTime.timeValue = -INF; ; oldTime = newTime)
    {
        if (lower)
        {
            events[0].timeValue = time(B[i]->prev, B[i], B[i]->next);
            events[1].timeValue = time(B[j]->prev, B[j], B[j]->next);
            events[2].timeValue = time(u->prev, u, v);
            events[3].timeValue = time(u, u->next, v);
            events[4].timeValue = time(u, v, v->next);
            events[5].timeValue = time(u, v->prev, v);
        }
        else
        {
            events[0].timeValue = -time(B[i]->prev, B[i], B[i]->next);
            events[1].timeValue = -time(B[j]->prev, B[j], B[j]->next);
            events[2].timeValue = -time(u->prev, u, v);
            events[3].timeValue = -time(u, u->next, v);
            events[4].timeValue = -time(u, v, v->next);
            events[5].timeValue = -time(u, v->prev, v);
        }
        //we find the movies in chronological time
        for (newTime.timeValue = INF, l = 0; l < 6; l++)
        {
            if (events[l].timeValue > oldTime.timeValue &&
                events[l].timeValue < newTime.timeValue)
            {
                minl = l;
                newTime = events[l];
            }
        }
        
        //if newTime==INF, no insertion/deletion events occured, and we break for loop
        if (newTime.timeValue == INF)
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
    for (k--; k >= 0; k--)
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
    for (DacFace *f : context.faces)
    {
        free(f);
    }
    context.faces.clear();
    free(context.upperP);
    return *context.m;
}

void dacConstructFullHull(DacContext &dacContext)
{
    //do the stuff for divide and conquer
    int i;
    int n = dacContext.numberOfPoints;
    
    DacVertex *P = dacContext.vertices;
    dacContext.upperP = (DacVertex *)malloc(sizeof(DacVertex) * n);
    memcpy(dacContext.upperP, P, sizeof(DacVertex) * n);
    DacVertex *list = sort(P, n);
    
    //Each vertex is inserted at most once and deleted at most once, so at most 2n events (facets).
    DacVertex **A = (DacVertex**)malloc(2 * n * sizeof(DacVertex));
    //work array
    DacVertex **B = (DacVertex**)malloc(2 * n * sizeof(DacVertex));
     dacHull(dacContext, list, n, A, B, true);
    
     //create faces by processing the events in event array A
    printHull(dacContext, A);

    
    DacVertex *upperList = sort(dacContext.upperP, n);
    DacVertex **C = (DacVertex**)malloc(2 * n * sizeof(DacVertex));
    //work array
    DacVertex **D = (DacVertex**)malloc(2 * n * sizeof(DacVertex));
    dacHull(dacContext, upperList, n, C, D, false);
    
    printHull(dacContext, C);
    
    //dirty normal check
    for (size_t j = 0; j != dacContext.faces.size(); ++j)
    {
        DacFace *face = dacContext.faces[j];
        DacFace *otherFace = dacContext.faces[(j + 2671) % dacContext.faces.size()];
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
    dacContext.processingState.facesOnHull = dacContext.faces.size();
    free(A);
    free(B);
    free(C);
    free(D);
}

void dacHullStep(DacContext &dacContext)
{
    //do the stuff for one divide and conquer iteration
}

void dacInitializeContext(DacContext &dacContext, Vertex *vertices, int numberOfPoints)
{
    if (dacContext.vertices)
    {
        free(dacContext.vertices);
    }
    dacContext.numberOfPoints = numberOfPoints;
    dacContext.initialized = true;
    nil.position = glm::vec3(INF, INF, INF); 
    nil.vIndex = 0; 
    nil.next = nullptr; 
    nil.prev = nullptr;
    dacCopyVertices(dacContext, vertices, numberOfPoints);
}

#endif
