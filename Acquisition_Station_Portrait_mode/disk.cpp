#include "disk.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define CMD "df -P -l -x iso9660 -x iso9600 | awk \'{if(NR!=1 && NF==6) print $2\" \"$3\" \"$6 }\'"

int GetDiskInfo(DiskInfo *diskPtr)
{
    FILE *fp = popen(CMD, "r");
    if(fp == NULL) {
        return -1;
    }
    diskPtr->allItem = {0};
    memcpy(diskPtr->allItem.diskName, "total", 6);
    DiskItem item = {0};
    while( fscanf(fp, "%" PRIu64" %" PRIu64" %s", &item.diskTotal, &item.diskUse, item.diskName) == 3) {
        item.diskUsePer = ceil(item.diskUse*100.0/item.diskTotal);
        diskPtr->items.push_back(item);
        //
        diskPtr->allItem.diskTotal += item.diskTotal;
        diskPtr->allItem.diskUse += item.diskUse;
        if(item.diskUsePer > diskPtr->allItem.diskUsePer){
            diskPtr->allItem.diskUsePer = item.diskUsePer;
        }
    }
    pclose(fp);
    return 0;
}
