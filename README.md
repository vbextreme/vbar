# vbar 1.3
```
┌─┬─┬───────────────────────────────────────────────────────┬───────┬─────┬───────────┬───┐
│1│2│                                                       │💻 0.1%│🌡45°│🔋 80% 3:10│ ⏻ │
├─┴─┴───────────────────────────────────────────────────────┴───────┴─────┴───────────┴───┤
│                                                                                         │
│                                                                                         │
│                                                                                         │
│                            bbbbbbbb                                                     │
│                            b::::::b                                                     │
│                            b::::::b                                                     │
│                            b::::::b                                                     │
│                             b:::::b                                                     │
│   vvvvvvv           vvvvvvv b:::::bbbbbbbbb      aaaaaaaaaaaaa   rrrrr   rrrrrrrrr      │
│    v:::::v         v:::::v  b::::::::::::::bb    a::::::::::::a  r::::rrr:::::::::r     │
│     v:::::v       v:::::v   b::::::::::::::::b   aaaaaaaaa:::::a r:::::::::::::::::r    │
│      v:::::v     v:::::v    b:::::bbbbb:::::::b           a::::a rr::::::rrrrr::::::r   │
│       v:::::v   v:::::v     b:::::b    b::::::b    aaaaaaa:::::a  r:::::r     r:::::r   │
│        v:::::v v:::::v      b:::::b     b:::::b  aa::::::::::::a  r:::::r     rrrrrrr   │
│         v:::::v:::::v       b:::::b     b:::::b a::::aaaa::::::a  r:::::r               │
│          v:::::::::v        b:::::b     b:::::ba::::a    a:::::a  r:::::r               │
│           v:::::::v         b:::::bbbbbb::::::ba::::a    a:::::a  r:::::r               │
│            v:::::v          b::::::::::::::::b a:::::aaaa::::::a  r:::::r               │
│             v:::v           b:::::::::::::::b   a::::::::::aa:::a r:::::r               │
│              vvv            bbbbbbbbbbbbbbbb     aaaaaaaaaa  aaaa rrrrrrr               │
│                                                                                         │
│                                                                                         │
│                                                                                         │
└─────────────────────────────────────────────────────────────────────────────────────────┘
```

another status bar for i3/dwm</br>
</br>
Released under GPL v3</br>

## How To
 see [wiki page](https://github.com/vbextreme/vbar/wiki/Home)

### Build and Install
 see [wiki page](https://github.com/vbextreme/vbar/wiki/Build)

### Configuration
 see [wiki page](https://github.com/vbextreme/vbar/wiki/Configure)

## News
* **1.3**  extend cpufreq</br>
* **1.2**  new blink method</br>
* **1.1**  fix some bug, add crash protect</br>
* **1.0**  first stable release </br>
* **0.24** dinamic file thermal</br>
* **0.23** fix bug theme and remove module</br>
* **0.22** fix some bug, separate file op</br>
* **0.21** fix many bug, add script for test</br>
* **0.20** fix some bug, add module ipc</br>
* **0.19** fix some bug, pover intp</br>
* **0.18** fix some bug, test dwm</br>
* **0.17** add dwm ipc, need test</br>
* **0.16** add many intp command, add new themes</br>
* **0.15** many fix bug, add many intp command, change themes</br>
* **0.14** fix some bug, optimize intp, add intp module.iconsel command</br>
* **0.13** add gperf, automatize build, complete optimization search event</br>
* **0.12** fix some bug, add critic temperature, begin optimization module event and dinamic load</br>
* **0.11** add option to change main config file, continue develope interpretate events </br>
* **0.10** add temperature module, modularize build</br>
* **0.9**  add cpufreq module</br>
* **0.8**  separate wireless from network, introduce module hide and force short format</br>
* **0.7**  fix some bug, add event and notify event</br>
* **0.6**  fix some bug, add network module</br>
* **0.5**  fix some bug, add power module</br>
* **0.4**  fix format, abstract ipc</br>
* **0.3**  fix bug, add static module</br>
* **0.2**  fix bug, add datetime module</br>
* **0.1**  fix bug, complete core</br>
* **0.0**  begin

## Release Schedule
- [X] v1.0
	- [X] core
		- [X] string
		- [X] memory
		- [X] delay
		- [X] config
		- [X] modules
		- [X] spawn
		- [X] vbar
			- [X] call event
			- [X] custom directory for config file
		- [X] file
	- [X] ipc
		- [X] i3
		- [X] dwm
	- [X] modules
		- [X] cpu
		- [X] memory
		- [X] date
		- [X] static
		- [X] net
		- [X] power
		- [X] temperature
		- [X] cpufreq
		- [X] extern ipc event
	- [X] documentation
		- [X] main
		- [X] build
		- [X] configure
		- [X] wm
		- [X] module
		- [X] cpu
		- [X] cpufreq
		- [X] memory
		- [X] datetime
		- [X] static
		- [X] power
		- [X] network
		- [X] wireless
		- [X] temperature
		- [X] ipc
		- [X] troubleshoot
		- [X] sidebar
	- [X] beta
		- [X] valgrind
		- [X] scan-build
		- [X] tester
- [ ] v2.0
	- [X] core
		- [X] crash protect
	- [ ] ipc
		- [X] more type blink
		- [ ] dwm color
		- [ ] spectrwm
		- [ ] lemonbar
	- [ ] modules
		- [ ] ip
		- [ ] meteo
		- [ ] audio
		- [ ] email
		- [ ] bluetooth
	- [ ] documentation
		- [ ] ip
		- [ ] meteo
		- [ ] audio
		- [ ] email
		- [ ] bluetooth
		- [ ] lemonbar
		- [ ] dwm color
		- [ ] spectrwm
		- [X] blink mode
		- [ ] make module
	- [ ] beta
		- [ ] valgrind
		- [ ] scan-build
		- [ ] splint
		- [ ] tester
- [ ] v3.0
	- [ ] modules
		- [ ] kconnect
	- [ ] documentation
		- [ ] kconnect
	- [ ] beta
		- [ ] valgrind
		- [ ] scan-build
		- [ ] splint
		- [ ] tester
- [ ] v4.0
	- [ ] ipc
		- [ ] custom bar
	- [ ] tools
		- [ ] fork i3bar
		- [ ] fork i3-input
		- [ ] fork i3-nagbar
	- [ ] documentation
		- [ ] custom bar
	- [ ] beta
		- [ ] valgrind
		- [ ] scan-build
		- [ ] splint
		- [ ] tester
- [ ] v5.0
	- [ ] custom bar
		- [ ] integrated menu
	- [ ] documentation
		- [ ] custom bar
	- [ ] beta
		- [ ] valgrind
		- [ ] scan-build
		- [ ] splint
		- [ ] tester
