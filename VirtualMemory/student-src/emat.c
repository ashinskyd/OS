#include "statistics.h"

#define MEMORY_ACCESS_TIME      100 /* 100 ns */
#define DISK_ACCESS_TIME   10000000 /* 10 million ns = 10 ms */

/*	Available variables from statistics that you may find useful
 *    count_pagefaults   - the number of page faults that occurred (includes required)                      
 *    count_tlbhits      - the number of tlbhits that occurred
 *    count_writes       - the number of stores/writes that occurred
 *    count_reads        - the number of reads that occurred
 * 	  disk_accesses		 - the number of disk accesses NOT including required faults
 * Any other values you might need are composites of the above values.  
 */

double compute_emat_all() {
   /* FIX ME - Compute the average memory access time, including required page faults
    * that occur when loading a new process.
    */
    double accessTime = 0;
    for (int i=0; i < count_pagefaults; i++)
    {
    	accessTime += (2*MEMORY_ACCESS_TIME);
    	accessTime += DISK_ACCESS_TIME;
    }
    for (i=0; i < ((count_reads+count_writes)-(count_pagefaults+count_tlbhits)); i++)
    {
    	accessTime += (2*MEMORY_ACCESS_TIME);
    }
    for (i=0; i<count_tlbhits; i++)
    {
    	accessTime += MEMORY_ACCESS_TIME;
    }
   return (accessTime/(count_reads+count_writes));
}

double compute_emat_unforced() {
   /* FIX ME - Compute the average memory access time NOT including required faults
    */
  double accessTime = 0;
    for (int i=0; i < count_diskaccesses; i++)
    {
    	accessTime += (2*MEMORY_ACCESS_TIME);
    	accessTime += DISK_ACCESS_TIME;
    }
    for (i=0; i < ((count_reads+count_writes)-(count_diskaccesses+count_tlbhits)); i++)
    {
    	accessTime += (2*MEMORY_ACCESS_TIME);
    }
    for (i=0; i<count_tlbhits; i++)
    {
    	accessTime += MEMORY_ACCESS_TIME;
    }
	
   return (accessTime/(count_reads+count_writes));
}
