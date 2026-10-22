/* CUSA00223 01.07. The switcher uses game functions, never save-file offsets. */
#ifndef ISS_KARMA_H
#define ISS_KARMA_H
#include <stdint.h>
#include <string.h>
#define ISS_KARMA_SITE 0x14d6f0u
#define ISS_KARMA_STOLEN 17u
#define ISS_KARMA_BRIDGE_MAX 1024u
#define ISS_KARMA_ATTRIBUTES 0xefacf8u
#define ISS_KARMA_SCORE_SETTER 0x1e4040u
#define ISS_KARMA_POWER_SETTER 0x1e2f10u
typedef struct { int supported, ready, mode, side, rank, pending, error, grants; float score; } IssKarmaStatus;
/* mode: 1 True Hero, 2 Infamous. Selection lasts for this play session. */
#ifdef __cplusplus
extern "C" {
#endif
void runtime_karma_attach(unsigned char *image);
void runtime_karma_request(int mode);
void runtime_karma_status(IssKarmaStatus *out);
void __attribute__((ms_abi)) runtime_karma_tick(void);
#ifdef __cplusplus
}
#endif
static inline void iss_karma_jump(unsigned char *p, uintptr_t target) {
    const unsigned char op[]={0xff,0x25,0,0,0,0};memcpy(p,op,6);memcpy(p+6,&target,8);
}
/* Called at a game function entry. Preserve all GPRs, flags, x87/SSE state and
 * all 16 full YMM registers across the host and guest calls. The stack is aligned
 * to 32 bytes and includes the Windows ABI's shadow space on both hosts. */
static inline unsigned iss_karma_bridge(unsigned char *p, uintptr_t helper,
                                        const unsigned char *original, uintptr_t resume) {
    unsigned n=0;
    const unsigned char push[]={0x9c,0x50,0x51,0x52,0x53,0x55,0x56,0x57,
        0x41,0x50,0x41,0x51,0x41,0x52,0x41,0x53,0x41,0x54,0x41,0x55,0x41,0x56,0x41,0x57,
        0x48,0x89,0xe3,0x48,0x83,0xe4,0xe0,0x48,0x81,0xec,0x20,0x04,0,0};
    memcpy(p+n,push,sizeof(push));n+=sizeof(push);
    for(unsigned i=0;i<16;++i){
        const unsigned char op[]={0xc4,(unsigned char)(i<8?0xe1:0x61),0x7e,0x7f,
            (unsigned char)(0x84+8*(i%8)),0x24};
        memcpy(p+n,op,6);n+=6;uint32_t d=32+32*i;memcpy(p+n,&d,4);n+=4;
    }
    const unsigned char fxsave[]={0x48,0x0f,0xae,0x84,0x24,0x20,0x02,0,0,0x48,0xb8};
    memcpy(p+n,fxsave,sizeof(fxsave));n+=sizeof(fxsave);memcpy(p+n,&helper,8);n+=8;
    p[n++]=0xff;p[n++]=0xd0;
    const unsigned char fxrestore[]={0x48,0x0f,0xae,0x8c,0x24,0x20,0x02,0,0};
    memcpy(p+n,fxrestore,sizeof(fxrestore));n+=sizeof(fxrestore);
    for(unsigned i=0;i<16;++i){
        const unsigned char op[]={0xc4,(unsigned char)(i<8?0xe1:0x61),0x7e,0x6f,
            (unsigned char)(0x84+8*(i%8)),0x24};
        memcpy(p+n,op,6);n+=6;uint32_t d=32+32*i;memcpy(p+n,&d,4);n+=4;
    }
    const unsigned char pop[]={0x48,0x89,0xdc,0x41,0x5f,0x41,0x5e,0x41,0x5d,0x41,0x5c,
        0x41,0x5b,0x41,0x5a,0x41,0x59,0x41,0x58,0x5f,0x5e,0x5d,0x5b,0x5a,0x59,0x58,0x9d};
    memcpy(p+n,pop,sizeof(pop));n+=sizeof(pop);
    memcpy(p+n,original,ISS_KARMA_STOLEN);n+=ISS_KARMA_STOLEN;
    iss_karma_jump(p+n,resume);return n+14;
}
#endif
