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
        for(size_t i = list.size; i < list.capacity; i++)
        {
            list.list[i] = {};
        }
    }
    
    if (list.size + 1 > list.capacity)
    {
        list.capacity *= 2;
        list.list = (T*)realloc(list.list, sizeof(T) * list.capacity);
        for(size_t i = list.size; i < list.capacity; i++)
        {
            list.list[i] = {};
        }
    }
    
    list.list[list.size++] = element;
}


template<typename T>
static void clear(List<T> &list)
{
    if(list.list)
    {
        free(list.list);
        list.list = nullptr;
    }
    
    list.size = 0;
    list.capacity = 0;
}

template<typename T>
static void init(List<T> &list)
{
    list.size = 0;
    list.capacity = 0;
    list.list = nullptr;
}

#endif

