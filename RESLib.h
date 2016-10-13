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
 * resources::find("comicsans.ttf:22") 
 *  -> std::string("/path/to/assets/ui/fonts/comicsans.ttf")
 * resources::loaded("comicsans.ttf:22")
 *  -> false
 * resources::get_font("comicsans.ttf:22");
 *  -> font<..., pts=22> const * at 0xAABBCCDD              [ 1 newly loaded instance ]
 * resources::loaded("comicsans.ttf:22")
 *  -> true
 * resources::get_font("comicsans.ttf:22");
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

/* 
 * Get item from resources cache by identifier.
 *  Returns nullptr if not found
 */
iresource const * get(const std::string & resource);

/* Get texture from resources by identifier */
texture const * get_texture(const std::string& resource);

/* Get texture as sprites_sheet from resources by identifier */
sprites_sheet const * get_sprites_sheet(const std::string& resource, 
                                        size_t sprite_width,
                                        size_t sprite_height);

/* Get data from resources by identifier */
data const * get_data(const std::string& resource);

/* Get font from resources by identifier */
ttf_font const * get_font(const std::string& resource,
                          size_t pt_size);

/* Get script from resources by identifier */
python::script const * get_script(const std::string& resource);

/* Initialize resources root */
void initialize(const std::string& assets_root);

/* Get path to the assets root */
std::string root_path();

/* Recursively lookup file system for an assets to match file names with give resource id */
std::string find(const std::string& resource);

/* Check if there is a resource present in cache with given id */
bool loaded(const std::string& resource);

/* Add new item to cache with id */
void add(std::string resource_id, std::string found_at, iresource const * r);

}

#endif //_RES_LIB_H