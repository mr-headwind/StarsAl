/*
**  Copyright (C) 2020 Anthony Buckley
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
** Description:	Main program include file
**
** Author:	Anthony Buckley
**
** History
**	10-Jun-2020	Initial
**
*/



/* Includes */

#include <project.h>


/* Defines */

#ifndef MAIN_HDR
#define MAIN_HDR
#endif

/* Structure to contain main interface items for easy access */

typedef struct _main_ui
{
    /* Main view widgets */
    GtkWidget *window;
    GtkWidget *cntl_box, *app_vbox;  
    GtkWidget *status_info;  

    /* Menu items */
    GtkWidget *menu_bar;  
    GtkWidget *file_menu, *edit_menu, *opt_menu, *help_menu;
    GtkWidget *file_hdr, *edit_hdr, *opt_hdr, *help_hdr;
    GtkWidget *new_proj, *open_proj, *close_proj, *file_exit;
    GtkWidget *edit_proj, *opt_prefs;
    GtkWidget *view_log, *help_about;
    GtkWidget *sep, *sep2;
    GtkAccelGroup *accel_group;

    /* Tool bars */

    /* Application widgets */
    GtkWidget *algn_vbox, *proc_hbox;  
    GtkWidget *image_area, *i_scroll_win;  
    GtkWidget *list_box, *l_scroll_win; 
    GtkWidget *heading_lbl;  
    GtkWidget *dummy;  

    /* Control widgets and items */
    GtkWidget *save_btn;
    GtkWidget *close_btn;

    /* Callback Handlers */
    int close_hndlr_id;

    /* Other */
    ProjectData *proj;
} MainUi;
