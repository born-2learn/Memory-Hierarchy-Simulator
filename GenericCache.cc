#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include "sim.h"
#include "constants.h"
#include "GenericCache.hpp"

//bool debug = true;

GenericCache::GenericCache(){

}

GenericCache::GenericCache(uint32_t blocksize, uint32_t size, uint32_t assoc, int N, int M, int cache_level, GenericCache* nextCache){

    this->blocksize = blocksize; this->size = size; this->assoc=assoc;this->nextCache=nextCache, this->cache_level=cache_level;
    this->N = N; this-> M = M;

    constant_M = M;
    number_of_sets = size/(assoc*blocksize);

    block_offset_width = (int)log2(blocksize);
    index_width = (int)log2(number_of_sets);
    tag_width = 32-block_offset_width-index_width;

    cacheBlocks = new BLOCKS *[number_of_sets]; //defining the entire cache
    for (int i=0; i<number_of_sets; i++){
        cacheBlocks[i] = new BLOCKS[assoc]; //assigning each set  
    }

    activeStreamBuffer = -1; activeMemoryBlock = -1;
     //Stream Buffer Init 
    if((N>0) && (M>0)){
        stream_buffer_present = true;
    }
    
    //Create stream buffer only if N>0 and M>0
    if (stream_buffer_present){
        streamBuffers = new StreamBuffers[N];
        //streamBuffers.memoryblocks = new int(M);
        for (int i=0; i<N; i++){
            streamBuffers[i].memoryblocks = new uint32_t(M);
            //streamBuffers[i].addressBlocks = new uint32_t(M);
            //streamBuffers[i].lru = i;
        }
        for (int i=0; i<N; i++){
            streamBuffers[i].lru = i;
            streamBuffers[i].v = false;
            streamBuffers[i].queue_pointer = 0;
            //printf("LRU %d",streamBuffers[i].lru );
            for (int j=0; j<M; j++){
                streamBuffers[i].memoryblocks[j]=0;
                //streamBuffers[i].addressBlocks[j]=0;
                //printf("%d ",streamBuffers[i].memoryblocks[j]);
            }
            //printf("\n");
        }
        //printf("N:%d M:%d\n", N, M);
        activeStreamBuffer = N-1;
        activeMemoryBlock = -1;

    }
    

    //Assigning zero values to all tags, valid bits and dirty bits
    for (int i =0; i<number_of_sets; i++ ){
        for (int j=0; j<assoc; j++){
            cacheBlocks[i][j].tag = 0;
            cacheBlocks[i][j].address = 0;
            cacheBlocks[i][j].lru = j;
            cacheBlocks[i][j].v = false;
            cacheBlocks[i][j].d = false;
        }
    }

}

void GenericCache::addressDecoder(uint32_t address,uint32_t *block_offset_addr,uint32_t *index_addr, uint32_t *tag_addr){

    //int block_mask = (1<<block_offset_width)-1;
    int block_mask = (1<<(tag_width+index_width)) - 1;
    int index_mask = (1<<index_width) -1;
    int tag_mask = (1<<tag_width) -1;

    //*block_offset_addr = address & block_mask;
    *block_offset_addr = (address>>(block_offset_width)) & block_mask;
    *index_addr = (address>>block_offset_width) & index_mask;
    *tag_addr = (address>>(block_offset_width+index_width)) & tag_mask;

}

void GenericCache::addressDecoder_sb(uint32_t address, uint32_t *index_addr, uint32_t *tag_addr){
    //int block_mask = (1<<(tag_width+index_width)) - 1;
    int index_mask = (1<<index_width) -1;
    int tag_mask = (1<<tag_width) -1;

    //*block_offset_addr = (address>>(block_offset_width)) & block_mask;
    *index_addr = (address) & index_mask;
    *tag_addr = (address>>(index_width)) & tag_mask;
}

