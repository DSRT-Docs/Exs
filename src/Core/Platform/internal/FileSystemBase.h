// src/Core/Platform/internal/FileSystemBase.h
#ifndef EXS_INTERNAL_FILESYSTEM_BASE_H
#define EXS_INTERNAL_FILESYSTEM_BASE_H

#include "../../../include/Exs/Core/Types/BasicTypes.h"
#include <string>
#include <vector>
#include <chrono>
#include <functional>

namespace Exs {
namespace Internal {
namespace FileSystem {

// File attributes enumeration
enum class Exs_FileAttribute {
    ReadOnly = 0x01,
    Hidden = 0x02,
    System = 0x04,
    Directory = 0x10,
    Archive = 0x20,
    Device = 0x40,
    Normal = 0x80,
    Temporary = 0x100,
    Sparse = 0x200,
    ReparsePoint = 0x400,
    Compressed = 0x800,
    Offline = 0x1000,
    NotContentIndexed = 0x2000,
    Encrypted = 0x4000
};

// File time information
struct Exs_FileTimeInfo {
    std::chrono::system_clock::time_point creationTime;
    std::chrono::system_clock::time_point lastAccessTime;
    std::chrono::system_clock::time_point lastWriteTime;
    std::chrono::system_clock::time_point changeTime;
};

// File system information
struct Exs_FileSystemInfo {
    std::string fileSystemType;
    uint64 totalSpace;
    uint64 freeSpace;
    uint64 availableSpace;
    uint32 sectorSize;
    uint32 clusterSize;
    uint32 maximumPathLength;
    bool caseSensitive;
    bool supportsUnicode;
    bool supportsHardLinks;
    bool supportsSymbolicLinks;
    bool supportsCompression;
    bool supportsEncryption;
};

// Directory entry information
struct Exs_DirectoryEntry {
    std::string name;
    std::string path;
    uint64 size;
    bool isDirectory;
    bool isRegularFile;
    bool isSymbolicLink;
    Exs_FileTimeInfo times;
    uint32 attributes;
    uint32 permissions; // Unix-style permissions
};

// File operation result
struct Exs_FileOperationResult {
    bool success;
    int32 errorCode;
    std::string errorMessage;
    uint64 bytesTransferred;
    std::chrono::milliseconds duration;
};

// Progress callback type
using Exs_ProgressCallback = std::function<bool(double progress, uint64 bytesTransferred, uint64 totalBytes)>;

// Base file system class
class Exs_FileSystemBase {
public:
    virtual ~Exs_FileSystemBase() = default;
    
    // File operations
    virtual bool fileExists(const std::string& path) const = 0;
    virtual uint64 getFileSize(const std::string& path) const = 0;
    virtual Exs_FileTimeInfo getFileTimes(const std::string& path) const = 0;
    virtual uint32 getFileAttributes(const std::string& path) const = 0;
    
    // Directory operations
    virtual bool directoryExists(const std::string& path) const = 0;
    virtual std::vector<Exs_DirectoryEntry> listDirectory(const std::string& path) const = 0;
    virtual std::vector<std::string> findFiles(const std::string& pattern) const = 0;
    
    // Create operations
    virtual bool createDirectory(const std::string& path) const = 0;
    virtual bool createDirectories(const std::string& path) const = 0;
    virtual bool createFile(const std::string& path) const = 0;
    
    // Delete operations
    virtual bool deleteFile(const std::string& path) const = 0;
    virtual bool deleteDirectory(const std::string& path, bool recursive = false) const = 0;
    
    // Copy operations
    virtual bool copyFile(const std::string& source, const std::string& destination, bool overwrite = false) const = 0;
    virtual Exs_FileOperationResult copyFileWithProgress(
        const std::string& source, 
        const std::string& destination, 
        const Exs_ProgressCallback& callback = nullptr) const = 0;
    
    // Move operations
    virtual bool moveFile(const std::string& source, const std::string& destination) const = 0;
    virtual bool moveDirectory(const std::string& source, const std::string& destination) const = 0;
    
    // Rename operations
    virtual bool renameFile(const std::string& oldPath, const std::string& newPath) const = 0;
    virtual bool renameDirectory(const std::string& oldPath, const std::string& newPath) const = 0;
    
    // File system information
    virtual Exs_FileSystemInfo getFileSystemInfo(const std::string& path) const = 0;
    virtual std::vector<Exs_FileSystemInfo> getAllFileSystemInfo() const = 0;
    
    // Path operations
    virtual std::string getAbsolutePath(const std::string& path) const = 0;
    virtual std::string getCanonicalPath(const std::string& path) const = 0;
    virtual std::string getRelativePath(const std::string& path, const std::string& base) const = 0;
    
    // Symbolic links
    virtual bool createSymbolicLink(const std::string& target, const std::string& link) const = 0;
    virtual bool createHardLink(const std::string& target, const std::string& link) const = 0;
    virtual std::string readSymbolicLink(const std::string& link) const = 0;
    
    // File permissions (Unix-style)
    virtual bool setFilePermissions(const std::string& path, uint32 permissions) const = 0;
    virtual uint32 getFilePermissions(const std::string& path) const = 0;
    
    // File ownership
    virtual bool setFileOwner(const std::string& path, const std::string& owner) const = 0;
    virtual std::string getFileOwner(const std::string& path) const = 0;
    
    // File content operations
    virtual std::string readFileText(const std::string& path) const = 0;
    virtual std::vector<uint8> readFileBinary(const std::string& path) const = 0;
    virtual bool writeFileText(const std::string& path, const std::string& content) const = 0;
    virtual bool writeFileBinary(const std::string& path, const std::vector<uint8>& data) const = 0;
    
    // File locking
    virtual bool lockFile(const std::string& path) const = 0;
    virtual bool unlockFile(const std::string& path) const = 0;
    
    // File monitoring
    virtual void startFileMonitoring(const std::string& path) const = 0;
    virtual void stopFileMonitoring(const std::string& path) const = 0;
    
    // Temporary files
    virtual std::string createTempFile(const std::string& prefix = "exs_") const = 0;
    virtual std::string createTempDirectory(const std::string& prefix = "exs_") const = 0;
    
    // Disk space
    virtual uint64 getFreeDiskSpace(const std::string& path) const = 0;
    virtual uint64 getTotalDiskSpace(const std::string& path) const = 0;
    
    // File hashing
    virtual std::string calculateFileHash(const std::string& path, const std::string& algorithm = "SHA256") const = 0;
    
    // File comparison
    virtual bool compareFiles(const std::string& path1, const std::string& path2) const = 0;
    
    // File compression
    virtual bool compressFile(const std::string& source, const std::string& destination) const = 0;
    virtual bool decompressFile(const std::string& source, const std::string& destination) const = 0;
};

// Factory function
Exs_FileSystemBase* Exs_CreateFileSystemInstance();

} // namespace FileSystem
} // namespace Internal
} // namespace Exs

#endif // EXS_INTERNAL_FILESYSTEM_BASE_H
