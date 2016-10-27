#ifndef DISKINFO_H
#define DISKINFO_H

#include <stdint.h>

//todo: umv: in future should be many partitions
struct DiskInfo
{
    uint32_t diskIndex;
    uint32_t* diskStartAddress;
    uint32_t* diskEndAddress;
    FF_Disk_t disk;
};


#endif
