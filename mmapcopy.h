#ifndef MMAPCOPY_H
#define MMAPCOPY_H


#include <iostream>
#include <fstream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>



#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <sstream>


using namespace std;

void copyFileUsingMemoryMapping(const char* sourceFile, const char* destinationFile, size_t blockSize) ;


#endif // MMAPCOPY_H
