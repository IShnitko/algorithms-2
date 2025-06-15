#ifndef DSU_H
#define DSU_H

#include <cstdlib>

typedef uint_fast32_t U32f;

class DSU {
    U32f* parent;
    U32f* rank;
    U32f size;
    
public:
    DSU(U32f n);
    ~DSU();
    U32f find(U32f u);
    void unite(U32f u, U32f v);
};

#endif // DSU_H