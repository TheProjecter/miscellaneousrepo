#ifndef C_TYPES_H
#define C_TYPES_H
// TODO: GET RID OF THIS FILE! __CHAR_ARY is a crappy hack to get j/ooc to cooperate
#define __CHAR_ARY char*

// NOTE: since bit-fields are not available in OOC, these
// structs need to be defined in C code and then as covers in OOC
typedef struct {
        unsigned int present : 1;
        unsigned int readwrite : 1;
        unsigned int user : 1;
        unsigned int writethrough : 1;
        unsigned int cachedisable : 1;
        unsigned int accessed : 1;
        unsigned int dirty : 1;
        unsigned int attributeindex : 1;
        unsigned int global : 1;
        unsigned int reserved : 3;
        unsigned int address : 20;
} _PageTableEntry;



typedef struct {
        unsigned int present : 1;
        unsigned int readwrite : 1;
        unsigned int user : 1;
        unsigned int writethrough : 1;
        unsigned int cachedisable : 1;
        unsigned int accessed : 1;
        unsigned int available : 1;
        unsigned int pagesize : 1;
        unsigned int global : 1;
        unsigned int reserved : 3;
        unsigned int address : 20;
} _PageDirEntry;
#endif
