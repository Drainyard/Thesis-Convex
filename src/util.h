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


#define Min(A,B) ((A < B) ? (A) : (B))
#define Max(A,B) ((A > B) ? (A) : (B))
#define Abs(x) ((x) < 0 ? -(x) : (x))

inline bool startsWith(const char *a, const char *b)
{
    if(strncmp(a, b, strlen(b)) == 0) return 1;
    return 0;
}

using coord_t = float;

static float randomFloat(std::uniform_real_distribution<coord_t>& d, std::mt19937& gen, float start, float end)
{
    return ((float)d(gen) / (float)d.max() * end) + start;
}

static double randomDouble(std::uniform_real_distribution<coord_t>& d, std::mt19937& gen, double start, double end)
{
    return ((double)d(gen) / (double)d.max() * end) + start;
}

static coord_t randomCoord(std::uniform_real_distribution<coord_t>& d, std::mt19937& gen, coord_t start, coord_t end)
{
    return ((coord_t)d(gen) / (coord_t)d.max() * end) + start;
}

static int randomInt(std::uniform_real_distribution<coord_t>& d, std::mt19937& gen, int start, int end)
{
    return ((int)d(gen) / (int)d.max() * end) + start;
}

static glm::vec4 randomColor(std::uniform_real_distribution<coord_t>& d, std::mt19937& gen)
{
    return glm::vec4((coord_t)randomInt(d, gen, 0, 255) / 255.0, (coord_t)randomInt(d, gen, 0, 255) / 255.0, (coord_t)randomInt(d, gen, 0, 255) / 255.0, 1.0);
}

struct edge
{
    int origin;
    int end;
};

// NOTE(Niels): Remember to free
char* concat(const char* left, const char* right)
{
    char* res = (char*)malloc(sizeof(char) * (strlen(left) + strlen(right) + 1));
    strcpy(res, left);
    strcat(res, right);
    res[strlen(left) + strlen(right) + 1] = '\0';
    return res;
}

bool FileExists(const char *file)
{
#if defined(__linux)
    return access(file, F_OK) != -1;
#else
    return PathFileExists(file) != 0;
#endif
}

int countLinesFromCurrent(FILE *file)
{
    auto curPos = ftell(file);
    int ch = 0;
    int lines = 0;
    
    while(!feof(file))
    {
        ch = fgetc(file);
        if(ch == '\n')
        {
            lines++;
        }
    }
    fseek(file, curPos, SEEK_SET);
    return lines;
}

#endif
