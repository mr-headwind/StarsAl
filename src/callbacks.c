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
** Description:	Module for (Main) Callback functions
**
** Author:	Anthony Buckley
**
** History
**	xx-xxx-2021	Initial code
*/


/* Includes */

#include <stdlib.h>  
#include <string.h>  
#include <libgen.h>  
#include <gtk/gtk.h>  
#include <main.h>
#include <project.h>
#include <defs.h>  


/* Defines */

#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))


/* Prototypes */

void OnNewProj(GtkWidget*, gpointer *user_data);
void OnOpenProj(GtkWidget*, gpointer *user_data);
void OnCloseProj(GtkWidget*, gpointer *user_data);
void OnEditProj(GtkWidget*, gpointer *user_data);
void OnImageSelect(GtkListBox*, GtkListBoxRow*, gpointer);
void OnPrefs(GtkWidget*, gpointer *user_data);
void OnViewLog(GtkWidget*, gpointer *user_data);
void OnAbout(GtkWidget*, gpointer *user_data);
void OnQuit(GtkWidget*, gpointer *user_data);


extern void edit_project_main(ProjectData *, GtkWidget *);
extern void open_project_main(ProjectData *, MainUi *);
extern void display_proj(ProjectData *, GtkWidget *);
extern void free_window_reg();
extern void close_open_ui();
extern int is_ui_reg(char *, int);
extern void log_msg(char*, char*, char*, GtkWidget*);
extern void app_msg(char*, char*, GtkWidget*);
extern char * log_name();
extern GtkWidget* view_file_main(char  *);
extern int user_prefs_main(GtkWidget *);
extern int about_main(GtkWidget *);
extern void delete_menu_items(GtkWidget *, char *);
extern int close_ui(char *);
extern gint query_dialog(GtkWidget *, char *, char *);
extern int get_user_pref(char *, char **);


/* Globals */

static const char *debug_hdr = "DEBUG-callbacks.c ";


/* Main UI Callbacks */


/* Callback - Open a new project */

void OnNewProj(GtkWidget *menu_item, gpointer *user_data)
{  
    GtkWidget *window;
    ProjectData *proj;

    window = (GtkWidget *) user_data;

    edit_project_main(NULL, window);

    return;
}  


/* Callback - Open a project */

void OnOpenProj(GtkWidget *menu_item, gpointer *user_data)
{  
    GtkWidget *window;
    ProjectData *proj;
    MainUi *m_ui;

    window = (GtkWidget *) user_data;
    m_ui = (MainUi *) g_object_get_data (G_OBJECT (window), "ui");

    open_project_main(proj, m_ui);
    display_proj(proj, m_ui);

    return;
}  


/* Callback - Close a project */

void OnCloseProj(GtkWidget *menu_item, gpointer *user_data)
{  

    return;
}  


/* Callback - Edit project settings */

void OnEditProj(GtkWidget *menu_item, gpointer *user_data)
{  
    GtkWidget *window;
    MainUi *m_ui;

    window = (GtkWidget *) user_data;
    m_ui = (MainUi *) g_object_get_data (G_OBJECT (window), "ui");

    edit_project_main(m_ui->proj, window);

    return;
}  


/* Callback - Image selection */

void OnImageSelect(GtkListBox *lstbox, GtkListBoxRow *row, gpointer user_data)
{  
    char *s;
    SelectListUi *lst;
    Image *img;

    /* Get data */
    lst = (SelectListUi *) user_data;
    img = (Image *) g_object_get_data (G_OBJECT (row), "image");

    gtk_label_set_text(GTK_LABEL (lst->dir_lbl), img->path);
    gtk_widget_show(lst->dir_lbl);

    s = (char *) malloc(100);			// date, w x h, iso, exposure
    sprintf(s, "ISO: %s Exp: %s W x H: %s x %s", img->img_exif.iso,
						 img->img_exif.exposure,
						 img->img_exif.width,
						 img->img_exif.height);
    gtk_label_set_text(GTK_LABEL (lst->meta_lbl), s);
    gtk_widget_show(lst->meta_lbl);

    free(s);
    return;
}  


/* Callback - Set up preferences */

void OnPrefs(GtkWidget *menu_item, gpointer *user_data)
{  
    GtkWidget *window;

    /* Get data */
    window = (GtkWidget *) user_data;

    /* Check if already open */
    if (is_ui_reg(USER_PREFS_UI, TRUE))
    	return;

    /* Open */
    user_prefs_main(window);

    return;
}  


/* Callback - Show About details */

void OnAbout(GtkWidget *menu_item, gpointer *user_data)
{  
    MainUi *m_ui;

    /* Check if already open */
    if (is_ui_reg(ABOUT_UI, TRUE))
    	return;

    /* Get data */
    m_ui = (MainUi *) user_data;

    /* Open */
    about_main(m_ui->window);

    return;
}  


/* Callback - View Log File details */

void OnViewLog(GtkWidget *view_log, gpointer *user_data)
{  
    GtkWidget *window;
    char *log_fn;

    /* Check if already open */
    if (is_ui_reg(VIEW_FILE_UI, TRUE))
    	return;

    /* Open */
    log_fn = log_name();
    window = (GtkWidget *) user_data;

    if (view_file_main(log_fn) == NULL)
    	log_msg("SYS9006", log_fn, "SYS9010", window);

    return;
}  


/* Callback - Quit */

void OnQuit(GtkWidget *window, gpointer *user_data)
{  
    /* Close any open windows */
    close_open_ui();
    free_window_reg();

    /* Main quit */
    gtk_main_quit();

    return;
}  


/* CALLBACK other functions */

