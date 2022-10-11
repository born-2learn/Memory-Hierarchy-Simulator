
struct BLOCKS{
    bool v;
    bool d;
    uint32_t tag;
    int lru;
};

class GenericCache
{
private:
    uint32_t blocksize;
    uint32_t size;
    uint32_t assoc;

    struct BLOCKS **cacheBlocks;
    GenericCache *nextCache;

    //uint32_t block_offset_addr;
    //uint32_t index_addr;
    //uint32_t tag_addr;

    int number_of_sets;
    int block_offset_width; int index_width; int tag_width;

    void LRU_Update(uint32_t, int); //accepts index and the LRU 
    void CacheWriteAdj(uint32_t);
    void CacheReadAdj(uint32_t);
    uint32_t evictVictim(uint32_t);

public:

    int reads=0; int read_misses=0; int writes=0; int write_misses=0; 
    float miss_rate=0; int writebacks = 0;

    int prefetch_read=0; int prefetch_read_misses=0;

    GenericCache();
    GenericCache(uint32_t, uint32_t, uint32_t,  GenericCache*);

    void addressDecoder(uint32_t, uint32_t*, uint32_t*, uint32_t*);
    void cacheRead(uint32_t);
    void cacheWrite(uint32_t);
    void PrintContents();
};


