/*
**  Copyright (C) 2021 Anthony Buckley
** 
**  This file is part of StarsAl.
** 
**  StarsAl is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**  
**  StarsAl is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**  
**  You should have received a copy of the GNU General Public License
**  along with StarsAl.  If not, see <http://www.gnu.org/licenses/>.
*/



/*
** Description:
**  Screen appearance setup
**
** Author:	Anthony Buckley
**
** History
**	01-Jul-2021	Initial code
**
*/


/* Defines */

#define SD_W 1600
#define SD_H 900
#define SD_SZ 3


/* Includes */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>  
#include <gdk/gdk.h> 


/* Prototypes */

void set_css();


/* Globals */

static const char *debug_hdr = "DEBUG-css.c ";

/*
** Colour guide
**		#800000 : Deep red or maroon
**		#400080 : Purple with shade of blue
**		#8a87c4 : Medium mauve blue
**		#fa8072 : Medium salmon
**		#e00b40 : Crimson
**		#b8860b : Dark golden yellow
**		#e6e6fa : Light bluey grey
**		#ebe7f8 : Light purple grey
**		#708090 : Gunmetal grey
**		#fbf6fe : Light mauve
**		#daf5d0 : Light green
**		#f8fcca : Light yellow
**		#cbd8fb : Light blue
**		#ffd6d6 : Light red
**		#000000 : Light black
*/

static char *css_data_fhd = 
	"@define-color DARK_BLUE rgba(0%,0%,50%,1.0); "
	"@define-color METAL_GREY rgba(55,83,103,1.0); "
	"@define-color WHITE1 rgba(255,255,255,1.0); "
	"button, entry, label { font-family: Sans; font-size: 12px; }"
	"label#data_1 { color: @DARK_BLUE; }"
	"label#data_2 { color: #800000; font-family: Sans; font-size: 11px; }"
	"label#data_2a { color: #800000; font-family: Sans; font-size: 15px; }"
	"label#data_3 { color: #400080; font-family: Sans; font-size: 10px; }"
	"label#data_4 { font-family: Sans; font-size: 11px; }"
	"label#data_5 { font-family: Sans; font-size: 14px; background-color: #708090; color: @WHITE1;}"
	"label#head_1 { color: #8a87c4; font-family: Sans; font-size: 11px; }"
	"label#title_1 { font-family: Sans; font-size: 18px; font-weight: bold; }"
	"label#title_2 { font-family: Serif; font-size: 18px; font-style: italic; color: #fa8072; }"
	"label#title_3 { font-family: Sans; font-size: 12px; color: @DARK_BLUE;}"
	"label#title_3a { font-family: Sans; font-size: 9px; color: @DARK_BLUE;}"
	"label#title_3b { font-family: Sans; font-size: 15px; color: @DARK_BLUE;}"
	"label#title_4 { font-family: Sans; font-size: 12px; font-weight: bold; }"
	"label#title_4a { font-family: Sans; font-size: 9px; font-weight: bold; }"
	"label#title_5 { font-family: Sans; font-size: 12px; color: #e00b40;}"
	"label#status { font-family: Sans; font-size: 12px; color: #b8860b; font-style: italic; }"
	"entry#ent_1 { color: @DARK_BLUE; }"
	"radiobutton#rad_1 { color: @DARK_BLUE; font-family: Sans; font-size: 12px; }"
	"radiobutton > label { color: @DARK_BLUE; font-family: Sans; font-size: 12px; }"
	"frame { background-color: #e6e6fa; border-radius: 8px}"
	"GtkFrame#clr1 { background-color: #ebe7f8; border-radius: 8px}"
	"frame > label { color: #800000; font-weight: 500; }"
	"combobox * { color: @METAL_GREY; font-family: Sans; font-size: 12px; }"
	"progressbar#pbar_1 { color: @DARK_BLUE; font-family: Sans; font-size: 10px; }"
	"#button_1 * { color: #708090; font-weight: bold; }"
	"#list_col_1 { font-family: Sans; font-size: 10px; font-weight: bold; }"
	"GtkListBox { background-color: #fbf6fe; }"
	"notebook * { font-family: Sans; font-size: 11px; }"
	"textview { font-family: Sans; font-size: 12px; }"
	"textview#txtview_1 * { font-family: Sans; font-size: 12px; background-color: #708090; color: @WHITE1; }"
	"box#box_1 { background-color: #708090; }"
	"box#box_2 { background-color: #000000; }"
	"box#btnbx_1 { background-color: #daf5d0; }"
	"box#btnbx_2 { background-color: #f8fcca; }"
	"box#btnbx_3 { background-color: #cbd8fb; }"
	"box#btnbx_4 { background-color: #ffd6d6; }"
	"button.link { font-family: Sans; font-size: 12px; color: @DARK_BLUE; }";


// These don't work
//"GtkLabel#title_deco1 { font: Comic Sans 15; font-weight: 500; color: #fa8072 }"
//"GtkLabel#title_deco2 { font-family: Comic Sans; font-size: 15px; font-style: italic; color: #fa8072 }"


/* Set up provider data and apply */

void set_css()
{
    int sd_flg;
    GError *err = NULL;

    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);

    gtk_style_context_add_provider_for_screen(screen,
    					      GTK_STYLE_PROVIDER(provider),
    					      GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(provider),
				    (const gchar *) css_data_fhd,
				    -1,
				    &err);

    if (err != NULL)
    {
    	printf("%s set_css  ****css error  %s\n", debug_hdr, err->message); fflush(stdout);
    	g_clear_error (&err);
    }

    g_object_unref(provider);

    return;
}
