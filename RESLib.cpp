#include "RESLib.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>


namespace resources
{

/* item in cache */
struct resource_descriptor 
{
  iresource * resource;
  std::string resource_name;
  std::string resource_postfix;
  boost::filesystem::path resource_path;

  void init(const std::string & resource_id,
            const std::string & p = std::string(),
            iresource * r = nullptr)
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
                      iresource * r = nullptr)
  {
    init(resource_id, p, r);
  }

};

typedef std::map<std::string, resource_descriptor> cache;

/* Static pointer to assets folder */
static boost::filesystem::path * g_assets_root;
static cache* g_cache = nullptr;

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
    SDLEx_LogError("resources::initialize() - assets root is not a directory '%s'",
      g_assets_root->string().c_str());
    throw std::exception("Assets root is not a directory");
  }

  g_cache = new cache();
  SDL_Log("resources::initialize() - assets ready at '%s'",
    g_assets_root->string().c_str());
}

std::string root_path()
{
  if (g_assets_root != nullptr)
    return g_assets_root->string();

  SDLEx_LogError("resources::root_path() - no assets root initialized");
  throw std::exception("no assets root");
}

/* Get item from cache by path. Returns nullptr if not found */
iresource * get(const std::string & resource)
{
  if (!g_cache || !g_assets_root) {
        SDLEx_LogError("resources::get_texture() - no assets root initialized");
        throw std::exception("no assets root");
    }
    cache::iterator it = g_cache->find(resource);
    if (it == g_cache->end()) {
        return nullptr;
    }
    return it->second.resource;
}

/* Get texture from cache by path. Returns nullptr if not found */
const texture & get_texture(const std::string& resource)
{
  iresource * res = get(resource);
  if (res == nullptr) {
    std::string found_at = find(resource);
    if (found_at.size()) {
      res = new texture(found_at);
      g_cache->insert(std::make_pair(resource, resource_descriptor(resource, found_at, res)));
    }
  }

  if (res == nullptr) {
    SDLEx_LogError("resources::get_texture - failed find texture '%s'", 
      resource.c_str());
    throw std::exception("Failed to find texture");
  }

  texture * tx = dynamic_cast<texture*> (res);
  if (tx == nullptr) {
    SDLEx_LogError("resources::get_texture - failed to cast iresource");
    throw std::exception("Failed to cast iresource");
  }
  tx->incref();
  return (*tx);
}

/* Get data from cache by path. Returns nullptr if not found */
const data & get_data(const std::string& resource)
{
  iresource * res = get(resource);
  if (res == nullptr) {
    std::string found_at = find(resource);
    if (found_at.size()) {
      res = new data(found_at);
      g_cache->insert(std::make_pair(resource, resource_descriptor(resource, found_at, res)));
    }
  }

  if (res == nullptr) {
    SDLEx_LogError("resources::get_texture - failed find texture '%s'", 
      resource.c_str());
    throw std::exception("Failed to find texture");
  }

  data * d = dynamic_cast<data*> (res);
  if (d == nullptr) {
    SDLEx_LogError("resources::get_data - failed to cast iresource");
    throw std::exception("Failed to cast iresource");
  }
  d->incref();
  return (*d);
}

const ttf_font & get_font(const std::string& resource)
{
  iresource * res = get(resource);
  if (res == nullptr) {
    std::string found_at = find(resource);
    if (found_at.size()) {
      res = new texture(found_at);
      g_cache->insert(std::make_pair(resource, resource_descriptor(resource, found_at, res)));
    }
  }

  if (res == nullptr) {
    SDLEx_LogError("resources::get_texture - failed find texture '%s'", 
      resource.c_str());
    throw std::exception("Failed to find texture");
  }

  ttf_font * f = dynamic_cast<ttf_font*> (res);
  if (f == nullptr) {
    SDLEx_LogError("resources::get_font - failed to cast iresource");
    throw std::exception("Failed to cast iresource");
  }
  f->incref();
  return (*f);
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
      SDLEx_LogError("resources::find() - no assets root initialized");
      throw std::exception("no assets root");
  }

  resource_descriptor descr(resource);
  boost::filesystem::path resource_name(descr.resource_name);
  boost::filesystem::path resource_filepath;
  if (!find_file(*g_assets_root, resource_name, resource_filepath)) {
    SDL_Log("resources::find - nothing found for resource name '%s'", resource.c_str());
    return std::string();
  }

  std::string abs_path = boost::filesystem::absolute(resource_filepath, *g_assets_root).string();
  SDL_Log("resources::find - found '%s' at '%s'", 
    resource.c_str(),
    abs_path.c_str());
  
  return abs_path;
}

bool exists(const std::string& resource)
{
  if (!g_cache || !g_assets_root) {
      SDLEx_LogError("resources::exists() - no assets root initialized");
      throw std::exception("no assets root");
  }
  return g_cache->find(resource) != g_cache->end() ? true : false;
}

}; //namespace resources