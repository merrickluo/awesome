/*
 * imagebox.c - imagebox widget
 *
 * Copyright © 2008 Julien Danjou <julien@danjou.info>
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

#include "widget.h"
#include "titlebar.h"

/** The imagebox private data structure */
typedef struct
{
    /** Imagebox image */
    image_t *image;
    color_t bg;
    alignment_t valign;
    bool resize;
} imagebox_data_t;

static area_t
imagebox_geometry(widget_t *widget, screen_t *screen, int height, int width)
{
    area_t geometry;
    imagebox_data_t *d = widget->data;

    if(d->image)
    {
        int iwidth = image_getwidth(d->image);
        int iheight = image_getheight(d->image);
        if(d->resize)
        {
            double ratio = (double) height / iheight;
            geometry.width = ratio * iwidth;
            if(geometry.width > width)
            {
                geometry.width = 0;
                geometry.height = 0;
            }
            else
                geometry.height = height;
        }
        else if(iwidth <= width)
        {
            geometry.width = iwidth;
            geometry.height = height;
        }
        else
        {
            geometry.width = 0;
            geometry.height = 0;
        }
    }
    else
    {
        geometry.width = 0;
        geometry.height = 0;
    }

    geometry.x = geometry.y = 0;

    return geometry;
}

static area_t
imagebox_extents(lua_State *L, widget_t *widget)
{
    area_t geometry = {
        .x = 0,
        .y = 0,
        .width = 0,
        .height = 0
    };
    imagebox_data_t *d = widget->data;

    if(d->image)
    {
        geometry.width = image_getwidth(d->image);
        geometry.height = image_getheight(d->image);
    }

    return geometry;
}

/** Draw an image.
 * \param widget The widget.
 * \param ctx The draw context.
 * \param geometry The geometry we draw in.
 * \param p A pointer to the object we're draw onto.
 */
static void
imagebox_draw(widget_t *widget, draw_context_t *ctx, area_t geometry, wibox_t *p)
{
    imagebox_data_t *d = widget->data;

    if(d->image && geometry.width && geometry.height)
    {
        if(d->bg.initialized)
            draw_rectangle(ctx, geometry, 1.0, true, &d->bg);

        int y = geometry.y;
        int iheight = image_getheight(d->image);
        double ratio = d->resize ? (double) geometry.height / iheight : 1;
        switch(d->valign)
        {
          case AlignBottom:
            y += geometry.height - iheight;
            break;
          case AlignCenter:
            y += (geometry.height - iheight) / 2;
            break;
          default:
            break;
        }

        draw_image(ctx, geometry.x, y, ratio, d->image);
    }
}

/** Delete a imagebox widget.
 * \param w The widget to destroy.
 */
static void
imagebox_destructor(widget_t *w)
{
    imagebox_data_t *d = w->data;
    image_unref(globalconf.L, d->image);
    p_delete(&d);
}

/** Imagebox widget.
 * \param L The Lua VM state.
 * \param token The key token.
 * \param resize Resize image.
 * \param valign Vertical alignment, top, bottom or center.
 * \return The number of elements pushed on stack.
 * \luastack
 * \lfield image The image to display.
 * \lfield bg The background color to use.
 */
static int
luaA_imagebox_index(lua_State *L, awesome_token_t token)
{
    widget_t *widget = luaL_checkudata(L, 1, "widget");
    imagebox_data_t *d = widget->data;

    switch(token)
    {
      case A_TK_IMAGE:
        image_push(L, d->image);
        break;
      case A_TK_BG:
        luaA_pushcolor(L, &d->bg);
        break;
      case A_TK_RESIZE:
        lua_pushboolean(L, d->resize);
        break;
      case A_TK_VALIGN:
        lua_pushstring(L, draw_align_tostr(d->valign));
        break;
      default:
        return 0;
    }

    return 1;
}

/** The __newindex method for a imagebox object.
 * \param L The Lua VM state.
 * \param token The key token.
 * \return The number of elements pushed on stack.
 */
static int
luaA_imagebox_newindex(lua_State *L, awesome_token_t token)
{
    widget_t *widget = luaL_checkudata(L, 1, "widget");
    imagebox_data_t *d = widget->data;

    switch(token)
    {
        const char *buf;
        size_t len;

      case A_TK_IMAGE:
        image_unref(L, d->image);
        d->image = image_ref(L, 3);
        break;
      case A_TK_BG:
        if(lua_isnil(L, 3))
            p_clear(&d->bg, 1);
        else if((buf = luaL_checklstring(L, 3, &len)))
            color_init_reply(color_init_unchecked(&d->bg, buf, len));
        break;
      case A_TK_RESIZE:
        d->resize = luaA_checkboolean(L, 3);
        break;
      case A_TK_VALIGN:
        if((buf = luaL_checklstring(L, 3, &len)))
            d->valign = draw_align_fromstr(buf, len);
        break;
      default:
        return 0;
    }

    widget_invalidate_bywidget(widget);

    return 0;
}


/** Create a new imagebox widget.
 * \param w The widget to initialize.
 * \return A brand new widget.
 */
widget_t *
widget_imagebox(widget_t *w)
{
    imagebox_data_t *d;
    w->draw = imagebox_draw;
    w->index = luaA_imagebox_index;
    w->newindex = luaA_imagebox_newindex;
    w->destructor = imagebox_destructor;
    w->geometry = imagebox_geometry;
    w->extents = imagebox_extents;
    w->data = d = p_new(imagebox_data_t, 1);
    d->resize = true;
    d->valign = AlignTop;

    return w;
}

// vim: filetype=c:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=80
