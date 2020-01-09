#include "skyline/utils/cpputils.hpp"

namespace skyline {

    bool endsWith(std::string const& str1, std::string const& str2){
        return str2.size() <= str1.size()
            && str1.find(str2, str1.size() - str2.size()) != str1.npos;
    }

    Result Utils::walkDirectory(std::string const& root, std::function<void(nn::fs::DirectoryEntry const&, std::shared_ptr<std::string>)> callback) {
        Result r;

        nn::fs::DirectoryHandle rootHandle;
        R_TRY(nn::fs::OpenDirectory(&rootHandle, root.c_str(), nn::fs::OpenDirectoryMode_All));

        s64 entryCount;
        r = nn::fs::GetDirectoryEntryCount(&entryCount, rootHandle);
        if(R_FAILED(r)){
            nn::fs::CloseDirectory(rootHandle);

            return r;
        }

        nn::fs::DirectoryEntry* entryBuffer = new nn::fs::DirectoryEntry[entryCount];
        r = nn::fs::ReadDirectory(&entryCount, entryBuffer, rootHandle, entryCount);
        if(R_FAILED(r)){
            delete[] entryBuffer;
            nn::fs::CloseDirectory(rootHandle);

            return r;
        }

        for(int i = 0; i < entryCount; i++) {
            nn::fs::DirectoryEntry& entry = entryBuffer[i];

            std::string entryStr(entry.name);
            std::string fullPath = root;

            while(endsWith(fullPath, "/"))
                fullPath.pop_back();
            fullPath += "/";
            fullPath += entryStr;

            if(entry.type == nn::fs::DirectoryEntryType_Directory)
                r = walkDirectory(fullPath, callback);

            if(R_FAILED(r)){
                delete[] entryBuffer;
                nn::fs::CloseDirectory(rootHandle);

                return r;
            }

            callback(entry, std::make_shared<std::string>(fullPath));
        }
        
        delete[] entryBuffer;
        nn::fs::CloseDirectory(rootHandle);

        return r;
    }

    Result Utils::readEntireFile(std::string const& str, void** dataptr, u64* length) {
        if(dataptr == NULL)
            return -1;
        nn::fs::FileHandle handle;
        R_TRY(nn::fs::OpenFile(&handle, str.c_str(), nn::fs::OpenMode_Read));

        s64 size;
        Result r = nn::fs::GetFileSize(&size, handle);
        if(R_FAILED(r))
        {
            nn::fs::CloseFile(handle);
            return r;
        }

        if(length != NULL)
            *length = size;

        void* bin = malloc(size);
        r = nn::fs::ReadFile(handle, 0, bin, size);
        if(R_FAILED(r)){
            // file reading failed, cleanup
            free(bin);
        } else
            *dataptr = bin;
        nn::fs::CloseFile(handle);
        return r;
    }

    Result Utils::readFile(std::string const& str, s64 offset, void* data, size_t length){
        if(data == NULL)
            return -1;
        nn::fs::FileHandle handle;
        R_TRY(nn::fs::OpenFile(&handle, str.c_str(), nn::fs::OpenMode_Read));

        Result r;
        s64 fileSize;
        r = nn::fs::GetFileSize(&fileSize, handle);
        if(R_FAILED(r)){
            nn::fs::CloseFile(handle);
            return r;
        }

        length = MIN((s64)length, fileSize);

        r = nn::fs::ReadFile(handle, offset, data, length);

        nn::fs::CloseFile(handle);
        return r;
    }

    Result Utils::writeFile(std::string const& str, s64 offset, void* data, size_t length){

        nn::fs::DirectoryEntryType entryType;
        Result rc = nn::fs::GetEntryType(&entryType, str.c_str());
        if(entryType == nn::fs::DirectoryEntryType_Directory)
            return -1;

        if(rc == 0x202) { // Path does not exist 
            R_TRY(nn::fs::CreateFile(str.c_str(), offset + length));
        } else
            return rc;

        nn::fs::FileHandle handle;
        R_TRY(nn::fs::OpenFile(&handle, str.c_str(), nn::fs::OpenMode_ReadWrite | nn::fs::OpenMode_Append));

        Result r;
        s64 fileSize;
        r = nn::fs::GetFileSize(&fileSize, handle);
        if(R_FAILED(r)){
            nn::fs::CloseFile(handle);
            return r;
        }

        if((u64)fileSize < offset + length){ // make sure we have enough space
            r = nn::fs::SetFileSize(handle, offset + length);

            if(R_FAILED(r)){
                nn::fs::CloseFile(handle);
                return r;
            }
        }

        r = nn::fs::WriteFile(handle, offset, data, length, nn::fs::WriteOption::CreateOption(nn::fs::WriteOptionFlag_Flush));

        nn::fs::CloseFile(handle);
        return r;
    }
};