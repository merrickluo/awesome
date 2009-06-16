/*
 * luaobject.h - useful functions for handling Lua objects
 *
 * Copyright © 2009 Julien Danjou <julien@danjou.info>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef AWESOME_COMMON_LUAOBJECT
#define AWESOME_COMMON_LUAOBJECT

#include <lauxlib.h>
#include "common/array.h"

/** Type for Lua references */
typedef int luaA_ref;

static inline int
luaA_settype(lua_State *L, const char *type)
{
    luaL_getmetatable(L, type);
    lua_setmetatable(L, -2);
    return 1;
}

DO_ARRAY(luaA_ref, luaA_ref, DO_NOTHING)

#define LUA_OBJECT_HEADER \
        luaA_ref_array_t refs;

/** Generic type for all objects.
 * All Lua objects can be casted to this type.
 */
typedef struct
{
    LUA_OBJECT_HEADER
} lua_object_t;

#define LUA_OBJECT_FUNCS(type, prefix, lua_type)                               \
    static inline type *                                                       \
    prefix##_new(lua_State *L)                                                 \
    {                                                                          \
        type *p = lua_newuserdata(L, sizeof(type));                            \
        p_clear(p, 1);                                                         \
        luaA_settype(L, lua_type);                                             \
        return p;                                                              \
    }                                                                          \
                                                                               \
    static inline int                                                          \
    prefix##_push(lua_State *L, type *item)                                    \
    {                                                                          \
        if(item)                                                               \
        {                                                                      \
            assert(item->refs.len);                                            \
            lua_rawgeti(L, LUA_REGISTRYINDEX, item->refs.tab[0]);              \
        }                                                                      \
        else                                                                   \
            lua_pushnil(L);                                                    \
        return 1;                                                              \
    }                                                                          \
                                                                               \
    static inline type *                                                       \
    prefix##_ref(lua_State *L, int ud)                                         \
    {                                                                          \
        if(lua_isnil(L, ud))                                                   \
            return NULL;                                                       \
        type *item = luaL_checkudata(L, ud, lua_type);                         \
        lua_pushvalue(L, ud);                                                  \
        luaA_ref_array_append(&item->refs, luaL_ref(L, LUA_REGISTRYINDEX));    \
        lua_remove(L, ud);                                                     \
        return item;                                                           \
    }                                                                          \
                                                                               \
    static inline void                                                         \
    prefix##_unref(lua_State *L, type *item)                                   \
    {                                                                          \
        if(item)                                                               \
        {                                                                      \
            assert(item->refs.len);                                            \
            luaL_unref(L, LUA_REGISTRYINDEX, item->refs.tab[0]);               \
            luaA_ref_array_take(&item->refs, 0);                               \
        }                                                                      \
    }

/** Garbage collect a Lua object.
 * \param L The Lua VM state.
 * \return The number of elements pushed on stack.
 */
static inline int
luaA_object_gc(lua_State *L)
{
    lua_object_t *item = lua_touserdata(L, 1);
    luaA_ref_array_wipe(&item->refs);
    return 0;
}

#endif

// vim: filetype=c:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=80
