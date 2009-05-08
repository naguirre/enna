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

#define _GNU_SOURCE
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <Ecore_File.h>

#include "enna.h"
#include "module.h"
#include "metadata.h"
#include "xml_utils.h"
#include "url_utils.h"
#include "logs.h"
#include "utils.h"

#define ENNA_MODULE_NAME        "metadata_nfo"
#define ENNA_GRABBER_NAME       "nfo"
#define ENNA_GRABBER_PRIORITY   2

typedef struct _Enna_Metadata_Nfo
{
    Evas *evas;
    Enna_Module *em;
} Enna_Metadata_Nfo;

static Enna_Metadata_Nfo *mod;

/****************************************************************************/
/*                          NFO Reader Helpers                              */
/****************************************************************************/

static void
nfo_parse_stream_video (Enna_Metadata *meta, xmlNode *video)
{
    xml_search_str (video, "codec",  &meta->video->codec);
    xml_search_int (video, "width",  &meta->video->width);
    xml_search_int (video, "height", &meta->video->height);
}

static void
nfo_parse_stream_audio (Enna_Metadata *meta, xmlNode *audio)
{
    xml_search_str (audio, "codec",    &meta->music->codec);
    xml_search_int (audio, "channels", &meta->music->channels);
}

static void
nfo_get_tvshow_art (Enna_Metadata *meta,
                    const char *filename, const char *art)
{
    char *cwd;
    char show_art[1024];
    struct stat st;
    int err;

    /* check if backdrop already exists */
    if (meta->backdrop)
        return;

    cwd = ecore_file_dir_get (filename);
    if (!cwd)
        return;

    memset (show_art, '\0', sizeof (show_art));
    snprintf (show_art, sizeof (show_art), "%s/%s", cwd, art);

    err = stat (show_art, &st);
    if (!err && S_ISREG (st.st_mode))
        meta->backdrop = strdup (show_art);
}

