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
