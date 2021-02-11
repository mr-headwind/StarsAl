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

/* Structure to contain main interface items for easy access */

typedef struct _main_ui
{
    /* Main view widgets */
    GtkWidget *window;
    GtkWidget *cntl_box, *app_hbox;  
    GtkWidget *status_info;  

    /* Menu items */
    GtkWidget *menu_bar;  
    GtkWidget *file_menu, *edit_menu, *view_menu, *opt_menu, *help_menu;
    GtkWidget *file_hdr, *edit_hdr, *view_hdr, *opt_hdr, *help_hdr;
    GtkWidget *new_proj, *open_proj, *close_proj, *file_exit;
    GtkWidget *edit_proj, *opt_prefs;
    GtkWidget *view_fit, *view_actual;
    GtkWidget *view_log, *help_about;
    GtkWidget *sep, *sep2;
    GtkAccelGroup *accel_group;

    /* Tool bars */

    /* Application widgets */
    GtkWidget *algn_vbox, *proc_hbox, *proj_hbox;  
    GtkWidget *image_area, *img_scroll_win;  
    GtkWidget *image_list_tree, *lst_scroll_win; 
    GtkTreeModel *model;
    GtkTreeSelection *select_image;
    GtkWidget *heading_lbl, *proj_name_lbl, *proj_desc_lbl;  
    GtkWidget *dummy;  
    GdkPixbuf *base_pixbuf;

    /* Control widgets and items */
    GtkWidget *save_btn;
    GtkWidget *close_btn;

    /* Callback Handlers */
    int close_hndlr_id;
    int sel_handler_id;

    /* Other */
    ProjectData *proj;
    char *curr_img_base, *curr_dark_base;
} MainUi;


enum ImageCol
    {
       BASE_IMG,
       IMAGE_TYPE,
       IMAGE_NM,
       IMG_TOOL_TIP,
       IMG_N_COLUMNS
    };

#endif
