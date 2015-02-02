#include "RESLib.h"

/* Static pointer to assets folder */
static char* _GM_AssetsRoot = "";
typedef std::map<std::string, void*> cache;
static cache* _GM_Cache = NULL;

void RES_Init(const std::string& assets_root)
{
  std::string cwd = GM_GetCurrentPath();
  _GM_AssetsRoot = _strdup(GM_JoinPaths(cwd, assets_root).c_str());

  _GM_Cache = new cache();
  SDL_Log("init(): ready root=%s", _GM_AssetsRoot);
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
  if (strlen(_GM_AssetsRoot) > 0) 
    return GM_JoinPaths(_GM_AssetsRoot, relPath);
  return relPath;
}

bool RES_CheckExists(const std::string& relPath)
{
  std::string path = GM_JoinPaths(_GM_AssetsRoot, relPath);
  FILE * f = std::fopen(path.c_str(), "r");
  if (f != NULL) {
    std::fclose(f);
    return true;
  }
  return false;
}