void GenericCache::cacheRead(uint32_t address){
    reads++;

    uint32_t block_offset_addr = 0;
    uint32_t index_addr = 0;
    uint32_t tag_addr = 0;

    
    addressDecoder(address, &block_offset_addr, &index_addr, &tag_addr);
    
    //else if (prefetch_request==true){
    //    addressDecoder_sb(address, &index_addr, &tag_addr);
    //}


    bool streamBuffer_HIT = false;
    if (stream_buffer_present){
        streamBuffer_HIT = readStreamBuffer(block_offset_addr);
    }

    for (int block=0; block<assoc; block++){
        
        if (cacheBlocks[index_addr][block].tag == tag_addr){ //cache hit
            
            if(debug){
                printf("%x: Read Hit in Cache L%d\n", tag_addr, cache_level);
            }
            LRU_Update(index_addr, cacheBlocks[index_addr][block].lru);
            
            if (stream_buffer_present){
                if (streamBuffer_HIT){
                    prefetch(block_offset_addr);
                }
            }
            return;
            
        }
    }

    if (!streamBuffer_HIT){
        read_misses+=1;
        CacheReadAdj(address);
        
    }
    if (debug){
        printf("%x: Read Miss in Cache L%d\n",tag_addr, cache_level);
    }
    
    

    //evicting the victim block
    int blockToBeUpdated;
    

    blockToBeUpdated = evictVictim(address);
   
    cacheBlocks[index_addr][blockToBeUpdated].v = true;
    cacheBlocks[index_addr][blockToBeUpdated].d = false;
    cacheBlocks[index_addr][blockToBeUpdated].tag = tag_addr;
    cacheBlocks[index_addr][blockToBeUpdated].address = address;
    LRU_Update(index_addr, cacheBlocks[index_addr][blockToBeUpdated].lru);
    prefetch(block_offset_addr);
}

void GenericCache::cacheWrite(uint32_t address){
    writes++;
    uint32_t block_offset_addr = 0;
    uint32_t index_addr = 0;
    uint32_t tag_addr = 0;

    
    addressDecoder(address, &block_offset_addr, &index_addr, &tag_addr);
    
    bool streamBuffer_HIT =false;
    streamBuffer_HIT = readStreamBuffer(tag_addr);

    for (int block=0; block<assoc; block++){
        
        if (cacheBlocks[index_addr][block].tag == tag_addr){ //cache hit
            
            if(debug){
                printf("%x: Write Hit in Cache L%d\n", tag_addr, cache_level);
            }
            
            cacheBlocks[index_addr][block].d = true;
            LRU_Update(index_addr, cacheBlocks[index_addr][block].lru);
            
            if (stream_buffer_present){
                if (streamBuffer_HIT){
                    prefetch(block_offset_addr);
                    //LRU_Update_stream_buffer(streamBuffers[i].lru);
                }
            }
            return;
            
        }
    }
    //cache miss

    if(!streamBuffer_HIT){
        write_misses+=1;
    }
    if(debug){
        printf("%x: Write Miss in Cache L%d\n", tag_addr, cache_level);
    }
    
    //evicting the victim block
    int blockToBeUpdated;
    
    blockToBeUpdated = evictVictim(address);
    if (streamBuffer_HIT == false){
        CacheReadAdj(address);
        //
    }
    cacheBlocks[index_addr][blockToBeUpdated].v = true;
    cacheBlocks[index_addr][blockToBeUpdated].d = true;
    cacheBlocks[index_addr][blockToBeUpdated].tag = tag_addr;
    cacheBlocks[index_addr][blockToBeUpdated].address = address;
    LRU_Update(index_addr, cacheBlocks[index_addr][blockToBeUpdated].lru);
    prefetch(block_offset_addr);
}

uint32_t GenericCache::evictVictim(uint32_t address){
    uint32_t block_offset_addr = 0;
    uint32_t index_addr = 0;
    uint32_t tag_addr = 0;

    addressDecoder(address, &block_offset_addr, &index_addr, &tag_addr);

    
    uint32_t victimBlock;
    uint32_t oldAddress;

    // Finding the cache block with max LRU count (least recently used)
    for(int block=0; block<assoc; block++){
        if (cacheBlocks[index_addr][block].lru== (assoc-1)){
            victimBlock = block;
            break;
        }
    }

    //Finding out if the block with max LRU count is dirty
    if (cacheBlocks[index_addr][victimBlock].d==true){

        //get the old address of victim block
        oldAddress = cacheBlocks[index_addr][victimBlock].address;

        if(debug){
            printf("dirty block identified");
        }
        CacheWriteAdj(oldAddress);
        
    }
    return victimBlock;

}

