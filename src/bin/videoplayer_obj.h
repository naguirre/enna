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

#ifndef VIDEOPLAYER_OBJ_H
#define VIDEOPLAYER_OBJ_H

#include <Elementary.h>
#include "file.h"

Evas_Object *enna_view_player_video_add(Evas_Object *parent);
void enna_view_player_video_uri_set(Evas_Object *o, Enna_File *f);
void enna_videoplayer_obj_cfg_register(void);
void enna_view_player_video_play(Evas_Object *o);

#endif
