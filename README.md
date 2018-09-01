# vbar 0.20 alpha
another status bar for i3/dwm</br>
</br>
Released under GPL v3

## News
* **0.20 alpha** fix some bug, add module ipc</br>
* **0.19 alpha** fix some bug, pover intp</br>
* **0.18 alpha** fix some bug, test dwm</br>
* **0.17 alpha** add dwm ipc, need test</br>
* **0.16 alpha** add many intp command, add new themes</br>
* **0.15 alpha** many fix bug, add many intp command, change themes</br>
* **0.14 alpha** fix some bug, optimize intp, add intp module.iconsel command</br>
* **0.13 alpha** add gperf, automatize build, complete optimization search event</br>
* **0.12 alpha** fix some bug, add critic temperature, begin optimization module event and dinamic load</br>
* **0.11 alpha** add option to change main config file, continue develope interpretate events </br>
* **0.10 alpha** add temperature module, modularize build</br>
* **0.9  alpha** add cpufreq module</br>
* **0.8  alpha** separate wireless from network, introduce module hide and force short format</br>
* **0.7  alpha** fix some bug, add event and notify event</br>
* **0.6  alpha** fix some bug, add network module</br>
* **0.5  alpha** fix some bug, add power module</br>
* **0.4  alpha** fix format, abstract ipc</br>
* **0.3  alpha** fix bug, add static module</br>
* **0.2  alpha** fix bug, add datetime module</br>
* **0.1  alpha** fix bug, complete core</br>
* **0.0  alpha** begin

## TODO
- [ ] core
	- [X] string
	- [X] memory
	- [X] delay
	- [X] config
	- [X] modules
		- [X] dinamic
	- [ ] spawn
		- [ ] add enviroment and argument
	- [X] vbar
		- [X] call event
		- [X] custom directory for config file
- [X] ipc
	- [X] i3
	- [X] dwm
	- [ ] spectrwm
- [ ] module
	- [X] cpu
	- [X] memory
	- [X] date
	- [X] static
	- [X] net
	- [X] power
	- [X] temperature
	- [X] cpufreq
	- [X] extern ipc event
	- [ ] ip
	- [ ] meteo
	- [ ] audio
	- [ ] email
	- [ ] kconnect
- [ ] documentation

## How To

### Build and Install
```
$ meson build
$ cd build
$ ninja
$ sudo ninja install
```

### Set Bar
edit ~/.config/i3/config
```
bar {
	status_command vbar
}
```

### Configuration
you need copy a basic config from themes directory on this repository in ~/.config/vbar/config</br>
```
mkdir ~/.config/vbar
cp -r themes/i3/* ~/.config/vbar
```

the file vbar/config is a global configuration file, you can set attribute for all modules and all modules inherit configuration.</br>
on main configuration file you decide order and location of modules

```
#set common property before load modules
color = 0xFF
background = 0x0
border = 0xFFFFFF
min_width = 300
align = center
separator = 1
separator_block_width = 1
markup = 0
hide = 0

#set config path if not use standard path
module.path = ~/.config/vbar/cpu/config
#load cpu module
module.load = cpu

#load memory module
#use default path ~/.config/vbar/memory/config
module.load = memory

```

for each module can overwrite the common property and setting others config</br>
example vbar/cpu/config
```
#enable blink
blink = 1
#speed of blink in ms
blink.time = 450
#blink when cpu is over 90%
blink.on = 90
#refresh time in ms
refresh = 1000
#icon 
icon = "💻"

#formatting output
#$@ icon 
#$0 all core
#$1 core 1
#$2 core 2
#$n etc etc

text.full = "$@ cpu $0%"
text.short = "$@ $0%"

#change format specific output
format[0] = 5.1

#change common property
color = 0x00FF00

#execute application when click module
event = "urxvt -e htop"

```
