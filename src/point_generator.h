#ifndef POINT_GENERATOR_H
#define POINT_GENERATOR_H

struct point_generator;

#define GENERATOR_FUNCTION(name) vertex* name(point_generator& pointGenerator, render_context& renderContext, coord_t min, coord_t max)

typedef GENERATOR_FUNCTION(generator_function);


struct point_generator
{
    generator_function* generatorFunction; 
    int numberOfPoints;
};

static GENERATOR_FUNCTION(GeneratePoints)
{
    auto res = (vertex*)malloc(sizeof(vertex) * pointGenerator.numberOfPoints);
    for(int i = 0; i < pointGenerator.numberOfPoints; i++)
    {
        coord_t x = RandomCoord(min, max);
        coord_t y = RandomCoord(min, max);
        coord_t z = RandomCoord(min, max);
        
        res[i].position = glm::vec3(x, y, z) - renderContext.originOffset;
        res[i].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    }
    return res;
}

static GENERATOR_FUNCTION(GeneratePointsOnSphere)
{
    auto radius = max / 2.0f;
    auto res = (vertex*)malloc(sizeof(vertex) * pointGenerator.numberOfPoints);
    for(int i = 0; i < pointGenerator.numberOfPoints; i++) 
    {
        coord_t theta = 2 * (coord_t)M_PI * RandomCoord(0.0, 1.0);
        coord_t phi = (coord_t)acos(1 - 2 * RandomCoord(0.0, 1.0));
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
        coord_t theta = 2 * (coord_t)M_PI * RandomCoord(0.0, 1.0);
        coord_t phi = (coord_t)acos(1 - 2 * RandomCoord(0.0, 1.0));
        
        auto r = RandomCoord(min, max);
        coord_t x = r * (coord_t)sin(phi) * (coord_t)cos(theta);
        coord_t y = r * (coord_t)sin(phi) * (coord_t)sin(theta);
        coord_t z = r * (coord_t)cos(phi);
        
        res[i].position = glm::vec3(x, y, z) - renderContext.originOffset;
        res[i].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    }
    return res;
}

#endif
