#include <vbarScript.h>

#define MONITOR "primary"
#define FONT "inconsolata"
#define FONT_SIZE 12
#define BACKGROUND rgb(10,10,10)
#define FOREGROUND rgb(255,255,255)
#define GRAY rgb(70,70,70)
#define DBLUE rgb(70,70,150)
#define HEIGHT 0
#define SPACING_TOP 0
#define SPACING_BOTTOM 5
#define TOPBAR 1
#define BORDER_SIZE 2

vbar_s* vbar;
gadget_s* clock;
gadget_s* cpu;
gadget_s* cpufreq;
gadget_s* mem;
gadget_s* powerstat;
#define WORKSPACE_N 30
gadget_s* workspace[WORKSPACE_N];
gadget_s* alsa;
gadget_s* pulse;
gadget_s* network;
gadget_s* error;
gadget_s* test;
gadget_s* script;

void script_event(gadget_s* g, gadgetEventType_e event, void* arg){
	switch( event ){
		case GADGET_EVENT_REFRESH:{
			gadget_script_shell_event(g, "/home/vbextreme/Project/c/app/vbar/test.sh", event, arg);
			const char* txt = gadget_script_txt(g);
			gadget_text(g, " %s ", txt);
			gadget_redraw(g);
		}
		break;

		case GADGET_EVENT_MOUSE_ENTER:
			gadget_border_color(g, rgb(30,30,220));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_LEAVE:
			gadget_border_color(g, rgb(220,30,30));
			gadget_redraw(g);
		break;
	
		case GADGET_EVENT_EXTEND_OPEN:
		case GADGET_EVENT_EXTEND_REFRESH:
		case GADGET_EVENT_EXTEND_CLOSE:
		case GADGET_EVENT_MOUSE_CLICK:
		case GADGET_EVENT_MOUSE_RELEASE:
		case GADGET_EVENT_MOUSE_PRESS:
		case GADGET_EVENT_MOUSE_MOVE:
		case GADGET_EVENT_MOUSE_DBLCLICK:{
			gadget_script_shell_event(g, "/home/vbextreme/Project/c/app/vbar/test.sh", event, arg);
			const char* txt = gadget_script_txt(g);
			gadget_script_label_reset(g);
			gadget_text(g, " %s ", txt);
			gadget_redraw(g);
		}
		break;
	}
}

void error_event(gadget_s* g, gadgetEventType_e event, void* arg){
	switch( event ){
		case GADGET_EVENT_REFRESH:
			gadget_text(g, " %s ", gadget_label_get(g));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_ENTER:
			gadget_border_color(g, rgb(30,30,220));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_LEAVE:
			gadget_border_color(g, rgb(220,30,30));
			gadget_redraw(g);
		break;
	
		case GADGET_EVENT_EXTEND_OPEN:
		case GADGET_EVENT_EXTEND_REFRESH:
		case GADGET_EVENT_EXTEND_CLOSE:
		case GADGET_EVENT_MOUSE_CLICK:
		case GADGET_EVENT_MOUSE_RELEASE:
		case GADGET_EVENT_MOUSE_PRESS:
		case GADGET_EVENT_MOUSE_MOVE:
		case GADGET_EVENT_MOUSE_DBLCLICK:
		break;
	}
}

