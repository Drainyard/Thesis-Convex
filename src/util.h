#ifndef UTIL_H
#define UTIL_H

#define Assert(Expression) if(!(Expression)) {printf("Assertion failed in: %s on line %d\n",__FILE__,__LINE__); asm("int $3");}

static float RandomFloat(float start, float end)
{
    return (rand() / (float)RAND_MAX * end) + start;
}

static int RandomInt(int start, int end)
{
    return (rand() % end) + start;
}

static glm::vec4 RandomColor()
{
    return glm::vec4((float)RandomInt(0, 255) / 255.0f, (float)RandomInt(0, 255) / 255.0f, (float)RandomInt(0, 255) / 255.0f, (float)RandomInt(100, 255) / 255.0f);
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
    list->listSize = 16;\
    list->data = (type*)malloc(sizeof(type) * list->listSize);\
}


#endif