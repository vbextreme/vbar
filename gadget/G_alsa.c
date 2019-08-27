#include <vbar.h>
#include <alsa/asoundlib.h>
#include <alsa/mixer.h>

//TODO unregister deadpoll in galsa_disable_callback

__private size_t TYPE = 0;

typedef struct galsa{
	char* card;
	char* channel;
	snd_mixer_t* hmixer;
    //snd_mixer_selem_id_t* msid;
	//snd_mixer_elem_t* mele;
	snd_ctl_t* mctl;
	unsigned int ctlfdcount;
	long min;
	long max;
}galsa_s;

__private err_t galsa_mixer_begin(galsa_s* a){
	 if( snd_mixer_open(&a->hmixer, 0) < 0){
		dbg_error("mixer open");
		return -1;
	}
    if( snd_mixer_attach(a->hmixer, a->card) < 0){
		dbg_error("mixer attach");
		goto ONERROR;
	}
    if( snd_mixer_selem_register(a->hmixer, NULL, NULL) < 0 ){
		dbg_error("mixer selem register");
		goto ONERROR;
	}
    if( snd_mixer_load(a->hmixer) < 0 ){
		dbg_error("mixer load");
		goto ONERROR;
	}
	return 0;
ONERROR:
	snd_mixer_close(a->hmixer);
	a->hmixer = NULL;
	return -1;
}

__private void galsa_mixer_end(galsa_s* a){
	snd_mixer_close(a->hmixer);
	a->hmixer = NULL;
}

__private void galsa_mixer_max_min(galsa_s* a){
	snd_mixer_selem_id_t* sme;
	snd_mixer_selem_id_alloca(&sme);
    snd_mixer_selem_id_set_index(sme, 0);
    snd_mixer_selem_id_set_name(sme, a->channel);
    
    snd_mixer_elem_t* mele = snd_mixer_find_selem(a->hmixer, sme);
	if( mele == NULL ){
		dbg_error("mixer find elem");
		return;
	}
    if( snd_mixer_selem_get_playback_volume_range(mele, &a->min, &a->max) < 0 ){
		dbg_error("mixer get volume range");
		return;
	}
	dbg_info("volume min:%ld max:%ld", a->min, a->max);
}

__private long galsa_mixer_volume_get(galsa_s* a){
	snd_mixer_selem_id_t* sme;
	snd_mixer_selem_id_alloca(&sme);
    snd_mixer_selem_id_set_index(sme, 0);
    snd_mixer_selem_id_set_name(sme, a->channel);
    
    snd_mixer_elem_t* mele = snd_mixer_find_selem(a->hmixer, sme);
	if( mele == NULL ){
		dbg_error("mixer find elem");
		return 0;
	}
	long ret = 0;
	snd_mixer_selem_get_playback_volume(mele, 0, &ret);
	dbg_info("get volume:%ld", ret);
	return ret;
}

__private void galsa_mixer_volume_set(galsa_s* a, long volume){
	snd_mixer_selem_id_t* sme;
	snd_mixer_selem_id_alloca(&sme);
    snd_mixer_selem_id_set_index(sme, 0);
    snd_mixer_selem_id_set_name(sme, a->channel);
    
    snd_mixer_elem_t* mele = snd_mixer_find_selem(a->hmixer, sme);
	if( mele == NULL ){
		dbg_error("mixer find elem");
		return;
	}
	dbg_info("set volume:%ld", volume);
	snd_mixer_selem_set_playback_volume_all(mele, volume);
}

__private err_t galsa_callback(__unused int ev, void* data);
__private err_t galsa_enable_callback(gadget_s* g, galsa_s* a){
	if( snd_ctl_open(&a->mctl, a->card, SND_CTL_READONLY) < 0 ){//SND_CTL_NONBLOCK) < 0 ){
		dbg_error("ctl open");
		return -1;
	}

	if( snd_ctl_subscribe_events(a->mctl, 1) < 0 ){
		dbg_error("subscribe_events");
		goto CTLERROR;
	}

	a->ctlfdcount = snd_ctl_poll_descriptors_count(a->mctl);
	if( a->ctlfdcount <= 0 ){
		dbg_error("get poll descriptor count");
		goto CTLERROR;
	}
	dbg_info("fd count %d", a->ctlfdcount);

	struct pollfd *pfds = calloc(a->ctlfdcount, sizeof(*pfds));
	iassert(pfds);
 
    int count = snd_ctl_poll_descriptors(a->mctl, pfds, a->ctlfdcount);
    if( count < 0 || count != (int)a->ctlfdcount ){
		dbg_error("get poll descriptos");
		free(pfds);
		goto CTLERROR;
    }
 
	for( unsigned i = 0; i < a->ctlfdcount; ++i){
		deadpoll_register(&g->vbar->events, pfds[i].fd, galsa_callback, g, 0);
    }

	free(pfds);

	return 0;

CTLERROR:
	snd_ctl_close(a->mctl);
	return -1;
}

