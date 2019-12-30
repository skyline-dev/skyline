/**
 * @file fs.h
 * @brief Filesystem implementation.
 */

#pragma once

#include "account.h"
#include "types.h"

namespace nn
{
    typedef u64 ApplicationId;

    namespace fs
    {
        typedef u64 UserId;
        struct DirectoryEntry;

        struct FileHandle
        {
            void* handle;
        };

        struct DirectoryHandle
        {
            void* handle;
        };

        enum DirectoryEntryType
        {
            DIRECTORY,
            FILE
        };

        enum OpenMode 
        {
            OpenMode_Read = 1 << 0,
            OpenMode_Write = 1 << 1,
            OpenMode_Append = 1 << 2,

            OpenMode_ReadWrite = OpenMode_Read | OpenMode_Write
        };

        enum WriteOptionFlag 
        {
            WriteOptionFlag_Flush = 1 << 0
        };

        struct WriteOption
        {
            int flags;

            static WriteOption CreateOption(int flags)
            {
                WriteOption op;
                op.flags = flags;
                return op;
            }
        };

        // ROM
        Result QueryMountRomCacheSize(u64 *size);
        Result QueryMountRomCacheSize(u64 *size, nn::ApplicationId);
        Result MountRom(char const *name, void *buffer, u64 bufferSize);
        Result CanMountRomForDebug();
        Result CanMountRom(nn::ApplicationId);
        Result QueryMountRomOnFileCacheSize(u64 *, nn::fs::FileHandle);
        Result MountRomOnFile(char const *, nn::fs::FileHandle, void *, u64);

        // SAVE
        Result EnsureSaveData(nn::account::Uid const &);
        Result MountSaveData(char const *, nn::fs::UserId);

        // FILE
        Result GetEntryType(nn::fs::DirectoryEntryType* type, char const* path);
        Result CreateFile(char const* filepath, s64 size);
        Result OpenFile(nn::fs::FileHandle *, char const* path, s32);
        Result SetFileSize(FileHandle fileHandle, s64 filesize);
        void CloseFile(FileHandle fileHandle);
        Result FlushFile(FileHandle fileHandle);
        Result DeleteFile(char const* filepath);
        Result ReadFile(u64 *, nn::fs::FileHandle, s64, void *, u64, s32 const &);
        Result WriteFile(FileHandle handle, s64 fileOffset, void const *buff, u64 size, WriteOption const &option);
        Result GetFileSize(s64* size, FileHandle fileHandle);

        // DIRECTORY
        // there are three open modes; dir, file, all
        Result OpenDirectory(DirectoryHandle* handle, char const* path, s32 openMode);
        void CloseDirectory(DirectoryHandle directoryHandle);
        Result ReadDirectory(s64*, DirectoryEntry*, DirectoryHandle directoryHandle, s64);
        Result CreateDirectory(char const* directorypath);

        // SD
        Result MountSdCard(char const *);
        Result MountSdCardForDebug(char const *);
        bool IsSdCardInserted();
        Result FormatSdCard();
        Result FormatSdCardDryRun();
        bool IsExFatSupported();
    };
};