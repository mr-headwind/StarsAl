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
**	12-Jul-2021	Initial
**
*/


/* Includes */

#include <gtk/gtk.h>


// Structure(s) to contain all project details.

#ifndef PROJECT_H
#define PROJECT_H


typedef struct _ImageListUi
{
    GtkWidget *img_grid;
    GtkWidget *img_lbl, *img_list, *sel_btn;
    GtkWidget *scroll_win;
} ImageListUi;


typedef struct _ProjectUi
{
    GtkWidget *window;
    GtkWidget *nm_grid, *btn_hbox, *proj_cntr;
    GtkWidget *proj_nm_lbl, *proj_nm, *proj_path_lbl;
    ImageListUi images;
    ImageListUi darks;
    GtkWidget *save_btn, *cancel_btn;
} ProjectUi;


typedef struct _ProjectData
{
    char *project_name;
    char *proj_path;
    int status;
    char *images_path;
    char *darks_path;
    GList *images_gl;
    GList *darks_gl;
} ProjectData;


typedef struct _ImgMeta
{
    char *camera;
    char *img_type;
    char *img_date;
    int img_width;
    int img_height;
    char *wh_unit;
    int iso;
    int exposure;
    char *exp_unit;
} ImgMeta;


typedef struct _Image
{
    char *img_nm;
    ImgMeta img_meta;
} Image;

#endif
