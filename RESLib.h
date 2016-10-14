/* 
 * GMLib - The GMLib library.
 * Copyright Stanislav Yudin, 2014-2016
 *
 * This component is a shared resources cache implementation for
 * core classes and types used by GMLib. Such resource classes are
 * - "texture"
 * - "ttf_font"
 * - "data" 
 * - other classes derived from the "iresource" interface.
 * The returned pointers to the constant shared objects are
 * managed by the resource cache. The pointers themselfs
 * can be re-assiged to other constant instances of resource
 * classes loaded manual. 
 * ---------------------------------------------------------------
 *
 * Resource ID Format
 *
 * A Resource ID is a string with format of [resource_name:resource_postfix].
 * A resource_name segment is a unique filename of the resource in the assets storage.
 * A resource_postfix is a an optional string witch might be used by certain resource
 * loaders, such as fonts.
 * Example:
 * resources::get_texture("cat.png");
 *  -> <texture ...> const * at 0x10101010               [ newly loaded instance ]
 * resources::find("comicsans.ttf") 
 *  -> std::string("/path/to/assets/ui/fonts/comicsans.ttf")
 * resources::loaded("comicsans.ttf")
 *  -> false
 * resources::get_font("comicsans.ttf", 12);
 *  -> font<..., pts=22> const * at 0xAABBCCDD              [ 1 newly loaded instance ]
 * resources::loaded("comicsans.ttf")
 *  -> true
 * resources::get("comicsans.ttf:12");
 *  -> font<..., pts=22> const * at 0xAABBCCDD              [ 2 same cached instance ]
 */

#ifndef _RES_LIB_H
#define _RES_LIB_H

#include "GMLib.h"
#include "GMTexture.h"
#include "GMSprites.h"
#include "GMData.h"
#include "GMPython.h"

namespace resources
{

// resource id is a string
typedef std::string resource_id;

/* 
 * Get item from resources cache by identifier.
 *  Returns nullptr if not found
 */
iresource const * get(const resource_id & resource);

/* Get texture from resources by identifier */
texture const * get_texture(const std::string & filename);

/* Get texture as sprites_sheet from resources by identifier */
sprites_sheet const * get_sprites_sheet(const std::string& filename, 
                                        size_t sprite_width,
                                        size_t sprite_height);

/* Get data from resources by identifier */
data const * get_data(const std::string & filename);

/* Get font from resources by identifier */
ttf_font const * get_font(const std::string & filename,
                          size_t pt_size);

/* Get script from resources by identifier */
python::script const * get_script(const std::string& filename);

/* Initialize resources */
void initialize(const strings_list & assets_root);

/* Lookup caches assets to match files with extensions list */
size_t find_files(const strings_list & ext_list, paths_list & output);

/* Lookup cached assets to match filename */
fs::path find_file(const fs::path & filename);
bool find_file(const fs::path & filename, fs::path & found);

/* Lookup cached assets to find path to resource's descriptor filename */
fs::path find_resource(const resource_id & resource);
bool find_resource(const resource_id & resource, fs::path & found);

/* Check if there is a resource present in cache with given id */
bool loaded(const resource_id & resource);

/* Add new item to cache with id */
void add(const resource_id & resource, const fs::path & found_at, iresource const * r);

/* Search directory recursively for a match with a filename */
bool lookup_file(const fs::path & directory, 
                 const fs::path & file_name,
                 fs::path & output);
}

#endif //_RES_LIB_H