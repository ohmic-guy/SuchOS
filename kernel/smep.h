#pragma once
#include <stdint.h>

void smep_smap_init(void);
int smep_enabled(void);
int smap_enabled(void);
void smap_allow(void);     /* CLAC — permit user memory access  */
void smap_deny(void);      /* STAC — re-enable SMAP protection  */
uint32_t rdtsc_seed(void); /* RDTSC — random seed for ASLR      */