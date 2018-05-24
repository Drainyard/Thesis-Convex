#ifndef POINT_GENERATOR_H
#define POINT_GENERATOR_H

struct point_generator;

#define GENERATOR_FUNCTION(name) vertex* name(point_generator& pointGenerator, coord_t min, coord_t max, glm::vec3 offset)
typedef GENERATOR_FUNCTION(generator_function);

enum GeneratorType
{
    InSphere,
    OnSphere,
    InCube,
    NormalizedSphere,
    ManyInternal,
};

struct point_generator
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

struct config_data
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
        while(fgets(buf, 64, f))
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
                sscanf(buf, "iterations %d\n", &testSet.iterations);
            }
            else
            {
                if(testSet.count == 0)
                {
                    testSet.count = (size_t)countLinesFromCurrent(f) + 1;
                    if(testSet.count == 0)
                    {
                        break;
                    }
                    testSet.testSet = (int*)malloc(sizeof(int) * testSet.count);
                }
                
                sscanf(buf, "%d\n", &testSet.testSet[i++]);
                log_a("%d\n", testSet.testSet[i - 1]);
            }
        }
        fclose(f);
    }
}

void readTestSets(char *buffer, List<TestSet> &list, FILE *f)
{
    init(list);
    auto prevPos = ftell(f);
    while(fgets(buffer, 64, f))
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
        else
        {
            fseek(f, prevPos, SEEK_SET);
            break;
        }
    }
    prevPos = ftell(f);
}

void loadConfig(const char* filePath, config_data &configData)
{
    FILE* f = fopen(filePath, "r");
    if(f)
    {
        bool set = false;
        char buffer[64];
        while(fgets(buffer, 64, f))
        {
            if(startsWith(buffer, "points"))
            {
                assert(!set);
                sscanf(buffer, "points %d", &configData.numberOfPoints);
            }
            else if(startsWith(buffer, "type"))
            {
                assert(!set);
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
                readTestSets(buffer, configData.incTestSets, f);
            }
            else if(startsWith(buffer, "d"))
            {
                readTestSets(buffer, configData.dacTestSets, f);
            }
            else if(startsWith(buffer, "q"))
            {
                readTestSets(buffer, configData.qhTestSets, f);
            }
        }
        fclose(f);
    }
}

static void initPointGenerator(point_generator& pointGenerator, GeneratorType type, int numberOfPoints)
{
    pointGenerator.type = type;
    pointGenerator.numberOfPoints = numberOfPoints;
}

static GENERATOR_FUNCTION(generatePoints)
{
    auto res = (vertex*)malloc(sizeof(vertex) * pointGenerator.numberOfPoints);
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
    UNUSED(min);
    auto radius = max / 2.0f;
    auto res = (vertex*)malloc(sizeof(vertex) * pointGenerator.numberOfPoints);
    for(int i = 0; i < pointGenerator.numberOfPoints; i++) 
    {
        coord_t theta = 2 * (coord_t)M_PI * randomCoord(pointGenerator.d, pointGenerator.gen, 0.0, 1.0);
        coord_t phi = (coord_t)acos(1 - 2 * randomCoord(pointGenerator.d, pointGenerator.gen, 0.0, 1.0));
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
    auto res = (vertex*)malloc(sizeof(vertex) * pointGenerator.numberOfPoints);
    for(int i = 0; i < pointGenerator.numberOfPoints; i++) 
    {
        coord_t theta = 2 * (coord_t)M_PI * randomCoord(pointGenerator.d, pointGenerator.gen, 0.0, 1.0);
        coord_t phi = (coord_t)acos(1 - 2 * randomCoord(pointGenerator.d, pointGenerator.gen, 0.0, 1.0));
        
        auto r = randomCoord(pointGenerator.d, pointGenerator.gen, min, max);
        coord_t x = r * (coord_t)sin(phi) * (coord_t)cos(theta);
        coord_t y = r * (coord_t)sin(phi) * (coord_t)sin(theta);
        coord_t z = r * (coord_t)cos(phi);
        
        res[i].position = glm::vec3(x, y, z) - offset;
        res[i].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    }
    return res;
}

GENERATOR_FUNCTION(generatePointsOnNormalizedSphere)
{
    auto res = (vertex*)malloc(sizeof(vertex) * pointGenerator.numberOfPoints);
    for(int i = 0; i < pointGenerator.numberOfPoints; i++)
    {
        coord_t x = randomCoord(pointGenerator.d, pointGenerator.gen, min, max);
        coord_t y = randomCoord(pointGenerator.d, pointGenerator.gen, min, max);
        coord_t z = randomCoord(pointGenerator.d, pointGenerator.gen, min, max);
        
        res[i].position = glm::normalize(glm::vec3(x, y, z) - offset) * max;
        res[i].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    }
    return res;
}

GENERATOR_FUNCTION(generatePointsManyInternal)
{
    auto res = (vertex*)malloc(sizeof(vertex) * pointGenerator.numberOfPoints);
    
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
