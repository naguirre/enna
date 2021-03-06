/*
 * GeeXboX Enna Media Center.
 * Copyright (C) 2005-2010 The Enna Project
 *
 * This file is part of Enna.
 *
 * Enna is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Enna is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Enna; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <Edje.h>
#include <Elementary.h>

#include "enna.h"
#include "enna_config.h"
#include "metadata.h"
#include "logs.h"
#include "buffer.h"
#include "utils.h"

#define SMART_NAME "enna_panel_infos"

#define VIDEO_DEFAULT_COVER "cover/movie"

typedef struct _Smart_Data Smart_Data;

struct _Smart_Data
{
    Evas_Object *o_edje;
    Evas_Object *o_img;
    Evas_Object *o_rating;
    Evas_Object *o_cover;
};

static void
_del(void *data, Evas *a, Evas_Object *obj, void *event_info)
{
    Smart_Data *sd = data;
    ENNA_OBJECT_DEL(sd->o_img);
    ENNA_OBJECT_DEL(sd->o_rating);
    ENNA_OBJECT_DEL(sd->o_cover);
    ENNA_FREE(sd);
}

/* externally accessible functions */
Evas_Object *
enna_panel_infos_add(Evas * evas)
{
    Smart_Data *sd;

    sd = calloc(1, sizeof(Smart_Data));

    sd->o_edje = edje_object_add(evas);
    edje_object_file_set(sd->o_edje,
                         enna_config_theme_get(),
                         "activity/video/panel_infos");
    evas_object_show(sd->o_edje);
    evas_object_data_set(sd->o_edje, "sd", sd);
    evas_object_event_callback_add(sd->o_edje, EVAS_CALLBACK_DEL,
                                   _del, sd);
    return sd->o_edje;
}

/****************************************************************************/
/*                          Information Panel                               */
/****************************************************************************/

void
enna_panel_infos_set_text(Evas_Object *obj, Enna_Metadata *m)
{
    Enna_Buffer *buf;
    const char *codec, *value;
    const char *alternative_title, *title, *categories, *year;
    const char *length, *director, *actors, *overview;
    Smart_Data *sd = evas_object_data_get(obj, "sd");
    int len;

    if (!sd)
        return;

    if (!m)
    {
        edje_object_part_text_set(sd->o_edje, "panel.textblock",
                                  _("No such information ..."));
        return;
    }

    buf = enna_buffer_new();

    enna_buffer_append(buf, "<h4><hl><sd><b>");
    alternative_title = enna_metadata_meta_get(m, "alternative_title", 1);
    title = enna_metadata_meta_get(m, "title", 1);
    enna_buffer_append(buf, alternative_title ? alternative_title : title);
    enna_buffer_append(buf, "</b></sd></hl></h4><br>");
    len = buf->len;

    categories = enna_metadata_meta_get(m, "category", 5);
    if (categories)
        enna_buffer_appendf(buf, "<h2>%s</h2><br>", categories);

    year = enna_metadata_meta_get(m, "year", 1);
    if (year)
        enna_buffer_append(buf, year);

    length = enna_metadata_meta_duration_get(m);
    if (length)
    {
        if (year)
            enna_buffer_append(buf, " - ");
        enna_buffer_appendf(buf, "%s", length);
    }

    enna_buffer_append(buf, "<br><br>");
    len += 8;

    director = enna_metadata_meta_get(m, "director", 1);
    if (director) {
        enna_buffer_append(buf, "<ul>");
        enna_buffer_append(buf, _("Director:"));    
        enna_buffer_appendf(buf, " </ul> %s<br>", director);
    }

    actors = enna_metadata_meta_get(m, "actor", 5);
    if (actors) {
        enna_buffer_append(buf, "<ul>");
        enna_buffer_append(buf, _("Cast:"));
        enna_buffer_appendf(buf, " </ul> %s<br>", actors);
    }

    if (director || actors)
        enna_buffer_append(buf, "<br>");

    overview = enna_metadata_meta_get(m, "synopsis", 1);
    if (overview)
        enna_buffer_appendf(buf, "%s", overview);

    enna_buffer_append(buf, "<br><br>");
    len += 8;

    codec = enna_metadata_meta_get(m, "video_codec", 1);
    if (codec)
    {
        const char *width, *height, *aspect;
        width  = enna_metadata_meta_get(m, "width", 1);
        height = enna_metadata_meta_get(m, "height", 1);
        aspect = enna_metadata_meta_get(m, "video_aspect", 1);
        enna_buffer_append(buf, "<hl>");
        enna_buffer_append(buf, _("Video:"));
        enna_buffer_appendf(buf, " </hl> %s", codec);
        if (width && height)
            enna_buffer_appendf(buf, ", %sx%s", width, height);
        if (aspect)
        {
            float ratio = enna_util_atof(aspect) / 10000;
            if (ratio)
                enna_buffer_appendf(buf, ", %.2f", ratio);
        }
        enna_buffer_appendf(buf, "<br>");
        eina_stringshare_del(width);
        eina_stringshare_del(height);
        eina_stringshare_del(aspect);
        eina_stringshare_del(codec);
    }
    codec = enna_metadata_meta_get(m, "audio_codec", 1);
    if (codec)
    {
        const char *channels, *bitrate;
        channels = enna_metadata_meta_get(m, "audio_channels", 1);
        bitrate  = enna_metadata_meta_get(m, "audio_bitrate", 1);
        enna_buffer_append(buf, "<hl>");
        enna_buffer_append(buf, _("Audio:"));
        enna_buffer_appendf(buf, " </hl> %s", codec);
        if (channels)
            enna_buffer_appendf(buf, ", %s ch.", channels);
        if (bitrate)
            enna_buffer_appendf(buf, ", %i kbps", atoi(bitrate) / 1000);
        enna_buffer_appendf(buf, "<br>");
        eina_stringshare_del(channels);
        eina_stringshare_del(bitrate);
        eina_stringshare_del(codec);
    }

    value = enna_metadata_meta_get(m, "filesize", 1);
    if (value) {
        enna_buffer_append(buf, "<hl>");
        enna_buffer_append(buf, _("Size:"));
        enna_buffer_appendf(buf, " </hl> %.2f MB<br>",
                atol(value) / 1024.0 / 1024.0);
    }
    eina_stringshare_del(value);

    /* tell if no more infos are available */
    if (buf->len == len)
      enna_buffer_append(buf, _("No further information can be retrieved"));

    edje_object_part_text_set(sd->o_edje, "panel.textblock", buf->buf);

    enna_buffer_free(buf);
    eina_stringshare_del(alternative_title);
    eina_stringshare_del(title);
    eina_stringshare_del(categories);
    eina_stringshare_del(year);
    eina_stringshare_del(length);
    eina_stringshare_del(director);
    eina_stringshare_del(actors);
    eina_stringshare_del(overview);
}

