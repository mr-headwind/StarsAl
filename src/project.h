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


typedef struct _ProjectData
{
    char *project_name;
    char *project_desc;
    char *project_path;
    int status, baseimg, basedark;
    GList *images_gl;
    GList *darks_gl;
} ProjectData;


typedef struct _ImgExif
{
    char *make;
    char *model;
    char *type;
    char *date;
    char *width;
    char *height;
    char *iso;
    char *exposure;
    char *f_stop;
} ImgExif;


typedef struct _Image
{
    char *nm;
    char *path;
    ImgExif img_exif;
} Image;

#endif
