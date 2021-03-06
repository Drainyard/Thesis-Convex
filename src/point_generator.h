#ifndef POINT_GENERATOR_H
#define POINT_GENERATOR_H

struct PointGenerator;

#define GENERATOR_FUNCTION(name) Vertex* name(PointGenerator& pointGenerator, glm::vec3 offset)
typedef GENERATOR_FUNCTION(GeneratorFunction);

enum GeneratorType
{
    InSphere,
    OnSphere,
    InCube,
    NormalizedSphere,
    ManyInternal,
    Clusters
};

struct PointGenerator
{
    int numberOfPoints;
    GeneratorType type;
    std::mt19937_64 gen;
    std::uniform_real_distribution<coord_t> d;
    coord_t min;
    coord_t max;
};

struct TestSet
{
    int *testSet;
    size_t count;
    int iterations;
    GeneratorType genType;
};

struct ConfigData
{
    int numberOfPoints;
    Vertex *vertices;
    GeneratorType genType;
    
    List<TestSet> qhTestSets;
    List<TestSet> incTestSets;
    List<TestSet> dacTestSets;
    
    Mesh loadedMesh;
    Vertex *meshVertices;
    int verticesInMesh;
};

void readTestSet(const char *filename, TestSet &testSet)
{
    FILE *f = fopen(filename, "r");
    if(f)
    {
        testSet.count = 0;
        testSet.iterations = 0;
        
        char buf[64];
        int i = 0;
        while(getData(buf, sizeof(buf), f))
        {
            if(startsWith(buf, "gen"))
            {
                int genType;
                sscanf(buf, "gen %d", &genType);
                if(genType > GeneratorType::ManyInternal || genType < 0)
                {
                    genType = GeneratorType::InSphere;
                }
                testSet.genType = (GeneratorType)genType;
            }
            else if(startsWith(buf, "iterations"))
            {
                sscanf(buf, "iterations %d", &testSet.iterations);
            }
            else if(!startsWith(buf, "#"))
            {
                if(testSet.count == 0)
                {
                    testSet.count = (size_t)countLinesFromCurrent(f) + 1;
                    if(testSet.count == 0)
                    {
                        break;
                    }
                    testSet.testSet = (int*)malloc(sizeof(int) * testSet.count);
                    rewind(f);
                    getData(buf, sizeof(buf), f);
                    getData(buf, sizeof(buf), f);
                    getData(buf, sizeof(buf), f);
                }
                
                sscanf(buf, "%d", &testSet.testSet[i++]);
            }
        }
        fclose(f);
    }
}

void readTestSets(char *buffer, size_t bufSize, List<TestSet> &list, FILE *f)
{
    init(list);
    auto prevPos = ftell(f);
    while(getData(buffer, bufSize, f))
    {
        if(startsWith(buffer, "set"))
        {
            char filename[64];
            sscanf(buffer, "set %s", filename);
            printf("Set: %s\n", filename);
            TestSet newSet = {};
            readTestSet(filename, newSet);
            addToList(list, newSet);
        }
        else if(!startsWith(buffer, "#"))
        {
            fseek(f, prevPos, SEEK_SET);
            break;
        }
    }
    prevPos = ftell(f);
}


static Vertex *loadWortman(const char *path, ConfigData &configData, glm::vec3 offset)
{
    FILE *f = fopen(path, "r");
    printf("Err: %s\n", strerror(errno));
    Vertex *vertices = nullptr;

    if(f)
    {
        char buffer[128];

        fgets(buffer, 128, f);
        sscanf(buffer, "%d", &configData.numberOfPoints);
		configData.numberOfPoints++;
        vertices = (Vertex*)malloc(sizeof(Vertex) * configData.numberOfPoints);
        int i = 0;
        while(fgets(buffer, 128, f))
        {
            auto &v = vertices[i++];
            glm::vec3 p;
            sscanf(buffer, "%f %f %f", &p.x, &p.y, &p.z);
            vertices[i].position = glm::vec3(p.x, p.y, p.z) - offset;
            vertices[i].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
        }

        fclose(f);
    }
    return vertices;
}

void loadConfig(const char* filePath, ConfigData &configData, RenderContext &renderContext)
{
    FILE* f = fopen(filePath, "r");
    configData.vertices = nullptr;
    if(f)
    {
        char buffer[64];
        while(getData(buffer, 64, f))
        {
            if(startsWith(buffer, "#"))
            {
                continue;
            }
            else if(startsWith(buffer, "points"))
            {
                sscanf(buffer, "points %d", &configData.numberOfPoints);
            }
            else if(startsWith(buffer, "type"))
            {
                int genType;
                sscanf(buffer, "type %d", &genType);
                if(genType > GeneratorType::Clusters)
                {
                    genType = GeneratorType::InSphere;
                }
                configData.genType = (GeneratorType)genType;
            }
            else if(startsWith(buffer, "i"))
            {
                readTestSets(buffer, 64, configData.incTestSets, f);
            }
            else if(startsWith(buffer, "d"))
            {
                readTestSets(buffer, 64, configData.dacTestSets, f);
            }
            else if(startsWith(buffer, "q"))
            {
                readTestSets(buffer, 64, configData.qhTestSets, f);
            }
            else if(startsWith(buffer, "mesh"))
            {
                char path[512];
                float scale;
                sscanf(buffer, "mesh %s %f", path, &scale);
                configData.meshVertices = LoadObjWithFaces(renderContext, path, configData.loadedMesh, &configData.verticesInMesh, scale, glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));
                
                configData.loadedMesh.position = glm::vec3(0.0f);
                configData.loadedMesh.scale = glm::vec3(globalScale);
            }
            else if(startsWith(buffer, "w"))
            {
                char path[128];
                sscanf(buffer, "w %s", path);
                configData.vertices = loadWortman(path, configData, renderContext.originOffset);
            }
        }
        fclose(f);
    }
}


