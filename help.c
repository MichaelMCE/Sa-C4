#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>


#include "as.h"
#include "libas.h"



void cmd_printHelp ()
{
	const char *help = 
	"\n"
	"\nCommands:" 
	"\n--presets                    : Display all presets"
	"\n--presets        <n1>,<n2>   : List presets from n1 to n2"
	"\n--ctrls                      : List each control and current value of active preset"
	"\n--preset         <n>         : Print control values of preset <n>"
	"\n--getxml         <n>         : Print XML (.pre) from slot <n>"
	"\n--getxmlall                  : Printout XML of every preset"
	"\n--bin            <n>         : Print preset binary data"
	"\n--select         <n>         : Select & make <n> the active working preset"
	"\n--getactive                  : Display active preset"
	"\n--reqlist        <n>         : Request from SourceAudio, a list of 5 most recent presets beginning from <n>"
	"\n--reqpreset      <n>         : Request preset <n>'s .pre from SourceAudio"
	"\n--reqsave        <n1>,<n2>   : Request preset <n1>, then save to pedal slot <n2>"
	"\n--reqstore       <n1>,<n2>   : Request preset <n1>, temp store to preset slot <n2>. Use --saveas to save"
	"\n--reqavailable               : Request total presets available [for download]"
	"\n--saveas         <n> <str>   : Save active preset with any modification to slot <n> with name \"str\""
	"\n--getname        <n>         : Print name of preset <n>"
	"\n--activename                 : Display slot + name of active preset"
	"\n--next                       : Go to and activate next pedal preset"
	"\n--previous                   : Go to and activate previous pedal preset"
	"\n--listctrls                  : Print list recognised controls for use with --get/setctrl"
	"\n--getctrl        <str>       : Print value of control <str>. Eg; \"--getctrl output\""
	"\n--setctrl        <str> <n>   : Set value of control.  Eg; \"--setctrl voice1_pan 66\""
	"\n--importstore    <str> <n>   : Load preset.pre from local storage (eg; hdd) to pedal slot <n>"
	"\n--importsave     <str> <n>   : Load preset.pre from local storage to pedal slot <n>"
	"\n--osbfexport                 : Create a .osbf device backup and send to stdout"
	"\n--osbfimport     <str>       : Import a device backup and send to pedal"
	"\n--hwcfg                      : Print hardware configuration"
	"\n--copy           <n1>,<n2>   : Copy preset from slot <n1> to slot <n2>"
	"\n--move           <n1>,<n2>   : Copy preset from slot <n1> to slot <n2> then delete preset <n1>"
	"\n--delete         <n>         : Erase preset"
	"\n--rename         <n> <str>   : Rename preset <n> to \"str\""
	"\n--isdup          <n1>,<n2>   : Check if preset <n1> is a duplicate of <n2>"
	"\n--finddup        <n>         : Check if preset is duplicated elsewhere. Excludes name"
	"\n--finddups                   : Search for duplicated presets. Excludes name"
	"\n--getflash       <n1>,<n2>   : Print n2 bytes from user Flash, at hex location n1"
	"\n--dumpflash                  : Display contents of user Flash storage"
	"\n--dumpeeprom                 : Display contents of user EEPROM"
	"\n--wait           <n>         : Wait <n> milliseconds before proceeding with next command"
	"\n--pause                      : Wait for keypress before proceeding with next command"
	"\n--version                    : Print version info"
	"\n--help                       : <This>"
	"\n"
	"\nNotes:"
	"\nDo not enter < or >, just the number or string."
	"\nNames and filepaths should be enclosed within \". Eg: sa-c4.exe --osbfimport \"my backup.osbf\""
	"\nPreset range is from 1 to 128: First is 1, last is 128."
	"\nOnly commands beginning with '--req' will query the SourceAudio website."
	"\nsequencerN_steps reports 2 less than actual value. Eg; A value of 14 indicates 16 steps."
	"\nSa-C4 requires exclusive access to the pedal and can not function whilst Neuro Desktop is running."
	"\nUse > to caputre output. Eg: sa-c4.exe --osbfexport > \"my backup.osbf\"."
	"\nWhen dumping Flash data, both address and length must be a multiple of 32."
	"\n";
	
	printf(help);
}



