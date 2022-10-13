#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include "sim.h"
#include "GenericCache.hpp"

void addressDecoder(uint32_t address, uint32_t blocksize, int sets, int *tb, int *ib, int *bb);


int main (int argc, char *argv[]) {

   FILE *fp;			// File pointer.
   char *trace_file;		// This variable holds the trace file name.
   cache_params_t params;	// Look at the sim.h header file for the definition of struct cache_params_t.
   char rw;			// This variable holds the request's type (read or write) obtained from the trace.
   uint32_t addr;		// This variable holds the request's address obtained from the trace.
				// The header file <inttypes.h> above defines signed and unsigned integers of various sizes in a machine-agnostic way.  "uint32_t" is an unsigned integer of 32 bits.
   bool stream_buffer_present = false;
   // Exit with an error if the number of command-line arguments is incorrect.
   if (argc != 9) {
      printf("Error: Expected 8 command-line arguments but was provided %d.\n", (argc - 1));
      exit(EXIT_FAILURE);
   }
    
   // "atoi()" (included by <stdlib.h>) converts a string (char *) to an integer (int).
   params.BLOCKSIZE = (uint32_t) atoi(argv[1]);
   params.L1_SIZE   = (uint32_t) atoi(argv[2]);
   params.L1_ASSOC  = (uint32_t) atoi(argv[3]);
   params.L2_SIZE   = (uint32_t) atoi(argv[4]);
   params.L2_ASSOC  = (uint32_t) atoi(argv[5]);
   params.PREF_N    = (uint32_t) atoi(argv[6]); // number of stream buffers
   params.PREF_M    = (uint32_t) atoi(argv[7]); //number of memory blocks in each stream buffer
   trace_file       = argv[8];

   GenericCache L1; GenericCache L2;
   if (params.L2_SIZE !=0){
      L2 = GenericCache(params.BLOCKSIZE, params.L2_SIZE, params.L2_ASSOC, params.PREF_N, params.PREF_M, 2, NULL);
      L1 = GenericCache(params.BLOCKSIZE, params.L1_SIZE, params.L1_ASSOC, 0, 0, 1, &L2);
   }
   else{
      L1 = GenericCache(params.BLOCKSIZE, params.L1_SIZE, params.L1_ASSOC, params.PREF_N, params.PREF_M, 1, NULL);
   }
   int L1_SETS = params.L1_SIZE/(params.L1_ASSOC*params.BLOCKSIZE);
   int L2_SETS = params.L2_SIZE/(params.L2_ASSOC*params.BLOCKSIZE);

   
   

   if ((params.PREF_N>0) && (params.PREF_M>0)){
      stream_buffer_present = true;
   }


   // Open the trace file for reading.
   fp = fopen(trace_file, "r"); //fp - file pointer
   if (fp == (FILE *) NULL) {
      // Exit with an error if file open failed.
      printf("Error: Unable to open file %s\n", trace_file);
      exit(EXIT_FAILURE);
   }
    
   // Print simulator configuration.
   printf("===== Simulator configuration =====\n");
   printf("BLOCKSIZE:  %u\n", params.BLOCKSIZE);
   printf("L1_SIZE:    %u\n", params.L1_SIZE);
   printf("L1_ASSOC:   %u\n", params.L1_ASSOC);
   printf("L2_SIZE:    %u\n", params.L2_SIZE);
   printf("L2_ASSOC:   %u\n", params.L2_ASSOC);
   printf("PREF_N:     %u\n", params.PREF_N);
   printf("PREF_M:     %u\n", params.PREF_M);
   printf("trace_file: %s\n", trace_file);
   //printf("===================================\n");


   // Read requests from the trace file and echo them back.
   while (fscanf(fp, "%c %x\n", &rw, &addr) == 2) {	// Stay in the loop if fscanf() successfully parsed two tokens as specified.

      if (rw == 'r'){
         //addressDecoder(addr, params.BLOCKSIZE, L1_SETS, &tb, &ib, &bb);
         //printf("%x %x %x\n", tb, ib, bb);

         //l1_readmisses++;
         //printf("%d\n", l1_readmisses);
         L1.cacheRead(addr);
         //printf("r %x\n", addr);
      }
         
      else if (rw == 'w'){
         //printf("w %x\n", addr);
         L1.cacheWrite(addr);
      }
         
      else {
         printf("Error: Unknown request type %c.\n", rw);
	 exit(EXIT_FAILURE);
      }

      ///////////////////////////////////////////////////////
      // Issue the request to the L1 cache instance here.
      ///////////////////////////////////////////////////////
      
    }

   printf("\n"); 
   printf("===== L1 contents =====\n");
   L1.PrintContents();

   if ((params.L2_SIZE ==0)&& (stream_buffer_present)){
      //print stream buffer contents
      printf("\n===== Stream Buffer(s) contents =====\n");
      L1.PrintStreamBufferContents();
   }

   if (params.L2_SIZE !=0){
      printf("\n");
      printf("===== L2 contents =====\n");
      L2.PrintContents();
   }
   //printf("%d", stream_buffer_present);
   if ((params.L2_SIZE !=0)&&(stream_buffer_present)){
      //print stream buffer contents
      printf("\n===== Stream Buffer(s) contents =====\n");
      L2.PrintStreamBufferContents();
   }

   printf("\n===== Measurements =====");
   printf("\na. L1 reads:\t\t%d", L1.reads);
   printf("\nb. L1 read misses:\t\t%d", L1.read_misses);
   printf("\nc. L1 writes:\t\t%d", L1.writes);
   printf("\nd. L1 write misses:\t\t%d", L1.write_misses);
   printf("\ne. L1 miss rate:\t\t%.4f", L1.miss_rate);
   printf("\nf. L1 writebacks:\t\t%d", L1.writebacks);
   printf("\ng. L1 prefetches:\t\t%d", L1.prefetch_read);
   printf("\nh. L2 reads (demand):\t\t%d", L2.reads);
   printf("\ni. L2 read misses (demand):\t\t%d", L2.read_misses);
   printf("\nj. L2 reads (prefetch):\t\t%d", L2.prefetch_read);
   printf("\nk. L2 read misses (prefetch):\t\t%d", L2.prefetch_read_misses);
   

   printf("\nl. L2 writes:\t\t%d", L2.writes);
   printf("\nm. L2 write misses:\t\t%d", L2.write_misses);
   if (params.L2_SIZE !=0){
      double l2_miss_rate = 0.0;
      double l2_read_misses = (double)L2.read_misses; double l2_reads = (double) L2.reads;
      l2_miss_rate = (double) l2_read_misses/l2_reads;
      printf("\nn. L2 miss rate:\t\t%.4f", l2_miss_rate);
   }
   else{
      printf("\nn. L2 miss rate:\t\t%.4f",0.0000 );
   }
   
   printf("\no. L2 writebacks:\t\t%d", L2.writebacks);
   printf("\np. L2 prefetches:\t\t%d", L2.prefetch_read);
   if (params.L2_SIZE !=0){
   printf("\nq. memory traffic:\t\t%d", L2.write_misses+L2.read_misses+L2.writebacks); 
   }
   else{
      printf("\nq. memory traffic:\t\t%d\n", L1.write_misses+L1.read_misses+L1.writebacks); 
   }

   return(0);
}