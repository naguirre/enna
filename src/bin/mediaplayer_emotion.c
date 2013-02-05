#include <Emotion.h>
#include "utils.h"
#include "logs.h"
#include "mediaplayer.h"
#include "enna_config.h"

#define SEEK_STEP_DEFAULT         10 /* seconds */
#define VOLUME_STEP_DEFAULT       5 /* percent */

typedef struct _Enna_Mediaplayer Enna_Mediaplayer;

struct _Enna_Mediaplayer
{
    PLAY_STATE play_state;
    Evas_Object *player;
    void (*event_cb)(void *data, enna_mediaplayer_event_t event);
    void *event_cb_data;
    char *uri;
    char *label;
    int audio_delay;
    char *engine;
    int subtitle_visibility;
    int subtitle_alignment;
    int subtitle_position;
    int subtitle_scale;
    int subtitle_delay;
    int framedrop;
    Enna_Playlist *cur_playlist;
};

typedef struct mediaplayer_cfg_s {
    char *engine;
} mediaplayer_cfg_t;

static mediaplayer_cfg_t mp_cfg;
static Enna_Mediaplayer *mp = NULL;


static const struct {
    const char *name;
} map_player_type[] = {
    { "generic"                                  },
    { "gstreamer"                            },
    { "xine"                                 },
    { NULL                                   }
};



static void
cfg_mediaplayer_section_load (const char *section)
{
    const char *value = NULL;
    int i;

    enna_log(ENNA_MSG_INFO, NULL, "parameters:");

    value = enna_config_string_get(section, "type");
    if (value)
    {
        enna_log(ENNA_MSG_INFO, NULL, " * type: %s", value);

        for (i = 0; map_player_type[i].name; i++)
        {
            if (!strcmp(value, map_player_type[i].name))
            {
                mp_cfg.engine = strdup(map_player_type[i].name);
                break;
            }
        }
    }
}

static void
cfg_mediaplayer_section_save (const char *section)
{
    int i;

    /* Default type */
    for (i = 0; map_player_type[i].name; i++)
    {
        if (!strcmp(mp_cfg.engine ,map_player_type[i].name))
        {
            enna_config_string_set(section, "type",
                                   map_player_type[i].name);
            break;
        }
    }
}

static void
cfg_mediaplayer_free (void)
{
    free(mp_cfg.engine);
}

static void
cfg_mediaplayer_section_set_default (void)
{
    cfg_mediaplayer_free();

    mp_cfg.engine           = strdup("xine");
}

static Enna_Config_Section_Parser cfg_mediaplayer = {
    "mediaplayer",
    cfg_mediaplayer_section_load,
    cfg_mediaplayer_section_save,
    cfg_mediaplayer_section_set_default,
    cfg_mediaplayer_free,
};

/* externally accessible functions */
int
enna_mediaplayer_supported_uri_type(enna_mediaplayer_uri_type_t type EINA_UNUSED)
{
    return 1;
}


void
enna_mediaplayer_cfg_register (void)
{
    enna_config_section_parser_register(&cfg_mediaplayer);
}

int
enna_mediaplayer_init(void)
{
    mp = calloc(1, sizeof(Enna_Mediaplayer));

    mp->uri = NULL;
    mp->label = NULL;

    mp->engine = strdup(mp_cfg.engine);
    mp->player = emotion_object_add(evas_object_evas_get(enna->layout));
    emotion_object_init(mp->player, mp->engine);
    evas_object_layer_set(mp->player, -1);
    mp->play_state = STOPPED;

    /* Create Ecore Event ID */
    ENNA_EVENT_MEDIAPLAYER_EOS = ecore_event_type_new();
    ENNA_EVENT_MEDIAPLAYER_METADATA_UPDATE = ecore_event_type_new();
    ENNA_EVENT_MEDIAPLAYER_START = ecore_event_type_new();
    ENNA_EVENT_MEDIAPLAYER_STOP = ecore_event_type_new();
    ENNA_EVENT_MEDIAPLAYER_PAUSE = ecore_event_type_new();
    ENNA_EVENT_MEDIAPLAYER_UNPAUSE = ecore_event_type_new();
    ENNA_EVENT_MEDIAPLAYER_PREV = ecore_event_type_new();
    ENNA_EVENT_MEDIAPLAYER_NEXT = ecore_event_type_new();
    ENNA_EVENT_MEDIAPLAYER_SEEK = ecore_event_type_new();

    return 1;
}

