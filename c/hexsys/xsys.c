/*
 * xsys.c - main functions for X-Sys 2
 * by mikeshoup
 * Copyright (C) 2003, 2004, 2005 Michael Shoup
 * Copyright (C) 2005, 2006, 2007 Tony Vroon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xchat-plugin.h"
#include "parse.h"
#include "match.h"
#include "xsys.h"

static xchat_plugin *ph;

static char format[bsize] = "%B%1%B[%2]";
unsigned int percentages = 1;

static void load_config();
static void save_config();

static int format_cb		(char *word[], char *word_eol[], void *userdata);
static int percentages_cb	(char *word[], char *word_eol[], void *userdata);
static int sysinfo_cb		(char *word[], char *word_eol[], void *userdata);
static int xsys_cb		(char *word[], char *word_eol[], void *userdata);
static int cpuinfo_cb		(char *word[], char *word_eol[], void *userdata);
static int uptime_cb		(char *word[], char *word_eol[], void *userdata);
static int osinfo_cb		(char *word[], char *word_eol[], void *userdata);
static int sound_cb		(char *word[], char *word_eol[], void *userdata);
static int netdata_cb		(char *word[], char *word_eol[], void *userdata);
static int netstream_cb		(char *word[], char *word_eol[], void *userdata);
static int disk_cb		(char *word[], char *word_eol[], void *userdata);
static int mem_cb		(char *word[], char *word_eol[], void *userdata);
static int video_cb		(char *word[], char *word_eol[], void *userdata);
static int ether_cb		(char *word[], char *word_eol[], void *userdata);
static int distro_cb		(char *word[], char *word_eol[], void *userdata);
#if 0
static int hwmon_cb		(char *word[], char *word_eol[], void *userdata);
#endif

int xchat_plugin_init(xchat_plugin *plugin_handle, char **plugin_name,
                      char **plugin_desc, char **plugin_version, char *arg)
{
	ph = plugin_handle;
	*plugin_name    = "HexSys";
	*plugin_desc    = "A sysinfo plugin";
	*plugin_version = VER_STRING;

	xchat_hook_command(ph, "HEXSYSFORMAT",XCHAT_PRI_NORM, format_cb,    NULL, NULL);
	xchat_hook_command(ph, "PERCENTAGES",XCHAT_PRI_NORM, percentages_cb,   NULL, NULL);
	xchat_hook_command(ph, "SYSINFO",    XCHAT_PRI_NORM, sysinfo_cb,   NULL, (void *) 0);
	xchat_hook_command(ph, "HEXSYS",       XCHAT_PRI_NORM, xsys_cb,      NULL, (void *) 0);
	xchat_hook_command(ph, "CPUINFO",    XCHAT_PRI_NORM, cpuinfo_cb,   NULL, (void *) 0);
	xchat_hook_command(ph, "SYSUPTIME",  XCHAT_PRI_NORM, uptime_cb,    NULL, (void *) 0);
	xchat_hook_command(ph, "OSINFO",     XCHAT_PRI_NORM, osinfo_cb,    NULL, (void *) 0);
	xchat_hook_command(ph, "SOUND",      XCHAT_PRI_NORM, sound_cb,     NULL, (void *) 0);
	xchat_hook_command(ph, "NETDATA",    XCHAT_PRI_NORM, netdata_cb,   NULL, (void *) 0);
	xchat_hook_command(ph, "NETSTREAM",  XCHAT_PRI_NORM, netstream_cb, NULL, (void *) 0);
	xchat_hook_command(ph, "DISKINFO",   XCHAT_PRI_NORM, disk_cb,      NULL, (void *) 0);
	xchat_hook_command(ph, "MEMINFO",    XCHAT_PRI_NORM, mem_cb,       NULL, (void *) 0);
	xchat_hook_command(ph, "VIDEO",      XCHAT_PRI_NORM, video_cb,     NULL, (void *) 0);
	xchat_hook_command(ph, "ETHER",      XCHAT_PRI_NORM, ether_cb,     NULL, (void *) 0);
	xchat_hook_command(ph, "DISTRO",     XCHAT_PRI_NORM, distro_cb,    NULL, (void *) 0);
#if 0
	xchat_hook_command(ph, "HWMON",      XCHAT_PRI_NORM, hwmon_cb,     NULL, (void *) 0);
#endif
	load_config();

	xchat_printf(ph, "HexSys %s Loaded Succesfully", VER_STRING);
	
	return 1;
}

static void save_config()
{
    xchat_pluginpref_set_str(ph, "format", format);
    xchat_pluginpref_set_int(ph, "percentages", percentages);
	return;
}

static void load_config()
{
    xchat_pluginpref_get_str(ph, "format", format);
    percentages = xchat_pluginpref_get_int(ph, "percentages");
}

static int format_cb(char *word[], char *word_eol[], void *userdata)
{
	if(*(word[2]) == '\0')
		xchat_printf(ph, "Current format string:\n%s", format);
	else
	{
		strncpy(format, word_eol[2], bsize);
		save_config();
	}
	return XCHAT_EAT_ALL;
}

static int percentages_cb(char *word[], char *word_eol[], void *userdata)
{
	if(*(word[2]) == '\0')
		if (percentages != 0)
			xchat_printf(ph, "Percentages currently enabled");
		else
			xchat_printf(ph, "Percentages currently disabled");		
	else
	{
		percentages = atoi(word[2]);
		save_config();
	}
	return XCHAT_EAT_ALL;
}

static int sysinfo_cb(char *word[], char *word_eol[], void *userdata)
{
	char sysinfo[bsize], buffer[bsize], cpu_model[bsize], cpu_cache[bsize], cpu_vendor[bsize];
	char os_host[bsize], os_user[bsize], os_kernel[bsize];
	unsigned long long mem_total, mem_free;
	unsigned int count;
	double cpu_freq;
	int giga = 0;
	sysinfo[0] = '\0';

// BEGIN OS PARSING
	if(xs_parse_os(os_user, os_host, os_kernel) != 0)
	{
		xchat_printf(ph, "ERROR in parse_os()");
		return XCHAT_EAT_ALL;
	}

	snprintf(buffer, bsize, "%s", os_kernel);
	format_output("os", buffer, format);
	strncat(sysinfo, buffer, bsize-strlen(sysinfo));

// BEGIN DISTRO PARSING
        if(xs_parse_distro(buffer) != 0)
		strncpy(buffer, "Unknown", bsize);
	format_output("distro", buffer, format);
	strcat(sysinfo, "\017 ");
	strncat(sysinfo, buffer, bsize-strlen(sysinfo));	
	
// BEGIN CPU PARSING
	if(xs_parse_cpu(cpu_model, cpu_vendor, &cpu_freq, cpu_cache, &count) != 0)
	{
		xchat_printf(ph, "ERROR in parse_cpu()");
		return XCHAT_EAT_ALL;
	}
	
	if(cpu_freq > 1000)
	{
		cpu_freq /= 1000;
		giga = 1;
	}
	
	if(giga) snprintf(buffer, bsize, "%d x %s (%s) @ %.2fGHz", count, cpu_model, cpu_vendor, cpu_freq);
	else snprintf(buffer, bsize, "%d x %s (%s) @ %.0fMHz", count, cpu_model, cpu_vendor, cpu_freq);
	format_output("cpu", buffer, format);
	strcat(sysinfo, "\017 ");
	strncat(sysinfo, buffer, bsize-strlen(sysinfo));

// BEGIN MEMORY PARSING
	if(xs_parse_meminfo(&mem_total, &mem_free, 0) == 1)
	{
		xchat_printf(ph, "ERROR in parse_meminfo!");
		return XCHAT_EAT_ALL;
	}
	snprintf(buffer, bsize, "%s", pretty_freespace("Physical", &mem_free, &mem_total));
	format_output("mem", buffer, format);	
	strcat(sysinfo, "\017 ");
	strncat(sysinfo, buffer, bsize-strlen(sysinfo));
	
// BEGIN DISK PARSING
	if(xs_parse_df(NULL, buffer))
	{
		xchat_printf(ph, "ERROR in parse_df");
		return XCHAT_EAT_ALL;
	}
	format_output("disk", buffer, format);
	strcat(sysinfo, "\017 ");
	strncat(sysinfo, buffer, bsize-strlen(buffer));
	
//BEGIN VIDEO PARSING
	if(xs_parse_video(buffer))
	{
		xchat_printf(ph, "ERROR in parse_video");
		return XCHAT_EAT_ALL;
	}
	format_output("video", buffer, format);
	strcat(sysinfo, "\017 ");
	strncat(sysinfo, buffer, bsize-strlen(buffer));
//BEGIN SOUND PARSING
	if(xs_parse_sound(buffer))
		strncpy(buffer, "Not present", bsize);
	format_output("sound", buffer, format);
	strcat(sysinfo, "\017 ");
	strncat(sysinfo, buffer, bsize-strlen(buffer));
	
	if((long)userdata)
		xchat_printf(ph, "%s", sysinfo);
	else
		xchat_commandf(ph, "say %s", sysinfo);	
	
	return XCHAT_EAT_ALL;
}

static int xsys_cb(char *word[], char *word_eol[], void *userdata)
{
	if((long)userdata)
		xchat_printf(ph, "You are using HexSys v%s", VER_STRING);
	else
		xchat_commandf(ph, "me is using HexSys v%s", VER_STRING);
	
	return XCHAT_EAT_ALL;
}

static int cpuinfo_cb(char *word[], char *word_eol[], void *userdata)
{
	char model[bsize], vendor[bsize], cache[bsize], buffer[bsize];
	unsigned int count;
	double freq;
	int giga = 0;
	
	if(xs_parse_cpu(model, vendor, &freq, cache, &count) != 0)
	{
		xchat_printf(ph, "ERROR in parse_cpu()");
		return XCHAT_EAT_ALL;
	}
	
	if(freq > 1000)
	{
		freq /= 1000;
		giga = 1;
	}
	
	if(giga) snprintf(buffer, bsize, "%d x %s (%s) @ %.2fGHz w/ %s L2 Cache", count, model, vendor, freq, cache);
	else snprintf(buffer, bsize, "%d x %s (%s) @ %.0fMHz w/ %s L2 Cache", count, model, vendor, freq, cache);
	
	format_output("cpu", buffer, format);
	
	if((long)userdata)
		xchat_printf(ph, "%s", buffer);
	else
		xchat_commandf(ph, "say %s", buffer);
	
	return XCHAT_EAT_ALL;
}

static int uptime_cb(char *word[], char *word_eol[], void *userdata)
{
	char buffer[bsize];
	int weeks, days, hours, minutes, seconds;
	
	if(xs_parse_uptime(&weeks, &days, &hours, &minutes, &seconds))
	{
		xchat_printf(ph, "ERROR in parse_uptime()");
		return XCHAT_EAT_ALL;
	}
	
	if( minutes != 0 || hours != 0 || days != 0 || weeks != 0 )
	{
		if( hours != 0 || days != 0 || weeks != 0 )
		{
			if( days  !=0 || weeks != 0 )
			{
                                if( weeks != 0 )
                                {
                                        snprintf( buffer, bsize, "%dw %dd %dh %dm %ds",
                                                  weeks, days, hours, minutes, seconds );
                                }
                                else
                                {
                                        snprintf( buffer, bsize, "%dd %dh %dm %ds",
                                                  days, hours, minutes, seconds );
                                }
                        }
                        else
                        {
                                snprintf( buffer, bsize, "%dh %dm %ds",
                                          hours, minutes, seconds );
                        }
                }
                else
                {
                        snprintf( buffer, bsize, "%dm %ds", minutes, seconds );
                }
        }

	format_output("uptime", buffer, format);
	
	if((long)userdata)
		xchat_printf(ph, "%s", buffer);
	else
		xchat_commandf(ph, "say %s", buffer);
	
	return XCHAT_EAT_ALL;
}

static int osinfo_cb(char *word[], char *word_eol[], void *userdata)
{
	char buffer[bsize], user[bsize], host[bsize], kernel[bsize];
	
	if(xs_parse_os(user, host, kernel) != 0)
	{
		xchat_printf(ph, "ERROR in parse_os()");
		return XCHAT_EAT_ALL;
	}

	snprintf(buffer, bsize, "%s@%s, %s", user, host, kernel);
	format_output("os", buffer, format);
	
	if((long)userdata)
		xchat_printf(ph, "%s", buffer);
	else
		xchat_commandf(ph, "say %s", buffer);
	
	return XCHAT_EAT_ALL;
}

static int sound_cb(char *word[], char *world_eol[], void *userdata)
{
	char sound[bsize];
	if(xs_parse_sound(sound) != 0)
	{
		xchat_printf(ph, "ERROR in parse_asound()!");
		return XCHAT_EAT_ALL;
	}
	
	format_output("sound", sound, format);
	
	if((long)userdata)
		xchat_printf(ph, "%s", sound);
	else
		xchat_commandf(ph, "say %s", sound);
	
	return XCHAT_EAT_ALL;
}

static int distro_cb(char *word[], char *word_eol[], void *userdata)
{
	char name[bsize];
	if(xs_parse_distro(name) != 0)
	{
		xchat_printf(ph, "ERROR in parse_distro()!");
		return XCHAT_EAT_ALL;
	}
	
	format_output("distro", name, format);
	if((long)userdata)
		xchat_printf(ph, "%s", name);
	else
		xchat_commandf(ph, "say %s", name);
	return XCHAT_EAT_ALL;
}	

static int netdata_cb(char *word[], char *word_eol[], void *userdata)
{
	char netdata[bsize];
	unsigned long long bytes_recv, bytes_sent;
	
	if(*word[2] == '\0')
	{
		xchat_printf(ph, "You must specify a network device! (eg.: /netdata eth0)");
		return XCHAT_EAT_ALL;
	}
	
	if(xs_parse_netdev(word[2], &bytes_recv, &bytes_sent) != 0)
	{
		xchat_printf(ph, "ERROR in parse_netdev");
		return XCHAT_EAT_ALL;
	}
	
	bytes_recv /= 1024;
	bytes_sent /= 1024;
	
	snprintf(netdata, bsize, "%s: %.1f MB Recieved, %.1f MB Sent", word[2], (double)bytes_recv/1024.0, (double)bytes_sent/1024.0);
	format_output("netdata", netdata, format);
	if((long)userdata)
		xchat_printf(ph, "%s", netdata);
	else
		xchat_commandf(ph, "say %s", netdata);
		
	return XCHAT_EAT_ALL;
}

static int netstream_cb(char *word[], char *word_eol[], void *userdata)
{
	char netstream[bsize], mag_r[3], mag_s[5];
	unsigned long long bytes_recv, bytes_sent, bytes_recv_p, bytes_sent_p;
	struct timespec ts = {1, 0};
	

	if(*word[2] == '\0')
	{
		xchat_printf(ph, "You must specify a network device! (eg.: /netstream eth0)");
		return XCHAT_EAT_ALL;
	}
	
	if(xs_parse_netdev(word[2], &bytes_recv, &bytes_sent) != 0)
	{
		xchat_printf(ph, "ERROR in parse_netdev");
		return XCHAT_EAT_ALL;
	}
	
	while(nanosleep(&ts, &ts) < 0);
	
	if(xs_parse_netdev(word[2], &bytes_recv_p, &bytes_sent_p) != 0)
	{
		xchat_printf(ph, "ERROR in parse_netdev");
		return XCHAT_EAT_ALL;
	}
	
	bytes_recv = (bytes_recv_p - bytes_recv);
	bytes_sent = (bytes_sent_p - bytes_sent);
	
	if(bytes_recv>1024)
	{
		bytes_recv /= 1024;
		snprintf(mag_r, 5, "KB/s");
	}
	else snprintf(mag_r, 5, "B/s");
	
	if(bytes_sent>1024)
	{
		bytes_sent /= 1024;
		snprintf(mag_s, 5, "KB/s");
	}
	else snprintf(mag_s, 5, "B/s");
	
	snprintf(netstream, bsize, "%s: Receiving %llu %s, Sending %llu %s", word[2], bytes_recv, mag_r, bytes_sent, mag_s);
	format_output("netstream", netstream, format);
	if((long)userdata)
		xchat_printf(ph, "%s", netstream);
	else
		xchat_commandf(ph, "say %s", netstream);
	
	return XCHAT_EAT_ALL;
}

static int disk_cb(char *word[], char *word_eol[], void *userdata)
{
	char string[bsize] = {0,};
	
	if(*word[2] == '\0')
	{
		if(xs_parse_df(NULL, string))
		{
			xchat_printf(ph, "ERROR in parse_df");
			return XCHAT_EAT_ALL;
		}
	}
	else
	{
		if(xs_parse_df(word[2], string))
		{
			xchat_printf(ph, "ERROR in parse_df");
			return XCHAT_EAT_ALL;
		}
	}
	
	format_output("disk", string, format);
	
	if((long)userdata)
		xchat_printf(ph, "%s", string);
	else
		xchat_commandf(ph, "say %s", string);
	
	return XCHAT_EAT_ALL;
}

static int mem_cb(char *word[], char *word_eol[], void *userdata)
{
	unsigned long long mem_total, mem_free, swap_total, swap_free;
	char string[bsize];
	
	if(xs_parse_meminfo(&mem_total, &mem_free, 0) == 1)
	{
		xchat_printf(ph, "ERROR in parse_meminfo!");
		return XCHAT_EAT_ALL;
	}
	if(xs_parse_meminfo(&swap_total, &swap_free, 1) == 1)
	{
		xchat_printf(ph, "ERROR in parse_meminfo!");
		return XCHAT_EAT_ALL;
	}

	snprintf(string, bsize, "%s - %s", pretty_freespace("Physical", &mem_free, &mem_total),
		 pretty_freespace("Swap", &swap_free, &swap_total));
	format_output("mem", string, format);
	
	if((long)userdata)
		xchat_printf(ph, "%s", string);
	else
		xchat_commandf(ph, "say %s", string);
	
	return XCHAT_EAT_ALL;
}

static int video_cb(char *word[], char *word_eol[], void *userdata)
{
	char vid_card[bsize], agp_bridge[bsize], buffer[bsize];
	int ret;
	if((ret = xs_parse_video(vid_card)) != 0)
	{
		xchat_printf(ph, "ERROR in parse_video! %d", ret);
		return XCHAT_EAT_ALL;
	}
	
	if(xs_parse_agpbridge(agp_bridge) != 0)
		snprintf(buffer, bsize, "%s", vid_card);
	else
		snprintf(buffer, bsize, "%s @ %s", vid_card, agp_bridge);
	
	format_output("video", buffer, format);
	if((long)userdata)
		xchat_printf(ph, "%s", buffer);
	else
		xchat_commandf(ph, "say %s", buffer);
	
	return XCHAT_EAT_ALL;
}

static int ether_cb(char *word[], char *word_eol[], void *userdata)
{
	char ethernet_card[bsize];
	if(xs_parse_ether(ethernet_card))
		strncpy(ethernet_card, "None found", bsize);
	format_output("ether", ethernet_card, format);
	if((long)userdata)
		xchat_printf(ph, "%s", ethernet_card);
	else
		xchat_commandf(ph, "say %s", ethernet_card);
	
	return XCHAT_EAT_ALL;
}

#if 0
static int hwmon_cb(char *word[], char *word_eol[], void *userdata)
{
	char chip[bsize], buffer[bsize];
	
	if(xs_parse_hwmon_chip(chip))
	{
		xchat_printf(ph, "ERROR No hardware monitoring support found");
		return XCHAT_EAT_ALL;
	}
	format_output("sensor", chip, format);
	strcat(chip, "\017 ");

	xs_parse_hwmon_temp(buffer, 1);
	format_output("temp1", buffer, format);
	strncat(chip, buffer, bsize);
	
	if((long)userdata)
		xchat_printf(ph, "%s", chip);
	else
		xchat_commandf(ph, "say %s", chip);
	
	return XCHAT_EAT_ALL;
}
#endif