void network_event(gadget_s* g, gadgetEventType_e event, void* arg){
	switch( event ){
		case GADGET_EVENT_REFRESH:{
			double r,t;
			const char* unitr = gadget_tohuman(&r, gadget_network_receive_speed(g), 0, 1024.0);
			const char* unitt = gadget_tohuman(&t, gadget_network_transmit_speed(g), 0, 1024.0);
			gadget_text(g, " %s %3d v%6.2f%s ^%6.2f%s ", gadget_network_essid_get(g), gadget_network_dbm_get(g), r, unitr, t, unitt);
			//gadget_text(g, " v%6.2lf%s", r, unitr);

			gadget_redraw(g);
		}
		break;

		case GADGET_EVENT_MOUSE_ENTER:
			gadget_border_color(g, rgb(30,30,220));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_LEAVE:
			gadget_border_color(g, rgb(220,30,30));
			gadget_redraw(g);
		break;
	
		case GADGET_EVENT_EXTEND_OPEN:
		case GADGET_EVENT_EXTEND_REFRESH:
		case GADGET_EVENT_EXTEND_CLOSE:
		case GADGET_EVENT_MOUSE_CLICK:
		case GADGET_EVENT_MOUSE_RELEASE:
		case GADGET_EVENT_MOUSE_PRESS:
		case GADGET_EVENT_MOUSE_MOVE:
		case GADGET_EVENT_MOUSE_DBLCLICK:
		break;
	}
}

void workspace_event(gadget_s* g, gadgetEventType_e event, void* arg){

	switch( event ){
		case GADGET_EVENT_REFRESH:{
			if( gadget_status_get(g) != WORKSPACE_N-1 ) break;
			size_t count = gadget_workspace_count(g);
			size_t active = gadget_workspace_active(g);

			if( count == 0 || active > count){
				gadget_text(g, "workspace error");
				break;
			}

			char** names = gadget_workspace_names(g);		
			for( size_t i = 0; i < count; ++i){
				gadget_hide(workspace[i], 0);
				gadget_text_reset(workspace[i]);
				gadget_text(workspace[i], " %s ", names[i]);
				gadget_background(workspace[i], i==active ? DBLUE : GRAY);
				gadget_redraw(workspace[i]);
			}
			for( size_t i = count; i < WORKSPACE_N; ++i){
				gadget_hide(workspace[i], 1);
				gadget_redraw(workspace[i]);
			}
		}
		break;

		case GADGET_EVENT_MOUSE_ENTER:
			gadget_border_color(g, rgb(30,30,220));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_LEAVE:
			gadget_border_color(g, rgb(220,30,30));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_CLICK:
			gadget_workspace_activate(g, gadget_status_get(g));
		break;

		case GADGET_EVENT_EXTEND_REFRESH:
		case GADGET_EVENT_EXTEND_OPEN:
		case GADGET_EVENT_EXTEND_CLOSE:
		case GADGET_EVENT_MOUSE_RELEASE:
		case GADGET_EVENT_MOUSE_PRESS:
		case GADGET_EVENT_MOUSE_MOVE:
		case GADGET_EVENT_MOUSE_DBLCLICK:
		break;
	}
}

void clock_event(gadget_s* g, gadgetEventType_e event, void* arg){
	switch( event ){
		case GADGET_EVENT_REFRESH:
			gadget_text(g, " ⌚ %02d.%02d.%02d ", gadget_clock_hour(g), gadget_clock_minutes(g), gadget_clock_seconds(g));
			gadget_redraw(g);
		break;
		case GADGET_EVENT_MOUSE_ENTER:
			gadget_border_color(g, rgb(30,30,220));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_LEAVE:
			gadget_border_color(g, rgb(220,30,30));
			gadget_redraw(g);
		break;
	
		case GADGET_EVENT_EXTEND_OPEN:
		case GADGET_EVENT_EXTEND_REFRESH:
		case GADGET_EVENT_EXTEND_CLOSE:
		case GADGET_EVENT_MOUSE_CLICK:
		case GADGET_EVENT_MOUSE_RELEASE:
		case GADGET_EVENT_MOUSE_PRESS:
		case GADGET_EVENT_MOUSE_MOVE:
		case GADGET_EVENT_MOUSE_DBLCLICK:
		break;
	}
}

