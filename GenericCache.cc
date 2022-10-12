#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include "sim.h"
#include "GenericCache.hpp"

bool debug = false;

GenericCache::GenericCache(){

}

GenericCache::GenericCache(uint32_t blocksize, uint32_t size, uint32_t assoc, int cache_level, GenericCache* nextCache){

    this->blocksize = blocksize; this->size = size; this->assoc=assoc;this->nextCache=nextCache, this->cache_level=cache_level;

    number_of_sets = size/(assoc*blocksize);

    block_offset_width = (int)log2(blocksize);
    index_width = (int)log2(number_of_sets);
    tag_width = 32-block_offset_width-index_width;

    cacheBlocks = new BLOCKS *[number_of_sets]; //defining the entire cache
    for (int i=0; i<number_of_sets; i++){
        cacheBlocks[i] = new BLOCKS[assoc]; //assigning each set  
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

    int block_mask = (1<<block_offset_width)-1;
    int index_mask = (1<<index_width) -1;
    int tag_mask = (1<<tag_width) -1;

    *block_offset_addr = address & block_mask;
    *index_addr = (address>>block_offset_width) & index_mask;
    *tag_addr = (address>>(block_offset_width+index_width)) & tag_mask;


}

void GenericCache::cacheRead(uint32_t address){
    reads++;

    uint32_t block_offset_addr = 0;
    uint32_t index_addr = 0;
    uint32_t tag_addr = 0;

    addressDecoder(address, &block_offset_addr, &index_addr, &tag_addr);

    for (int block=0; block<assoc; block++){
        
        if (cacheBlocks[index_addr][block].tag == tag_addr){ //cache hit
            
            if(debug){
                printf("%x: Read Hit in Cache L%d\n", tag_addr, cache_level);
            }
            
            LRU_Update(index_addr, cacheBlocks[index_addr][block].lru);
            return;
        }
    }

    read_misses+=1;
    if (debug){
        printf("%x: Read Miss in Cache L%d\n",tag_addr, cache_level);
    }
    
    

    //evicting the victim block
    int blockToBeUpdated;
    

    blockToBeUpdated = evictVictim(address);
    CacheReadAdj(address);
    cacheBlocks[index_addr][blockToBeUpdated].v = true;
    cacheBlocks[index_addr][blockToBeUpdated].d = false;
    cacheBlocks[index_addr][blockToBeUpdated].tag = tag_addr;
    cacheBlocks[index_addr][blockToBeUpdated].address = address;
    LRU_Update(index_addr, cacheBlocks[index_addr][blockToBeUpdated].lru);
}

void GenericCache::cacheWrite(uint32_t address){
    writes++;
    uint32_t block_offset_addr = 0;
    uint32_t index_addr = 0;
    uint32_t tag_addr = 0;

    addressDecoder(address, &block_offset_addr, &index_addr, &tag_addr);
    

    for (int block=0; block<assoc; block++){
        
        if (cacheBlocks[index_addr][block].tag == tag_addr){ //cache hit
            
            if(debug){
                printf("%x: Write Hit in Cache L%d\n", tag_addr, cache_level);
            }
            
            cacheBlocks[index_addr][block].d = true;
            LRU_Update(index_addr, cacheBlocks[index_addr][block].lru);
            return;
        }
    }

    write_misses+=1;
    if(debug){
        printf("%x: Write Miss in Cache L%d\n", tag_addr, cache_level);
    }
    
    //evicting the victim block
    int blockToBeUpdated;
    
    blockToBeUpdated = evictVictim(address);
    CacheReadAdj(address); 
    cacheBlocks[index_addr][blockToBeUpdated].v = true;
    cacheBlocks[index_addr][blockToBeUpdated].d = true;
    cacheBlocks[index_addr][blockToBeUpdated].tag = tag_addr;
    cacheBlocks[index_addr][blockToBeUpdated].address = address;
    LRU_Update(index_addr, cacheBlocks[index_addr][blockToBeUpdated].lru);
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
        //cacheBlocks[index_addr][blockToBeUpdated].d == false;
        //cacheBlocks[index_addr][blockToBeUpdated].v == false;
    }
    //cacheBlocks[index_addr][blockToBeUpdated].d == false;
    return victimBlock;

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

void GenericCache::CacheReadAdj(uint32_t address){
    //writebacks+=1;
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
    float num = (float)read_misses+write_misses;
    float den = (float)reads+writes;
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
        
        /*
        for (int block=0; block<assoc; block++){
            if (cacheBlocks[set][block].d==true){
                dirty_bit = 'D';
            }
            printf("%x %c\t", cacheBlocks[set][block].tag, dirty_bit);
        }
        printf("\n");*/
    }
}