__private void galsa_disable_callback(__unused gadget_s* g, galsa_s* a){
	snd_ctl_subscribe_events(a->mctl, 0);
	snd_ctl_close(a->mctl);
	a->mctl = NULL;
}

__private err_t galsa_callback(__unused int ev, void* data){
	gadget_s* g = data;
	galsa_s* a = g->data;
	dbg_info("callback alsa");
	snd_ctl_event_t *event;
	snd_ctl_event_alloca(&event);
	if( snd_ctl_read(a->mctl, event) < 0 ){
		dbg_error("ctl read");
		return 0;
	}	
	if( snd_ctl_event_get_type(event) != SND_CTL_EVENT_ELEM ){
		return 0;
	}

	//int numid = snd_ctl_event_elem_get_numid(event);
	//int interface = snd_ctl_event_elem_get_interface(event);
	//int device = snd_ctl_event_elem_get_device(event);
	//int subdevice = snd_ctl_event_elem_get_subdevice(event);
	const char* name = snd_ctl_event_elem_get_name(event);
	//int index = snd_ctl_event_elem_get_index(event);
 
	unsigned int mask = snd_ctl_event_elem_get_mask(event);
	if( mask == SND_CTL_EVENT_MASK_REMOVE ){
		dbg_info("alsa callback name:'%s' event:remove", name);
	}
	if( mask & SND_CTL_EVENT_MASK_VALUE ){
		dbg_info("alsa callback name:'%s' event:value", name);
		if( !strcmp(name,"Master Playback Volume") ){
			if( g->event ){
				vbar_label_reset(g);
				g->event(g, GADGET_EVENT_REFRESH, NULL);
			}
		}
	}
	if (mask & SND_CTL_EVENT_MASK_INFO){
		dbg_info("alsa callback name:'%s' event:info", name);
	}
	if (mask & SND_CTL_EVENT_MASK_ADD){
		dbg_info("alsa callback name:'%s' event:add", name);
	}
	if (mask & SND_CTL_EVENT_MASK_TLV){
		dbg_info("alsa callback name:'%s' event:tlv", name);
	}
	return 0;
}

__private void galsa_card_set(gadget_s* g, char* name){
	if( g->type != TYPE ) return;
	galsa_s* a = g->data;
	a->card = name;
}

__private void galsa_channel_set(gadget_s* g, char* name){
	if( g->type != TYPE ) return;
	galsa_s* a = g->data;
	a->channel = name;
}

__private void galsa_volume_set(gadget_s* g, long volume){
	if( g->type != TYPE ) return;
	galsa_s* a = g->data;
	if( galsa_mixer_begin(a) ) return;
	volume += a->min;
	if( volume < a->min ) volume = a->min;
	if( volume > a->max ) volume = a->max;
	galsa_mixer_volume_set(a, volume);
	galsa_mixer_end(a);
}

__private long galsa_volume_get(gadget_s* g){
	if( g->type != TYPE ) return -1;
	long ret;
	galsa_s* a = g->data;
	if( galsa_mixer_begin(a) ) return 0;
	ret = galsa_mixer_volume_get(a);
	galsa_mixer_end(a);
	ret -= a->min;
	return ret;
}

__private long galsa_volume_max_get(gadget_s* g){
	if( g->type != TYPE ) return -1;
	galsa_s* a = g->data;
	return (a->max-a->min);
}

__private void galsa_connect(gadget_s* g){
	if( g->type != TYPE ) return;
	galsa_s* a = g->data;
	if( a->hmixer ) galsa_mixer_end(a);
	galsa_mixer_begin(a);
	galsa_mixer_max_min(a);
	galsa_mixer_end(a);
	galsa_enable_callback(g, a);
}

__private int galsa_free(gadget_s* g){
	galsa_s* a = g->data;
	if( a->hmixer ) galsa_mixer_end(a);
	if( a->mctl ) galsa_disable_callback(g, a);
	free(a);
	return 0;
}
	
int gadget_alsa_load(gadget_s* g){
	galsa_s* a = mem_new(galsa_s);
	a->card = "default";
	a->channel = "Master";
	a->hmixer = NULL;
	a->mctl = NULL;
    
	g->data = a;
	g->ellapse = NULL;
	g->free = galsa_free;
	return 0;
}

void gadget_alsa_register(vbar_s* vb){
	dbg_info("register alsa");
	TYPE = gadget_type_get(vb, "alsa");
	config_add_symbol(vb, "gadget_alsa_card_set", galsa_card_set);
	config_add_symbol(vb, "gadget_alsa_channel_set", galsa_channel_set);
	config_add_symbol(vb, "gadget_alsa_volume_set", galsa_volume_set);
	config_add_symbol(vb, "gadget_alsa_volume_get", galsa_volume_get);
	config_add_symbol(vb, "gadget_alsa_volume_max_get", galsa_volume_max_get);
	config_add_symbol(vb, "gadget_alsa_connect", galsa_connect);
}