void cpu_event(gadget_s* g, gadgetEventType_e event, void* arg){
	switch( event ){
		case GADGET_EVENT_REFRESH:
			gadget_text(g, " 💻 %6.2lf%% ", gadget_cpu_load_average(g, 0));
			//gadget_text(g, "%6.2lf%%", gadget_cpu_load_average(g, 0));

			gadget_redraw(g);
		break;

		case GADGET_EVENT_EXTEND_REFRESH:
			gadget_extend_label(g, 1, " 💻[1] %6.2lf%%", gadget_cpu_load_average(g, 1) );
			gadget_extend_label(g, 1, " 💻[2] %6.2lf%%", gadget_cpu_load_average(g, 2) );
			gadget_extend_label(g, 1, " 💻[3] %6.2lf%%", gadget_cpu_load_average(g, 3) );
			gadget_extend_label(g, 1, " 💻[4] %6.2lf%%", gadget_cpu_load_average(g, 4) );
			gadget_extend_label(g, 2, " 💻[1] %1.2lfGhz", gadget_cpufreq_fq_get(0, 0) / 1000000.0 );
			gadget_extend_label(g, 2, " 💻[2] %1.2lfGhz", gadget_cpufreq_fq_get(0, 1) / 1000000.0 );
			gadget_extend_label(g, 2, " 💻[3] %1.2lfGhz", gadget_cpufreq_fq_get(0, 2) / 1000000.0 );
			gadget_extend_label(g, 2, " 💻[4] %1.2lfGhz", gadget_cpufreq_fq_get(0, 3) / 1000000.0 );
			for(int i = 0; i < 4; ++i){
				char* tmp = gadget_cpufreq_current_governor_get(i);
				gadget_extend_label(g, 3, " 💻[%1d] %s", i, tmp );
				gadget_cpufreq_current_governor_free(tmp);
			}
			gadget_extend_redraw(g);
		break;

		case GADGET_EVENT_EXTEND_OPEN:
			gadget_extend_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_CLICK:
			gadget_extend_toggle(g);
		break;

		case GADGET_EVENT_MOUSE_ENTER:
			gadget_border_color(g, rgb(30,30,220));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_LEAVE:
			gadget_border_color(g, rgb(220,30,30));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_EXTEND_CLOSE:
		case GADGET_EVENT_MOUSE_RELEASE:
		case GADGET_EVENT_MOUSE_PRESS:
		case GADGET_EVENT_MOUSE_MOVE:
		case GADGET_EVENT_MOUSE_DBLCLICK:
		break;
	}
}

void cpufreq_event(gadget_s* g, gadgetEventType_e event, void* arg){
	switch( event ){
		case GADGET_EVENT_REFRESH:
		case GADGET_EVENT_EXTEND_REFRESH:
		case GADGET_EVENT_EXTEND_OPEN:
		case GADGET_EVENT_MOUSE_CLICK:
		case GADGET_EVENT_MOUSE_ENTER:
		case GADGET_EVENT_MOUSE_LEAVE:
		case GADGET_EVENT_EXTEND_CLOSE:
		case GADGET_EVENT_MOUSE_RELEASE:
		case GADGET_EVENT_MOUSE_PRESS:
		case GADGET_EVENT_MOUSE_MOVE:
		case GADGET_EVENT_MOUSE_DBLCLICK:
		break;
	}
}

void mem_event(gadget_s* g, gadgetEventType_e event, void* arg){
	#define UNIT MIB
	switch( event ){
		case GADGET_EVENT_REFRESH:
			gadget_text(g, " 🐏 %.2lf GiB ", (double)gadget_memory_used_get(g)/UNIT);
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_ENTER:
			gadget_border_color(g, rgb(30,30,220));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_LEAVE:
			gadget_border_color(g, rgb(220,30,30));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_EXTEND_REFRESH:
		case GADGET_EVENT_EXTEND_OPEN:
		case GADGET_EVENT_MOUSE_CLICK:
		case GADGET_EVENT_EXTEND_CLOSE:
		case GADGET_EVENT_MOUSE_RELEASE:
		case GADGET_EVENT_MOUSE_PRESS:
			system("notify-send -a \"hello\" world");
		break;
		case GADGET_EVENT_MOUSE_MOVE:
		case GADGET_EVENT_MOUSE_DBLCLICK:
		break;
	}
}

