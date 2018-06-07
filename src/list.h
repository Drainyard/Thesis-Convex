#ifndef LIST_H
#define LIST_H

template<typename T>
struct List
{
    size_t size = 0;
    size_t capacity = 0;
    T *list = nullptr;
    
    T &operator[](size_t index)
    {
#if DEBUG
        if(index >= this->size)
        {
            assert(false);
        }
#endif
        return this->list[index];
    }
    
    
    T &operator[](int index)
    {
#if DEBUG
        if(index >= (int)this->size)
        {
            assert(false);
        }
#endif
        return this->list[index];
    }
    
    T *begin() {return this->list;}
    T *end() {return this->list + this->size;}
    
};

template<typename T>
static void addToList(List<T> &list, T element)
{
    if (list.capacity == 0)
    {
        list.capacity = 2;
        list.list = (T*)malloc(sizeof(T) * list.capacity);
    }
    
    if (list.size + 1 > list.capacity)
    {
        list.capacity *= 2;
        list.list = (T*)realloc(list.list, sizeof(T) * list.capacity);
    }
    
    list.list[list.size++] = element;
}

template<typename T>
static void addToList(List<T> &list, std::initializer_list<T> elements)
{
    if(list.capacity == 0)
    {
        list.capacity = (size_t)(pow(elements.size(), log2(elements.size())));
        list.list = (T*)malloc(sizeof(T) * list.capacity);
    }
    
    if(list.size + elements.size() > list.capacity)
    {
        list.capacity = (size_t)(pow(list.size + elements.size(), log2(list.size + elements.size())));
        list.list = (T*)realloc(list.list, sizeof(T) * list.capacity);
    }
    
    for(auto &e : elements)
    {
        list.list[list.size++] = e;
    }
}

template<typename T>
static void clear(List<T> &list, size_t capacity = 0)
{
    if(list.list)
    {
        free(list.list);
    }
    
    list.list = nullptr;
    list.size = 0;
    list.capacity = (size_t)pow(capacity, (size_t)ceil(log2(capacity)));
    
    if(list.capacity > 0)
    {
        list.list = (T*)malloc(sizeof(T) * list.capacity);
    }
}

template<typename T>
static void init(List<T> &list, size_t capacity = 0)
{
    list.size = 0;
    list.capacity = (size_t)pow(capacity, (size_t)ceil(log2(capacity)));
    
    list.list = nullptr;
    
    if(list.capacity > 0)
    {
        list.list = (T*)malloc(sizeof(T) * list.capacity);
    }
    
}

#endif

