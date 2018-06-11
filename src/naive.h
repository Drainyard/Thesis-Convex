struct ch_face
{
    int v1;
    int v2;
    int v3;
};

mesh& NaiveConvexHull(render_context& renderContext, vertex* vertices, int numVertices)
{
    mesh& m = InitEmptyMesh(renderContext);
    m.position = glm::vec3(0.0f);
    m.scale = glm::vec3(globalScale);
    m.numFaces = 0;
    m.facesSize = 0;
    
    auto extremePoints = FindExtremePoints(vertices, numVertices);
    auto epsilon = 3 * (extremePoints[1] + extremePoints[3] + extremePoints[5]) * FLT_EPSILON;
    
    for(int i = 0; i < numVertices; i++)
    {
        for(int j = 0; j < numVertices; j++)
        {
            if(j == i)
                continue;
            for(int k = 0; k < numVertices; k++)
            {
                if(j == k || k == i)
                    continue;
                
                bool above = true;
                bool firstPoint = true;
                bool partOfConvex = true;
                
                auto duplicate = false;
                
                for(int fIndex = 0; fIndex < m.numFaces; fIndex++)
                {
                    if((m.faces[fIndex].vertices[0] == i || m.faces[fIndex].vertices[0] == j || m.faces[fIndex].vertices[0] == k) && (m.faces[fIndex].vertices[1] == i || m.faces[fIndex].vertices[1] == j || m.faces[fIndex].vertices[1] == k) && (m.faces[fIndex].vertices[2] == i || m.faces[fIndex].vertices[2] == j || m.faces[fIndex].vertices[2] == k))
                    {
                        duplicate = true;
                        break;
                    }
                }
                
                if(duplicate)
                    continue;
                
                face f;
                f.vertices[0] = i;
                f.vertices[1] = j;
                f.vertices[2] = k;
                f.faceNormal = ComputeFaceNormal(f, vertices);
                
                for(int p = 0; p < numVertices; p++)
                {
                    if(p == j || p == k || p == i)
                        continue;
                    
                    if(IsPointOnPositiveSide(f, vertices[p], epsilon))
                    {
                        if(firstPoint)
                        {
                            above = true;
                        }
                        
                        if(!above)
                        {
                            partOfConvex = false;
                            break;
                        }
                        firstPoint = false;
                    }
                    else
                    {
                        if(firstPoint)
                        {
                            above = false;
                        }
                        
                        if(above)
                        {
                            partOfConvex = false;
                            break;
                        }
                        firstPoint = false;
                    }
                    
                }
                if(partOfConvex)
                {
                    AddFace(m, i, j, k, vertices, numVertices);
                }
            }
        }
    }
    return m;
}

