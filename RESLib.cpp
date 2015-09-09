#include "RESLib.h"
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

/* Static pointer to assets folder */
static path * _GM_AssetsRoot;
typedef std::map<std::string, void*> cache;
static cache* _GM_Cache = NULL;

void RES_Init(const std::string& assets_root)
{
  path cwd(GM_GetCurrentPath());
  cwd /= assets_root;
  _GM_AssetsRoot = new path(cwd);
  _GM_Cache = new cache();
  SDL_Log("RES_Init(): assets ready at [%s]", _GM_AssetsRoot->string().c_str());
}

std::string RES_GetAssetsRoot()
{
  return _GM_AssetsRoot->string();
}

void* RES_Get(const std::string& id)
{
    if (_GM_Cache == NULL) {
        SDLEx_LogError("RES_Get() - not initialized");
        throw std::exception("Resources are not initialized");
    }
    cache::iterator it = _GM_Cache->find(id);
    if (it == _GM_Cache->end()) {
        return nullptr;
    }
    return it->second;
}

void RES_Put(const std::string& id, void* res)
{
    if (res == NULL) {
        SDLEx_LogError("RES_Put() - invalid item given");
        throw std::exception("Invalid cache item given");
    }
    if (_GM_Cache == NULL) {
        SDLEx_LogError("RES_Put() - not initialized");
        throw std::exception("Resources are not initialized");
    }
    cache::iterator it = _GM_Cache->find(id);
    if (it != _GM_Cache->end()) {
        SDLEx_LogError("RES_Put() - item already in cache. id=%s, ptr=%p", 
            it->first.c_str(), it->second);
        throw std::exception("Item already in cache");
    }
    _GM_Cache->insert(std::make_pair(id, res));
}

std::string RES_GetFullPath(const std::string& relPath)
{
  path rel(relPath);
  //check if path is abs
  if (rel.is_absolute()) return relPath;
  //combite relative path with assets root
  path root(_GM_AssetsRoot ? _GM_AssetsRoot->string() : ".");
  root /= rel;
  return root.string();
}

bool RES_CheckExists(const std::string& relPath)
{
  path itm(_GM_AssetsRoot ? _GM_AssetsRoot->string() : ".");
  path rel(relPath);
  if (rel.is_absolute()) return exists(rel);
  itm /= rel;
  return exists(itm);
}