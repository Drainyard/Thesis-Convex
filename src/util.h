#ifndef UTIL_H
#define UTIL_H

#undef assert

#ifdef DEBUG
#ifdef _WIN32
#define assert(Expression) if(!(Expression)) {printf("Assertion failed in: %s on line %d\n",__FILE__,__LINE__); __debugbreak();}
#else
#define assert(Expression) if(!(Expression)) {printf("Assertion failed in: %s on line %d\n",__FILE__,__LINE__); asm("int $3");}
#endif
#else
#define assert(Expression)
#endif

#ifdef DEBUG
#define log(Msg, ...) printf(Msg, ## __VA_ARGS__)
#else
#define log(Msg, ...)
#endif

#define log_a(Msg, ...) printf(Msg, ## __VA_ARGS__)

#define UNUSED(var) (void)var

using coord_t = float;

static float randomFloat(float start, float end)
{
    return (rand() / (float)RAND_MAX * end) + start;
}

static double randomDouble(double start, double end)
{
    return (rand() / (double)RAND_MAX * end) + start;
}

static coord_t randomCoord(coord_t start, coord_t end)
{
    return (rand() / (coord_t)RAND_MAX * end) + start;
}

static int randomInt(int start, int end)
{
    return (rand() % end) + start;
}

static glm::vec4 randomColor()
{
    return glm::vec4((coord_t)randomInt(0, 255) / 255.0, (coord_t)randomInt(0, 255) / 255.0, (coord_t)randomInt(0, 255) / 255.0, 1.0);
}

inline bool startsWith(const char *a, const char *b)
{
    if(strncmp(a, b, strlen(b)) == 0) return 1;
    return 0;
}

struct edge
{
    int origin;
    int end;
};



#endif
