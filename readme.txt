
A SourceAudio C4 Synth pedal controller and manipulator.
Version 25.05.2023 (date of compile)
MM.


Commands: 
--presets                    : List presets from pedal
--presets        <n1>,<n2>   : List presets from n1 to n2
--ctrls                      : List each control and current value of selected (active) preset
--preset         <n>         : Print control values of preset <n>
--getxml         <n>         : Print XML (.pre) from slot <n>
--getxmlall                  : Printout XML of every preset
--bin            <n>         : Print preset binary data
--select         <n>         : Select & make <n> the active working preset
--getactive                  : Print active preset
--reqlist        <n>         : Request from SourceAudio, a list of 5 most recent presets beginning from <n>
--reqpreset      <n>         : Request preset <n>'s .pre from SourceAudio
--reqsave        <n1>,<n2>   : Request preset <n1>, then save to pedal slot <n2>
--reqstore       <n1>,<n2>   : Request preset <n1>, temp store to preset slot <n2>. Use --saveas to save
--reqavailable               : Request total presets available [for download]
--saveas         <n> <str>   : Save active preset with any modification to slot <n> with name "str"
--getname        <n>         : Print name of preset
--activename                 : Print slot number + name of active preset
--next                       : Go to and activate next pedal preset
--previous                   : Go to and activate previous pedal preset
--listctrls                  : Print list recognised controls for use with --get/setctrl
--getctrl        <str>       : Print value of control <str>. Eg; "--getctrl output"
--setctrl        <str> <n>   : Set value of control.  Eg; "--setctrl voice1_pan 66"
--importstore    <str> <n>   : Load preset.pre from local storage (eg; hdd) to pedal slot <n>
--importsave     <str> <n>   : Load preset.pre from local storage to pedal slot <n>
--osbfexport                 : Create a .osbf device backup and send to stdout
--osbfimport     <str>       : Import a device backup and send to pedal
--hwcfg                      : Print hardware configuration
--copy           <n1>,<n2>   : Copy preset from slot <n1> to slot <n2>
--move           <n1>,<n2>   : Copy preset from slot <n1> to slot <n2> then delete preset <n1>
--delete         <n>         : Erase preset
--rename         <n> <str>   : Rename preset <n> to "str"
--isdup          <n1>,<n2>   : Check if preset <n1> is a duplicate of <n2>
--finddup        <n>         : Check if preset is duplicated elsewhere. Excludes name
--finddups                   : Search for duplicated presets. Excludes name
--getflash       <n1>,<n2>   : Print n2 bytes from user Flash, at hex location n1
--dumpflash                  : Print contents of user Flash storage
--dumpeeprom                 : Print contents of user EEPROM
--wait           <n>         : Wait <n> milliseconds before proceeding with next command
--pause                      : Wait for keypress before proceeding with next command
--version                    : Print version info
--help                       : <this>

Usage notes:
Do not enter < or >, just the number or string.
Names and filepaths should be enclosed within ". Eg: sa-c4.exe --osbfimport "my backup.osbf"
Preset range is from 1 to 128: First is 1, last is 128.
Only commands beginning with '--req' will query the SourceAudio website.
sequencerN_steps reports 2 less than actual value. Eg; A value of 14 indicates 16 steps.
Sa-C4 requires exclusive access to the pedal and can not function whilst Neuro Desktop is running.
Use > to caputre output. Eg; sa-c4.exe --osbfexport > "my backup.osbf".
When dumping Flash data, both address and length must be a multiple of 32.
Commands may be serialized.


Usage examples:
 
 List each control and its value for preset 3:
sa-c4.exe --preset 3

 Set preset 25 as working preset, increase sensitivity, lower volume then save as a new preset to slot 26:
sa-c4.exe --select 25 --setctrl input1_gain 250 --setctrl output 55 --saveas 26 "my new preset"

 Copy preset from slot 10 to slot 123:
sa-c4.exe --copy 10,123

 Print complete list of controls usable with --setctrl and --getctrl:
sa-c4.exe --listctrls

 Display first 3 pages of latest downloadable presets:
sa-c4.exe --reqlist 1 --pause --reqlist 6 --pause --reqlist 11 --pause

 Get latest preset from SourceAudio, save to slot 10 then display its name along with input gain, volume and balance levels:
sa-c4.exe --reqsave 1,10 --getname 10 --getctrl input1_gain --getctrl output --getctrl output_balance

 Display hardware config settings then wait for keypress before exiting:
sa-c4.exe --hwcfg --pause