void
enna_mediaplayer_shutdown(void)
{
    if (!mp)
        return;

    ENNA_FREE(mp->uri);
    ENNA_FREE(mp->label);
    ENNA_FREE(mp->engine);
    emotion_object_play_set(mp->player, EINA_FALSE);
    if (mp->player)
        evas_object_del(mp->player);
    ENNA_FREE(mp);
}

Evas_Object *enna_mediaplayer_obj_get(void)
{
    return mp->player;
}

Enna_File *
enna_mediaplayer_current_file_get(void)
{
    Enna_File *item;

    if (!mp->cur_playlist || mp->play_state != PLAYING)
        return NULL;

    item = eina_list_nth(mp->cur_playlist->playlist, mp->cur_playlist->selected);
    if (!item)
        return NULL;

    return item;
}

const char *
enna_mediaplayer_get_current_uri(void)
{
  Enna_File *item;

  if (!mp->cur_playlist || mp->play_state != PLAYING)
      return NULL;

  item = eina_list_nth(mp->cur_playlist->playlist, mp->cur_playlist->selected);
  if (!item->uri)
    return NULL;

  return strdup(item->uri);
}

void
enna_mediaplayer_file_append(Enna_Playlist *enna_playlist, Enna_File *file)
{
    Enna_File *f;
    f = enna_file_ref(file);
    enna_playlist->playlist = eina_list_append(enna_playlist->playlist, f);
}

int
enna_mediaplayer_play(Enna_Playlist *enna_playlist)
{
    mp->cur_playlist = enna_playlist;

    switch (mp->play_state)
    {
    case STOPPED:
    {
      Enna_File *item;
        item = eina_list_nth(enna_playlist->playlist,
                             enna_playlist->selected);
        emotion_object_play_set(mp->player, EINA_FALSE);
        if (item && item->uri)
            emotion_object_file_set(mp->player, item->mrl);
        emotion_object_play_set(mp->player, EINA_TRUE);
        if (item && item->type == ENNA_FILE_FILM)
        {
            evas_object_show(mp->player);
            evas_object_hide(enna->layout);
        }
        mp->play_state = PLAYING;
        ecore_event_add(ENNA_EVENT_MEDIAPLAYER_START, NULL, NULL, NULL);
        break;
    }
    case PLAYING:
        enna_mediaplayer_pause();
        break;
    case PAUSE:
        emotion_object_play_set(mp->player, EINA_TRUE);
        mp->play_state = PLAYING;
        ecore_event_add(ENNA_EVENT_MEDIAPLAYER_UNPAUSE,
                        NULL, NULL, NULL);
        break;
    default:
        break;
    }

    return 0;
}

int
enna_mediaplayer_select_nth(Enna_Playlist *enna_playlist,int n)
{
    if (n < 0 || n > eina_list_count(enna_playlist->playlist) - 1)
        return -1;

    enna_log(ENNA_MSG_EVENT, NULL, "select %d", n);
    enna_playlist->selected = n;

    return 0;
}

int
enna_mediaplayer_selected_get(Enna_Playlist *enna_playlist)
{
    return enna_playlist->selected;
}

int
enna_mediaplayer_stop(void)
{
    emotion_object_play_set(mp->player, EINA_FALSE);
    emotion_object_position_set(mp->player, 0);
    mp->play_state = STOPPED;
    evas_object_hide(mp->player);
    evas_object_show(enna->layout);
    ecore_event_add(ENNA_EVENT_MEDIAPLAYER_STOP, NULL, NULL, NULL);

    return 0;
}

int
enna_mediaplayer_pause(void)
{
    emotion_object_play_set(mp->player, EINA_FALSE);
    mp->play_state = PAUSE;
    ecore_event_add(ENNA_EVENT_MEDIAPLAYER_PAUSE, NULL, NULL, NULL);

    return 0;
}

static void
enna_mediaplayer_change(Enna_Playlist *enna_playlist, int type)
{
    Enna_File *item;

    item = eina_list_nth(enna_playlist->playlist, enna_playlist->selected);
    enna_log(ENNA_MSG_EVENT, NULL, "select %d", enna_playlist->selected);
    if (!item)
        return;

    enna_mediaplayer_stop();
    enna_mediaplayer_play(enna_playlist);
    ecore_event_add(type, NULL, NULL, NULL);
}