static void
nfo_parse (Enna_Metadata *meta, const char *filename)
{
    xmlDocPtr doc = NULL;
    xmlNode *movie, *fileinfo, *streamdetails;
    xmlChar *tmp;

    struct stat st;
    char *buf;
    ssize_t n;
    int err, fd;
    int tvshow = 0;

    /* read NFO file */
    err = stat (filename, &st);
    buf = calloc (1, st.st_size + 1);
    fd = open (filename, O_RDONLY);
    n = read (fd, buf, st.st_size);
    close (fd);

    /* parse its XML content */
    doc = get_xml_doc_from_memory (buf);
    free (buf);
    if (!doc)
        goto error;

    movie = get_node_xml_tree (xmlDocGetRootElement (doc), "movie");
    if (!movie)
    {
        tvshow = 1;
        movie = get_node_xml_tree (xmlDocGetRootElement (doc),
                                   "episodedetails");
        if (!movie)
            goto error;
    }

    fileinfo = get_node_xml_tree (movie, "fileinfo");
    if (!fileinfo)
        goto error;

    streamdetails = get_node_xml_tree (fileinfo, "streamdetails");
    if (streamdetails)
    {
        xmlNode *audio, *video;

        video = get_node_xml_tree (streamdetails, "video");
        if (video)
            nfo_parse_stream_video (meta, video);

        audio = get_node_xml_tree (streamdetails, "audio");
        if (audio)
            nfo_parse_stream_audio (meta, audio);
    }

    /* movie title */
    if (!meta->title)
    {
        tmp = get_prop_value_from_xml_tree (movie, "title");
        if (tmp)
        {
            xmlChar *show_title = NULL, *season = NULL, *episode = NULL;

            if (tvshow)
            {
                char *cwd;

                cwd = ecore_file_dir_get (filename);
                if (cwd)
                {
                    char dir_nfo[1024];

                    memset (dir_nfo, '\0', sizeof (dir_nfo));
                    snprintf (dir_nfo, sizeof (dir_nfo),
                              "%s/../tvshow.nfo", cwd);

                    err = stat (dir_nfo, &st);
                    if (!err && S_ISREG (st.st_mode))
                    {
                        xmlDocPtr ds = NULL;
                        xmlNode *tvshow = NULL;

                        buf = calloc (1, st.st_size + 1);
                        fd = open (dir_nfo, O_RDONLY);
                        n = read (fd, buf, st.st_size);
                        close (fd);

                        /* parse its XML content */
                        ds = get_xml_doc_from_memory (buf);
                        free (buf);

                        if (doc)
                            tvshow =
                                get_node_xml_tree (xmlDocGetRootElement (ds),
                                                   "tvshow");

                        if (tvshow)
                            show_title =
                                get_prop_value_from_xml_tree (tvshow,
                                                              "title");

                        xmlFreeDoc (ds);
                    }
                }

                season = get_prop_value_from_xml_tree (movie, "season");
                episode = get_prop_value_from_xml_tree (movie, "episode");
            }

            if (season && episode)
            {
                char title[1024];

                memset (title, '\0', sizeof (title));
                if (show_title)
                    snprintf (title, sizeof (title), "%s - S%.2dE%.2d - %s",
                              (char *) show_title, atoi ((char *) season),
                              atoi ((char *) episode), tmp);
                else
                    snprintf (title, sizeof (title), "S%.2dE%.2d - %s",
                              atoi ((char *) season),
                              atoi ((char *) episode), tmp);

                meta->title = strdup (title);

                if (season)
                    xmlFree (season);
                if (episode)
                    xmlFree (episode);
            }
            else
                meta->title = strdup ((char *) tmp);

            enna_log (ENNA_MSG_EVENT, ENNA_MODULE_NAME,
                      "Title: %s", meta->title);
            xmlFree (tmp);
        }
    }

    /* rating */
    xml_search_int (movie, "rating", &meta->rating);
    meta->rating /= 2;

    /* runtime */
    xml_search_int (movie, "runtime", &meta->runtime);

    /* plot */
    xml_search_str (movie, "plot", &meta->overview);

    /* production year */
    xml_search_int (movie, "year", &meta->year);

    /* genre */
    xml_search_str (movie, "genre", &meta->categories);

    /* director */
    xml_search_str (movie, "director", &meta->director);

    /* studio */
    xml_search_str (movie, "studio", &meta->studio);

    /* actors */
    if (!meta->actors)
    {
        xmlNode *act;
        int found = 0;

        act = get_node_xml_tree (movie, "actor");
        while (act && found < 5)
        {
            char *actor = NULL, *role = NULL;

            if (strcmp ((char *) act->name, "actor"))
            {
                act = act->next;
                continue;
            }

            xml_search_str (act, "name", &actor);
            xml_search_str (act, "role", &role);

            if (actor)
            {
                char str[128];

                memset (str, '\0', sizeof (str));
                if (role)
                    snprintf (str, sizeof (str), "%s (%s)", actor, role);
                else
                    snprintf (str, sizeof (str), "%s", actor);
                enna_metadata_add_actors (meta, str);
                found++;
            }

            ENNA_FREE (actor);
            ENNA_FREE (role);
            act = act->next;
        }
    }

    /* special hack to retrieve tv show cover if none exists for the file */
    /* NB: localfiles grabber has already been passed */
    if (tvshow)
        nfo_get_tvshow_art (meta, filename, "../fanart.jpg");

 error:
    if (doc)
        xmlFreeDoc (doc);
}

/****************************************************************************/
/*                        Private Module API                                */
/****************************************************************************/

static void
nfo_grab (Enna_Metadata *meta, int caps)
{
    char *s, *e, *base;
    char nfo[1024];
    struct stat st;

    if (!meta)
        return;

    s = strchr (meta->uri, '/');
    if (!s)
        return;

    e = strrchr (s + 2, '.');
    if (!e)
        return;

    base = strndup (s + 2, strlen (s + 2) - strlen (e));
    memset (nfo, '\0', sizeof (nfo));
    snprintf (nfo, sizeof (nfo), "%s.nfo", base);
    free (base);

    stat (nfo, &st);
    if (!S_ISREG (st.st_mode))
        return;

    enna_log (ENNA_MSG_EVENT, ENNA_MODULE_NAME,
              "Grabbing info from %s", nfo);

    nfo_parse (meta, nfo);
}

static Enna_Metadata_Grabber grabber = {
    ENNA_GRABBER_NAME,
    ENNA_GRABBER_PRIORITY,
    1,
    ENNA_GRABBER_CAP_COVER,
    nfo_grab,
};

/****************************************************************************/
/*                         Public Module API                                */
/****************************************************************************/

Enna_Module_Api module_api =
{
    ENNA_MODULE_VERSION,
    ENNA_MODULE_METADATA,
    ENNA_MODULE_NAME
};

void
module_init (Enna_Module *em)
{
    if (!em)
        return;

    mod = calloc (1, sizeof (Enna_Metadata_Nfo));

    mod->em = em;
    mod->evas = em->evas;

    enna_metadata_add_grabber (&grabber);
}

void
module_shutdown (Enna_Module *em)
{
    free (mod);
}
