#include "RESLib.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>


namespace resources
{

/* item in cache */
struct resource_descriptor 
{
  iresource const * resource;
  std::string resource_name;
  std::string resource_postfix;
  boost::filesystem::path resource_path;

  void init(const std::string & resource_id,
            const std::string & p = std::string(),
            iresource const * r = nullptr)
  {
    resource = r;
    resource_postfix = std::string("");
    resource_name = std::string(resource_id);
    resource_path = boost::filesystem::path(p);

    if (resource_name.find(':') != std::string::npos) {
      std::vector<std::string> parts;
      split_string(resource_name, ':', parts);
      resource_name = parts[0];
      resource_postfix = parts[1];
    }
  }

  resource_descriptor(const std::string & resource_id,
                      const std::string & p = std::string(),
                      iresource const * r = nullptr)
  {
    init(resource_id, p, r);
  }

};

typedef std::map<std::string, resource_descriptor> cache;

/* Static pointer to assets folder */
static boost::filesystem::path * g_assets_root;
static cache* g_cache = nullptr;

void add(std::string resource_id, std::string found_at, iresource const * r)
{
  g_cache->insert(std::make_pair(resource_id, resource_descriptor(resource_id, found_at, r)));
#ifdef GM_DEBUG
  SDL_Log("%s - loaded new item %s",
    __METHOD_NAME__,
    resource_id.c_str());
#endif
}

void initialize(const std::string& assets_root)
{
  auto assets = boost::filesystem::path(assets_root);
  if ( !assets.is_absolute() ) {
    g_assets_root = new boost::filesystem::path(
      boost::filesystem::current_path() / assets);
  }
  else {
    g_assets_root = new boost::filesystem::path(assets);
  }

  if (!boost::filesystem::is_directory(*g_assets_root)) {
    SDLEx_LogError("%s - assets root is not a directory '%s'",
      __METHOD_NAME__,
      g_assets_root->string().c_str());
    throw std::exception("Assets root is not a directory");
  }

  g_cache = new cache();
#ifdef GM_DEBUG
  SDL_Log("%s - assets ready at '%s'",
    __METHOD_NAME__,
    g_assets_root->string().c_str());
#endif
}

std::string root_path()
{
  if (g_assets_root != nullptr)
    return g_assets_root->string();

  SDLEx_LogError("%s - no assets root initialized",
    __METHOD_NAME__);
  throw std::exception("No assets root defined");
}

/* Get item from cache by path. Returns nullptr if not found */
iresource const * get(const std::string & resource)
{
  if (!g_cache || !g_assets_root) {
        SDLEx_LogError("%s - no assets root initialized",
          __METHOD_NAME__);
        throw std::exception("No assets root defined");
    }
    cache::iterator it = g_cache->find(resource);
    if (it == g_cache->end()) {
        return nullptr;
    }
    return it->second.resource;
}

/* Get texture from cache by path. Returns nullptr if not found */
texture const * get_texture(const std::string& resource)
{
  iresource const * res = get(resource);
  if (res == nullptr) {
    std::string found_at = find(resource);
    if (found_at.size()) {
      res = new texture(found_at);
      add(resource, found_at, res);
    }
  }

  if (res == nullptr) {
    SDLEx_LogError("%s - failed find texture '%s'", 
      __METHOD_NAME__,
      resource.c_str());
    throw std::exception("Failed to find texture");
  }

  texture const * tx = dynamic_cast<texture const *> (res);
  if (tx == nullptr) {
    SDLEx_LogError("%s - failed to cast iresource",
      __METHOD_NAME__);
    throw std::exception("Failed to cast iresource");
  }
  return tx;
}

/* Get data from cache by path. Returns nullptr if not found */
data const * get_data(const std::string& resource)
{
  iresource const * res = get(resource);
  if (res == nullptr) {
    std::string found_at = find(resource);
    if (found_at.size()) {
      res = new data(found_at);
      add(resource, found_at, res);
    }
  }

  if (res == nullptr) {
    SDLEx_LogError("%s - failed find data '%s'",
      __METHOD_NAME__,
      resource.c_str());
    throw std::exception("Failed to find data");
  }

  data const * d = dynamic_cast<data const *> (res);
  if (d == nullptr) {
    SDLEx_LogError("%s - failed to cast iresource",
      __METHOD_NAME__);
    throw std::exception("Failed to cast iresource");
  }
  return d;
}

ttf_font const * get_font(const std::string& resource)
{
  iresource const * res = get(resource);
  if (res == nullptr) {
    std::string found_at = find(resource);
    if (found_at.size()) {
      resource_descriptor dsc(resource);
      res = new ttf_font(found_at, atoi(dsc.resource_postfix.c_str()));
      add(resource, found_at, res);
    }
  }

  if (res == nullptr) {
    SDLEx_LogError("%s - failed find font '%s'",
      __METHOD_NAME__,
      resource.c_str());
    throw std::exception("Failed to find font");
  }

  ttf_font const * f = dynamic_cast<ttf_font const*> (res);
  if (f == nullptr) {
    SDLEx_LogError("%s - failed to cast iresource",
      __METHOD_NAME__);
    throw std::exception("Failed to cast iresource");
  }
  return f;
}


python::script const * get_script(const std::string& resource)
{
  iresource const * res = get(resource);
  if (res == nullptr) {
    std::string found_at = find(resource);
    if (found_at.size()) {
      resource_descriptor dsc(resource);
      res = new python::script(found_at);
      add(resource, found_at, res);
    }
  }

  if (res == nullptr) {
    SDLEx_LogError("%s - failed find script '%s'",
      __METHOD_NAME__,
      resource.c_str());
    throw std::exception("Failed to find script");
  }

  python::script const * s = dynamic_cast<python::script const*> (res);
  if (s == nullptr) {
    SDLEx_LogError("%s - failed to cast iresource",
      __METHOD_NAME__);
    throw std::exception("Failed to cast iresource");
  }
  return s;
}

static bool find_file(const boost::filesystem::path & directory, 
                      const boost::filesystem::path & file_name,
                      boost::filesystem::path & output) 
{
  const boost::filesystem::recursive_directory_iterator end;
  const auto it = std::find_if(boost::filesystem::recursive_directory_iterator(directory), end,
                          [&file_name](const boost::filesystem::directory_entry& e) {
                            return e.path().filename() == file_name;
                          });
  if (it == end) {
    return false;
  } else {
    output = it->path();
    return true;
  }
}

std::string find(const std::string& resource)
{
  if (!g_cache || !g_assets_root) {
      SDLEx_LogError("%s - no assets root initialized",
        __METHOD_NAME__);
      throw std::exception("no assets root");
  }

  resource_descriptor descr(resource);
  boost::filesystem::path resource_name(descr.resource_name);
  boost::filesystem::path resource_filepath;
  if (!find_file(*g_assets_root, resource_name, resource_filepath)) {
    SDL_Log("%s - nothing found for resource id '%s'", 
      __METHOD_NAME__,
      resource.c_str());
    return std::string();
  }

  return boost::filesystem::absolute(resource_filepath, *g_assets_root).string();
}

bool loaded(const std::string& resource)
{
  if (!g_cache || !g_assets_root) {
      SDLEx_LogError("%s - no assets root initialized",
        __METHOD_NAME__);
      throw std::exception("no assets root");
  }
  return g_cache->find(resource) != g_cache->end() ? true : false;
}

}; //namespace resources