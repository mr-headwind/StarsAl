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
** Description:	Main program include file
**
** Author:	Anthony Buckley
**
** History
**	xx-xxx-2021	Initial
**
*/




/* Defines */

#ifndef MAIN_HDR
#define MAIN_HDR
#endif

/* Structure to contain main interface items for easy access */

typedef struct _main_ui
{
    /* Main view widgets */
    GtkWidget *window;
    GtkWidget *cntl_box, *app_box;  
    GtkWidget *status_info;  

    /* Menu items */
    GtkWidget *menu_bar;  
    GtkWidget *file_menu, *edit_menu, *help_menu;
    GtkWidget *file_hdr, *edit_hdr, *help_hdr;
    GtkWidget *file_exit;
    GtkWidget *edit_prefs;
    GtkWidget *view_log, *help_about;
    GtkWidget *sep, *sep2;
    GtkAccelGroup *accel_group;

    /* Tool bars */

    /* Application widgets */
    GtkWidget *dummy;  

    /* Control widgets and items */
    GtkWidget *save_btn;
    GtkWidget *close_btn;

    /* Callback Handlers */
    int close_hndlr_id;

    /* Other */
} MainUi;