void alsa_event(gadget_s* g, gadgetEventType_e event, void* arg){
	#define ASTEPV 15
	switch( event ){
		case GADGET_EVENT_REFRESH:{
			long max = gadget_alsa_volume_max_get(g);
			long volume = gadget_alsa_volume_get(g);
			gadget_text(g, " alsa ➖ %3ld%% ➕ ", (100*volume)/max);
			gadget_redraw(g);
		}
		break;

		case GADGET_EVENT_MOUSE_ENTER:
			gadget_border_color(g, rgb(30,30,220));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_LEAVE:
			gadget_border_color(g, rgb(220,30,30));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_CLICK:{
			vbarMouse_s* event = arg;
			//printf("X:%u Y%u\n", event->x, event->y);
			long volume = gadget_alsa_volume_get(g);
			long max = gadget_alsa_volume_max_get(g);
			long step = max / ASTEPV;
			//printf("volume:%ld ", volume);
			if( event->x > 75 && event->x < 90 ){
				//printf("decrease %ld", volume - step);
				gadget_alsa_volume_set(g, event->button == 1 ? volume - step : 0);
			}
			else if( event->x > 125 && event->x < 135 ){
				//printf("increase %ld", volume + step);
				gadget_alsa_volume_set(g, event->button == 1 ? volume + step : max);
			}
			//printf("\n");
		}
		break;

		case GADGET_EVENT_EXTEND_REFRESH:
		case GADGET_EVENT_EXTEND_OPEN:
		case GADGET_EVENT_EXTEND_CLOSE:
		case GADGET_EVENT_MOUSE_RELEASE:
		case GADGET_EVENT_MOUSE_PRESS:
		case GADGET_EVENT_MOUSE_MOVE:
		case GADGET_EVENT_MOUSE_DBLCLICK:
		break;
	}
}

void pulse_event(gadget_s* g, gadgetEventType_e event, void* arg){
	#define PSTEPV 15
	switch( event ){
		case GADGET_EVENT_REFRESH:{
			int volume = gadget_pulseaudio_volume_get(g);
			gadget_text(g, " pulse ➖ %3ld%% ➕ ", volume);
			gadget_redraw(g);
		}
		break;

		case GADGET_EVENT_MOUSE_ENTER:
			gadget_border_color(g, rgb(30,30,220));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_LEAVE:
			gadget_border_color(g, rgb(220,30,30));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_CLICK:{
			vbarMouse_s* event = arg;
			size_t mst = gadget_text_lenght(g, " pulse ");
			size_t men = gadget_text_lenght(g, "➖") + mst;
			size_t pst = gadget_text_lenght(g, " 000% ") + men;
			size_t pen = gadget_text_lenght(g, "➕") + pst;
			//printf("X:%u Y%u\n", event->x, event->y);
			if( event->x > mst && event->x < men ){
				//printf("delta %d", -PSTEPV);
				gadget_pulseaudio_volume_delta(g, -PSTEPV);
			}
			else if( event->x > pst && event->x < pen ){
				//printf("delta %d", PSTEPV);
				gadget_pulseaudio_volume_delta(g, PSTEPV);
			}
			//printf("\n");
		}
		break;

		case GADGET_EVENT_EXTEND_REFRESH:
		case GADGET_EVENT_EXTEND_OPEN:
		case GADGET_EVENT_EXTEND_CLOSE:
		case GADGET_EVENT_MOUSE_RELEASE:
		case GADGET_EVENT_MOUSE_PRESS:
		case GADGET_EVENT_MOUSE_MOVE:
		case GADGET_EVENT_MOUSE_DBLCLICK:
		break;
	}
}
void powerstat_event(gadget_s* g, gadgetEventType_e event, void* arg){
	switch( event ){
		case GADGET_EVENT_REFRESH:
			gadget_text(g, "🔋 %3d%% %2d:%02d ", gadget_powerstat_capacity_get(g), gadget_powerstat_timeleft_hours_get(g), gadget_powerstat_timeleft_minutes_get(g));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_EXTEND_REFRESH:
			gadget_extend_label(g, 0, "voltage now: %5.2lfV", gadget_powerstat_voltage_now_get(g, 1000000) );
			gadget_extend_label(g, 0, " voltage min: %5.2lfV", gadget_powerstat_voltage_min_get(g, 1000000) );
			gadget_extend_label(g, 1, "energy full: %6.2lfW/h", gadget_powerstat_energy_full_get(g, 1000000) );
			gadget_extend_label(g, 1, " energy now: %6.2lfW/h", gadget_powerstat_energy_now_get(g, 1000000) );
			gadget_extend_label(g, 2, "status: %s", gadget_powerstat_status_get(g));
			gadget_extend_redraw(g);
		break;

		case GADGET_EVENT_EXTEND_OPEN:
			gadget_change_interval(g, 1000);
			gadget_extend_redraw(g);
		break;

		case GADGET_EVENT_EXTEND_CLOSE:
			gadget_change_interval(g, 60000);
		break;

		case GADGET_EVENT_MOUSE_CLICK:
			gadget_extend_toggle(g);
		break;

		case GADGET_EVENT_MOUSE_ENTER:
			gadget_border_color(g, rgb(30,30,220));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_LEAVE:
			gadget_border_color(g, rgb(220,30,30));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_RELEASE:
		case GADGET_EVENT_MOUSE_PRESS:
		case GADGET_EVENT_MOUSE_MOVE:
		case GADGET_EVENT_MOUSE_DBLCLICK:
		break;
	}
}

