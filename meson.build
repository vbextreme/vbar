project('vbar', 'c')

# compiler flags
incdir = include_directories('include')

add_global_arguments('-std=gnu11', language: 'c')
add_global_arguments('-Wall', language: 'c')
add_global_arguments('-Wextra', language: 'c')
if( get_option('optimize') > 0 )
	add_global_arguments('-O2', language: 'c')
	add_global_arguments('-march=native', language: 'c')
endif

#cc = meson.get_compiler('c')
#m_dep = cc.find_library('m', required : true)
deps = [  ] 

conf = configuration_data()


# core
src = [ 'core/vbar.c', 'core/config.c', 'core/delay.c', 'core/string.c', 'core/memory.c', 'core/modules.c', 'core/spawn.c' ]

# select ipc
opt_wm = get_option('wm')
if opt_wm == '' 
	if (find_program('i3', required: false, native: true)).found()
		opt_wm = 'i3wm'
	else
		error('no wm available')
	endif
endif

if opt_wm == 'i3wm'
	src += [ 'ipc/i3pc.c' ]
endif

# select modules
v_mod = ''
p_mod = ''
gen_mod = 'int @0@_mod_load(module_s* mod, char* path);'

if get_option('Mcpu') != 0
	NAME='cpu'
	PROT='cpu'
	
	src += ['modules/M_cpu.c']
	
	opt_mcpuncores = get_option('m-cpu-ncores')
	if opt_mcpuncores > 0
		debugstr = '-DNCORES_MAX=@0@'
		debugstr = debugstr.format(opt_mcpuncores)
		add_global_arguments(debugstr, language: 'c')
	else
		warning('use more memory with default ncores')
	endif

	am = '{"@0@", @1@_mod_load, "~/.config/vbar/@0@/config"},'
	v_mod += am.format(NAME,PROT)
	p_mod += gen_mod.format(PROT)
	message('module cpu enabled')
endif

if get_option('Mcpufreq') != 0
	NAME='cpufreq'
	PROT='cpufreq'
	
	src += ['modules/M_cpufreq.c']
	
	am = '{"@0@", @1@_mod_load, "~/.config/vbar/@0@/config"},'
	v_mod += am.format(NAME,PROT)
	p_mod += gen_mod.format(PROT)
	message('module cpufreq enabled')
endif

if get_option('Mmemory') != 0
	NAME='memory'
	PROT='mem'

	src += ['modules/M_memory.c']
	
	am = '{"@0@", @1@_mod_load, "~/.config/vbar/@0@/config"},'
	v_mod += am.format(NAME,PROT)
	p_mod += gen_mod.format(PROT)
	message('module memory enabled')
endif

if get_option('Mtime') != 0
	NAME='datetime'
	PROT='datetime'

	src += ['modules/M_time.c']

	am = '{"@0@", @1@_mod_load, "~/.config/vbar/@0@/config"},'
	v_mod += am.format(NAME,PROT)
	p_mod += gen_mod.format(PROT)
	message('module time enabled')
endif

if get_option('Mstatic') != 0
	NAME='static'
	PROT='static'

	src += ['modules/M_static.c']
	
	am = '{"@0@", @1@_mod_load, "~/.config/vbar/@0@/config"},'
	v_mod += am.format(NAME,PROT)
	p_mod += gen_mod.format(PROT)
	message('module static enabled')
endif

if get_option('Mpower') != 0
	NAME='power'
	PROT='power'

	src += ['modules/M_power.c']
	am = '{"@0@", @1@_mod_load, "~/.config/vbar/@0@/config"},'
	v_mod += am.format(NAME,PROT)
	p_mod += gen_mod.format(PROT)
	message('module power enabled')
endif

if get_option('Mnet') != 0
	NAME='net'
	PROT='net'

	src += ['modules/M_net.c']
	am = '{"@0@", @1@_mod_load, "~/.config/vbar/@0@/config"},'
	v_mod += am.format(NAME,PROT)
	p_mod += gen_mod.format(PROT)
	message('module net enabled')
endif

if get_option('Mwireless') != 0
	NAME='wireless'
	PROT='wireless'

	src += ['modules/M_wireless.c']
	am = '{"@0@", @1@_mod_load, "~/.config/vbar/@0@/config"},'
	v_mod += am.format(NAME,PROT)
	p_mod += gen_mod.format(PROT)
	message('module wireless enabled')
endif

if get_option('Mtemperature') != 0
	NAME='temperature'
	PROT='temp'

    src += ['modules/M_temperature.c']
	am = '{"@0@", @1@_mod_load, "~/.config/vbar/@0@/config"},'
	v_mod += am.format(NAME,PROT)
	p_mod += gen_mod.format(PROT)
	message('module temperature enabled')
endif

conf.set('AUTO_VECTOR_MODULE', v_mod)
conf.set('AUTO_PROTO_MODULE', p_mod)

# debug
opt_debug = get_option('debug')
if opt_debug > 0
	message('debug enabled')
	debugstr = '-DEF_DEBUG_ENABLE=@0@'
	debugstr = debugstr.format(opt_debug)
	add_global_arguments(debugstr, language: 'c')
endif

opt_debug_color = get_option('dolor')
if opt_debug_color > 0
	message('color debug enabled')
	debugstr = '-DEF_DEBUG_COLOR=@0@'
	debugstr = debugstr.format(opt_debug_color)
	add_global_arguments(debugstr, language: 'c')
endif

if get_option('assert') > 0
	message('assertion enabled')
	add_global_arguments('-DEF_ASSERT_ENABLE=1', language: 'c')
endif

# license
message('copyright vbextreme 2018')
message('release under GPLv3')

# build
configure_file(output: 'config.h', configuration: conf)
executable('vbar', src, include_directories: incdir, dependencies: deps, install: true)

