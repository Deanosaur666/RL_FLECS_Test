#define LIST_(T) T##_List
#define PLIST_(T) T##_PList

#define DECLARE_LIST(T) typedef struct LIST_(T) { \
    T * arr; \
    int capacity; \
    int size; \
    int esize; \
} LIST_(T)

#define DECLARE_PLIST(T) typedef struct PLIST_(T) { \
    T ** arr; \
    int capacity; \
    int size; \
    int esize; \
} PLIST_(T)

#define NEWLIST(T) (LIST_(T)){ malloc( 1 * sizeof(T) ), 1, 0, sizeof(T) }
#define NEWPLIST(T) (PLIST_(T)){ malloc( 1 * sizeof(T *) ), 1, 0, sizeof(T *) }

#define FREELIST(L) free(L.arr)

#define LIST_ADD(L, E) \
if(L.size >= L.capacity) { \
    L.capacity *= 2; \
    L.arr = realloc(L.arr, L.esize * L.capacity); \
} \
L.arr[L.size] = E; \
L.size ++;

#define LIST_GET(L, i) L.arr[i]
#define LIST_SET(L, i, val) L.arr[i] = val