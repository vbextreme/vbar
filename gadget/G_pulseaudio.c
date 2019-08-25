#include <vbar.h>
#include <pulse/pulseaudio.h>

//WARNING pulseaudio works in other threads, callback is generate from thread and gadget_event is executed from this thread, vbar for now not support concurrency
//alternative is to create queue and poll to this, same polybar
//
//fork from polybar

#define PULSE_CLASS "vbar"
#define PULSE_DEFAULT_SINK "@DEFAULT_SINK@"

typedef struct pulseaudio{
    struct pa_context* ctx;
    struct pa_threaded_mainloop* tml;
	struct pa_cvolume cvolume;
	int mute;
    pa_volume_t max_volume;
	char* sinkname;
	const char* sname;
	const char* ssname;
	unsigned mindex;
	int success;
	socket_s ipc;
}pulseaudio_s;

__private void pulseaudio_context_state_callback(struct pa_context *context, void *userdata) {
	pulseaudio_s* pa = userdata;
	switch (pa_context_get_state(context)) {
		case PA_CONTEXT_READY:
		case PA_CONTEXT_TERMINATED:
		case PA_CONTEXT_FAILED:
			pa_threaded_mainloop_signal(pa->tml, 0);
		break;

		default:
		case PA_CONTEXT_UNCONNECTED:
		case PA_CONTEXT_CONNECTING:
		case PA_CONTEXT_AUTHORIZING:
		case PA_CONTEXT_SETTING_NAME:
		break;
	}
}

__private void pulseaudio_sink_info_callback(__unused struct pa_context* context, const struct pa_sink_info *info, int eol, void *userdata) {
	pulseaudio_s* pa = userdata;
	if( !eol && info ){
		pa->mindex = info->index;
		pa->sname = info->name;
	}
	pa_threaded_mainloop_signal(pa->tml, 0);
}

__private void pulseaudio_simple_callback(__unused struct pa_context* context, int success, void *userdata) {
	pulseaudio_s* pa = userdata;
	pa->success = success;
	pa_threaded_mainloop_signal(pa->tml, 0);
}

__private void pulseaudio_get_sink_volume_callback(__unused struct pa_context* context, const struct pa_sink_info *info, __unused int bo, void *userdata) {
	pulseaudio_s* pa = userdata;
	if( info ){
		pa->cvolume = info->volume;
		pa->mute = info->mute;
	}
	pa_threaded_mainloop_signal(pa->tml, 0);
}

#define PULSE_EVENT_NEW 1
#define PULSE_EVENT_SERVER 2
#define PULSE_EVENT_REMOVE 3
#define PULSE_EVENT_CHANGE 4

__private void pulseaudio_wait_loop(struct pa_operation *op, struct pa_threaded_mainloop *loop);
__private void pulseaudio_update_volume(pulseaudio_s* pa, struct pa_operation *o);

__private void pulseaudio_process_events(gadget_s* g, int evtype){
	dbg_info("EVENT");
	pulseaudio_s* pa = g->data;
	//pa_threaded_mainloop_lock(pa->tml);
	//struct pa_operation* o = NULL;
	
	switch( evtype ){
		case PULSE_EVENT_NEW:
			if( pa->sname ){
				pa_context_get_sink_info_by_name(pa->ctx, pa->sname, pulseaudio_sink_info_callback, pa);
				//pulseaudio_wait_loop(o, pa->tml);
			}
		break;
		
		case PULSE_EVENT_SERVER:
			if( pa->sname ) {
				break;
			}
			pa_context_get_sink_info_by_name(pa->ctx, PULSE_DEFAULT_SINK, pulseaudio_sink_info_callback, pa);
			//pulseaudio_wait_loop(o, pa->tml);
		break;
      
		case PULSE_EVENT_REMOVE:
			pa_context_get_sink_info_by_name(pa->ctx, PULSE_DEFAULT_SINK, pulseaudio_sink_info_callback, pa);
			//pulseaudio_wait_loop(o, pa->tml);
		break;
		default:
		break;
    }
    //pulseaudio_update_volume(pa, o);
	pa_context_get_sink_info_by_index(pa->ctx, pa->mindex, pulseaudio_get_sink_volume_callback, pa);

	//pa_threaded_mainloop_unlock(pa->tml);
	vbar_ipc_send(&pa->ipc, 1, g->instance, GADGET_EVENT_REFRESH);
}