int
enna_mediaplayer_next(Enna_Playlist *enna_playlist)
{
    enna_playlist->selected++;
    if(enna_playlist->selected >
       eina_list_count(enna_playlist->playlist) - 1)
    {
        enna_playlist->selected--;
        return -1;
    }

    enna_mediaplayer_change(enna_playlist, ENNA_EVENT_MEDIAPLAYER_NEXT);
    return 0;
}

int
enna_mediaplayer_prev(Enna_Playlist *enna_playlist)
{
    enna_playlist->selected--;
    if (enna_playlist->selected < 0)
    {
        enna_playlist->selected = 0;
        return -1;
    }

    enna_mediaplayer_change(enna_playlist, ENNA_EVENT_MEDIAPLAYER_PREV);
    return 0;
}

double
enna_mediaplayer_position_get(void)
{
    return emotion_object_position_get(mp->player);
}

int
enna_mediaplayer_position_percent_get(void)
{
    return (int) (emotion_object_progress_status_get(mp->player)) * 100;
}

double
enna_mediaplayer_length_get(void)
{
    return (mp->play_state == PAUSE || mp->play_state == PLAYING) ?
        emotion_object_play_length_get(mp->player) : 0.0;
}

static void
enna_mediaplayer_seek(int value , SEEK_TYPE type)
{
    double new_time, old_time, length;

    enna_log(ENNA_MSG_EVENT, NULL, "Seeking to: %d%c",
             value, type == SEEK_ABS_PERCENT ? '%' : 's');

    if (mp->play_state == PAUSE || mp->play_state == PLAYING)
    {
        Enna_Event_Mediaplayer_Seek_Data *ev;

        ev = calloc(1, sizeof(Enna_Event_Mediaplayer_Seek_Data));
        if (!ev)
            return;

        ev->seek_value = value;
        ev->type       = type;

        if(emotion_object_seekable_get(mp->player))
        {
            if(type == SEEK_ABS_PERCENT)
            {
                length = enna_mediaplayer_length_get();
                new_time = (value * length) / 100.0;
            }
            else if(type == SEEK_ABS_SECONDS)
                new_time = (double)value;
            else if(type == SEEK_REL_SECONDS)
            {
                old_time = enna_mediaplayer_position_get();
                new_time = (double)(value) + old_time;
            }
            else
                new_time = 0;
        ecore_event_add(ENNA_EVENT_MEDIAPLAYER_SEEK, ev, NULL, NULL);
            emotion_object_position_set(mp->player, new_time);
        }
        else
        {
            enna_log(ENNA_MSG_EVENT, NULL, "No Seeking avaible");
        }
    }
}

void
enna_mediaplayer_position_set(int seconds)
{
    enna_mediaplayer_seek(seconds, SEEK_ABS_SECONDS);
}

void
enna_mediaplayer_seek_percent(int percent)
{
    enna_mediaplayer_seek(percent, SEEK_ABS_PERCENT);
}

void
enna_mediaplayer_seek_relative(int seconds)
{
    enna_mediaplayer_seek(seconds, SEEK_REL_SECONDS);
}

void
enna_mediaplayer_default_seek_backward(void)
{
    enna_mediaplayer_seek_relative(-SEEK_STEP_DEFAULT);
}

void
enna_mediaplayer_default_seek_forward(void)
{
    enna_mediaplayer_seek_relative(SEEK_STEP_DEFAULT);
}

void
enna_mediaplayer_video_resize(int x, int y, int w, int h)
{
    printf("Resize %d %d %d %d\n", x, y, w, h);
    evas_object_resize(mp->player, w, h);
    evas_object_move(mp->player, x, y);
}

int
enna_mediaplayer_playlist_load(const char *filename EINA_UNUSED)
{
    return 0;
}

int
enna_mediaplayer_playlist_save(const char *filename EINA_UNUSED)
{
    return 0;
}



void
enna_mediaplayer_playlist_clear(Enna_Playlist *enna_playlist)
{
    Enna_File *f;

    EINA_LIST_FREE(enna_playlist->playlist, f)
        enna_file_free(f);
    enna_playlist->playlist = NULL;
    enna_playlist->selected = 0;
}

