###########
# project #
###########
project('vbar', 'c')
projectName = 'vbar'

cc = meson.get_compiler('c')

###########
# include #
###########

includeDir = include_directories('include')

###############
# source file #
###############

gadget_name = [ 'clock', 'cpu', 'cpufreq', 'memory', 'powerstat', 'workspace', 'alsa', 'pulseaudio', 'network', 'label', 'script' ]

src = [ 'core/main.c', 'core/bar.c', 'core/vbar.c' ]
src += [ 'ef/delay.c', 'ef/file.c', 'ef/memory.c', 'ef/optex.c', 'ef/deadpoll.c', 'ef/heap.c', 'ef/protect.c', 'ef/safe.c', 'ef/signal.c', 'ef/spawn.c' ]
src += [ 'ef/hashalg.c', 'ef/rbhash.c', 'ef/phq.c', 'ef/trie.c', 'ef/stack.c' ] 
src += [ 'ef/string.c', 'ef/utf8.c', 'ef/ft.c', 'ef/image.c', 'ef/vectorization.c', 'ef/xorg.c' ]
src += [ 'ef/list.c', 'ef/socket.c' ]
src += [ 'ef/json.c' ]
src += [ 'ef/proc.c', 'ef/sysclass.c' ]

##################
# compiler flags #
##################

# warnings
add_global_arguments('-Wall', language: 'c')
add_global_arguments('-Wextra', language: 'c')
add_global_arguments('-Wuninitialized', language: 'c')
add_global_arguments('-Winit-self', language: 'c')
add_global_arguments('-Wstrict-aliasing', language: 'c')
add_global_arguments('-Wstrict-overflow', language: 'c')
add_global_arguments('-Wfloat-equal', language: 'c')
add_global_arguments('-Wvla', language: 'c')

#reentrant malloc
add_global_arguments('-pthread', language: 'c')
add_global_link_arguments('-pthread', language:'c')

# standard
add_global_arguments('-std=gnu11', language: 'c')

# open mp
if get_option('openmp') > 0
  message('openmp enabled')
  add_global_arguments('-fopenmp', language:'c')
  add_global_link_arguments('-fopenmp', language:'c')
  add_global_arguments('-DOMP_ENABLE=1', language: 'c')
endif 

# gprof
if get_option('gprof') > 0
  add_global_arguments('-pg', language:'c')
  add_global_link_arguments('-pg', language:'c')
endif

# optimization
if( get_option('optimize') == 3 )
  message('agressive optimization enabled')
  add_global_arguments('-O3', language: 'c')
  add_global_arguments('-march=native', language: 'c')
  add_global_arguments('-mtune=native', language: 'c')
elif( get_option('optimize') == 2 )
  message('local optimization enabled')
  add_global_arguments('-O2', language: 'c')
  add_global_arguments('-march=native', language: 'c')
  add_global_arguments('-mtune=native', language: 'c')
elif( get_option('optimize') == 1 )
  message('generic optimization enabled')
  add_global_arguments('-O2', language: 'c')
endif

if get_option('autovectorization') > 0
  if( get_option('optimize') < 2 )
	error('required option optimize > 1')
  endif
  message('vectorization enabled')
  add_global_arguments('-ftree-vectorize', language:'c')
  add_global_arguments('-DVECTORIZE=1', language:'c')
endif

##########################
# libraries dependencies #
##########################

math_dep = cc.find_library('m', required : true)
png_dep = cc.find_library('png', required : true)
tcc_dep = cc.find_library('tcc', required: true)
dynamic_dep = cc.find_library('dl', required : true)

libDeps = [ math_dep, png_dep, tcc_dep, dynamic_dep ]

if get_option('G_alsa') > 0 
  alsa_dep = cc.find_library('asound', required: true)
  libDeps += [ alsa_dep ]
endif

ft_dep = dependency('freetype2', required: true)
fconfig_dep = dependency('fontconfig', required: true)

if get_option('G_pulseaudio') > 0 
  pulse_dep = cc.find_library('pulse', required: true)
  libDeps += [ pulse_dep ]
endif

libDeps += [ ft_dep, fconfig_dep ]

#wand_dep = dependency('MagickWand', required: true)
xcb_dep = dependency('xcb', required: true)
xcb_icccm__dep = dependency('xcb-icccm', required: true)
xcb_composite_dep = dependency('xcb-composite', required: true)
xcb_ewmh_dep = dependency('xcb-ewmh', required: true)
xcb_util_dep = dependency('xcb-util', required: true)
xcb_image_dep = dependency('xcb-image', required: true)
xcb_error_dep = dependency('xcb-errors', required: false)
if xcb_error_dep.found()
  message('enable xcb_error')
  add_global_arguments('-DXCB_ERROR_ENABLE', language: 'c')
