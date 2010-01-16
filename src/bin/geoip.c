/*
 * GeeXboX Enna Media Center.
 * Copyright (C) 2005-2009 The Enna Project
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

#include "enna.h"
#include "enna_config.h"
#include "xml_utils.h"
#include "url_utils.h"
#include "logs.h"
//#include "geoip.h"

#define ENNA_MODULE_NAME      "geoip"

#define GEOIP_QUERY           "http://www.ipinfodb.com/ip_query.php"
#define MAX_URL_SIZE          1024

char *
enna_get_city_by_ip (void)
{
    url_data_t data;
    url_t handler;
    xmlDocPtr doc = NULL;
    xmlNode *n;
    xmlChar *tmp;
    char *city = NULL;
    char *country = NULL;
    char *geo = NULL;

    handler = url_new();
    if (!handler)
        goto error;

    /* proceed with IP Geolocalisation request */
    enna_log(ENNA_MSG_EVENT, ENNA_MODULE_NAME,
             "Search Request: %s", GEOIP_QUERY);

    data = url_get_data(handler, GEOIP_QUERY);
    if (data.status != 0)
        goto error;

    enna_log(ENNA_MSG_EVENT, ENNA_MODULE_NAME,
             "Search Reply: %s", data.buffer);

    /* parse the XML answer */
    doc = get_xml_doc_from_memory(data.buffer);
    ENNA_FREE(data.buffer);
    if (!doc)
        goto error;

    n = xmlDocGetRootElement(doc);

    /* check for existing city */
    tmp = get_prop_value_from_xml_tree(n, "Status");
    if (!tmp || xmlStrcmp(tmp, (unsigned char *) "OK"))
    {
        enna_log(ENNA_MSG_WARNING, ENNA_MODULE_NAME,
                 "Error returned by website.");
        goto error;
    }

    tmp = get_prop_value_from_xml_tree(n, "CountryCode");
    if (tmp)
        country = strdup((char *) tmp);

    tmp = get_prop_value_from_xml_tree(n, "City");
    if (tmp)
        city = strdup((char *) tmp);

    if (city)
    {
        char res[256];
        if (country)
            snprintf(res, sizeof(res), "%s, %s", city, country);
        else
            snprintf(res, sizeof(res), "%s", city);
        geo = strdup(res);

        enna_log(ENNA_MSG_INFO, ENNA_MODULE_NAME,
                 "Geolocalized in: %s.", geo);
    }

error:
    if (doc)
    {
        xmlFreeDoc(doc);
        doc = NULL;
    }

    url_free(handler);
    ENNA_FREE(country);
    ENNA_FREE(city);

    return geo;
}