void vbar_main(void){
	vbar = vbar_get();
	vbar_monitor_set(vbar, NULL);
	vbar_fonts_set(vbar, FONT, FONT_SIZE);
	vbar_fonts_set(vbar, "Symbola", FONT_SIZE);
	//vbar_fonts_set(vbar, "Fira Mono", FONT_SIZE);


	vbar_colors_set(vbar, BACKGROUND, FOREGROUND);
	vbar_height_set(vbar, HEIGHT, SPACING_TOP, SPACING_BOTTOM);
	vbar_topbar_set(vbar, TOPBAR);
	vbar_border_width_set(vbar, BORDER_SIZE);
	vbar_icon_load(vbar, "/usr/share/icons/Adwaita/16x16/apps/preferences-system-time-symbolic.symbolic.png", "clock", rgb(50, 50, 50));
	vbar_icon_load(vbar, "/usr/share/icons/Adwaita/96x96/apps/preferences-system-time-symbolic.symbolic.png", "clock2", rgb( 50, 150, 150));


	for( size_t i = 0; i < WORKSPACE_N; ++i){
		char name[80];
		sprintf(name, "%lu", i);
		workspace[i] = gadget_new(vbar, "workspace", name);
		gadget_event_register(workspace[i], workspace_event);	
		gadget_align(workspace[i], VBAR_ALIGNED_LEFT);
		gadget_border_color(workspace[i], rgb(220,30,30));
		gadget_border(workspace[i], VBAR_BORDER_BOTTOM);
		gadget_status_set(workspace[i], i);
		gadget_hide(workspace[i], 1);
		gadget_interval(workspace[i], 0);
		gadget_start(workspace[i]);
	}
	gadget_workspace_enable_events(workspace[WORKSPACE_N-1]);


	clock = gadget_new(vbar, "clock", "italy");
	gadget_event_register(clock, clock_event);	
	gadget_align(clock, VBAR_ALIGNED_CENTER);
	//gadget_background(clock, rgb(50,150,150));
	//gadget_icon(clock, "clock2");
	gadget_border_color(clock, rgb(220,30,30));
	gadget_border(clock, VBAR_BORDER_BOTTOM);
	gadget_interval(clock, 1000);
	gadget_start(clock);

	powerstat = gadget_new(vbar, "powerstat", "all");
	gadget_event_register(powerstat, powerstat_event);	
	gadget_align(powerstat, VBAR_ALIGNED_RIGHT);
	gadget_border_color(powerstat, rgb(220,30,30));
	gadget_border(powerstat, VBAR_BORDER_BOTTOM);
	gadget_powerstat_device_set(powerstat, "BAT0");
	gadget_interval(powerstat, 60000);
	gadget_extend_enable(powerstat, 3);
	gadget_start(powerstat);

	alsa = gadget_new(vbar, "alsa", "all");
	gadget_event_register(alsa, alsa_event);	
	gadget_align(alsa, VBAR_ALIGNED_RIGHT);
	gadget_border_color(alsa, rgb(220,30,30));
	gadget_border(alsa, VBAR_BORDER_BOTTOM);
	gadget_alsa_connect(alsa);
	gadget_interval(alsa, 0);
	gadget_start(alsa);

	pulse = gadget_new(vbar, "pulseaudio", "all");
	gadget_event_register(pulse, pulse_event);	
	gadget_align(pulse, VBAR_ALIGNED_RIGHT);
	gadget_border_color(pulse, rgb(220,30,30));
	gadget_border(pulse, VBAR_BORDER_BOTTOM);
	gadget_pulseaudio_connect(pulse);
	gadget_interval(pulse, 0);
	gadget_start(pulse);

	mem = gadget_new(vbar, "memory", "all");
	gadget_event_register(mem, mem_event);	
	gadget_align(mem, VBAR_ALIGNED_RIGHT);
	gadget_border_color(mem, rgb(220,30,30));
	gadget_border(mem, VBAR_BORDER_BOTTOM);
	gadget_interval(mem, 2000);
	gadget_start(mem);

	cpu = gadget_new(vbar, "cpu", "all");
	gadget_event_register(cpu, cpu_event);	
	//gadget_align(cpu, VBAR_ALIGNED_LEFT);
	//gadget_align(cpu, VBAR_ALIGNED_CENTER);
	gadget_align(cpu, VBAR_ALIGNED_RIGHT);
	gadget_border_color(cpu, rgb(220,30,30));
	gadget_border(cpu, VBAR_BORDER_BOTTOM);
	//gadget_background(cpu, rgb(50,150,150));
	gadget_interval(cpu, 800);
	gadget_extend_enable(cpu, 4);
	gadget_extend_background(cpu, 0, rgb(10,10,55));
	gadget_start(cpu);

	cpufreq = gadget_new(vbar, "cpufreq", "all");
	gadget_event_register(cpufreq, cpufreq_event);	
	gadget_interval(cpufreq, 0);
	gadget_start(cpufreq);

	network = gadget_new(vbar, "network", "all");
	gadget_event_register(network, network_event);	
	gadget_align(network, VBAR_ALIGNED_RIGHT);
	gadget_border_color(network, rgb(220,30,30));
	gadget_border(network, VBAR_BORDER_BOTTOM);
	gadget_network_device_set(network, "wlp3s0");
	gadget_interval(network, 1000);
	gadget_start(network);
/*
	if( !(test = gadget_new(vbar, "return null", "all")) ){
		error = gadget_new(vbar, "label", "all");
		gadget_event_register(error, error_event);	
		gadget_align(error, VBAR_ALIGNED_LEFT);
		gadget_border_color(error, rgb(220,30,30));
		gadget_border(error, VBAR_BORDER_BOTTOM);
		gadget_background(error, rgb(100,10,10));
		gadget_label_set(error, "error: on create gadget");
		gadget_interval(error, 0);
		gadget_start(error);
	}
*/
	script = gadget_new(vbar, "script", "all");
	gadget_event_register(script, script_event);	
	gadget_align(script, VBAR_ALIGNED_LEFT);
	gadget_border_color(script, rgb(220,30,30));
	gadget_border(script, VBAR_BORDER_BOTTOM);
	gadget_interval(script, 1000);
	gadget_start(script);



	//gadget_colors_set(clock, BACKGROUND, FOREGROUND");
	//gadget_interval_set(clock, 60000);
	//gadget_events_set(clock, clock_event);
}
