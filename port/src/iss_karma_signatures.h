#ifndef ISS_KARMA_SIGNATURES_H
#define ISS_KARMA_SIGNATURES_H
#include <stdint.h>
static inline uint32_t karma_hash(const unsigned char *p,unsigned n){uint32_t h=2166136261u;for(unsigned i=0;i<n;++i)h=(h^p[i])*16777619u;return h;}
static inline int iss_karma_signatures(const unsigned char *image,uint64_t size){
    if(size<0x14d701 || karma_hash(image+0x14d6f0,0x11)!=0xbc9fa80eu)return 0;
    if(size<0x1e44d1 || karma_hash(image+0x1e4040,0x491)!=0x634aa4bcu)return 0;
    if(size<0x1e30cd || karma_hash(image+0x1e2f10,0x1bd)!=0xa1a1de6u)return 0;
    if(size<0x2307ec || karma_hash(image+0x230780,0x6c)!=0xfaf95892u)return 0;
    return 1;
}
#endif
