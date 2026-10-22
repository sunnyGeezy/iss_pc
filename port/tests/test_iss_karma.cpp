#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <cstdlib>
static constexpr int MEM_RESERVE=0, MEM_COMMIT=0, PAGE_READWRITE=0, PAGE_EXECUTE_READ=1;
using DWORD=unsigned long;
static void* VirtualAlloc(void* where,size_t bytes,int,int){
    void* p=mmap(where,bytes,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|(where?MAP_FIXED_NOREPLACE:0),-1,0);
    return p==MAP_FAILED?nullptr:p;
}
static bool VirtualProtect(void* p,size_t bytes,int,DWORD*){return mprotect(p,bytes,PROT_READ|PROT_EXEC)==0;}
static void* GetCurrentProcess(){return nullptr;}
static void FlushInstructionCache(void*,void*,size_t){}
#endif
#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>
#include "../src/iss_karma.h"
#include "../src/iss_karma_signatures.h"

static unsigned score_events,grant_events;
static bool reject_score;
extern "C" void __attribute__((sysv_abi)) score_fixture(void* v,uint64_t,float score){
    if(reject_score)return;
    auto* a=(unsigned char*)v;int side=score>=0,rank=5;
    memcpy(a+0xc0,&score,4);memcpy(a+0xc4,&side,4);memcpy(a+0xc8,&rank,4);++score_events;
}
extern "C" void __attribute__((sysv_abi)) grant_fixture(void* v,int id,uint32_t flags){
    assert(id>=0 && id<222);memcpy((unsigned char*)v+0xe8+id*4,&flags,4);++grant_events;
}
extern "C" void __attribute__((ms_abi,naked)) clobber_vectors(){
    __asm__ volatile(
        "vpxor %ymm0,%ymm0,%ymm0\n vpxor %ymm1,%ymm1,%ymm1\n"
        "vpxor %ymm2,%ymm2,%ymm2\n vpxor %ymm3,%ymm3,%ymm3\n"
        "vpxor %ymm4,%ymm4,%ymm4\n vpxor %ymm5,%ymm5,%ymm5\n"
        "vpxor %ymm6,%ymm6,%ymm6\n vpxor %ymm7,%ymm7,%ymm7\n"
        "vpxor %ymm8,%ymm8,%ymm8\n vpxor %ymm9,%ymm9,%ymm9\n"
        "vpxor %ymm10,%ymm10,%ymm10\n vpxor %ymm11,%ymm11,%ymm11\n"
        "vpxor %ymm12,%ymm12,%ymm12\n vpxor %ymm13,%ymm13,%ymm13\n"
        "vpxor %ymm14,%ymm14,%ymm14\n vpxor %ymm15,%ymm15,%ymm15\n"
        "xor %eax,%eax\n xor %ecx,%ecx\n xor %edx,%edx\n ret\n");
}
extern "C" void __attribute__((sysv_abi,naked)) vector_harness(void*,void*,void*){
    __asm__ volatile(
        "push %r12\n push %r13\n push %r14\n"
        "mov %rdi,%r12\n mov %rsi,%r13\n mov %rdx,%r14\n"
        "vmovdqu 0(%r13),%ymm0\n vmovdqu 32(%r13),%ymm1\n"
        "vmovdqu 64(%r13),%ymm2\n vmovdqu 96(%r13),%ymm3\n"
        "vmovdqu 128(%r13),%ymm4\n vmovdqu 160(%r13),%ymm5\n"
        "vmovdqu 192(%r13),%ymm6\n vmovdqu 224(%r13),%ymm7\n"
        "vmovdqu 256(%r13),%ymm8\n vmovdqu 288(%r13),%ymm9\n"
        "vmovdqu 320(%r13),%ymm10\n vmovdqu 352(%r13),%ymm11\n"
        "vmovdqu 384(%r13),%ymm12\n vmovdqu 416(%r13),%ymm13\n"
        "vmovdqu 448(%r13),%ymm14\n vmovdqu 480(%r13),%ymm15\n"
        "stc\n call *%r12\n pushfq\n pop %rax\n mov %rax,512(%r14)\n"
        "vmovdqu %ymm0,0(%r14)\n vmovdqu %ymm1,32(%r14)\n"
        "vmovdqu %ymm2,64(%r14)\n vmovdqu %ymm3,96(%r14)\n"
        "vmovdqu %ymm4,128(%r14)\n vmovdqu %ymm5,160(%r14)\n"
        "vmovdqu %ymm6,192(%r14)\n vmovdqu %ymm7,224(%r14)\n"
        "vmovdqu %ymm8,256(%r14)\n vmovdqu %ymm9,288(%r14)\n"
        "vmovdqu %ymm10,320(%r14)\n vmovdqu %ymm11,352(%r14)\n"
        "vmovdqu %ymm12,384(%r14)\n vmovdqu %ymm13,416(%r14)\n"
        "vmovdqu %ymm14,448(%r14)\n vmovdqu %ymm15,480(%r14)\n"
        "pop %r14\n pop %r13\n pop %r12\n ret\n");
}
int main(int argc,char**argv){
    std::vector<unsigned char> elf;
    if(argc==2){FILE* f=fopen(argv[1],"rb");assert(f);fseek(f,0,SEEK_END);long length=ftell(f);rewind(f);
        elf.resize(length);assert(fread(elf.data(),1,length,f)==(size_t)length);fclose(f);}
    auto* image=(unsigned char*)VirtualAlloc((void*)0x800000000ull,0x2000000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    auto* heap=(unsigned char*)VirtualAlloc((void*)0x1000000000ull,0x10000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    assert(image && heap);
    if(!elf.empty()){
    for(auto site:std::vector<std::pair<unsigned,unsigned>>{{0x14d6f0,17},{0x1e4040,0x491},{0x1e2f10,0x1bd},{0x230780,108}})
        memcpy(image+site.first,elf.data()+site.first+0x4000,site.second);
    assert(iss_karma_signatures(image,0x2000000));assert(!iss_karma_signatures(image,0x14d700));
    for(unsigned site:{0x14d6f0,0x1e4040,0x1e2f10,0x230780}){
        image[site]^=1;assert(!iss_karma_signatures(image,0x2000000));image[site]^=1;
    }
    }else {assert(!iss_karma_signatures(image,0x2000000));puts("No game ELF supplied: positive game-signature checks skipped; rejection verified.");}
    iss_karma_jump(image+ISS_KARMA_SCORE_SETTER,(uintptr_t)&score_fixture);
    iss_karma_jump(image+ISS_KARMA_POWER_SETTER,(uintptr_t)&grant_fixture);
    DWORD old;assert(VirtualProtect(image+0x1e2000,0x3000,PAGE_EXECUTE_READ,&old));
    auto* a=image+ISS_KARMA_ATTRIBUTES;
    auto put=[&](unsigned offset,uint32_t value){memcpy(a+offset,&value,4);};
    auto flag=[&](int id){uint32_t v;memcpy(&v,a+0xe8+id*4,4);return v;};
    uintptr_t hero=(uintptr_t)(heap+0x8000),list=(uintptr_t)heap,manager=(uintptr_t)(heap+0x100);
    memcpy(image+0x1d0cfe0,&hero,8);memcpy(image+0x1d0d3a0,&list,8);memcpy(heap,&manager,8);
    *(int*)(image+0x1d0d3a8)=1;
    for(int id=0;id<222;++id){
        uintptr_t record=(uintptr_t)(heap+0x1000+id*0x30);memcpy((void*)(manager+0x108+id*8),&record,8);
        *(int*)record=id;((signed char*)record)[0x2c]=-1;((signed char*)record)[0x2d]=-1;
    }
    auto requirement=[&](int id,int side,int rank,unsigned bits=0){
        auto* p=heap+0x1000+id*0x30;p[0x24]=(unsigned char)bits;p[0x2c]=(unsigned char)side;p[0x2d]=(unsigned char)rank;
    };
    requirement(20,1,5);requirement(21,0,5);requirement(22,-1,3);
    requirement(23,1,5,1); // base/quest record is never granted
    requirement(24,1,6); // future/invalid rank is never granted
    requirement(25,1,5);put(0xe8+25*4,16); // script-forced off
    put(0xe8,7);put(0xe8+17*4,7); // Hero and Smoke earned; other families unavailable
    put(0xe8+139*4,0);
    float score=330;memcpy(a+0xc0,&score,4);put(0xc4,1);put(0xc8,1);
    memset(a+0x1000,0xa5,256);put(0xe8+8*4,0x12345678); // unrelated quest flag
    runtime_karma_attach(image);runtime_karma_tick();
    IssKarmaStatus s{};runtime_karma_status(&s);assert(s.ready && !s.mode);
    runtime_karma_request(1);runtime_karma_tick();runtime_karma_status(&s);
    assert(s.mode==1 && s.score==4300 && s.side==1 && s.rank==5 && !s.error);
    assert((flag(20)&7)==7 && flag(21)==0 && (flag(22)&7)==7);
    assert(flag(23)==0 && flag(24)==0 && flag(25)==16 && flag(172)==0 && flag(139)==0);
    unsigned grants=grant_events;runtime_karma_tick();assert(grant_events==grants && score_events==1);
    runtime_karma_request(2);runtime_karma_tick();runtime_karma_status(&s);
    assert(s.mode==2 && s.score==-4300 && s.side==0 && s.rank==5 && (flag(21)&7)==7);
    for(int i=0;i<20;++i){runtime_karma_request(i%2+1);runtime_karma_tick();}
    assert(flag(8)==0x12345678);for(int i=0;i<256;++i)assert(a[0x1000+i]==0xa5);
    reject_score=true;runtime_karma_request(1);runtime_karma_tick();runtime_karma_status(&s);
    assert(s.error==1 && s.mode==0);
    runtime_karma_attach(nullptr);runtime_karma_request(1);runtime_karma_status(&s);assert(!s.pending && !s.supported);

    auto* bridge=(unsigned char*)VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);assert(bridge);
    unsigned char nops[17];memset(nops,0x90,17);bridge[3000]=0xc3;
    unsigned n=iss_karma_bridge(bridge,(uintptr_t)&clobber_vectors,nops,(uintptr_t)(bridge+3000));assert(n<=ISS_KARMA_BRIDGE_MAX);
    assert(VirtualProtect(bridge,4096,PAGE_EXECUTE_READ,&old));FlushInstructionCache(GetCurrentProcess(),nullptr,0);
    unsigned char before[512],after[520];for(unsigned i=0;i<512;++i)before[i]=(unsigned char)(i*17+31);
    vector_harness(bridge,before,after);assert(!memcmp(before,after,512));assert(*(uint64_t*)(after+512)&1);
    puts("PASS: karma modes, repeated switches, eligible upgrades, story/quest canaries, failure paths, full YMM and flags preservation");
}