__private void pulseaudio_subscribe_callback(__unused struct pa_context* context, pa_subscription_event_type_t t, uint32_t idx, void* userdata) {
	gadget_s* g = userdata;
	pulseaudio_s* pa = g->data;
	switch( t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK ){
		case PA_SUBSCRIPTION_EVENT_SERVER:
			switch(t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) {
				case PA_SUBSCRIPTION_EVENT_CHANGE:
					pulseaudio_process_events(g, PULSE_EVENT_SERVER);
				break;
			}
		break;
		case PA_SUBSCRIPTION_EVENT_SINK:
			switch(t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) {
				case PA_SUBSCRIPTION_EVENT_NEW:
					pulseaudio_process_events(g, PULSE_EVENT_NEW);
				break;
				case PA_SUBSCRIPTION_EVENT_CHANGE:
					if( idx == pa->mindex ){
						pulseaudio_process_events(g, PULSE_EVENT_CHANGE);
					}
				break;
				case PA_SUBSCRIPTION_EVENT_REMOVE:
					if( idx == pa->mindex ){
						pulseaudio_process_events(g, PULSE_EVENT_REMOVE);
					}
				break;
			}
		break;
	}
	pa_threaded_mainloop_signal(pa->tml, 0);
}

__private void pulseaudio_wait_loop(struct pa_operation *op, struct pa_threaded_mainloop *loop){
	while( pa_operation_get_state(op) != PA_OPERATION_DONE )
		pa_threaded_mainloop_wait(loop);
	pa_operation_unref(op);
}

__private void pulseaudio_update_volume(pulseaudio_s* pa, struct pa_operation *o) {
	o = pa_context_get_sink_info_by_index(pa->ctx, pa->mindex, pulseaudio_get_sink_volume_callback, pa);
	pulseaudio_wait_loop(o, pa->tml);
}

__private err_t pulseaudio_init(gadget_s* g, pulseaudio_s* pa, int max_volume){
	pa->tml = pa_threaded_mainloop_new();
	if( !pa->tml ){
		dbg_error("pulseaudio new mainloop");
		return -1;
	}
	pa_threaded_mainloop_lock(pa->tml);
  
	pa->ctx = pa_context_new(pa_threaded_mainloop_get_api(pa->tml), PULSE_CLASS);
	if( !pa->ctx ){
		pa_threaded_mainloop_unlock(pa->tml);
		pa_threaded_mainloop_free(pa->tml);
		dbg_error("pulseaudio context new");
		return -1;
	}

	pa_context_set_state_callback(pa->ctx, pulseaudio_context_state_callback, pa);

	if( pa_context_connect(pa->ctx, NULL, PA_CONTEXT_NOFLAGS, NULL) < 0 ){
		pa_context_disconnect(pa->ctx);
		pa_context_unref(pa->ctx);
		pa_threaded_mainloop_unlock(pa->tml);
		pa_threaded_mainloop_free(pa->tml);
		dbg_error("pulseaudio connection");
		return -1;
	}

	if( pa_threaded_mainloop_start(pa->tml) < 0){
		pa_context_disconnect(pa->ctx);
		pa_context_unref(pa->ctx);
		pa_threaded_mainloop_unlock(pa->tml);
		pa_threaded_mainloop_free(pa->tml);
		dbg_error("pulseaudio start mainloop.");
		return -1;
	}

	dbg_info("started mainloop");

	pa_threaded_mainloop_wait(pa->tml);
	if( pa_context_get_state(pa->ctx) != PA_CONTEXT_READY ){
		pa_threaded_mainloop_unlock(pa->tml);
		pa_threaded_mainloop_stop(pa->tml);
		pa_context_disconnect(pa->ctx);
		pa_context_unref(pa->ctx);
		pa_threaded_mainloop_free(pa->tml);
		dbg_error("Could not connect to pulseaudio server");
	}

	struct pa_operation* op = NULL;
	if( pa->sinkname ){
		op = pa_context_get_sink_info_by_name(pa->ctx, pa->sinkname, pulseaudio_sink_info_callback, pa);
		pulseaudio_wait_loop(op, pa->tml);
	}
	
	if( !pa->sname ) {
		op = pa_context_get_sink_info_by_name(pa->ctx, PULSE_DEFAULT_SINK, pulseaudio_sink_info_callback, pa);
		pulseaudio_wait_loop(op, pa->tml);
		dbg_info("pulseaudio using default sink %s", pa->sname);
	} 
	else{
		dbg_info("pulseaudio: using sink %s", pa->sname);
	}

	pa->max_volume = max_volume ? PA_VOLUME_UI_MAX : PA_VOLUME_NORM;
	//pa->ssname = pa->sinkname;

	pa_subscription_mask_t eventtypes = (PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SERVER);
	op = pa_context_subscribe(pa->ctx, eventtypes, pulseaudio_simple_callback, pa);
	pulseaudio_wait_loop(op, pa->tml);
	if( !pa->success ){
		dbg_error("Failed to subscribe to sink");
		return -1;
	}
	pa_context_set_subscribe_callback(pa->ctx, pulseaudio_subscribe_callback, g);

	pulseaudio_update_volume(pa, op);

	pa_threaded_mainloop_unlock(pa->tml);
	return 0;
}

__private int pulseaudio_free(gadget_s* g){
	pulseaudio_s* pa = g->data;
	pa_threaded_mainloop_stop(pa->tml);
	pa_context_disconnect(pa->ctx);
	pa_context_unref(pa->ctx);
	pa_threaded_mainloop_free(pa->tml);
	return 0;
}

