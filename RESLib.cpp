#include "RESLib.h"

/* Static pointer to assets folder */
static char* _GM_AssetsRoot = NULL;
static cache* _GM_Cache = NULL;

void RES_Init(const std::string& assets_root)
{
  std::string cwd = GM_GetCurrentPath();
  _GM_AssetsRoot = _strdup(GM_JoinPaths(cwd, assets_root).c_str());

  _GM_Cache = new cache();
  SDL_Log("init(): ready root=%s", _GM_AssetsRoot);
}

cache_item* RES_GetItem(const std::string& item_path)
{
    if (_GM_Cache == NULL) {
        SDLEx_LogError("get_item() - not initialized to read path=%s", item_path.c_str());
        throw std::exception("Resources are not initialized");
    }
    cache::iterator it = _GM_Cache->find(item_path);
    if (it == _GM_Cache->end()) {
        return nullptr;
    }
    SDL_assert(it->second->path() == item_path);
    return it->second;
}

void RES_AddItem(cache_item* itm)
{
    if (itm == NULL) {
        SDLEx_LogError("add_item() - invalid item given");
        throw std::exception("Invalid cache item given");
    }
    if (_GM_Cache == NULL) {
        SDLEx_LogError("add_item() - not initialized to add item with path=%s", itm->path().c_str());
        throw std::exception("Resources are not initialized");
    }
    const std::string& item_path = itm->path();
    cache::iterator it = _GM_Cache->find(item_path);
    if (it != _GM_Cache->end()) {
        SDLEx_LogError("add_item() - item already in cache. path=%s, ptr=%p", 
            it->first.c_str(), it->second);
        throw std::exception("Item already in cache");
    }
    _GM_Cache->insert(std::make_pair(item_path, itm));
}

std::string RES_GetFullPath(const std::string& relPath)
{
  return GM_JoinPaths(_GM_AssetsRoot, relPath);
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

void surface::load_file(const std::string& path)
{
    if (_GM_AssetsRoot == NULL)
        throw std::exception("Resources are not initialized");
    
    std::string full_path = RES_GetFullPath(path);
    _ptr = GM_LoadSurface(full_path.c_str());
    if (!_ptr) throw std::exception("Failed to load surface file");
}

void texture::load_file(const std::string& path)
{
    if (_GM_AssetsRoot == NULL)
        throw std::exception("Resources are not initialized");

    std::string full_path = RES_GetFullPath(path);
    _ptr = GM_LoadTexture(full_path.c_str());
    if (!_ptr) throw std::exception("Failed to load texture file");
}

void font::load_file(const std::string& path)
{
    if (_GM_AssetsRoot == NULL)
        throw std::exception("Resources are not initialized");

    std::string full_path = RES_GetFullPath(path);
    _ptr = new font_collection(full_path);
    if (!_ptr) throw std::exception("Failed to load font file");
}

void data::load_file(const std::string& path)
{
    if (_GM_AssetsRoot == NULL)
        throw std::exception("Resources are not initialized");

    std::string full_path = RES_GetFullPath(path);
    _ptr = GM_LoadJSON(full_path.c_str());
    if (!_ptr) throw std::exception("Failed to load data file");
}

font_collection::font_collection(const std::string& font_file)
{
    _font_file = font_file;
    std::ifstream fs(_font_file);
    if (!fs.good()) {
        SDLEx_LogError("font_collection: Failed to open %s", _font_file.c_str());
        throw std::exception("Failed to open font file");
    }
}

font_collection::~font_collection()
{
    fonts_map::iterator it = _sizes.begin();
    for(; it != _sizes.end(); ++it) {
        TTF_CloseFont(it->second);
    }
    _sizes.clear();
}

TTF_Font* font_collection::get_ptsize(int ptsize)
{
    if (_sizes.find(ptsize) == _sizes.end()) {
        _sizes[ptsize] = GM_LoadFont(_font_file.c_str(), ptsize);
    }
    return _sizes.at(ptsize);
}