else
   message('disable xcb_error')
endif
xcb_randr_dep = dependency('xcb-randr', required: true)
xkb_dep = dependency('xkbcommon', required: true)
xkb_x11_dep = dependency('xkbcommon-x11', required: true)
libDeps += [ xcb_dep, xcb_icccm__dep, xcb_ewmh_dep, xcb_util_dep, xcb_image_dep, xkb_dep, xkb_x11_dep, xcb_composite_dep, xcb_error_dep, xcb_randr_dep ]

# template
#m_dep = cc.find_library('m', required : true)
pcre2_dep = cc.find_library('pcre2-8', required : true)
utf8_dep = cc.find_library('unistring', required : true)
libDeps += [ pcre2_dep, utf8_dep ]

#########################
# software dependencies #
#########################

perl = find_program('perl', required: true)
gadget_build = find_program('./gadget/gadget.build', required: true)

#################
# Configuration #
#################

conf_gadget = configuration_data()

l_proto = ''
r_proto = ''
l_vect = ''
r_vect = ''
gen_proto_load = 'int gadget_@0@_load(gadget_s*); '
gen_proto_reg = 'void gadget_@0@_register(vbar_s*); '
gen_vect_load = '{"@0@", gadget_@0@_load},'
gen_vect_reg = 'gadget_@0@_register,'
gen_msg = 'enable @0@ gadget'
gadget_src = [ ]

foreach g : gadget_name
  enabled = get_option('G_@0@'.format(g))
  if( enabled > 0 )
	gsrc = 'gadget/G_@0@.c'.format(g)
	gadget_src += [ gsrc ]
	l_proto += gen_proto_load.format(g)
	r_proto += gen_proto_reg.format(g)
	l_vect += gen_vect_load.format(g)
	r_vect += gen_vect_reg.format(g)
	message(gen_msg.format(g))
  endif
endforeach

conf_gadget.set('GADGET_AUTOGEN_PROTO_LOAD', l_proto)
conf_gadget.set('GADGET_AUTOGEN_VECTOR_LOAD', l_vect)
conf_gadget.set('GADGET_AUTOGEN_PROTO_REGISTER', r_proto)
conf_gadget.set('GADGET_AUTOGEN_VECTOR_REGISTER', r_vect)

configure_file(output: 'gadget_autogen.h', configuration: conf_gadget)

src += gadget_src

#################
# Custom Target #
#################

gadget_target = custom_target('vbarBuildScript',
  output: 'vbarScript.h',
  input: gadget_src,
  command: [ gadget_build, '@OUTPUT@', '@INPUT@' ]
)
src += [ gadget_target ]

#########
# debug #
#########

opt_debug = get_option('ebug')
if opt_debug > 0
  debugstr = 'debug enabled @0@'
  debugstr = debugstr.format(opt_debug)
  message(debugstr)
  debugstr = '-DDEBUG_ENABLE=@0@'
  debugstr = debugstr.format(opt_debug)
  add_global_arguments(debugstr, language: 'c')
  add_global_arguments('-g', language: 'c')
  #add_global_arguments('-ftree-vectorizer-verbose=5', language:'c')
  #add_global_arguments('-fopt-info-loop-optimized', language:'c')
  #add_global_arguments('-fopt-info-vec-optimized', language:'c')
  #add_global_arguments('-fopt-info-vec-missed', language:'c')
endif

if get_option('olor') > 0
  message('color debug enabled')
  add_global_arguments('-DDEBUG_COLOR=1', language: 'c')
endif

if get_option('assert') > 0
  message('assertion enabled')
  add_global_arguments('-DASSERT_ENABLE=1', language: 'c')
endif

###########
# install #
###########

if get_option('homedir') != ''
  message('enable install launch')
  tod = join_paths(get_option('homedir'),'.config/vbar')
  install_data('launch', install_dir: tod, install_mode: ['rwxr-----', get_option('user'), get_option('user')])
endif

meson.add_install_script('meson.extra.install')

###########
# license #
###########

message('copyright vbextreme 2018')
message('released under GPLv3')

#########
# build #
#########

executable(projectName, src, include_directories: includeDir, dependencies: libDeps, install: true)


