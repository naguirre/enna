/*
 * Copyright (C) 2005-2009 The Enna Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software and its Copyright notices. In addition publicly
 * documented acknowledgment must be given that this software has been used if
 * no source code of this software is made available publicly. This includes
 * acknowledgments in either Copyright notices, Manuals, Publicity and
 * Marketing documents or any documentation provided with any product
 * containing this software. This License does not apply to any software that
 * links to the libraries provided by this software (statically or
 * dynamically), but only to the software provided.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "enna.h"
#include "image.h"
#include "utils.h"

#define SMART_NAME "enna_image"

typedef struct _Smart_Data Smart_Data;

struct _Smart_Data
{
    Evas_Object *o_smart;
    Evas_Coord x, y, w, h;
    Evas_Object *obj;
    char fill_inside :1;
};

/* local subsystem functions */
static void _enna_image_smart_reconfigure(Smart_Data * sd);

/* local subsystem globals */
static Evas_Smart *_e_smart = NULL;

void enna_image_file_set(Evas_Object * obj, const char *file, const char *key)
{
    INTERNAL_ENTRY;
    /*evas_object_image_load_size_set(sd->obj, 32, 32);*/
    evas_object_image_file_set(sd->obj, file, key);
    _enna_image_smart_reconfigure(sd);
}

const char * enna_image_file_get(Evas_Object * obj)
{
    Smart_Data *sd;
    const char *file;

    sd = evas_object_smart_data_get(obj);
    if (!sd)
        return NULL;

    evas_object_image_file_get(sd->obj, &file, NULL);
    return file;
}

void enna_image_smooth_scale_set(Evas_Object * obj, int smooth)
{
    INTERNAL_ENTRY;
    evas_object_image_smooth_scale_set(sd->obj, smooth);
}

int enna_image_smooth_scale_get(Evas_Object * obj)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    if (!sd)
        return 0;

    return evas_object_image_smooth_scale_get(sd->obj);
}

void enna_image_alpha_set(Evas_Object * obj, int alpha)
{
    INTERNAL_ENTRY;
    evas_object_image_alpha_set(sd->obj, alpha);
}

int enna_image_alpha_get(Evas_Object * obj)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    if (!sd)
        return 0;

    return evas_object_image_alpha_get(sd->obj);
}

void enna_image_load_size_set(Evas_Object * obj, int w, int h)
{
    INTERNAL_ENTRY;
    evas_object_image_load_size_set(sd->obj, w, h);
}

void enna_image_size_get(Evas_Object * obj, int *w, int *h)
{
    INTERNAL_ENTRY;
    evas_object_image_size_get(sd->obj, w, h);
}

int enna_image_fill_inside_get(Evas_Object * obj)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    if (sd->fill_inside)
        return 1;
    return 0;
}

void enna_image_fill_inside_set(Evas_Object * obj, int fill_inside)
{
    INTERNAL_ENTRY;

    if (((sd->fill_inside) && (fill_inside)) || ((!sd->fill_inside)
            && (!fill_inside)))
        return;
    sd->fill_inside = fill_inside;
    _enna_image_smart_reconfigure(sd);
}

void enna_image_data_set(Evas_Object * obj, void *data, int w, int h)
{
    INTERNAL_ENTRY;
    evas_object_image_size_set(sd->obj, w, h);
    evas_object_image_data_copy_set(sd->obj, data);
}

void * enna_image_data_get(Evas_Object * obj, int *w, int *h)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    if (!sd)
        return NULL;
    evas_object_image_size_get(sd->obj, w, h);
    return evas_object_image_data_get(sd->obj, 0);
}

void enna_image_preload(Evas_Object *obj, Evas_Bool cancel)
{
    INTERNAL_ENTRY;
    evas_object_image_preload(sd->obj, cancel);
}

