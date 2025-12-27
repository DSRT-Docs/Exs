// src/Core/Platform/Windows/FileSystemWindows.cpp
#include "../internal/FileSystemBase.h"
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <aclapi.h>
#include <sddl.h>
#include <winioctl.h>
#include <fileapi.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cwchar>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")

namespace Exs {
namespace Internal {
namespace FileSystem {

class Exs_FileSystemWindows : public Exs_FileSystemBase {
public:
    Exs_FileSystemWindows() = default;
    virtual ~Exs_FileSystemWindows() = default;
    
    bool fileExists(const std::string& path) const override {
        std::wstring wpath = stringToWide(path);
        DWORD attrib = GetFileAttributesW(wpath.c_str());
        return (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
    }
    
    uint64 getFileSize(const std::string& path) const override {
        std::wstring wpath = stringToWide(path);
        WIN32_FILE_ATTRIBUTE_DATA fad;
        
        if (GetFileAttributesExW(wpath.c_str(), GetFileExInfoStandard, &fad)) {
            LARGE_INTEGER size;
            size.HighPart = fad.nFileSizeHigh;
            size.LowPart = fad.nFileSizeLow;
            return size.QuadPart;
        }
        return 0;
    }
    
    Exs_FileTimeInfo getFileTimes(const std::string& path) const override {
        Exs_FileTimeInfo info;
        std::wstring wpath = stringToWide(path);
        
        HANDLE hFile = CreateFileW(wpath.c_str(), GENERIC_READ, FILE_SHARE_READ, 
                                  nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        
        if (hFile != INVALID_HANDLE_VALUE) {
            FILETIME ftCreate, ftAccess, ftWrite, ftChange;
            
            if (GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite)) {
                info.creationTime = fileTimeToSystemClock(ftCreate);
                info.lastAccessTime = fileTimeToSystemClock(ftAccess);
                info.lastWriteTime = fileTimeToSystemClock(ftWrite);
                
                // Change time requires additional info
                BY_HANDLE_FILE_INFORMATION fileInfo;
                if (GetFileInformationByHandle(hFile, &fileInfo)) {
                    info.changeTime = fileTimeToSystemClock(fileInfo.ftLastWriteTime);
                }
            }
            
            CloseHandle(hFile);
        }
        
        return info;
    }
    
    uint32 getFileAttributes(const std::string& path) const override {
        std::wstring wpath = stringToWide(path);
        DWORD attrib = GetFileAttributesW(wpath.c_str());
        
        if (attrib == INVALID_FILE_ATTRIBUTES) {
            return 0;
        }
        
        uint32 exsAttrib = 0;
        
        if (attrib & FILE_ATTRIBUTE_READONLY) exsAttrib |= (uint32)Exs_FileAttribute::ReadOnly;
        if (attrib & FILE_ATTRIBUTE_HIDDEN) exsAttrib |= (uint32)Exs_FileAttribute::Hidden;
        if (attrib & FILE_ATTRIBUTE_SYSTEM) exsAttrib |= (uint32)Exs_FileAttribute::System;
        if (attrib & FILE_ATTRIBUTE_DIRECTORY) exsAttrib |= (uint32)Exs_FileAttribute::Directory;
        if (attrib & FILE_ATTRIBUTE_ARCHIVE) exsAttrib |= (uint32)Exs_FileAttribute::Archive;
        if (attrib & FILE_ATTRIBUTE_NORMAL) exsAttrib |= (uint32)Exs_FileAttribute::Normal;
        if (attrib & FILE_ATTRIBUTE_TEMPORARY) exsAttrib |= (uint32)Exs_FileAttribute::Temporary;
        if (attrib & FILE_ATTRIBUTE_SPARSE_FILE) exsAttrib |= (uint32)Exs_FileAttribute::Sparse;
        if (attrib & FILE_ATTRIBUTE_REPARSE_POINT) exsAttrib |= (uint32)Exs_FileAttribute::ReparsePoint;
        if (attrib & FILE_ATTRIBUTE_COMPRESSED) exsAttrib |= (uint32)Exs_FileAttribute::Compressed;
        if (attrib & FILE_ATTRIBUTE_OFFLINE) exsAttrib |= (uint32)Exs_FileAttribute::Offline;
        if (attrib & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) exsAttrib |= (uint32)Exs_FileAttribute::NotContentIndexed;
        if (attrib & FILE_ATTRIBUTE_ENCRYPTED) exsAttrib |= (uint32)Exs_FileAttribute::Encrypted;
        
        return exsAttrib;
    }
    
    bool directoryExists(const std::string& path) const override {
        std::wstring wpath = stringToWide(path);
        DWORD attrib = GetFileAttributesW(wpath.c_str());
        return (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY));
    }
    
