#ifndef _DISK_INFO_H_
#define _DISK_INFO_H_

#include <inttypes.h>
#include <vector>

typedef struct {
    char diskName[64];
    uint64_t diskTotal;
    uint64_t diskUse;
    uint64_t diskUsePer;
}DiskItem;

typedef struct {
    DiskItem allItem;
    std::vector<DiskItem> items;
}DiskInfo;

int GetDiskInfo(DiskInfo *diskPtr);

#endif //_DISK_INFO_H_
