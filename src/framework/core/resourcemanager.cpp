#include "resourcemanager.h"

#include <framework/platform/platform.h>
#include <framework/luascript/luainterface.h>

#include <physfs.h>

ResourceManager g_resources;

void ResourceManager::init(const char* argv0)
{
    PHYSFS_init(argv0);

    // try to find modules directory, all data lives there
    std::string baseDir = PHYSFS_getBaseDir();

    std::string possibleDirs[] = { "modules",
                                   baseDir + "modules",
                                   baseDir + "../modules",
                                   baseDir + "../share/otclient/modules",
                                   "" };

    bool found = false;
    for(const std::string& dir : possibleDirs) {
        if(g_resources.addToSearchPath(dir)) {
            logInfo("Using modules directory '", dir.c_str(), "'");
            found = true;
            break;
        }
    }

    if(!found)
        throw std::runtime_error("could not find modules directory");

    // setup write directory
    std::string dir = g_platform.getAppUserDir();
    if(g_resources.setWriteDir(dir))
        g_resources.addToSearchPath(dir);
    else
        throw std::runtime_error("could not setup write directory");
}

void ResourceManager::terminate()
{
    PHYSFS_deinit();
}

bool ResourceManager::setWriteDir(const std::string& path)
{
    if(!PHYSFS_setWriteDir(path.c_str()))
        return false;
    return true;
}

bool ResourceManager::addToSearchPath(const std::string& path, bool insertInFront)
{
    if(!PHYSFS_addToSearchPath(path.c_str(), insertInFront ? 0 : 1))
        return false;
    return true;
}

void ResourceManager::searchAndAddPackages(const std::string& packagesDir, const std::string& packageExt, bool append)
{
    auto files = listDirectoryFiles(resolvePath(packagesDir));
    for(const std::string& file : files) {
        if(boost::ends_with(file, packageExt))
            addToSearchPath(packagesDir + "/" + file, !append);
    }
}

bool ResourceManager::fileExists(const std::string& fileName)
{
    return (PHYSFS_exists(resolvePath(fileName).c_str()) && !PHYSFS_isDirectory(resolvePath(fileName).c_str()));
}

bool ResourceManager::directoryExists(const std::string& directoryName)
{
    return (PHYSFS_exists(resolvePath(directoryName).c_str()) && PHYSFS_isDirectory(resolvePath(directoryName).c_str()));
}

void ResourceManager::loadFile(const std::string& fileName, std::iostream& out)
{
    std::string fullPath = resolvePath(fileName);
    out.clear(std::ios::goodbit);
    PHYSFS_file* file = PHYSFS_openRead(fullPath.c_str());
    if(!file) {
        out.clear(std::ios::failbit);
        throw std::runtime_error(fw::mkstr("failed to load file '", fullPath.c_str(), "': ", PHYSFS_getLastError()));
    } else {
        int fileSize = PHYSFS_fileLength(file);
        if(fileSize > 0) {
            std::vector<char> buffer(fileSize);
            PHYSFS_read(file, (void*)&buffer[0], 1, fileSize);
            out.write(&buffer[0], fileSize);
        } else
            out.clear(std::ios::eofbit);
        PHYSFS_close(file);
        out.seekg(0, std::ios::beg);
    }
}

std::string ResourceManager::loadFile(const std::string& fileName)
{
    std::stringstream fin;
    loadFile(fileName, fin);
    return fin.str();
}

bool ResourceManager::saveFile(const std::string& fileName, const uchar* data, uint size)
{
    PHYSFS_file* file = PHYSFS_openWrite(resolvePath(fileName).c_str());
    if(!file)
        return false;

    PHYSFS_write(file, (void*)data, size, 1);
    PHYSFS_close(file);
    return true;
}

bool ResourceManager::saveFile(const std::string& fileName, std::istream& in)
{
    std::streampos oldPos = in.tellg();
    in.seekg(0, std::ios::end);
    std::streampos size = in.tellg();
    in.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);
    in.read(&buffer[0], size);
    bool ret = saveFile(fileName, (const uchar*)&buffer[0], size);
    in.seekg(oldPos, std::ios::beg);
    return ret;
}

bool ResourceManager::saveFile(const std::string& fileName, const std::string& data)
{
    return saveFile(fileName, (const uchar*)data.c_str(), data.size());
}

bool ResourceManager::deleteFile(const std::string& fileName)
{
    return PHYSFS_delete(resolvePath(fileName).c_str()) != 0;
}

std::list<std::string> ResourceManager::listDirectoryFiles(const std::string& directoryPath)
{
    std::list<std::string> files;
    auto rc = PHYSFS_enumerateFiles(resolvePath(directoryPath).c_str());

    for(int i = 0; rc[i] != NULL; i++)
        files.push_back(rc[i]);

    PHYSFS_freeList(rc);
    return files;
}

std::string ResourceManager::resolvePath(const std::string& path)
{
    std::string fullPath;
    if(boost::starts_with(path, "/"))
        fullPath = path.substr(1);
    else {
        std::string scriptPath = g_lua.currentSourcePath();
        if(!scriptPath.empty()) {
            fullPath += scriptPath + "/";
        }
        fullPath += path;
    }
    return fullPath;
}