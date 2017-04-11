#include <iostream>
#include <cstring>
#include "FileSystem.h"

DirectoryCache fcache;
FileSystem fs;

void File::SetFileName(const char *filename)
{
}

const char *File::GetFileName()
{
}

void File::SetDirectory(bool isDirectory)
{
}

bool File::IsDirectory()
{
}

FileSystem::FileSystem()
{
}

FileSystem::~FileSystem()
{
}

void FileSystem::ls(const char *file)
{
}


bool FileSystem::InsertFile(const char *filename, bool IsDir)
{
}

bool FileSystem::DeleteFile(const char *filename)
{
}

DirectoryCache::DirectoryCache()
{
}

DirectoryCache::~DirectoryCache()
{
}

File *DirectoryCache::CacheLookup(const char *directoryName)
{
}

bool DirectoryCache::CacheInsert(const char *directoryName, File *location)
{
}

void DirectoryCache::ShowCache()
{
}


int main()
{
    int choice = 0;

    do {
        char filename[50];
        std::cout << "\n **********************" << std::endl;
        std::cout << " 1. Insert a file." << std::endl;
        std::cout << " 2. Insert a directory." << std::endl;
        std::cout << " 3. Delete a file." << std::endl;
        std::cout << " 4. Delete a directory." << std::endl;
        std::cout << " 5. ls" << std::endl;
        std::cout << " 6. Show Cache" << std::endl;
        std::cout << " 0. Exit" << std::endl;
        std::cout << " Enter your choice. : ";
        std::cin  >> choice;

        if (choice > 0 && choice < 6)
        {
            std::cout << "\t\tInput File/Directory Name : ";
            std::cin  >> filename;
        }

        switch (choice) {
            case 1:
                fs.InsertFile(filename, false);
                break;
            case 2:
                fs.InsertFile(filename, true);
                break;
            case 3:
                fs.DeleteFile(filename);
                break;
            case 4:
                fs.DeleteFile(filename);
                break;
            case 5:
                fs.ls(filename);
                break;
            case 6:
                fcache.ShowCache();
                break;
            default:
                break;
        }

    } while (choice != 0);

}