void
enna_panel_infos_set_cover(Evas_Object *obj, Enna_Metadata *m)
{
    Evas_Object *cover;
    char *file = NULL;
    int from_vfs = 1;
    const char *cv;
    Smart_Data *sd = evas_object_data_get(obj, "sd");

    if (!sd)
        return;

    if (!m)
    {
        file = strdup(VIDEO_DEFAULT_COVER);
        from_vfs = 0;
    }

    cv = enna_metadata_meta_get(m, "cover", 1);
    if (!file && cv)
    {
        char dst[1024] = { 0 };

        snprintf(dst, sizeof(dst), "%s/covers/%s",
                 enna_util_data_home_get(), cv);
        if (ecore_file_exists(dst))
            file = strdup(dst);
    }

    if (!file)
    {
        file = strdup(VIDEO_DEFAULT_COVER);
        from_vfs = 0;
    }

    if (from_vfs)
    {
        cover = elm_icon_add(sd->o_edje);
        elm_image_file_set(cover, file, NULL);
    }
    else
    {
        cover = edje_object_add(evas_object_evas_get(sd->o_edje));
        edje_object_file_set(cover, enna_config_theme_get(), file);
    }


    ENNA_OBJECT_DEL(sd->o_cover);
    sd->o_cover = cover;
    edje_object_part_swallow(sd->o_edje,
                             "panel.cover.swallow", sd->o_cover);

    /* hide underlying shadows with default movie poster cover */
    edje_object_signal_emit(sd->o_edje,
                            (!strcmp(file, VIDEO_DEFAULT_COVER)) ?
                            "shadow,hide" : "shadow,show", "enna");

    eina_stringshare_del(cv);
    eina_stringshare_del(file);
}

void
enna_panel_infos_set_rating(Evas_Object *obj, Enna_Metadata *m)
{
    Evas_Object *rating = NULL;
    const char *rt;
    Smart_Data *sd = evas_object_data_get(obj, "sd");

    if (!sd)
        return;

    rt = enna_metadata_meta_get(m, "rating", 1);
    if (rt)
    {
        char rate[16];
        int r;

        r = MMAX(atoi(rt), 0);
        r = MMIN(r, 5);
        memset(rate, '\0', sizeof(rate));
        snprintf(rate, sizeof(rate), "rating/%d", r);
        rating = edje_object_add(evas_object_evas_get(sd->o_edje));
        edje_object_file_set(rating, enna_config_theme_get(), rate);
    }

    ENNA_OBJECT_DEL(sd->o_rating);
    sd->o_rating = rating;
    edje_object_part_swallow(sd->o_edje,
                             "panel.rating.swallow", sd->o_rating);
    eina_stringshare_del(rt);
}