    std::vector<Exs_DirectoryEntry> listDirectory(const std::string& path) const override {
        std::vector<Exs_DirectoryEntry> entries;
        std::wstring wpath = stringToWide(path);
        
        if (wpath.back() != L'\\' && wpath.back() != L'/') {
            wpath += L"\\";
        }
        
        wpath += L"*";
        
        WIN32_FIND_DATAW findData;
        HANDLE hFind = FindFirstFileW(wpath.c_str(), &findData);
        
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                // Skip . and ..
                if (wcscmp(findData.cFileName, L".") == 0 || 
                    wcscmp(findData.cFileName, L"..") == 0) {
                    continue;
                }
                
                Exs_DirectoryEntry entry;
                entry.name = wideToString(findData.cFileName);
                entry.path = path + "\\" + entry.name;
                
                // Check if it's a directory
                entry.isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                entry.isRegularFile = !entry.isDirectory;
                entry.isSymbolicLink = (findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
                
                // Get file size
                LARGE_INTEGER size;
                size.HighPart = findData.nFileSizeHigh;
                size.LowPart = findData.nFileSizeLow;
                entry.size = size.QuadPart;
                
                // Get times
                entry.times.creationTime = fileTimeToSystemClock(findData.ftCreationTime);
                entry.times.lastAccessTime = fileTimeToSystemClock(findData.ftLastAccessTime);
                entry.times.lastWriteTime = fileTimeToSystemClock(findData.ftLastWriteTime);
                
                // Convert attributes
                entry.attributes = 0;
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) 
                    entry.attributes |= (uint32)Exs_FileAttribute::ReadOnly;
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) 
                    entry.attributes |= (uint32)Exs_FileAttribute::Hidden;
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) 
                    entry.attributes |= (uint32)Exs_FileAttribute::System;
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
                    entry.attributes |= (uint32)Exs_FileAttribute::Directory;
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) 
                    entry.attributes |= (uint32)Exs_FileAttribute::Archive;
                
                entries.push_back(entry);
                
            } while (FindNextFileW(hFind, &findData));
            
