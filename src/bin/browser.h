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

#ifndef BROWSER_H
#define BROWSER_H

#include "vfs.h"

typedef struct _Browser_Selected_File_Data Browser_Selected_File_Data;

struct _Browser_Selected_File_Data
{
    Enna_Class_Vfs *vfs;
    Enna_Vfs_File *file;
    Eina_List *files;
};

typedef enum _Enna_Browser_View_Type Enna_Browser_View_Type;

enum _Enna_Browser_View_Type
{
    ENNA_BROWSER_VIEW_LIST,
    ENNA_BROWSER_VIEW_COVER,
    ENNA_BROWSER_VIEW_WALL
};

Evas_Object    *enna_browser_add(Evas * evas);
void            enna_browser_view_add(Evas_Object *obj, Enna_Browser_View_Type view_type);
void            enna_browser_root_set(Evas_Object *obj, Enna_Class_Vfs *vfs);
void            enna_browser_show_file_set(Evas_Object *obj, unsigned char show);
void            enna_browser_event_feed(Evas_Object *obj, void *event_info);
int             enna_browser_select_label(Evas_Object *obj, const char *label);
#endif /* BROWSER_H */