bool GenericCache::readStreamBuffer(uint32_t address){
    
    for (int i=0; i<N; i++){
        for (int j=0; j<M; j++){
            if (streamBuffers[i].memoryblocks[j] == address){
                activeStreamBuffer = i;
                activeMemoryBlock = j;
                LRU_Update_stream_buffer(streamBuffers[i].lru);
                return true;
            }
        }
    }
    return false;
}

void GenericCache::prefetch(uint32_t block_offset_addr){
    if (debug){
        printf("Prefetching for address %x\n",block_offset_addr);
    }
    int presentAt = -1; //present at what memory location
    int presentIn = -1; //present in which stream buffer

    int lru_sb = 0; //Stream buffer with highest LRU val
    for (int i=0; i<N; i++){
        //printf("%d%d ", streamBuffers[i].lru, (N-1));
        if (streamBuffers[i].lru == (N-1)){
                lru_sb = i;
                if(debug) {
                    //printf("LRU SB: %d \n", lru_val);
                }
        }
    }
    
    for (int i=0; i<N; i++){
        
        for (int j=0; j<M; j++){
            if (streamBuffers[i].memoryblocks[j]==block_offset_addr){
                presentIn = i; 
                presentAt = j;
                streamBuffers[i].queue_pointer = j;
            }
        }
    }
    if (presentAt==-1){
        //streamBuffers[lru_sb].queue_pointer = presentAt+1;
        //printf("%d %d \n", N, presentAt);
        streamBuffers[lru_sb].v = true;
        LRU_Update_stream_buffer(streamBuffers[lru_sb].lru);
        for (int j=0; j<M; j++){
    
            if (debug) 
            {
                //printf("BO address: %x ",block_offset_addr);
            }

            uint32_t newAddress = ((block_offset_addr+j+1)<<block_offset_width);

            if (debug)
            {
                //printf("New address: %x ",newAddress);
            }
            bool isPresentinSB = false;
            for (int k=0; k<M; k++){
                if (streamBuffers[lru_sb].memoryblocks[k] == (block_offset_addr + j + 1)){
                    isPresentinSB = true;
                    break;
                }
            }
            if (!isPresentinSB){
                CacheReadAdj(newAddress);
                prefetch_read++;

                streamBuffers[lru_sb].memoryblocks[j] = (block_offset_addr + j + 1);
            }
            
            
            if (debug){
                printf("%x ", streamBuffers[lru_sb].memoryblocks[j]);
            }
                
        }
        if (debug){
            printf("\n");
        }
    }
    else if (presentAt!=-1){
        //streamBuffers[presentIn].queue_pointer = presentAt+1;
        streamBuffers[presentIn].v = true;
        LRU_Update_stream_buffer(streamBuffers[presentIn].lru);

        //experimental rotation code
        /*
        int pos = presentAt;
        int dir = 0;
        uint32_t temp = 0;
        while(pos)  
        {  
            if(dir)  
            {  
                temp = streamBuffers[presentIn].memoryblocks[0];  
                for(int i = 0; i < M - 1; i++)  
                    streamBuffers[presentIn].memoryblocks[i] = streamBuffers[presentIn].memoryblocks[i + 1];  
    
                streamBuffers[presentIn].memoryblocks[M - 1] = temp;  
            }  
            else  
            {  
                temp = streamBuffers[presentIn].memoryblocks[N - 1];  
                for(int i = M - 1; i > 0; i--)  
                    streamBuffers[presentIn].memoryblocks[i] = streamBuffers[presentIn].memoryblocks[i - 1];  
    
                streamBuffers[presentIn].memoryblocks[0] = temp;  
            }  
    
            pos--;  
        }
        for (int j=presentAt; j<M; j++){
            uint32_t newAddress = ((block_offset_addr+(presentAt+1)+ j)<<block_offset_width);
            CacheReadAdj(newAddress);
            prefetch_read++;
            streamBuffers[presentIn].memoryblocks[j] = (block_offset_addr + (presentAt+1)+ j  );
        }*/
        
        for (int j=0; j<=presentAt; j++){
            uint32_t newAddress = ((block_offset_addr+(M-presentAt)+ j)<<block_offset_width);

            bool isPresentinSB = false;
            for (int k=0; k<M; k++){
                if (streamBuffers[presentIn].memoryblocks[k] == (block_offset_addr + j + 1)){
                    isPresentinSB = true;
                    break;
                }
            }
            if (!isPresentinSB){
                CacheReadAdj(newAddress);
                prefetch_read++;
                streamBuffers[presentIn].memoryblocks[j] = (block_offset_addr + (M-presentAt)+ j  );
            }
            
        }
    }
    
    
}