            FindClose(hFind);
        }
        
        return entries;
    }
    
    std::vector<std::string> findFiles(const std::string& pattern) const override {
        std::vector<std::string> files;
        std::wstring wpattern = stringToWide(pattern);
        
        WIN32_FIND_DATAW findData;
        HANDLE hFind = FindFirstFileW(wpattern.c_str(), &findData);
        
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    files.push_back(wideToString(findData.cFileName));
                }
            } while (FindNextFileW(hFind, &findData));
            
            FindClose(hFind);
        }
        
        return files;
    }
    
    bool createDirectory(const std::string& path) const override {
        std::wstring wpath = stringToWide(path);
        return CreateDirectoryW(wpath.c_str(), nullptr) != 0;
    }
    
    bool createDirectories(const std::string& path) const override {
        std::wstring wpath = stringToWide(path);
        
        // Split path into components
        std::vector<std::wstring> components;
        std::wstringstream wss(wpath);
        std::wstring component;
        
        while (std::getline(wss, component, L'\\')) {
            if (!component.empty()) {
                components.push_back(component);
            }
        }
        
        // Build path incrementally
        std::wstring currentPath;
        if (wpath.size() > 2 && wpath[1] == L':') {
            currentPath = wpath.substr(0, 2); // Drive letter
        }
        
        for (const auto& comp : components) {
            currentPath += L"\\" + comp;
            
            if (!directoryExists(wideToString(currentPath))) {
                if (!CreateDirectoryW(currentPath.c_str(), nullptr)) {
                    if (GetLastError() != ERROR_ALREADY_EXISTS) {
                        return false;
                    }
                }
            }
        }
        
        return true;
    }
    
    bool createFile(const std::string& path) const override {
        std::wstring wpath = stringToWide(path);
        HANDLE hFile = CreateFileW(wpath.c_str(), GENERIC_WRITE, 0, nullptr, 
                                  CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
        
        if (hFile != INVALID_HANDLE_VALUE) {
            CloseHandle(hFile);
            return true;
        }
        
        return false;
    }
    
    bool deleteFile(const std::string& path) const override {
        std::wstring wpath = stringToWide(path);
        return DeleteFileW(wpath.c_str()) != 0;
    }
    
    bool deleteDirectory(const std::string& path, bool recursive) const override {
        std::wstring wpath = stringToWide(path);
        
        if (recursive) {
            // Remove directory and all contents
            SHFILEOPSTRUCTW fileOp = {0};
            wpath += L'\0'; // Double null-terminated
            
            fileOp.wFunc = FO_DELETE;
            fileOp.pFrom = wpath.c_str();
            fileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
            
            return SHFileOperationW(&fileOp) == 0;
        } else {
            // Remove empty directory
            return RemoveDirectoryW(wpath.c_str()) != 0;
        }
    }
    
    bool copyFile(const std::string& source, const std::string& destination, bool overwrite) const override {
        std::wstring wsource = stringToWide(source);
        std::wstring wdest = stringToWide(destination);
        
        return CopyFileW(wsource.c_str(), wdest.c_str(), !overwrite) != 0;
    }
    
    Exs_FileOperationResult copyFileWithProgress(const std::string& source, 
                                                 const std::string& destination, 
                                                 const Exs_ProgressCallback& callback) const override {
        Exs_FileOperationResult result;
        result.success = false;
        
        std::wstring wsource = stringToWide(source);
        std::wstring wdest = stringToWide(destination);
        
        // Get file size for progress reporting
        uint64 fileSize = getFileSize(source);
        
        // Use CopyFileEx for progress reporting
        auto startTime = std::chrono::steady_clock::now();
        
        auto progressRoutine = [](LARGE_INTEGER TotalFileSize,
                                 LARGE_INTEGER TotalBytesTransferred,
                                 LARGE_INTEGER StreamSize,
                                 LARGE_INTEGER StreamBytesTransferred,
                                 DWORD dwStreamNumber,
                                 DWORD dwCallbackReason,
                                 HANDLE hSourceFile,
                                 HANDLE hDestinationFile,
                                 LPVOID lpData) -> DWORD {
            
            auto* callbackPtr = static_cast<Exs_ProgressCallback*>(lpData);
            if (callbackPtr && *callbackPtr) {
                double progress = 0.0;
                if (TotalFileSize.QuadPart > 0) {
                    progress = static_cast<double>(TotalBytesTransferred.QuadPart) / 
                              TotalFileSize.QuadPart * 100.0;
                }
                
                if (!(*callbackPtr)(progress, TotalBytesTransferred.QuadPart, 
                                   TotalFileSize.QuadPart)) {
                    return PROGRESS_CANCEL;
                }
            }
            
            return PROGRESS_CONTINUE;
        };
        
        Exs_ProgressCallback cb = callback;
        BOOL copyResult = CopyFileExW(wsource.c_str(), wdest.c_str(), 
                                     progressRoutine, &cb, FALSE, 0);
        
        auto endTime = std::chrono::steady_clock::now();
        result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        if (copyResult) {
            result.success = true;
            result.bytesTransferred = fileSize;
        } else {
            result.errorCode = GetLastError();
            result.errorMessage = getLastErrorMessage();
        }
        
        return result;
    }
    
    bool moveFile(const std::string& source, const std::string& destination) const override {
        std::wstring wsource = stringToWide(source);
        std::wstring wdest = stringToWide(destination);
        
        return MoveFileW(wsource.c_str(), wdest.c_str()) != 0;
    }
    
    bool moveDirectory(const std::string& source, const std::string& destination) const override {
        return moveFile(source, destination);
    }
    
    bool renameFile(const std::string& oldPath, const std::string& newPath) const override {
        return moveFile(oldPath, newPath);
    }
    
    bool renameDirectory(const std::string& oldPath, const std::string& newPath) const override {
        return moveFile(oldPath, newPath);
    }
    
    Exs_FileSystemInfo getFileSystemInfo(const std::string& path) const override {
        Exs_FileSystemInfo info;
        std::wstring wpath = stringToWide(path);
        
        // Get volume information
        wchar_t volumeName[MAX_PATH + 1] = {0};
        wchar_t fileSystemName[MAX_PATH + 1] = {0};
        DWORD serialNumber = 0;
        DWORD maxComponentLen = 0;
        DWORD fileSystemFlags = 0;
        
        if (GetVolumeInformationW(wpath.c_str(), volumeName, MAX_PATH, &serialNumber,
                                 &maxComponentLen, &fileSystemFlags, fileSystemName, MAX_PATH)) {
            info.fileSystemType = wideToString(fileSystemName);
            info.maximumPathLength = maxComponentLen;
            info.supportsUnicode = (fileSystemFlags & FILE_UNICODE_ON_DISK) != 0;
            info.supportsCompression = (fileSystemFlags & FILE_FILE_COMPRESSION) != 0;
            info.supportsEncryption = (fileSystemFlags & FILE_SUPPORTS_ENCRYPTION) != 0;
        }
        
        // Get disk space
        ULARGE_INTEGER freeBytes, totalBytes, totalFreeBytes;
        if (GetDiskFreeSpaceExW(wpath.c_str(), &freeBytes, &totalBytes, &totalFreeBytes)) {
            info.totalSpace = totalBytes.QuadPart;
            info.freeSpace = freeBytes.QuadPart;
            info.availableSpace = totalFreeBytes.QuadPart;
        }
        
        // Get sector and cluster size
        DWORD sectorsPerCluster, bytesPerSector, freeClusters, totalClusters;
        if (GetDiskFreeSpaceW(wpath.c_str(), &sectorsPerCluster, &bytesPerSector,
                            &freeClusters, &totalClusters)) {
            info.sectorSize = bytesPerSector;
            info.clusterSize = sectorsPerCluster * bytesPerSector;
        }
        
        // Determine case sensitivity (Windows is case-insensitive but case-preserving)
        info.caseSensitive = false;
        
        // Check for hard links and symbolic links support
        OSVERSIONINFOEX osvi;
        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        
        GetVersionEx((OSVERSIONINFO*)&osvi);
        info.supportsHardLinks = (osvi.dwMajorVersion >= 6); // Vista and later
        info.supportsSymbolicLinks = (osvi.dwMajorVersion >= 6); // Vista and later
        
        return info;
    }
    
    std::vector<Exs_FileSystemInfo> getAllFileSystemInfo() const override {
        std::vector<Exs_FileSystemInfo> allInfo;
        
        wchar_t drives[MAX_PATH];
        DWORD driveMask = GetLogicalDrives();
        
        if (driveMask == 0) {
            return allInfo;
        }
        
        wchar_t drive[] = L"A:\\";
        
        for (int i = 0; i < 26; i++) {
            if (driveMask & (1 << i)) {
                drive[0] = L'A' + i;
                
                UINT driveType = GetDriveTypeW(drive);
                if (driveType == DRIVE_FIXED || driveType == DRIVE_REMOVABLE || 
                    driveType == DRIVE_REMOTE) {
                    
                    std::string drivePath = wideToString(drive);
                    allInfo.push_back(getFileSystemInfo(drivePath));
                }
            }
        }
        
        return allInfo;
    }
    
    std::string getAbsolutePath(const std::string& path) const override {
        std::wstring wpath = stringToWide(path);
        wchar_t absolutePath[MAX_PATH];
        
        if (GetFullPathNameW(wpath.c_str(), MAX_PATH, absolutePath, nullptr)) {
            return wideToString(absolutePath);
        }
        
        return path;
    }
    
    std::string getCanonicalPath(const std::string& path) const override {
        // On Windows, canonical path is similar to absolute path
        return getAbsolutePath(path);
    }
    
    std::string getRelativePath(const std::string& path, const std::string& base) const override {
        std::wstring wpath = stringToWide(path);
        std::wstring wbase = stringToWide(base);
        wchar_t relativePath[MAX_PATH];
        
        if (PathRelativePathToW(relativePath, wbase.c_str(), FILE_ATTRIBUTE_DIRECTORY,
                              wpath.c_str(), fileExists(path) ? FILE_ATTRIBUTE_NORMAL : FILE_ATTRIBUTE_DIRECTORY)) {
            return wideToString(relativePath);
        }
        
        return path;
    }
    
    bool createSymbolicLink(const std::string& target, const std::string& link) const override {
        std::wstring wtarget = stringToWide(target);
        std::wstring wlink = stringToWide(link);
        
        DWORD flags = 0;
        if (directoryExists(target)) {
            flags = SYMBOLIC_LINK_FLAG_DIRECTORY;
        }
        
        // Requires administrator privileges on older Windows versions
        return CreateSymbolicLinkW(wlink.c_str(), wtarget.c_str(), flags) != 0;
    }
    
    bool createHardLink(const std::string& target, const std::string& link) const override {
        std::wstring wtarget = stringToWide(target);
        std::wstring wlink = stringToWide(link);
        
        return CreateHardLinkW(wlink.c_str(), wtarget.c_str(), nullptr) != 0;
    }
    
    std::string readSymbolicLink(const std::string& link) const override {
        std::wstring wlink = stringToWide(link);
        HANDLE hFile = CreateFileW(wlink.c_str(), GENERIC_READ, FILE_SHARE_READ,
                                  nullptr, OPEN_EXISTING, 
                                  FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, 
                                  nullptr);
        
        if (hFile == INVALID_HANDLE_VALUE) {
            return "";
        }
        
        BYTE buffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
        REPARSE_DATA_BUFFER* reparseData = (REPARSE_DATA_BUFFER*)buffer;
        DWORD bytesReturned;
        
        if (DeviceIoControl(hFile, FSCTL_GET_REPARSE_POINT, nullptr, 0,
                           reparseData, MAXIMUM_REPARSE_DATA_BUFFER_SIZE,
                           &bytesReturned, nullptr)) {
            
            if (reparseData->ReparseTag == IO_REPARSE_TAG_SYMLINK) {
                wchar_t* target = reparseData->SymbolicLinkReparseBuffer.PathBuffer + 
                                 reparseData->SymbolicLinkReparseBuffer.PrintNameOffset / sizeof(wchar_t);
                CloseHandle(hFile);
                return wideToString(target);
            }
        }
        
        CloseHandle(hFile);
        return "";
    }
    
    bool setFilePermissions(const std::string& path, uint32 permissions) const override {
        // Windows uses ACLs, not Unix-style permissions
        // This is a simplified implementation
        std::wstring wpath = stringToWide(path);
        
        PSECURITY_DESCRIPTOR pSD = nullptr;
        PACL pACL = nullptr;
        EXPLICIT_ACCESS_W ea[3];
        
        // Build ACL
        ZeroMemory(ea, sizeof(ea));
        
        // Owner permissions
        ea[0].grfAccessPermissions = GENERIC_ALL;
        ea[0].grfAccessMode = SET_ACCESS;
        ea[0].grfInheritance = NO_INHERITANCE;
        ea[0].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
        ea[0].Trustee.ptstrName = L"CURRENT_USER";
        
        // Group permissions
        ea[1].grfAccessPermissions = GENERIC_READ | GENERIC_EXECUTE;
        ea[1].grfAccessMode = SET_ACCESS;
        ea[1].grfInheritance = NO_INHERITANCE;
        ea[1].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
        ea[1].Trustee.ptstrName = L"BUILTIN\\Users";
        
        // Everyone permissions
        ea[2].grfAccessPermissions = GENERIC_READ;
        ea[2].grfAccessMode = SET_ACCESS;
        ea[2].grfInheritance = NO_INHERITANCE;
        ea[2].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
        ea[2].Trustee.ptstrName = L"EVERYONE";
        
        DWORD dwResult = SetEntriesInAclW(3, ea, nullptr, &pACL);
        if (dwResult != ERROR_SUCCESS) {
            return false;
        }
        
        // Set security descriptor
        pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
        if (pSD == nullptr) {
            LocalFree(pACL);
            return false;
        }
        
        if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) {
            LocalFree(pSD);
            LocalFree(pACL);
            return false;
        }
        
        if (!SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE)) {
            LocalFree(pSD);
            LocalFree(pACL);
            return false;
        }
        
        // Apply security descriptor to file
        bool result = SetFileSecurityW(wpath.c_str(), DACL_SECURITY_INFORMATION, pSD) != 0;
        
        LocalFree(pSD);
        LocalFree(pACL);
        
        return result;
    }
    
    uint32 getFilePermissions(const std::string& path) const override {
        // Simplified implementation - returns basic Unix-style permissions
        std::wstring wpath = stringToWide(path);
        
        DWORD attrib = GetFileAttributesW(wpath.c_str());
        uint32 permissions = 0;
        
        if (attrib != INVALID_FILE_ATTRIBUTES) {
            // Basic mapping
            permissions |= 0400; // User read
            if (!(attrib & FILE_ATTRIBUTE_READONLY)) {
                permissions |= 0200; // User write
            }
            permissions |= 0100; // User execute (if file)
            
            if (attrib & FILE_ATTRIBUTE_DIRECTORY) {
                permissions |= 0111; // All execute for directories
            }
            
            // Group and others get read-only
            permissions |= 0444; // Group and others read
        }
        
        return permissions;
    }
    
    bool setFileOwner(const std::string& path, const std::string& owner) const override {
        // Windows file ownership requires administrator privileges
        // This is a simplified implementation
        return false;
    }
    
    std::string getFileOwner(const std::string& path) const override {
        std::wstring wpath = stringToWide(path);
        
        PSECURITY_DESCRIPTOR pSD = nullptr;
        DWORD dwResult = GetNamedSecurityInfoW(wpath.c_str(), SE_FILE_OBJECT,
                                              OWNER_SECURITY_INFORMATION,
                                              nullptr, nullptr, nullptr, nullptr, &pSD);
        
        if (dwResult != ERROR_SUCCESS) {
            return "";
        }
        
        PSID pOwnerSid = nullptr;
        BOOL bOwnerDefaulted;
        if (!GetSecurityDescriptorOwner(pSD, &pOwnerSid, &bOwnerDefaulted)) {
            LocalFree(pSD);
            return "";
        }
        
        // Convert SID to name
        wchar_t name[256];
        wchar_t domain[256];
        DWORD nameSize = 256;
        DWORD domainSize = 256;
        SID_NAME_USE sidType;
        
        if (LookupAccountSidW(nullptr, pOwnerSid, name, &nameSize,
                             domain, &domainSize, &sidType)) {
            LocalFree(pSD);
            return wideToString(std::wstring(domain) + L"\\" + std::wstring(name));
        }
        
        LocalFree(pSD);
        return "";
    }
    
    std::string readFileText(const std::string& path) const override {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            return "";
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        
        return content;
    }
    
    std::vector<uint8> readFileBinary(const std::string& path) const override {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return {};
        }
        
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        std::vector<uint8> buffer(size);
        if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            return buffer;
        }
        
        return {};
    }
    
    bool writeFileText(const std::string& path, const std::string& content) const override {
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        file.write(content.c_str(), content.size());
        return file.good();
    }
    
    bool writeFileBinary(const std::string& path, const std::vector<uint8>& data) const override {
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        return file.good();
    }
    
    bool lockFile(const std::string& path) const override {
        std::wstring wpath = stringToWide(path);
        HANDLE hFile = CreateFileW(wpath.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
                                  nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        
        if (hFile == INVALID_HANDLE_VALUE) {
            return false;
        }
        
        OVERLAPPED overlapped = {0};
        if (LockFileEx(hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, MAXDWORD, MAXDWORD, &overlapped)) {
            CloseHandle(hFile);
            return true;
        }
        
        CloseHandle(hFile);
        return false;
    }
    
    bool unlockFile(const std::string& path) const override {
        std::wstring wpath = stringToWide(path);
        HANDLE hFile = CreateFileW(wpath.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
                                  nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        
        if (hFile == INVALID_HANDLE_VALUE) {
            return false;
        }
        
        OVERLAPPED overlapped = {0};
        if (UnlockFileEx(hFile, 0, MAXDWORD, MAXDWORD, &overlapped)) {
            CloseHandle(hFile);
            return true;
        }
        
        CloseHandle(hFile);
        return false;
    }
    
    void startFileMonitoring(const std::string& path) const override {
        // Implementation would use ReadDirectoryChangesW
    }
    
    void stopFileMonitoring(const std::string& path) const override {
        // Implementation would stop monitoring
    }
    
    std::string createTempFile(const std::string& prefix) const override {
        wchar_t tempPath[MAX_PATH];
        wchar_t tempFile[MAX_PATH];
        
        GetTempPathW(MAX_PATH, tempPath);
        
        std::wstring wprefix = stringToWide(prefix);
        GetTempFileNameW(tempPath, wprefix.c_str(), 0, tempFile);
        
        return wideToString(tempFile);
    }
    
    std::string createTempDirectory(const std::string& prefix) const override {
        wchar_t tempPath[MAX_PATH];
        GetTempPathW(MAX_PATH, tempPath);
        
        wchar_t tempDir[MAX_PATH];
        std::wstring wprefix = stringToWide(prefix);
        
        // Create unique directory name
        GetTempFileNameW(tempPath, wprefix.c_str(), 0, tempDir);
        DeleteFileW(tempDir); // Remove the file created by GetTempFileName
        CreateDirectoryW(tempDir, nullptr);
        
        return wideToString(tempDir);
    }
    
    uint64 getFreeDiskSpace(const std::string& path) const override {
        std::wstring wpath = stringToWide(path);
        ULARGE_INTEGER freeBytes, totalBytes, totalFreeBytes;
        
        if (GetDiskFreeSpaceExW(wpath.c_str(), &freeBytes, &totalBytes, &totalFreeBytes)) {
            return freeBytes.QuadPart;
        }
        
        return 0;
    }
    
    uint64 getTotalDiskSpace(const std::string& path) const override {
        std::wstring wpath = stringToWide(path);
        ULARGE_INTEGER freeBytes, totalBytes, totalFreeBytes;
        
        if (GetDiskFreeSpaceExW(wpath.c_str(), &freeBytes, &totalBytes, &totalFreeBytes)) {
            return totalBytes.QuadPart;
        }
        
        return 0;
    }
    
    std::string calculateFileHash(const std::string& path, const std::string& algorithm) const override {
        // Simplified implementation - would use CryptoAPI or other library
        return "";
    }
    
    bool compareFiles(const std::string& path1, const std::string& path2) const override {
        auto data1 = readFileBinary(path1);
        auto data2 = readFileBinary(path2);
        
        if (data1.size() != data2.size()) {
            return false;
        }
        
        return memcmp(data1.data(), data2.data(), data1.size()) == 0;
    }
    
    bool compressFile(const std::string& source, const std::string& destination) const override {
        // Would use Windows Compression API
        return false;
    }
    
    bool decompressFile(const std::string& source, const std::string& destination) const override {
        // Would use Windows Compression API
        return false;
    }
    
private:
    std::wstring stringToWide(const std::string& str) const {
        if (str.empty()) return L"";
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
        std::wstring wstr(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
        return wstr;
    }
    
    std::string wideToString(const std::wstring& wstr) const {
        if (wstr.empty()) return "";
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), nullptr, 0, nullptr, nullptr);
        std::string str(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size_needed, nullptr, nullptr);
        return str;
    }
    
    std::chrono::system_clock::time_point fileTimeToSystemClock(const FILETIME& ft) const {
        ULARGE_INTEGER ull;
        ull.LowPart = ft.dwLowDateTime;
        ull.HighPart = ft.dwHighDateTime;
        
        // FileTime is in 100-nanosecond intervals since January 1, 1601
        // Convert to system_clock (since January 1, 1970)
        const uint64 EPOCH_DIFFERENCE = 116444736000000000ULL; // 100-ns intervals between 1601 and 1970
        
        if (ull.QuadPart > EPOCH_DIFFERENCE) {
            ull.QuadPart -= EPOCH_DIFFERENCE;
            return std::chrono::system_clock::from_time_t(ull.QuadPart / 10000000ULL);
        }
        
        return std::chrono::system_clock::time_point();
    }
    
    std::string getLastErrorMessage() const {
        DWORD error = GetLastError();
        if (error == 0) {
            return "No error";
        }
        
        LPSTR messageBuffer = nullptr;
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
                                    FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, error,
                                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
                                    (LPSTR)&messageBuffer, 0, nullptr);
        
        std::string message(messageBuffer, size);
        LocalFree(messageBuffer);
        
        return message;
    }
};

// Factory function implementation
Exs_FileSystemBase* Exs_CreateFileSystemInstance() {
    return new Exs_FileSystemWindows();
}

} // namespace FileSystem
} // namespace Internal
} // namespace Exs
