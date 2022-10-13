
struct BLOCKS{
    bool v;
    bool d;
    uint32_t tag;
    uint32_t address;
    int lru;
};
struct StreamBuffers{ // N stream buffers to be created and initialized
    bool v;
    int lru;
    uint32_t *memoryblocks; // memory blocks of stream buffer of size M
};

class GenericCache
{
private:
    uint32_t blocksize;
    uint32_t size;
    uint32_t assoc;
    int N;
    int M;
    int cache_level;
    struct StreamBuffers *streamBuffers;
    struct BLOCKS **cacheBlocks;
    GenericCache *nextCache;

    //uint32_t block_offset_addr;
    //uint32_t index_addr;
    //uint32_t tag_addr;

    int number_of_sets;
    int block_offset_width; int index_width; int tag_width;
    bool stream_buffer_present = false;

    int activeStreamBuffer;//n
    int activeMemoryBlock;//m

    void LRU_Update(uint32_t, int); //accepts index and the LRU 
    void LRU_Update_stream_buffer(int);
    void CacheWriteAdj(uint32_t);
    void CacheReadAdj(uint32_t);
    uint32_t evictVictim(uint32_t);

public:

    int reads=0; int read_misses=0; int writes=0; int write_misses=0; 
    double miss_rate=0; int writebacks = 0;

    int prefetch_read=0; int prefetch_read_misses=0;

    GenericCache();
    GenericCache(uint32_t, uint32_t, uint32_t, int, int, int, GenericCache*);

    void addressDecoder(uint32_t, uint32_t*, uint32_t*, uint32_t*);
    void cacheRead(uint32_t);
    void cacheWrite(uint32_t);
    bool readStreamBuffer(uint32_t);
    void prefetch(uint32_t);
    void PrintContents();
    void PrintStreamBufferContents();
};


