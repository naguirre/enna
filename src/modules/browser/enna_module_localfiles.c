/* Interface */

#include "enna.h"

static void           _class_init(int dummy);
static void           _class_shutdown(int dummy);
static Evas_List     *_class_browse_up(const char *path);
static Evas_List     *_class_browse_down();

static int            em_init(Enna_Module *em);
static int            em_shutdown(Enna_Module *em);

typedef struct _Enna_Module_Music Enna_Module_Music;

struct _Enna_Module_Music
{
   Evas *e;
   Evas_Object *o_edje;
   Evas_Object *o_scroll;
   Evas_Object *o_list;
   Enna_Module *em;
   const char *uri;
   const char *prev_uri;
};

static Enna_Module_Music *mod;

EAPI Enna_Module_Api module_api =
{
    ENNA_MODULE_VERSION,
    "localfiles"
};

static Enna_Class_Vfs class =
{
    "localfiles",
    1,
    "Browse Local Files",
    NULL,
    "icon/dd",
    {
        _class_init,
        _class_shutdown,
	_class_browse_up,
	_class_browse_down
    },
};


static Evas_List *_class_browse_up(const char *path)
{
   printf("Local files Browse\n");

   /* Browse Root */
   if (!path)
     {
	Evas_List *files = NULL;
	Evas_List *l;
	/* FIXME: this list should come from config ! */
	for (l = enna_config->music_local_root_directories; l; l = l->next)
	  {
	     Enna_Vfs_File *file;
	     Enna_Config_Root_Directories *root;

	     root = l->data;
	     file = calloc(1, sizeof(Enna_Vfs_File));
	     file->uri = root->uri;
	     file->label = root->label;
	     file->icon_file = NULL;
	     file->is_directory = 1;
	     file->icon = evas_stringshare_add("icon/hd");
	     files = evas_list_append(files, file);
	  }
	evas_stringshare_del(mod->prev_uri);
	evas_stringshare_del(mod->uri);
	mod->prev_uri = NULL;
	mod->uri = NULL;
	printf("browse_up : prev uri : %s; uri : %s\n", mod->prev_uri, mod->uri);
	return files;
     }
   else if (strstr(path, "file://"))
     {
	Ecore_List *files = NULL;
	char *filename = NULL;
	Evas_List *files_list = NULL;
	Evas_List *dirs_list = NULL;
	Evas_List *l;
	char dir[PATH_MAX];

	files = ecore_file_ls(path+7);
	ecore_list_sort(files, ECORE_COMPARE_CB(strcasecmp), ECORE_SORT_MIN);
	filename = ecore_list_first_goto(files);

	while ((filename = (char *)ecore_list_next(files)) != NULL)
	  {
	     sprintf(dir, "%s/%s", path, filename);
	     if (filename[0] == '.')
	       continue;
	     else if (ecore_file_is_dir(dir+7))
	       {
		  Enna_Vfs_File  *f;

		  f = calloc(1, sizeof(Enna_Vfs_File));
		  f->uri = strdup(dir);
		  f->label = strdup(filename);
		  f->is_directory = 1;
		  f->icon = evas_stringshare_add("icon/directory");
		  dirs_list = evas_list_append(dirs_list, f);
		  if (mod->prev_uri)
		    {
		       if (!strcmp(dir, mod->prev_uri))
			 f->is_selected = 1;
		       else
			 f->is_selected = 0;
		    }
	       }
	     /* FIXME : filters should come from config */
	     else
	       {
		  Enna_Vfs_File  *f;
		  f = calloc(1, sizeof(Enna_Vfs_File));
		  f->uri  = strdup(dir);
		  f->label = strdup(filename);
		  f->icon = evas_stringshare_add("icon/music");
		  f->is_directory = 0;
		  files_list = evas_list_append(files_list, f);
	       }
	  }
	/* File after dir */
	for (l = files_list; l; l = l->next)
	  {
	     dirs_list = evas_list_append(dirs_list, l->data);
	  }
	evas_stringshare_del(mod->prev_uri);
	mod->prev_uri = mod->uri;
	mod->uri = evas_stringshare_add(path);
	printf("browse_up : prev uri : %s; uri : %s\n", mod->prev_uri, mod->uri);
	return dirs_list;
     }

   return NULL;

}

static unsigned char _uri_is_root(const char *uri)
{
   Evas_List *l;

   for (l = enna_config->music_local_root_directories; l; l = l->next)
     {
	Enna_Config_Root_Directories *root = l->data;
	printf("is root : %s %s\n", root->uri, uri);
	if (!strcmp(root->uri, uri))
	  {
	     return 1;
	  }
     }
   return 0;
}

static Evas_List *_class_browse_down()
{

   /* Browse Root */
   if (mod->uri && strstr(mod->uri, "file://"))
     {
	char *path_tmp = NULL;
	char *p;
	Evas_List *files = NULL;

	if (_uri_is_root(mod->uri))
	  {
	     Evas_List *files = NULL;
	     Evas_List *l;
	     /* FIXME: this list should come from config ! */
	     for (l = enna_config->music_local_root_directories; l; l = l->next)
	       {
		  Enna_Vfs_File *file;
		  Enna_Config_Root_Directories *root;

		  root = l->data;
		  file = calloc(1, sizeof(Enna_Vfs_File));
		  file->uri = root->uri;
		  file->label = root->label;
		  file->icon_file = NULL;
		  file->is_directory = 1;
		  file->icon = evas_stringshare_add("icon/hd");
		  files = evas_list_append(files, file);
	       }
	     evas_stringshare_del(mod->prev_uri);
	     evas_stringshare_del(mod->uri);
	     mod->prev_uri = NULL;
	     mod->uri = NULL;
	     printf("browse_down : prev uri : %s; uri : %s\n", mod->prev_uri, mod->uri);
	     return files;
	  }

	path_tmp = strdup(mod->uri);
	if (path_tmp[strlen(mod->uri) - 1] == '/')
	  path_tmp[strlen(mod->uri) - 1] = 0;
	p = strrchr(path_tmp, '/');
	if (p && *(p - 1) == '/')
	  *(p) = 0;
	else if (p)
	  *(p) = 0;

	files = _class_browse_up(path_tmp);

	//if (mod->prev_uri) evas_stringshare_del(mod->prev_uri);
	//if(mod->uri) evas_stringshare_del(mod->uri);
	mod->uri = evas_stringshare_add(path_tmp);
	printf("browse_down : prev uri : %s; uri : %s\n", mod->prev_uri, mod->uri);
	return files;
     }

   return NULL;

}

static void _class_init(int dummy)
{
}

static void _class_shutdown(int dummy)
{
}

/* Module interface */

static int
em_init(Enna_Module *em)
{
    mod = calloc(1, sizeof(Enna_Module_Music));
    mod->em = em;
    em->mod = mod;
    mod->prev_uri = NULL;
    enna_vfs_append("localfiles", ENNA_CAPS_MUSIC | ENNA_CAPS_VIDEO | ENNA_CAPS_PHOTO, &class);

    return 1;
}


static int
em_shutdown(Enna_Module *em)
{

    Enna_Module_Music *mod;

    mod = em->mod;;
    evas_object_del(mod->o_edje);
    evas_object_del(mod->o_scroll);
    evas_object_del(mod->o_list);
    return 1;
}

EAPI void
module_init(Enna_Module *em)
{
    if (!em)
        return;

    if (!em_init(em))
        return;
}

EAPI void
module_shutdown(Enna_Module *em)
{
    em_shutdown(em);
}
