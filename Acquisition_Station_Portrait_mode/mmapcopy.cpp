#include "mmapcopy.h"





void copyFileUsingMemoryMapping(const char* sourceFile, const char* destinationFile, size_t blockSize) {
    // 打开源文件
    int srcFd = open(sourceFile, O_RDONLY);
    if (srcFd == -1) {
        perror("Error opening source file");
        return;
    }

    // 获取源文件大小
    struct stat statBuf;
    if (fstat(srcFd, &statBuf) == -1) {
        perror("Error getting file size");
        close(srcFd);
        return;
    }
    size_t fileSize = statBuf.st_size;

    // 打开目标文件
    int destFd = open(destinationFile, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (destFd == -1) {
        perror("Error opening destination file");
        close(srcFd);
        return;
    }

    // 调整目标文件大小
    if (ftruncate(destFd, fileSize) == -1) {
        perror("Error resizing destination file");
        close(srcFd);
        close(destFd);
        return;
    }

    // 循环分块拷贝文件
    size_t offset = 0;
    while (offset < fileSize) {
        // 计算当前块的大小（最后一块可能小于 blockSize）
        size_t currentBlockSize = (offset + blockSize <= fileSize) ? blockSize : fileSize - offset;

        // 映射源文件当前块
        void* srcMapping = mmap(nullptr, currentBlockSize, PROT_READ, MAP_PRIVATE, srcFd, offset);
        if (srcMapping == MAP_FAILED) {
            perror("Error mapping source file to memory");
            close(srcFd);
            close(destFd);
            return;
        }

        // 映射目标文件当前块
        void* destMapping = mmap(nullptr, currentBlockSize, PROT_READ | PROT_WRITE, MAP_SHARED, destFd, offset);
        if (destMapping == MAP_FAILED) {
            perror("Error mapping destination file to memory");
            munmap(srcMapping, currentBlockSize);
            close(srcFd);
            close(destFd);
            return;
        }

        // 拷贝当前块内容
        memcpy(destMapping, srcMapping, currentBlockSize);

        // 解除映射
        munmap(srcMapping, currentBlockSize);
        munmap(destMapping, currentBlockSize);

        // 更新偏移量
        offset += currentBlockSize;
    }

    // 关闭文件描述符
    close(srcFd);
    close(destFd);

    std::cout << "File copied successfully using memory mapping!" << std::endl;
}



int _test_mmap_main(int argc, char *argv[]) {

    //  const char* sourceFile = "source.txt";
    //  const char* destinationFile = "destination.txt";

    //  string cmd = "df -h | grep   ";
    //  cmd	= cmd + argv[1];
    //  cmd.c_str()

      string sourceFile = argv[1];
    string destinationFile = argv[2];

    size_t blockSize = 400 * 1024 * 1024; // 400MB 每块

    copyFileUsingMemoryMapping(sourceFile.c_str(), destinationFile.c_str(), blockSize);

    return 0;
}


// 编译
//  g++ -o testnpm testnmp.cpp