void GenericCache::LRU_Update(uint32_t index_addr, int lru_val){
    for (int i=0; i<assoc; i++){
        if (cacheBlocks[index_addr][i].lru <lru_val){
            cacheBlocks[index_addr][i].lru++;

        }
        else if (cacheBlocks[index_addr][i].lru == lru_val){
            cacheBlocks[index_addr][i].lru = 0;
        }
    }
}

void GenericCache::LRU_Update_stream_buffer(int lru_sb){
    
    for (int i=0; i<N; i++){
        if (streamBuffers[i].lru < lru_sb){
            streamBuffers[i].lru++;
            
        }
        else if (streamBuffers[i].lru == lru_sb){
            streamBuffers[i].lru = 0;
        }
        
    }
}

void GenericCache::CacheReadAdj(uint32_t address){
    
    if (nextCache == NULL){
    }
    else{
        nextCache->cacheRead(address);
    }
}

void GenericCache::CacheWriteAdj(uint32_t address){
    writebacks+=1;

    if(nextCache == NULL){
    }
    else{
        nextCache->cacheWrite(address);
    }
}

void GenericCache::PrintContents(){
    double num = (double)read_misses+write_misses;
    double den = (double)reads+writes;
    miss_rate = num/den;
    
    for (int set=0; set<number_of_sets; set++){
        bool printSet = false;

        // check if there is at lease 1 valid block in set
        for (int i=0; i<assoc; i++){
            if (cacheBlocks[set][i].v){
                printSet = true;
                break;
            }
        }
        char dirty_bit = ' ';
        if (printSet){
            printf("set  %d: ", set);
            for (int k=0; k<assoc; k++){
                for (int i=0; i<assoc; i++){
                    if (cacheBlocks[set][i].lru==k){
                        if (cacheBlocks[set][i].v){
                            if (cacheBlocks[set][i].d==true){
                                dirty_bit = 'D';
                            }
                            else if(cacheBlocks[set][i].d==false) {
                                dirty_bit = ' ';
                            }
                            printf("%x %c  ", cacheBlocks[set][i].tag, dirty_bit);
                        }
                    }
                }
            }
            printf("\n");
        }
        
    }
}

void selection_sort(uint32_t* arr, int len)
{
    uint32_t min, temp;
    // Iterating for each element except first element.
    for (int i = 0; i < len - 1; i++)
    {
        min = arr[i];
        // Finding the minimum element from unsorted array and adding to sorted array.
        for (int j = i; j < len; j++)
        {
            if (min > arr[j])
            {
                min = arr[j];
                temp = j;
            }
        }
        arr[temp] = arr[i];
        arr[i] = min;
    }
}

void GenericCache::PrintStreamBufferContents(){
    /*
    for (int i=0; i<N; i++){
        size_t len = sizeof(streamBuffers[i].memoryblocks)/sizeof(streamBuffers[i].memoryblocks[0]);
        selection_sort(streamBuffers[i].memoryblocks, len);

        for (int j = 0; j < M; j++)
        {
            //printf("%x ",streamBuffers[i].memoryblocks[j]);
        }
        //printf("\n");
    }*/

    for (int i=0; i<N; i++){
        for (int j=0; j<N; j++){
            if (streamBuffers[j].lru == i){
                if (streamBuffers[j].v){

                    //printf("%d ", streamBuffers[j].queue_pointer);
                    
                    for (int memBlocks=streamBuffers[j].queue_pointer; memBlocks<M; memBlocks++){
                        printf("%x ", streamBuffers[j].memoryblocks[memBlocks]);
                    }
                    for (int memBlocks=0; memBlocks<streamBuffers[j].queue_pointer; memBlocks++){
                        printf("%x ", streamBuffers[j].memoryblocks[memBlocks]);
                    }
                    //for (int memBlocks=0; memBlocks<M; memBlocks++){
                    //    printf("%x ", streamBuffers[j].memoryblocks[memBlocks]);
                    //}
                
                }printf("\n");
            }
        }
        
    }

}