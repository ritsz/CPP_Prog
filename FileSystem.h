
#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

#include <iostream>
#include <stdint.h>
#include <cstring>

/*
 * Represents a file and directory element.
 */
class File {
    char *filename;
    bool isDirectory;
    /*
     * Add new private members after this.
     */
    public:
    void SetFileName(const char *filename);
    const char *GetFileName();
    void SetDirectory(bool isDirectory);
    bool IsDirectory();
};

/*
 * This is your file system.
 */
class FileSystem {
    enum {
            FILEFOUND = 0,
            DIRECTORYFOUND,
            EMPTY,
            NODIRECTORY,
            NOFILES,
            NOMEMORY,
            UNKNOWN
    } RETURN;
    
    public:

    FileSystem(); 
    ~FileSystem();
    void ls(const char *);
    bool InsertFile(const char *filename, bool isDirectory);
    bool DeleteFile(const char *filename);
};


/*
 * adler 32 hash expects data buffer and size of input.
 * returns 32 bit hash value.
 */
unsigned int hash_adler32(const void *buf, size_t buflength) {
    const uint8_t *buffer = (const uint8_t*)buf;

    uint32_t s1 = 1;
    uint32_t s2 = 0;

    for (size_t n = 0; n < buflength; n++) {
        s1 = (s1 + buffer[n]) % 65521;
        s2 = (s2 + s1) % 65521;
    }     
    return (s2 << 16) | s1;
}


class DirectoryCache {
    public:
    DirectoryCache();
    ~DirectoryCache();
    File*CacheLookup(const char *directoryName);
    bool CacheInsert(const char *directoryName, File *location);
    void ShowCache();
};


#endif /* _LINKEDLIST_H_ */
