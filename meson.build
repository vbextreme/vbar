project('vbar', 'c')

src = [ 'core/vbar.c', 'core/config.c', 'core/delay.c', 'core/string.c', 'core/memory.c', 'core/modules.c', 'core/spawn.c' ]
src += [ 'ipc/i3pc.c' ]
src += ['modules/M_cpu.c' , 'modules/M_memory.c', 'modules/M_time.c', 'modules/M_static.c', 'modules/M_power.c', 'modules/M_net.c']

incdir = include_directories('include')

add_global_arguments('-std=gnu11', language: 'c')
add_global_arguments('-Wall', language: 'c')
add_global_arguments('-Wextra', language: 'c')

#cc = meson.get_compiler('c')
#m_dep = cc.find_library('m', required : true)
deps = [  ] 

opt_mcpuncores = get_option('m-cpu-ncores')
if opt_mcpuncores > 0
	debugstr = '-DNCORES_MAX=@0@'
	debugstr = debugstr.format(opt_mcpuncores)
	add_global_arguments(debugstr, language: 'c')
	#TODO auto set ncpu
endif

if( get_option('optimize') > 0 )
  add_global_arguments('-O2', language: 'c')
  add_global_arguments('-march=native', language: 'c')
endif

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

message('copyright vbextreme 2018')
message('release under GPLv3')

executable('vbar', src, include_directories: incdir, dependencies: deps, install: true)

