#ifndef POINT_GENERATOR_H
#define POINT_GENERATOR_H

struct PointGenerator;

#define GENERATOR_FUNCTION(name) Vertex* name(PointGenerator& pointGenerator, coord_t min, coord_t max, glm::vec3 offset)
typedef GENERATOR_FUNCTION(GeneratorFunction);

enum GeneratorType
{
    InSphere,
    OnSphere,
    InCube,
    NormalizedSphere,
    ManyInternal,
};

struct PointGenerator
{
    int numberOfPoints;
    GeneratorType type;
    std::mt19937 gen;
    std::uniform_real_distribution<coord_t> d;
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
    GeneratorType genType;
    
    List<TestSet> qhTestSets;
    List<TestSet> incTestSets;
    List<TestSet> dacTestSets;
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

void loadConfig(const char* filePath, ConfigData &configData)
{
    FILE* f = fopen(filePath, "r");
    if(f)
    {
        char buffer[64];
        while(getData(buffer, 64, f))
        {
            if(startsWith(buffer, "points"))
            {
                sscanf(buffer, "points %d", &configData.numberOfPoints);
            }
            else if(startsWith(buffer, "type"))
            {
                int genType;
                sscanf(buffer, "type %d", &genType);
                if(genType > GeneratorType::ManyInternal)
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
        }
        fclose(f);
    }
}

static void initPointGenerator(PointGenerator& pointGenerator, GeneratorType type, int numberOfPoints)
{
    pointGenerator.type = type;
    pointGenerator.numberOfPoints = numberOfPoints;
}

static GENERATOR_FUNCTION(generatePoints)
{
    auto res = (Vertex*)malloc(sizeof(Vertex) * pointGenerator.numberOfPoints);
    for(int i = 0; i < pointGenerator.numberOfPoints; i++)
    {
        coord_t x = randomCoord(pointGenerator.d, pointGenerator.gen, min, max);
        coord_t y = randomCoord(pointGenerator.d, pointGenerator.gen, min, max);
        coord_t z = randomCoord(pointGenerator.d, pointGenerator.gen, min, max);
        
        res[i].position = glm::vec3(x, y, z) - offset;
        res[i].color = glm::vec4(0.0L, 1.0L, 1.0L, 1.0L);
    }
    return res;
}

static GENERATOR_FUNCTION(generatePointsOnSphere)
{
    UNUSED(min);
    auto radius = max / 2.0L;
    auto res = (Vertex*)malloc(sizeof(Vertex) * pointGenerator.numberOfPoints);
    for(int i = 0; i < pointGenerator.numberOfPoints; i++) 
    {
        coord_t theta = 2 * (coord_t)M_PI * randomCoord(pointGenerator.d, pointGenerator.gen, 0.0, 1.0);
        coord_t phi = (coord_t)acos(1 - 2 * randomCoord(pointGenerator.d, pointGenerator.gen, 0.0, 1.0));
        coord_t x = (coord_t)sin(phi) * (coord_t)cos(theta) * radius;
        coord_t y = (coord_t)sin(phi) * (coord_t)sin(theta) * radius;
        coord_t z = (coord_t)cos(phi) * radius;
        
        res[i].position = glm::vec3(x, y, z) - offset;
        res[i].color = glm::vec4(0.0L, 1.0L, 1.0L, 1.0L);
    }
    return res;
}

static GENERATOR_FUNCTION(generatePointsInSphere)
{
    auto res = (Vertex*)malloc(sizeof(Vertex) * pointGenerator.numberOfPoints);
    for(int i = 0; i < pointGenerator.numberOfPoints; i++) 
    {
        coord_t theta = 2 * (coord_t)M_PI * randomCoord(pointGenerator.d, pointGenerator.gen, 0.0, 1.0);
        coord_t phi = (coord_t)acos(1 - 2 * randomCoord(pointGenerator.d, pointGenerator.gen, 0.0, 1.0));
        
        auto r = randomCoord(pointGenerator.d, pointGenerator.gen, min, max);
        coord_t x = r * (coord_t)sin(phi) * (coord_t)cos(theta);
        coord_t y = r * (coord_t)sin(phi) * (coord_t)sin(theta);
        coord_t z = r * (coord_t)cos(phi);
        
        res[i].position = glm::vec3(x, y, z) - offset;
        res[i].color = glm::vec4(0.0L, 1.0L, 1.0L, 1.0L);
    }
    return res;
}

GENERATOR_FUNCTION(generatePointsOnNormalizedSphere)
{
    auto res = (Vertex*)malloc(sizeof(Vertex) * pointGenerator.numberOfPoints);
    for(int i = 0; i < pointGenerator.numberOfPoints; i++)
    {
        coord_t x = randomCoord(pointGenerator.d, pointGenerator.gen, min, max);
        coord_t y = randomCoord(pointGenerator.d, pointGenerator.gen, min, max);
        coord_t z = randomCoord(pointGenerator.d, pointGenerator.gen, min, max);
        
        auto o = glm::vec3((coord_t)offset.x, (coord_t)offset.y, (coord_t)offset.z);
        
        auto v = glm::normalize(glm::vec3((float)x, (float)y, (float)z) - offset) * (float)max;
        res[i].position = glm::vec3((coord_t)v.x, (coord_t)v.y, (coord_t)v.z);
        res[i].color = glm::vec4(0.0L, 1.0L, 1.0L, 1.0L);
    }
    return res;
}

GENERATOR_FUNCTION(generatePointsManyInternal)
{
    auto res = (Vertex*)malloc(sizeof(Vertex) * pointGenerator.numberOfPoints);
    
    int pointsOnOutside = 50;
    
    glm::vec3 total = glm::vec3(0.0L);
    
    for(int i = 0; i < pointGenerator.numberOfPoints- (Min(pointsOnOutside - 1, pointGenerator.numberOfPoints) - 1); i++)
    {
        coord_t x = randomCoord(pointGenerator.d, pointGenerator.gen, max / 5.0L - min, max / 5.0L);
        coord_t y = randomCoord(pointGenerator.d, pointGenerator.gen, max / 5.0L - min, max / 5.0L);
        coord_t z = randomCoord(pointGenerator.d, pointGenerator.gen, max / 5.0L - min, max / 5.0L);
        
        res[i].position = glm::vec3(x, y, z) - offset;
        res[i].color = glm::vec4(0.0L, 1.0L, 1.0L, 1.0L);
        total += res[i].position;
    }
    
    total = glm::vec3(total.x / (pointGenerator.numberOfPoints - pointsOnOutside), total.y / (pointGenerator.numberOfPoints - pointsOnOutside), total.z / (pointGenerator.numberOfPoints - pointsOnOutside));
    
    auto radius = max / 2.0L;
    for(int i = pointGenerator.numberOfPoints- (Min(pointsOnOutside - 1, pointGenerator.numberOfPoints) - 1); i < pointGenerator.numberOfPoints; i++)
    {
        coord_t theta = 2 * (coord_t)M_PI * randomCoord(pointGenerator.d, pointGenerator.gen, 0.0, 1.0);
        coord_t phi = (coord_t)acos(1 - 2 * randomCoord(pointGenerator.d, pointGenerator.gen, 0.0, 1.0));
        coord_t x = (coord_t)sin(phi) * (coord_t)cos(theta) * radius;
        coord_t y = (coord_t)sin(phi) * (coord_t)sin(theta) * radius;
        coord_t z = (coord_t)cos(phi) * radius;
        
        res[i].position = glm::vec3(x, y, z) + total - offset;
        res[i].color = glm::vec4(0.0L, 1.0L, 1.0L, 1.0L);
    }
    
    return res;
}

static GENERATOR_FUNCTION(generate)
{
    switch(pointGenerator.type)
    {
        case GeneratorType::InSphere:
        {
            return generatePointsInSphere(pointGenerator, min, max, offset);
        }
        break;
        case GeneratorType::OnSphere:
        {
            return generatePointsOnSphere(pointGenerator, min, max, offset);
        }
        break;
        case GeneratorType::InCube:
        {
            return generatePoints(pointGenerator, min, max, offset);
        }
        break;
        case GeneratorType::NormalizedSphere:
        {
            return generatePointsOnNormalizedSphere(pointGenerator, min, max, offset);
        }
        break;
        case GeneratorType::ManyInternal:
        {
            return generatePointsManyInternal(pointGenerator, min, max, offset);
        }
        break;
    }
    return nullptr;
}



#endif