Enna_Metadata *
enna_mediaplayer_metadata_get(Enna_Playlist *enna_playlist)
{
    Enna_File *item;

    item = eina_list_nth(enna_playlist->playlist, enna_playlist->selected);
    if (!item)
        return NULL;

    if (item->uri)
        return enna_metadata_meta_new((char *) item->mrl);

    return NULL;
}

int
enna_mediaplayer_playlist_count(Enna_Playlist *enna_playlist)
{
    return eina_list_count(enna_playlist->playlist);
}

PLAY_STATE
enna_mediaplayer_state_get(void)
{
    return mp->play_state;
}

Enna_Playlist *
enna_mediaplayer_playlist_create(void)
{
    Enna_Playlist *enna_playlist;

    enna_playlist = calloc(1, sizeof(Enna_Playlist));
    enna_playlist->selected = 0;
    enna_playlist->playlist = NULL;
    return enna_playlist;
}

void
enna_mediaplayer_playlist_free(Enna_Playlist *enna_playlist)
{
    eina_list_free(enna_playlist->playlist);
    free(enna_playlist);
}

void
enna_mediaplayer_playlist_stop_clear(Enna_Playlist *enna_playlist)
{
    enna_mediaplayer_playlist_clear(enna_playlist);
    emotion_object_play_set(mp->player, EINA_FALSE);
    emotion_object_position_set(mp->player, 0);
    mp->play_state = STOPPED;
    ecore_event_add(ENNA_EVENT_MEDIAPLAYER_STOP, NULL, NULL, NULL);
}

void
enna_mediaplayer_send_input(enna_input event EINA_UNUSED)
{
}

int
enna_mediaplayer_volume_get(void)
{
    return (mp && mp->player) ? emotion_object_audio_volume_get(mp->player) * 100 : 0;
}

void
enna_mediaplayer_volume_set(int value)
{
    emotion_object_audio_volume_set(mp->player, value / 100.0);
}

void
enna_mediaplayer_default_increase_volume(void)
{
    int vol = enna_mediaplayer_volume_get();
    vol = MMIN(vol + VOLUME_STEP_DEFAULT, 100);
    emotion_object_audio_volume_set(mp->player, vol);
}

void
enna_mediaplayer_default_decrease_volume(void)
{
    int vol = enna_mediaplayer_volume_get();
    vol = MMAX(vol - VOLUME_STEP_DEFAULT, 0);
    emotion_object_audio_volume_set(mp->player, vol);
}

void
enna_mediaplayer_mute(void)
{
    Eina_Bool m;

    m = emotion_object_audio_mute_get(mp->player);
    emotion_object_audio_mute_set(mp->player, m ?
                          EINA_FALSE : EINA_TRUE);
}

int
enna_mediaplayer_mute_get(void)
{
    Eina_Bool m;

    if (!mp)
      return 0;

    m = emotion_object_audio_mute_get(mp->player);
    
    return m;
}

void
enna_mediaplayer_audio_previous(void)
{

}

void
enna_mediaplayer_audio_next(void)
{

}

void
enna_mediaplayer_audio_increase_delay(void)
{

}

void
enna_mediaplayer_audio_decrease_delay(void)
{

}

void
enna_mediaplayer_subtitle_set_visibility(void)
{

}

void
enna_mediaplayer_subtitle_previous(void)
{

}

void
enna_mediaplayer_subtitle_next(void)
{

}

void
enna_mediaplayer_subtitle_set_alignment(void)
{
    
}

void
enna_mediaplayer_subtitle_increase_position(void)
{
    
}

void
enna_mediaplayer_subtitle_decrease_position(void)
{
    
}

void
enna_mediaplayer_subtitle_increase_scale(void)
{
    
}

void
enna_mediaplayer_subtitle_decrease_scale(void)
{
    
}

void
enna_mediaplayer_subtitle_increase_delay(void)
{
    /* Nothing to do for emotion */
}

void
enna_mediaplayer_subtitle_decrease_delay(void)
{
    /* Nothing to do for emotion */
}

void
enna_mediaplayer_set_framedrop(void)
{
    /* Nothing to do for emotion */
}
