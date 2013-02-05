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

#include <string.h>

#include <Ecore_Con.h>

#include "enna.h"
#include "enna_config.h"
#include "url_utils.h"
#include "utils.h"
#include "logs.h"
#include "geoip.h"

#define ENNA_MODULE_NAME      "geoip"

#define GEOIP_QUERY           "http://geoip.ubuntu.com/lookup"
#define MAX_URL_SIZE          1024

int ENNA_EVENT_GEO_LOC_DETECTED;

typedef enum _TAG_ID
{
    TAG_ID_IP,
    TAG_ID_COUNTRYCODE,
    TAG_ID_CITY,
    TAG_ID_LONGITUDE,
    TAG_ID_LATITUDE,
    TAG_ID_SENTINEL,
}TAG_ID;

static struct {
    TAG_ID id;
    const char *tag;
    Eina_Bool found;
    char *value;
} xml_struct[] = {
    { TAG_ID_IP, "Ip", EINA_FALSE, NULL},
    { TAG_ID_COUNTRYCODE, "CountryCode", EINA_FALSE, NULL},
    { TAG_ID_CITY, "City", EINA_FALSE, NULL},
    { TAG_ID_LONGITUDE, "Latitude", EINA_FALSE, NULL},
    { TAG_ID_LATITUDE, "Longitude", EINA_FALSE, NULL},
    { TAG_ID_SENTINEL, NULL, 0},
};
    

static Eina_Bool
_xml_tag_cb(void *data, Eina_Simple_XML_Type type, const char *content,
            unsigned offset, unsigned length)
{
   Eina_Array *array = data;
   char str[512];
   int i;

   if (type == EINA_SIMPLE_XML_OPEN)
     {
         for (i = 0; xml_struct[i].tag; i++)
         {
             if (!strncmp(xml_struct[i].tag, content, strlen(xml_struct[i].tag)))
             {
                 xml_struct[i].found = EINA_TRUE;
                 break;
             }
         }
     }
   else if (type == EINA_SIMPLE_XML_DATA)
     {
         for (i = 0; xml_struct[i].tag; i++)
         {
             if (xml_struct[i].found)
             {
                 char buffer[length+1];
                 snprintf(buffer, sizeof(buffer), "%s", content);
                 xml_struct[i].value = strdup(buffer);
                 xml_struct[i].found = EINA_FALSE;
                 break;
             }
         }
     }

   return EINA_TRUE;
}


static Eina_Bool
url_data_cb(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *ev) 
{ 
    Ecore_Con_Event_Url_Data *urldata = ev;
    Geo *geo = NULL;
    int i;

    ecore_con_url_data_get(urldata->url_con); 

    if (!urldata->size)
        return 0;

    enna_log(ENNA_MSG_EVENT, ENNA_MODULE_NAME,
             "Search Reply: %s", urldata->data);

    geo = calloc(1, sizeof(Geo));

    /* parse the XML answer */
    eina_simple_xml_parse(urldata->data, urldata->size, EINA_TRUE, _xml_tag_cb, geo);
    
    for (i = 0; xml_struct[i].tag; i++)
    {
        switch (xml_struct[i].id)
        {
        case TAG_ID_COUNTRYCODE:
            geo->country = xml_struct[TAG_ID_COUNTRYCODE].value;
            break;
        case TAG_ID_CITY :
            geo->city = xml_struct[TAG_ID_CITY].value;
            break;
        case TAG_ID_LONGITUDE:
            geo->longitude = enna_util_atof(xml_struct[TAG_ID_LONGITUDE].value);
            break;
        case TAG_ID_LATITUDE:
            geo->latitude = enna_util_atof(xml_struct[TAG_ID_LATITUDE].value);
            break;
        default:
            break;
        }
    }
     
    if (geo->city)
    {
        char res[256];
        if (geo->country)
            snprintf(res, sizeof(res), "%s, %s", geo->city, geo->country);
        else
            snprintf(res, sizeof(res), "%s", geo->city);
        geo->geo = strdup(res);

        enna_log(ENNA_MSG_INFO, ENNA_MODULE_NAME,
                 "Geolocalized in: %s (%f ; %f).", geo->geo, geo->latitude, geo->longitude);
    }

    enna->geo_loc = geo;
    ecore_event_add(ENNA_EVENT_GEO_LOC_DETECTED, NULL, NULL, NULL);
    
    return 1; 
} 


void
enna_get_geo_by_ip (void)
{
    Ecore_Con_Url *url;
    Ecore_Event_Handler *handler;

    ENNA_EVENT_GEO_LOC_DETECTED = ecore_event_type_new();
    ecore_con_url_init();
    url = ecore_con_url_new(GEOIP_QUERY);
    handler = ecore_event_handler_add(ECORE_CON_EVENT_URL_DATA, 
                                      url_data_cb, NULL); 

    /* proceed with IP Geolocalisation request */
    ecore_con_url_get(url);

    enna_log(ENNA_MSG_EVENT, ENNA_MODULE_NAME,
             "Search Request: %s", GEOIP_QUERY);

  

    return;
}

void
enna_geo_free (Geo *geo)
{
    if (!geo)
        return;

    if (geo->city)
        ENNA_FREE (geo->city);

    if (geo->country)
        ENNA_FREE (geo->country);

    if (geo->geo)
        ENNA_FREE (geo->geo); 

    ENNA_FREE (geo);
}
