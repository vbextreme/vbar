project('vbar', 'c')

##################
# compiler flags #
##################

incdir = include_directories('include')

add_global_arguments('-std=gnu11', language: 'c')
add_global_arguments('-Wall', language: 'c')
add_global_arguments('-Wextra', language: 'c')
add_global_arguments('-Wuninitialized', language: 'c')
add_global_arguments('-Winit-self', language: 'c')
add_global_arguments('-Wstrict-aliasing', language: 'c')
add_global_arguments('-Wstrict-overflow', language: 'c')
add_global_arguments('-Wfloat-equal', language: 'c')
add_global_arguments('-Wvla', language: 'c')
if( get_option('optimize') > 0 )
  add_global_arguments('-O2', language: 'c')
  add_global_arguments('-march=native', language: 'c')
  add_global_arguments('-mtune=native', language: 'c')
endif
add_global_arguments('-DEF_MEM_PKEY_DISABLE=1', language: 'c')

cc = meson.get_compiler('c')

#m_dep = cc.find_library('m', required : true)
deps = [  ] 


#########################
# software dependencies #
#########################

bash = find_program('bash', required: true)
sed = find_program('sed', required: true)
gperf_software = find_program('gperf', required: true)
gperf_custom_software = find_program('gperf.custom', required: true)

########
# core #
########

src = [ 'core/vbar.c', 'core/config.c', 'core/delay.c', 'core/string.c', 'core/memory.c', 'core/modules.c', 'core/spawn.c', 'core/optex.c', 'core/intp.c', 'core/file.c' ]
src += [ 'core/heap.c', 'core/protect.c', 'core/safe.c', 'core/signal.c' ]

##############
# select ipc #
##############

src += ['ipc/icommonpc.c']

opt_wm = get_option('wm')
if opt_wm == '' 
  if (find_program('i3', required: false, native: true)).found()
	opt_wm = 'i3wm'
  elif (find_program('dwm', required: false, native: true)).found()
	opt_wm = 'dwm'
  else
	opt_wm = 'i3wm'
	warning('no wm available, configure with -Dwm=')
  endif
endif

if opt_wm == 'i3wm'
  src += [ 'ipc/i3pc.c' ]
elif opt_wm == 'dwm'
  src += [ 'ipc/idpc.c' ]
  tmpdep = cc.find_library('X11', required : true)
  deps += [ tmpdep ]  
endif

##################
# select modules #
##################
modules = [ 'cpu', 'cpufreq', 'memory', 'datetime', 'static', 'power', 'network', 'wireless', 'temperature', 'ipc' ]

v_mod = ''
p_mod = ''
gen_mod = 'int @0@_mod_load(module_s* mod, char* path);'
gen_vec = '{"@0@", @0@_mod_load, "~/.config/vbar/@0@/config"},'
gen_msg = 'enable @0@ module'
conf_mod = configuration_data()

foreach mod : modules
  enm = get_option('M@0@'.format(mod))
  if( enm > 0 )
	sss = 'modules/M_@0@.c'.format(mod)
	src += [ sss ]

	v_mod += gen_vec.format(mod)
	p_mod += gen_mod.format(mod)
	message(gen_msg.format(mod))
  endif
endforeach 

conf_mod.set('AUTO_VECTOR_MODULE', v_mod)
conf_mod.set('AUTO_PROTO_MODULE', p_mod)

gperf_mod_src = custom_target('gperf hash_mods.h', 
  output: 'hash_mods.h',
  input: 'gperf/mods.gperf', 
  command: [gperf_custom_software, 'hash_mods', 'HMODS_', '@INPUT@', '@OUTPUT@']
)
src += [ gperf_mod_src ]

############################
### extra option modules ###
############################

opt_mcpuncores = get_option('m-cpu-ncores')
if opt_mcpuncores > 0
  debugstr = '-DNCORES_MAX=@0@'
  debugstr = debugstr.format(opt_mcpuncores)
  add_global_arguments(debugstr, language: 'c')
else
  r = run_command('bash', '-c', 'cat /proc/cpuinfo | grep processor | wc -l')
  l = r.stdout().strip()
  ncpu = l.to_int()
  mmm = 'autoset cpu to @0@'
  message(mmm.format(ncpu))
endif

gperf_intp_src = custom_target('gperf hash_intp.h', 
  output: 'hash_intp.h',
  input: 'gperf/intp.gperf', 
  command: [gperf_custom_software, 'hash_intp', 'HINTP_', '@INPUT@', '@OUTPUT@']
)
src += [ gperf_intp_src ]

#########
# debug #
#########

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

###########
# license #
###########

message('copyright vbextreme 2018')
message('released under GPLv3')

#########
# build #
#########

configure_file(output: 'config_mod.h', configuration: conf_mod)
executable('vbar', src, include_directories: incdir, dependencies: deps, install: true)