/* local subsystem globals */
static void _enna_image_smart_reconfigure(Smart_Data * sd)
{
    int iw, ih;
    Evas_Coord x, y, w, h;

    ih = 0;
    ih = 0;
    evas_object_image_size_get(sd->obj, &iw, &ih);
    iw = MMAX(iw, 1);
    ih = MMAX(ih, 1);

    if (sd->fill_inside)
    {
        w = sd->w;
        h = ((double)ih * w) / (double)iw;
        if (h > sd->h)
        {
            h = sd->h;
            w = ((double)iw * h) / (double)ih;
        }
        x = sd->x + ((sd->w - w) / 2);
        y = sd->y + ((sd->h - h) / 2);
        evas_object_move(sd->obj, x, y);
        evas_object_image_fill_set(sd->obj, 0, 0, w, h);
        evas_object_resize(sd->obj, w, h);
    }
    else
    {
        /*w = sd->w;
        h = ((double)ih * w) / (double)iw;
        if (h < sd->h)
        {
            h = sd->h;
            w = ((double)iw * h) / (double)ih;
        }*/
        evas_object_move(sd->obj, sd->x, sd->y);
        evas_object_image_fill_set(sd->obj, 0, 0, sd->w, sd->h);
        evas_object_resize(sd->obj, sd->w, sd->h);
    }


}

static void _enna_image_preload_cb(void *data, Evas *evas, Evas_Object *obj,
    void *event_info)
{
    Smart_Data *sd = data;

    if (!sd) return;

    evas_object_smart_callback_call(sd->o_smart, "preload", NULL);

}

static void _smart_add(Evas_Object * obj)
{
    Smart_Data *sd;

    sd = calloc(1, sizeof(Smart_Data));
    if (!sd)
        return;
    sd->obj = evas_object_image_add(evas_object_evas_get(obj));
    sd->x = 0;
    sd->y = 0;
    sd->w = 0;
    sd->h = 0;
    sd->fill_inside = 1;
    sd->o_smart = obj;
    evas_object_smart_member_add(sd->obj, obj);
    evas_object_smart_data_set(obj, sd);
    evas_object_event_callback_add(sd->obj, EVAS_CALLBACK_IMAGE_PRELOADED,
        _enna_image_preload_cb, sd);

}

static void _smart_del(Evas_Object * obj)
{
    INTERNAL_ENTRY;
    evas_object_del(sd->obj);
    free(sd);
}

static void _smart_move(Evas_Object * obj, Evas_Coord x, Evas_Coord y)
{
    INTERNAL_ENTRY;

    if ((sd->x == x) && (sd->y == y))
        return;
    sd->x = x;
    sd->y = y;
    _enna_image_smart_reconfigure(sd);
}

static void _smart_resize(Evas_Object * obj, Evas_Coord w, Evas_Coord h)
{
    INTERNAL_ENTRY;

    if ((sd->w == w) && (sd->h == h))
        return;
    sd->w = w;
    sd->h = h;
    _enna_image_smart_reconfigure(sd);
}

static void _smart_show(Evas_Object * obj)
{
    INTERNAL_ENTRY;
    evas_object_show(sd->obj);
}

static void _smart_hide(Evas_Object * obj)
{
    INTERNAL_ENTRY;
    evas_object_hide(sd->obj);
}

static void _smart_color_set(Evas_Object * obj, int r, int g, int b, int a)
{
    INTERNAL_ENTRY;
    evas_object_color_set(sd->obj, r, g, b, a);
}

static void _smart_clip_set(Evas_Object * obj, Evas_Object * clip)
{
    INTERNAL_ENTRY;
    evas_object_clip_set(sd->obj, clip);
}

static void _smart_clip_unset(Evas_Object * obj)
{
    INTERNAL_ENTRY;
    evas_object_clip_unset(sd->obj);
}

static void _enna_image_smart_init(void)
{
    static const Evas_Smart_Class sc = {
       SMART_NAME,
       EVAS_SMART_CLASS_VERSION,
       _smart_add,
       _smart_del,
       _smart_move,
       _smart_resize,
       _smart_show,
       _smart_hide,
       _smart_color_set,
       _smart_clip_set,
       _smart_clip_unset,
       NULL,
       NULL
    };

    if (!_e_smart)
       _e_smart = evas_smart_class_new(&sc);
}

/* externally accessible functions */
Evas_Object *
enna_image_add(Evas * evas)
{
    _enna_image_smart_init();
    return evas_object_smart_add(evas, _e_smart);
}