__private void gpulse_sink_set(gadget_s* g, char* name){
	pulseaudio_s* pa = g->data;
	if( name && * name ){
		pa->sinkname = name;
	}
}

__private const char* gpulse_sink_get(gadget_s* g){
	pulseaudio_s* pa = g->data;
	return pa->sinkname;
}

__private int gpulse_volume_get(gadget_s* g){
	pulseaudio_s* pa = g->data;
	return (int)(pa_cvolume_max(&pa->cvolume) * 100.0f / PA_VOLUME_NORM + 0.5f);
}

__private void gpulse_volume_set(gadget_s* g, float percentage){
	pulseaudio_s* pa = g->data;
	pa_threaded_mainloop_lock(pa->tml);
	pa_volume_t vol = (((PA_VOLUME_NORM - PA_VOLUME_MUTED) * percentage) /100.0) + 0.5 + PA_VOLUME_MUTED;
	pa_cvolume_scale(&pa->cvolume, vol);
	struct pa_operation *op = pa_context_set_sink_volume_by_index(pa->ctx, pa->mindex, &pa->cvolume, pulseaudio_simple_callback, pa);
	pulseaudio_wait_loop(op, pa->tml);
	if( !pa->success ){
		dbg_warning("Failed to set sink volume.");
	}
	pa_threaded_mainloop_unlock(pa->tml);
}

__private void gpulse_volume_delta(gadget_s* g, int delta) {
	dbg_info("pulse delta %d", delta);
	pulseaudio_s* pa = g->data;
	pa_threaded_mainloop_lock(pa->tml);
	pa_volume_t vol = (((PA_VOLUME_NORM - PA_VOLUME_MUTED) * abs(delta)) /100.0) + 0.5 + PA_VOLUME_MUTED;

	if( delta > 0 ){
		pa_volume_t current = pa_cvolume_max(&pa->cvolume);
	
		if( current + vol <= pa->max_volume ){
			pa_cvolume_inc(&pa->cvolume, vol);
		}
		else if( current < pa->max_volume ){
			pa_cvolume_scale(&pa->cvolume, pa->max_volume);
		}
		else{
			dbg_warning("pulseaudio maximum volume reached");
		}
	}
	else{
		pa_cvolume_dec(&pa->cvolume, vol);
	}

	struct pa_operation *op = pa_context_set_sink_volume_by_index(pa->ctx, pa->mindex, &pa->cvolume, pulseaudio_simple_callback, pa);
	pulseaudio_wait_loop(op, pa->tml);
	if( !pa->success ){
		dbg_error("Failed to set sink volume");
	}
	pa_threaded_mainloop_unlock(pa->tml);
}

__private void gpulse_mute_set(gadget_s* g, int mode){
	pulseaudio_s* pa = g->data;
	pa_threaded_mainloop_lock(pa->tml);
	struct pa_operation *op = pa_context_set_sink_mute_by_index(pa->ctx, pa->mindex, mode, pulseaudio_simple_callback, pa);
	pulseaudio_wait_loop(op, pa->tml);
	if( !pa->success ){
		dbg_error("Failed to mute sink");
	}
	pa_threaded_mainloop_unlock(pa->tml);
}

__private int gpulse_mute_get(gadget_s* g){
	pulseaudio_s* pa = g->data;
	return pa->mute;
}

__private void gpulse_connect(gadget_s* g){
	pulseaudio_s* pa = g->data;
	pulseaudio_init(g, pa, 0);
}

int gadget_pulseaudio_load(gadget_s* g){
	pulseaudio_s* pa = mem_new(pulseaudio_s);
	pa->ctx = NULL;
	pa->max_volume = PA_VOLUME_UI_MAX;
	pa->tml = NULL;
	pa->sname = NULL;
	pa->ssname = NULL;
	vbar_ipc_client_connect(&pa->ipc);

	g->data = pa;
	g->ellapse = NULL;
	g->free = pulseaudio_free;

	return 0;
}

void gadget_pulseaudio_register(vbar_s* vb){
	dbg_info("register pulseaudio");
	config_add_symbol(vb, "gadget_pulseaudio_connect", gpulse_connect);
	config_add_symbol(vb, "gadget_pulseaudio_sink_set", gpulse_sink_set);
	config_add_symbol(vb, "gadget_pulseaudio_sink_get", gpulse_sink_get);
	config_add_symbol(vb, "gadget_pulseaudio_volume_set", gpulse_volume_set);
	config_add_symbol(vb, "gadget_pulseaudio_volume_get", gpulse_volume_get);
	config_add_symbol(vb, "gadget_pulseaudio_volume_delta", gpulse_volume_delta);
	config_add_symbol(vb, "gadget_pulseaudio_mute_set", gpulse_mute_set);
	config_add_symbol(vb, "gadget_pulseaudio_mute_get", gpulse_mute_get);
}

