# vbar alpha 5
another status bar for i3</br>
</br>
Released under GPL v3

## News
* **alpha 5** fix some bug, add power module</br>
* **alpha 4** fix format, abstract ipc</br>
* **alpha 3** fix bug, add static module</br>
* **alpha 2** fix bug, add datetime module</br>
* **alpha 1** fix bug, complete core</br>
* **alpha 0** begin

## TODO
- [ ] core
	- [X] string
	- [X] memory
	- [X] delay
	- [X] config
	- [X] modules
	- [ ] spawn
		- [ ] add enviroment and argument
	- [ ] vbar
		- [X] call event
		- [ ] custom directory for config file
- [X] ipc
	- [X] i3
- [ ] module
	- [X] cpu
	- [X] memory
	- [X] date
	- [X] static
	- [ ] net
	- [X] power
	- [ ] temperature
	- [ ] cpufreq
	- [ ] pcre?
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
#load cpu module
module.type[0] = cpu
#set path of cpu config file, is optional and this is a default path:
module.path[0] = ~/.config/vbar/cpu/config

#load memory module
module.type[1] = memory
#default path
module.path[1] = ~/.config/vbar/memory/config

#common property
color = 0xFF
background = 0x0
border = 0xFFFFFF
min_width = 300
align = center
separator = 1
separator_block_width = 1
markup = 0
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
