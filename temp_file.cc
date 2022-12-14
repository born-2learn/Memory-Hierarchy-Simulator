struct generic_cache_block{
   bool v; // valid bit
   bool d; // dirty bit
   uint32_t tag; // tag bits (hex to decimal)
   int lru;
   //uint32_t data; // data (will be NULL, not required in Project 1)
};
/*
struct generic_cache{
   uint32_t SETS;
   uint32_t ASSOC;
   
   block_contents **blocks;
   blocks = new block_contents*[SETS];

};

typedef struct {
   uint32_t BLOCKSIZE;
   uint32_t SIZE;
   uint32_t ASSOC; // blocks per set
   uint32_t sets;
   struct set sets;
} generic_cache_module;*/


//main

if ((!params.L2_SIZE) && (!params.L2_ASSOC)){
      generic_cache_block L2[L2_SETS][params.L2_ASSOC];
      for (int i=0; i<L2_SETS; i++)
      {
         for (int j=0; j<params.L2_ASSOC; j++){
            L2[i][j].v = false;
            L2[i][j].d = false;
            L2[i][j].tag = 0;
            L2[i][j].lru = 0;
         }
      }
   }
   

   generic_cache_block L1[L1_SETS][params.L1_ASSOC];

   for (int i=0; i<L1_SETS; i++)
   {
      for (int j=0; j<params.L1_ASSOC; j++){
         L1[i][j].v = false;
         L1[i][j].d = false;
         L1[i][j].tag = 0;
         L1[i][j].lru = 0;
      }
   }
   printf("%d\n", L1[0][0].tag);
   //printf("%d\n", L2[0][0].tag);


void addressDecoder(uint32_t address, uint32_t blocksize, int sets, int *tb, int *ib, int *bb){

   int block_offset_width = (int)log2(blocksize);
   int index_width = (int)log2(sets);
   int tag_width = 32-block_offset_width-index_width;

   int block_mask = (1<<index_width)-1;
   int block_offset_bits = address & block_mask;
   int index_mask = (1<<index_width) -1;
   int index_bits = (address>>block_offset_width) & index_mask;
   int tag_mask = (1<<tag_width) -1;
   int tag_bits = (address>>(block_offset_width+index_width)) & tag_mask;

   *tb = tag_bits; *ib=index_bits; *bb=block_offset_bits;

   printf("%x: %x %x %x\n",address, tag_bits, index_bits, block_offset_bits);
}

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