static void initPointGenerator(PointGenerator& pointGenerator, GeneratorType type, int numberOfPoints, coord_t min = 0.0, coord_t max = 200.0)
{
    pointGenerator.type = type;
    pointGenerator.numberOfPoints = numberOfPoints;
    //std::uniform_real_distribution<coord_t> d(min, max);
    pointGenerator.min = min;
    pointGenerator.max = max;
    //pointGenerator.d = d;
}

static GENERATOR_FUNCTION(generatePoints)
{
    auto res = (Vertex*)malloc(sizeof(Vertex) * pointGenerator.numberOfPoints);
    auto min = pointGenerator.min;
    auto max = pointGenerator.max;
    for(int i = 0; i < pointGenerator.numberOfPoints; i++)
    {
        coord_t x = randomCoord(pointGenerator.d, pointGenerator.gen, min, max);
        coord_t y = randomCoord(pointGenerator.d, pointGenerator.gen, min, max);
        coord_t z = randomCoord(pointGenerator.d, pointGenerator.gen, min, max);
        
        res[i].position = glm::vec3(x, y, z) - offset;
        res[i].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    }
    return res;
}



static GENERATOR_FUNCTION(generatePointsOnSphere)
{
    auto max = pointGenerator.max;
    auto radius = (coord_t)max / 2.0f;
    auto res = (Vertex*)malloc(sizeof(Vertex) * pointGenerator.numberOfPoints);
    for(int i = 0; i < pointGenerator.numberOfPoints; i++) 
    {
        coord_t theta = 2.0f * (coord_t)M_PI * randomCoord(pointGenerator.d, pointGenerator.gen, 0.0f, 1.0f);
        coord_t phi = (coord_t)acos(1.0f - 2.0f * randomCoord(pointGenerator.d, pointGenerator.gen, 0.0f, 1.0f));
        coord_t x = (coord_t)sin(phi) * (coord_t)cos(theta) * radius;
        coord_t y = (coord_t)sin(phi) * (coord_t)sin(theta) * radius;
        coord_t z = (coord_t)cos(phi) * radius;
        
        res[i].position = glm::vec3(x, y, z) - offset;
        res[i].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    }
    return res;
}

static GENERATOR_FUNCTION(generatePointsInSphere)
{
    auto min = pointGenerator.min;
    auto max = pointGenerator.max;
    auto res = (Vertex*)malloc(sizeof(Vertex) * pointGenerator.numberOfPoints);
    for(int i = 0; i < pointGenerator.numberOfPoints; i++) 
    {
        coord_t theta = 2 * (coord_t)M_PI * randomCoord(pointGenerator.d, pointGenerator.gen, 0.0, 1.0);
        coord_t phi = (coord_t)acos(1.0f - 2.0f * randomCoord(pointGenerator.d, pointGenerator.gen, 0.0, 1.0));
        
        auto r = randomCoord(pointGenerator.d, pointGenerator.gen, min, max);
        coord_t x = r * (coord_t)sin(phi) * (coord_t)cos(theta);
        coord_t y = r * (coord_t)sin(phi) * (coord_t)sin(theta);
        coord_t z = r * (coord_t)cos(phi);
        
        res[i].position = glm::vec3(x, y, z) - offset;
        res[i].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    }
    return res;
}


