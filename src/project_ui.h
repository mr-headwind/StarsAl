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
** Description:	Project details
**
** Author:	Anthony Buckley
**
** History
**	31-Jan-2021	Initial
**
*/


/* Includes */

#include <gtk/gtk.h>
#include <main.h>


// Structure(s) to contain all project ui details.

#ifndef PROJECTUI_H
#define PROJECTUI_H


typedef struct _SelectListUi
{
    GtkWidget *sel_fr, *sel_hbox, *sel_vbox;
    GtkWidget *btn_vbox, *sel_btn, *clear_btn, *remove_btn;
    GtkWidget *list_box, *scroll_win;
    GtkWidget *meta_lbl, *dir_lbl;
    GList *img_files;
    int sel_handler_id;
} SelectListUi;


typedef struct _ProjectUi
{
    GtkWidget *window;
    GtkWidget *title_fr;
    GtkWidget *main_vbox, *nm_grid, *btn_hbox, *proj_cntr;
    GtkWidget *proj_nm_lbl, *proj_nm, *proj_desc_lbl, *proj_desc, *proj_path_lbl;
    SelectListUi images;
    SelectListUi darks;
    GtkWidget *save_btn, *cancel_btn;
    int close_handler_id;
    MainUi *m_ui;
} ProjectUi;

#endif
