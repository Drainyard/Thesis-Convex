 mesh& NaiveConvexHull(render_context& renderContext, vertex* vertices, int numVertices)
 {
     mesh& m = InitEmptyMesh(renderContext);
     m.position = glm::vec3(0.0f);
     m.scale = glm::vec3(globalScale);
     m.numFaces = 0;
     m.facesSize = 0;
     
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
                 for(int p = 0; p < numVertices; p++)
                 {
                     if(p == j || p == k || p == i)
                         continue;
                     face f;
                     f.vertices[0] = i;
                     f.vertices[1] = j;
                     f.vertices[2] = k;
                     f.faceNormal = ComputeFaceNormal(f, vertices);
                     if(IsPointOnPositiveSide(f, vertices[p], vertices))
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
                     AddFace(renderContext, m, i, j, k, vertices, numVertices);
                 }
             }
         }
     }
     return m;
 }
 
