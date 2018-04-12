#ifndef POINT_GENERATOR_H
#define POINT_GENERATOR_H

struct point_generator;

#define GENERATOR_FUNCTION(name) vertex* name(point_generator& pointGenerator, render_context& renderContext, coord_t min, coord_t max)
typedef GENERATOR_FUNCTION(generator_function);

enum GeneratorType
{
    InSphere,
    OnSphere,
    InCube,
    NormalizedSphere,
    ManyInternal
};

struct point_generator
{
    int numberOfPoints;
    GeneratorType type;
};

static GENERATOR_FUNCTION(GeneratePoints)
{
    auto res = (vertex*)malloc(sizeof(vertex) * pointGenerator.numberOfPoints);
    for(int i = 0; i < pointGenerator.numberOfPoints; i++)
    {
        coord_t x = randomCoord(min, max);
        coord_t y = randomCoord(min, max);
        coord_t z = randomCoord(min, max);
        
        res[i].position = glm::vec3(x, y, z) - renderContext.originOffset;
        res[i].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    }
    return res;
}

static GENERATOR_FUNCTION(GeneratePointsOnSphere)
{
    UNUSED(min);
    auto radius = max / 2.0f;
    auto res = (vertex*)malloc(sizeof(vertex) * pointGenerator.numberOfPoints);
    for(int i = 0; i < pointGenerator.numberOfPoints; i++) 
    {
        coord_t theta = 2 * (coord_t)M_PI * randomCoord(0.0, 1.0);
        coord_t phi = (coord_t)acos(1 - 2 * randomCoord(0.0, 1.0));
        coord_t x = (coord_t)sin(phi) * (coord_t)cos(theta) * radius;
        coord_t y = (coord_t)sin(phi) * (coord_t)sin(theta) * radius;
        coord_t z = (coord_t)cos(phi) * radius;
        
        res[i].position = glm::vec3(x, y, z) - renderContext.originOffset;
        res[i].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    }
    return res;
}

static GENERATOR_FUNCTION(GeneratePointsInSphere)
{
    auto res = (vertex*)malloc(sizeof(vertex) * pointGenerator.numberOfPoints);
    for(int i = 0; i < pointGenerator.numberOfPoints; i++) 
    {
        coord_t theta = 2 * (coord_t)M_PI * randomCoord(0.0, 1.0);
        coord_t phi = (coord_t)acos(1 - 2 * randomCoord(0.0, 1.0));
        
        auto r = randomCoord(min, max);
        coord_t x = r * (coord_t)sin(phi) * (coord_t)cos(theta);
        coord_t y = r * (coord_t)sin(phi) * (coord_t)sin(theta);
        coord_t z = r * (coord_t)cos(phi);
        
        res[i].position = glm::vec3(x, y, z) - renderContext.originOffset;
        res[i].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    }
    return res;
}

GENERATOR_FUNCTION(GeneratePointsOnNormalizedSphere)
{
    auto res = (vertex*)malloc(sizeof(vertex) * pointGenerator.numberOfPoints);
    for(int i = 0; i < pointGenerator.numberOfPoints; i++)
    {
        coord_t x = randomCoord(min, max);
        coord_t y = randomCoord(min, max);
        coord_t z = randomCoord(min, max);
        
        res[i].position = glm::normalize(glm::vec3(x, y, z) - renderContext.originOffset);
        res[i].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    }
    return res;
}

GENERATOR_FUNCTION(GeneratePointsManyInternal)
{
    auto res = (vertex*)malloc(sizeof(vertex) * pointGenerator.numberOfPoints);
    
    int pointsOnOutside = 50;
    
    glm::vec3 total = glm::vec3(0.0f);
    
    for(int i = 0; i < pointGenerator.numberOfPoints - pointsOnOutside; i++)
    {
        coord_t x = randomCoord(max / 5.0f - min, max / 5.0f);
        coord_t y = randomCoord(max / 5.0f - min, max / 5.0f);
        coord_t z = randomCoord(max / 5.0f - min, max / 5.0f);
        
        res[i].position = glm::vec3(x, y, z) - renderContext.originOffset;
        res[i].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
        total += res[i].position;
    }
    
    total = glm::vec3(total.x / (pointGenerator.numberOfPoints - pointsOnOutside), total.y / (pointGenerator.numberOfPoints - pointsOnOutside), total.z / (pointGenerator.numberOfPoints - pointsOnOutside));
    
    auto radius = max / 2.0f;
    for(int i = pointGenerator.numberOfPoints - pointsOnOutside; i < pointGenerator.numberOfPoints; i++)
    {
        coord_t theta = 2 * (coord_t)M_PI * randomCoord(0.0, 1.0);
        coord_t phi = (coord_t)acos(1 - 2 * randomCoord(0.0, 1.0));
        coord_t x = (coord_t)sin(phi) * (coord_t)cos(theta) * radius;
        coord_t y = (coord_t)sin(phi) * (coord_t)sin(theta) * radius;
        coord_t z = (coord_t)cos(phi) * radius;
        
        res[i].position = glm::vec3(x, y, z) + total - renderContext.originOffset;
        res[i].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    }
    
    return res;
}

static GENERATOR_FUNCTION(Generate)
{
    switch(pointGenerator.type)
    {
        case GeneratorType::InSphere:
        {
            return GeneratePointsInSphere(pointGenerator, renderContext, min, max);
        }
        break;
        case GeneratorType::OnSphere:
        {
            return GeneratePointsOnSphere(pointGenerator, renderContext, min, max);
        }
        break;
        case GeneratorType::InCube:
        {
            return GeneratePoints(pointGenerator, renderContext, min, max);
        }
        break;
        case GeneratorType::NormalizedSphere:
        {
            return GeneratePointsOnNormalizedSphere(pointGenerator, renderContext, min, max);
        }
        break;
        case GeneratorType::ManyInternal:
        {
            return GeneratePointsManyInternal(pointGenerator, renderContext, min, max);
        }
        break;
    }
}


#endif
