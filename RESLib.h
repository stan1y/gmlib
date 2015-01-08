#ifndef _RES_LIB_H
#define _RES_LIB_H

#include "GMLib.h"

/* generic resource interface */
class cache_item {
public:
    virtual ~cache_item() {};
    const std::string& path() { return _path; }
    void set_path(const std::string& p) { _path = p; }

protected:
    std::string _path;
};

template <class T>
class resource: public cache_item {
public:
    resource() { _ptr = NULL; }
    virtual void load_file(const std::string& path) = 0;

    void set(T* p)  { _ptr = p; }
    T* get() const { return _ptr; }

protected:
    T* _ptr;
};

/* TTF_Fonts collection per pt size */
class font_collection {
public:
    typedef std::map<int, TTF_Font*> fonts_map;
    
    font_collection(const std::string& font_file);
    ~font_collection();
    TTF_Font* get_ptsize(int ptsize);

private:
    fonts_map _sizes;
    std::string _font_file;
};

/* type-specific resource loaders */
class surface: public resource<SDL_Surface> {
public:
    ~surface() { if(_ptr) SDL_FreeSurface(_ptr); }
    void load_file(const std::string& path);
};

class texture: public resource<SDL_Texture> {
public:
    ~texture() { if(_ptr) SDL_DestroyTexture(_ptr); }
    void load_file(const std::string& path);
};

class font: public resource<font_collection> {
public:
    ~font() { if(_ptr) delete _ptr; }
    void load_file(const std::string& path);
};

class data: public resource<json_t> {
public:
  ~data() { if(_ptr) json_decref(_ptr); }
  void load_file(const std::string& path);

};

/* std::map-based cache type */
typedef std::map<std::string, cache_item*> cache;

/* Get item from cache by path. Returns nullptr if not found */
cache_item* RES_GetItem(const std::string&);

/* Append item to cache. Raises exception if item already there */
void RES_AddItem(cache_item*);

/* Initialize resources root */
void RES_Init(const std::string& assets_root);

/* Return full path to a resource file */
std::string RES_GetFullPath(const std::string& relPath);

/* Generic load resource from path */
template<class ResType>
const ResType* RES_Load(const std::string& res_file, bool persist = true)
{
    cache_item* item = RES_GetItem(res_file);
    if (item != NULL) {
        ResType* res = dynamic_cast<ResType*> (item);
        if (!res) {
            SDLEx_LogError("RES_LoadItem(%s) failed to cast cache_item to resource type",
                res_file.c_str());
            throw std::exception("Failed to cast cache_item");
        }
        return res;
    }
    ResType* new_res = new ResType();
    new_res->load_file(res_file);
    new_res->set_path(res_file);
    if (persist) RES_AddItem(new_res);
    return new_res;
}

#endif //_RES_LIB_H