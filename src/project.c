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
** Description:
**  StarsAl project maintenance functions
**
** Author:	Anthony Buckley
**
** History
**	22-Sep-2020	Initial code
**
*/


/* Defines */



/* Includes */

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <defs.h>
#include <project.h>
#include <preferences.h>


/* Prototypes */

ProjectData * new_proj_data();
int convert_exif(ImgExif *, int *, int *, int *, GtkWidget *);
void free_img(gpointer);
void close_project(ProjectData *);
int save_proj_init(ProjectData *, ProjectUi *);

extern int val_str2numb(char *, int *, char *, GtkWidget *);
extern int check_dir(char *);
extern int make_dir(char *);


/* Globals */

static const char *debug_hdr = "DEBUG-project.c ";




/* Create new project data structure*/

ProjectData * new_proj_data()
{
    ProjectData *proj = (ProjectData *) malloc(sizeof(ProjectData));
    memset(proj, 0, sizeof(ProjectData));

    return proj;
}


/* Convert iso, width and height to integer */

int convert_exif(ImgExif *e, int *iso, int *w, int *h, GtkWidget *window)
{
    if (val_str2numb(e->iso, iso, e->iso, window) == FALSE)
	return FALSE;

    if (val_str2numb(e->width, w, e->width, window) == FALSE)
	return FALSE;

    if (val_str2numb(e->height, h, e->height, window) == FALSE)
	return FALSE;

    return TRUE;
}


/* Free an image or dark */

void free_img(gpointer data)
{
    Image *img;
    ImgExif *e;

    img = (Image *) data;
    e = &(img->img_exif);

    free(img->nm);
    free(img->path);

    free(e->make);
    free(e->model);
    free(e->type);
    free(e->date);
    free(e->width);
    free(e->height);
    free(e->iso);
    free(e->exposure);
    free(e->f_stop);

    free(img);

    return;
}


/* Close (not delete) the current project */

void close_project(ProjectData *proj)
{
    /* Free the listed images and darks */
    g_list_free_full(proj->images_gl, (GDestroyNotify) free_img);
    g_list_free_full(proj->darks_gl, (GDestroyNotify) free_img);

    proj->images_gl = NULL;
    proj->darks_gl = NULL;

    /* Free remaining */
    free(proj->project_name);
    free(proj->project_path);

    /* Free project */
    free(proj);

    return;
}


/* Write the initial project file */

int save_proj_init(ProjectData *proj, ProjectUi *p_ui)
{
    int nml, pathl;
    FILE *fd = NULL;
    char buf[256];
    char *proj_fn;

    /* Project directory exists */
    if (check_dir((char *) proj->project_path) == FALSE)
	if (make_dir((char *) proj->project_path) == FALSE)
	    return FALSE;

    /* New or overwrite file */
    nml = strlen(proj->project_name);
    pathl = strlen(proj->project_path);
    proj_fn = (char *) malloc(pathl + nml + 7);
    sprintf(proj_fn, "%s/%s_data", proj->project_path, proj->project_name);

    if ((fd = fopen(proj_fn, "w")) == (FILE *) NULL)
    {
	free(proj_fn);
	return FALSE;
    }

    /* Write new values */
    /*
    	create xml template
    	determine size of buffer required
    	construct data inside xml tags
    	write file
    */
    /*
    pref_list = g_list_first(pref_list_head);

    while(pref_list != NULL)
    {
    	UserPrefData *Preference = (UserPrefData *) pref_list->data;

    	if (Preference->val != NULL)
    	{
	    sprintf(buf, "%s|%s\n", Preference->key, Preference->val);
	    
	    if ((fputs(buf, fd)) == EOF)
	    {
		free(prefs_fn);
		log_msg("SYS9005", prefs_fn, "SYS9005", window);
		return FALSE;
	    }
    	}

	pref_list = g_list_next(pref_list);
    }
    */

    /* Close off */
    fclose(fd);
    free(proj_fn);

    return TRUE;
}
