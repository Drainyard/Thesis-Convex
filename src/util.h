#ifndef UTIL_H
#define UTIL_H

#ifdef DEBUG
#ifdef _WIN32
#define Assert(Expression) if(!(Expression)) {printf("Assertion failed in: %s on line %d\n",__FILE__,__LINE__); __debugbreak();}
#else
#define Assert(Expression) if(!(Expression)) {printf("Assertion failed in: %s on line %d\n",__FILE__,__LINE__); asm("int $3");}
#endif
#else
#define Assert(Expression)
#endif

#ifdef DEBUG
#define Log(Msg, ...) printf(Msg, ## __VA_ARGS__)
#else
#define Log(Msg, ...)
#endif

#define Log_A(Msg, ...) printf(Msg, ## __VA_ARGS__)

using coord_t = float;

static float RandomFloat(float start, float end)
{
    return (rand() / (float)RAND_MAX * end) + start;
}

static double RandomDouble(double start, double end)
{
    return (rand() / (double)RAND_MAX * end) + start;
}

static coord_t RandomCoord(coord_t start, coord_t end)
{
    return (rand() / (coord_t)RAND_MAX * end) + start;
}

static int RandomInt(int start, int end)
{
    return (rand() % end) + start;
}

static glm::vec4 RandomColor()
{
    return glm::vec4((coord_t)RandomInt(0, 255) / 255.0, (coord_t)RandomInt(0, 255) / 255.0, (coord_t)RandomInt(0, 255) / 255.0, 1.0);
}

inline bool StartsWith(const char *a, const char *b)
{
    if(strncmp(a, b, strlen(b)) == 0) return 1;
    return 0;
}

#define list(type)\
struct type ## _list\
{\
    type* data;\
    int count;\
    int listSize;\
    \
    type& operator[](int index)\
    {\
        Assert(index < this->count);\
        return this->data[index];\
    }\
    \
};\
\
void Add(type ## _list* list, type& item)\
{\
    if(list->count + 1 >= list->listSize)\
    {\
        list->listSize *= 2;\
        list->data = (type*)realloc(list->data, list->listSize * sizeof(type));\
    }\
    list->data[list->count++] = item;\
}\
void Remove(type ## _list* list, type* item)\
{\
    bool found = false;\
    for(int i = 0; i < list->count - 1; i++)\
    {\
        if(!found && &list->data[i] == item)\
        {\
            found = true;\
        }\
        else if(found)\
        {\
            list->data[i] = list->data[i + 1];\
        }\
    }\
    \
    list->count--;\
}\
void Init##type##List(type## _list* list, int initSize = 16)\
{\
    list->count = 0;\
    list->listSize = initSize;\
    list->data = (type*)malloc(sizeof(type) * list->listSize);\
}


struct edge
{
    int origin;
    int end;
};

list(edge)

#define SWAP(t,x,y)	{ t = x; x = y; y = t; }

char *malloc();

#define INC_NEW(p,type)	if ((p=(type *) malloc (sizeof(type))) == NULL) {\
				printf ("Out of Memory!\n");\
				exit(0);\
			}

#define INC_FREE(p)		if (p) { free ((char *) p); p = NULL; }


#define INC_ADD( head, p )  if ( head )  { \
				p->next = head; \
				p->prev = head->prev; \
				head->prev = p; \
				p->prev->next = p; \
			} \
			else { \
				head = p; \
				head->next = head->prev = p; \
			}

#define INC_DELETE( head, p ) if ( head )  { \
				if ( head == head->next ) \
					head = NULL;  \
				else if ( p == head ) \
					head = head->next; \
				p->next->prev = p->prev;  \
				p->prev->next = p->next;  \
				INC_FREE( p ); \
			} 



#endif
