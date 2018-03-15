#ifndef INCREMENTAL_H
#define INCREMENTAL_H

typedef struct iVertexStruct isVertex;
typedef isVertex* iVertex;

typedef struct iEdgeStruct isEdge;
typedef isEdge* iEdge;

typedef struct iFaceStruct isFace;
typedef isFace* iFace;

struct iVertexStruct {
    int v[3];
    int vNum;//id?
    iVertex next, prev;
};

struct iEdgeStruct {
    iFace adjFace[2];
    iVertex endPoints[2];
    iEdge next, prev;
};

struct iFaceStruct {
    iEdge edge[3];
    iVertex vertex[3];
    iFace next, prev;
};

iVertex iVertices = nullptr;
iEdge iEdges = nullptr;
iFace iFaces = nullptr;

#endif