static GENERATOR_FUNCTION(generatePointsInClusters)
{
    auto res = (Vertex*)malloc(sizeof(Vertex) * pointGenerator.numberOfPoints);
    auto min = pointGenerator.min;
    auto max = pointGenerator.max;
    
    auto n_clusters = 10;
    auto pointsPerCluster = pointGenerator.numberOfPoints / n_clusters;
    
    PointGenerator p;
    p.numberOfPoints = pointsPerCluster;
    p.type = pointGenerator.type;
    p.gen = pointGenerator.gen;
    p.d = pointGenerator.d;
    p.gen.seed((unsigned int)time(NULL));
    
    auto partOfMax = max / (n_clusters * 10);
    
    for(size_t i = 0; i < n_clusters; i++)
    {
        auto r1 = randomCoord(p.d, p.gen, min + partOfMax * i, partOfMax + (partOfMax * i));
        auto r2 = randomCoord(p.d, p.gen, min + partOfMax * i, partOfMax + (partOfMax * i));
        p.min = Min(r1, r2);
        p.max = Max(r1, r2);
        initPointGenerator(p, p.type, pointsPerCluster, p.min, p.max);
        auto newOffset = glm::vec3(offset.x + ((randomInt(p.d, p.gen, 0, 1) ? -1 : 1) * randomCoord(p.d, p.gen, max / 2, max)), offset.y + ((randomInt(p.d, p.gen, 0, 1) ? -1 : 1) * randomCoord(p.d, p.gen, max / 2, max)), offset.z + ((randomInt(p.d, p.gen, 0, 1) ? -1 : 1) *randomCoord(p.d, p.gen, max / 2, max)));
        auto cluster = generatePointsInSphere(p, newOffset);
        
        memcpy(res + (pointsPerCluster * i), cluster, sizeof(Vertex) * pointsPerCluster);
    }
    
    return res;
}


GENERATOR_FUNCTION(generatePointsOnNormalizedSphere)
{
    auto min = pointGenerator.min;
    auto max = pointGenerator.max;
    auto res = (Vertex*)malloc(sizeof(Vertex) * pointGenerator.numberOfPoints);
    for(int i = 0; i < pointGenerator.numberOfPoints; i++)
    {
        coord_t x = randomCoord(pointGenerator.d, pointGenerator.gen, min, max);
        coord_t y = randomCoord(pointGenerator.d, pointGenerator.gen, min, max);
        coord_t z = randomCoord(pointGenerator.d, pointGenerator.gen, min, max);
        
        auto o = glm::vec3((coord_t)offset.x, (coord_t)offset.y, (coord_t)offset.z);
        
        auto v = glm::normalize(glm::vec3((float)x, (float)y, (float)z)) * (float)max - offset;
        res[i].position = glm::vec3((coord_t)v.x, (coord_t)v.y, (coord_t)v.z);
        res[i].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    }
    return res;
}

GENERATOR_FUNCTION(generatePointsManyInternal)
{
    auto min = pointGenerator.min;
    auto max = pointGenerator.max;
    auto res = (Vertex*)malloc(sizeof(Vertex) * pointGenerator.numberOfPoints);
    
    int pointsOnOutside = 50;
    
    glm::vec3 total = glm::vec3(0.0f);
    
    for(int i = 0; i < pointGenerator.numberOfPoints- (Min(pointsOnOutside - 1, pointGenerator.numberOfPoints) - 1); i++)
    {
        coord_t x = randomCoord(pointGenerator.d, pointGenerator.gen, max / 5.0f - min, max / 5.0f);
        coord_t y = randomCoord(pointGenerator.d, pointGenerator.gen, max / 5.0f - min, max / 5.0f);
        coord_t z = randomCoord(pointGenerator.d, pointGenerator.gen, max / 5.0f - min, max / 5.0f);
        
        res[i].position = glm::vec3(x, y, z) - offset;
        res[i].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
        total += res[i].position;
    }
    
    total = glm::vec3(total.x / (pointGenerator.numberOfPoints - pointsOnOutside), total.y / (pointGenerator.numberOfPoints - pointsOnOutside), total.z / (pointGenerator.numberOfPoints - pointsOnOutside));
    
    auto radius = max / 2.0f;
    for(int i = pointGenerator.numberOfPoints- (Min(pointsOnOutside - 1, pointGenerator.numberOfPoints) - 1); i < pointGenerator.numberOfPoints; i++)
    {
        coord_t theta = 2 * (coord_t)M_PI * randomCoord(pointGenerator.d, pointGenerator.gen, 0.0, 1.0);
        coord_t phi = (coord_t)acos(1 - 2 * randomCoord(pointGenerator.d, pointGenerator.gen, 0.0, 1.0));
        coord_t x = (coord_t)sin(phi) * (coord_t)cos(theta) * radius;
        coord_t y = (coord_t)sin(phi) * (coord_t)sin(theta) * radius;
        coord_t z = (coord_t)cos(phi) * radius;
        
        res[i].position = glm::vec3(x, y, z) + total - offset;
        res[i].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    }
    
    return res;
}

static GENERATOR_FUNCTION(generate)
{
    switch(pointGenerator.type)
    {
        case GeneratorType::InSphere:
        {
            return generatePointsInSphere(pointGenerator,offset);
        }
        break;
        case GeneratorType::OnSphere:
        {
            return generatePointsOnSphere(pointGenerator, offset);
        }
        break;
        case GeneratorType::InCube:
        {
            return generatePoints(pointGenerator, offset);
        }
        break;
        case GeneratorType::NormalizedSphere:
        {
            return generatePointsOnNormalizedSphere(pointGenerator, offset);
        }
        break;
        case GeneratorType::ManyInternal:
        {
            return generatePointsManyInternal(pointGenerator, offset);
        }
        break;
        case GeneratorType::Clusters:
        {
            return generatePointsInClusters(pointGenerator, offset);
        }
        break;
    }
    return nullptr;
}



#endif
