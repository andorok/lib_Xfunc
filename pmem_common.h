#ifndef _PMEM_COMMON_H
#define _PMEM_COMMON_H

#ifdef __KERNEL__
  #include <linux/types.h>
#else
  #include <sys/types.h>
#endif

typedef struct
{
    unsigned long virt;
    unsigned long phys;
} pmem_address_t;

/* ioctl "switch" flags */
#define PMEM_MAGIC 'y'

#define CONVERT_ADDRESS     _IOWR(PMEM_MAGIC, 1, pmem_address_t*)

#endif

