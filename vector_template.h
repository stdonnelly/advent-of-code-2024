#pragma once

// Macro to define a generic vector
#define DEF_VEC(T)                                           \
    typedef struct T##Vec                                    \
    {                                                        \
        T *arr;                                              \
        size_t len;                                          \
        size_t cap;                                          \
    } T##Vec;                                                \
    void resize##T(T##Vec *this, size_t new_cap)             \
    {                                                        \
        this->arr = realloc(this->arr, sizeof(T) * new_cap); \
        this->cap = new_cap;                                 \
    }                                                        \
    void append##T(T##Vec *this, T val)                      \
    {                                                        \
        if (this->len + 1 > this->cap)                       \
            resize##T(this, this->cap ? this->cap * 2 : 1);  \
        *(this->arr + (this->len++)) = val;                  \
    }                                                        \
    T##Vec new##T##Vec()                                     \
    {                                                        \
        T##Vec o = {NULL, 0, 0};                             \
        return o;                                            \
    }
