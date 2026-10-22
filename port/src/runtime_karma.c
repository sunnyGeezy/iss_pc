#include "iss_karma.h"
#include <stdatomic.h>
#include <math.h>
#include <stddef.h>

static unsigned char *image;
static atomic_int supported, ready, requested, selected, failure, grant_count;
static atomic_int public_side, public_rank;
static _Atomic float public_score;
static _Thread_local int inside;
static atomic_flag working=ATOMIC_FLAG_INIT;
/* Generated from the named, non-prototype upgrade IDs in this exact game. */
#include "iss_karma_powers.h"
typedef void (__attribute__((sysv_abi)) *ScoreSetter)(void *,uint64_t,float);
typedef void (__attribute__((sysv_abi)) *PowerSetter)(void *,int,uint32_t);
static uint32_t u32(const void *p){uint32_t v;memcpy(&v,p,4);return v;}
static uintptr_t ptr(const void *p){uintptr_t v;memcpy(&v,p,sizeof(v));return v;}
static int guest_ptr(uintptr_t p){return p>=0x1000000000ull && p<0x10000000000ull;}

void runtime_karma_attach(unsigned char *base){
    image=base;atomic_store(&supported,base!=NULL);atomic_store(&ready,0);
    atomic_store(&requested,0);atomic_store(&selected,0);atomic_store(&failure,0);atomic_store(&grant_count,0);
}
void runtime_karma_request(int mode){
    if((mode==1 || mode==2) && atomic_load(&supported) && atomic_load(&ready))
        atomic_store(&requested,mode);
}
void runtime_karma_status(IssKarmaStatus *out){
    *out=(IssKarmaStatus){atomic_load(&supported),atomic_load(&ready),atomic_load(&selected),
        atomic_load(&public_side),atomic_load(&public_rank),atomic_load(&requested),
        atomic_load(&failure),atomic_load(&grant_count),atomic_load(&public_score)};
}
/* Called only on the game's own camera update thread, never from the overlay. */
void __attribute__((ms_abi)) runtime_karma_tick(void){
    if(!image || inside)return;
    unsigned char *a=image+ISS_KARMA_ATTRIBUTES;
    float score;memcpy(&score,a+0xc0,4);
    int side=(int)u32(a+0xc4),rank=(int)u32(a+0xc8);
    int valid=guest_ptr(ptr(image+0x1d0cfe0)) && isfinite(score) && fabsf(score)<=4300.01f &&
        (side==0 || side==1) && rank>=0 && rank<=5;
    atomic_store(&ready,valid);atomic_store(&public_score,score);
    atomic_store(&public_side,side);atomic_store(&public_rank,rank);
    if(!valid)return;
    if(atomic_flag_test_and_set(&working))return;
    int request=atomic_exchange(&requested,0),mode=request ? request : atomic_load(&selected);
    if(!mode){atomic_flag_clear(&working);return;}
    inside=1;
    const float target=mode==1 ? 4300.0f : -4300.0f;
    ScoreSetter set_score=(ScoreSetter)(image+ISS_KARMA_SCORE_SETTER);
    if(score!=target)set_score(a,0,target);
    memcpy(&score,a+0xc0,4);side=(int)u32(a+0xc4);rank=(int)u32(a+0xc8);
    if(score!=target || side!=(mode==1?1:0) || rank!=5){
        atomic_store(&failure,1);atomic_store(&selected,0);inside=0;atomic_flag_clear(&working);return;
    }
    if(request){
        int granted=0;
        uintptr_t list=ptr(image+0x1d0d3a0);
        int count=(int)u32(image+0x1d0d3a8);
        uintptr_t manager=(count>0 && count<4096 && guest_ptr(list)) ? ptr((void*)list) : 0;
        if(!guest_ptr(manager)){
            atomic_store(&failure,2);atomic_store(&selected,mode);inside=0;atomic_flag_clear(&working);return;
        }
        PowerSetter grant=(PowerSetter)(image+ISS_KARMA_POWER_SETTER);
        for(unsigned i=0;i<ISS_KARMA_UPGRADE_COUNT;++i){
            int id=iss_karma_upgrades[i].id,family=iss_karma_upgrades[i].family;
            uint32_t family_flags=u32(a+0xe8+family*4),flags=u32(a+0xe8+id*4);
            if(!(family_flags&1) || (flags&16))continue;
            uintptr_t record=ptr((void*)(manager+0x108+id*8));
            if(!guest_ptr(record))continue;
            unsigned char *r=(unsigned char*)record;
            int required_side=(int)(int8_t)r[0x2c],required_rank=(int)(int8_t)r[0x2d];
            /* Never grant base families, quest powers, or the opposing karma branch. */
            if((u32(r+0x24)&1) || (required_side!=-1 && required_side!=side) ||
               (required_rank!=-1 && (required_rank<0 || required_rank>5)))continue;
            if((flags&7)!=7){grant(a,id,flags|7);++granted;}
        }
        atomic_store(&grant_count,granted);atomic_store(&failure,0);
        atomic_store(&selected,mode);
    }
    atomic_store(&public_score,score);atomic_store(&public_side,side);atomic_store(&public_rank,rank);
    inside=0;atomic_flag_clear(&working);
}
