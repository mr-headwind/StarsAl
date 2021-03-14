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

void OnNewProj(GtkWidget *, gpointer);
void OnOpenProj(GtkWidget *, gpointer);
void OnCloseProj(GtkWidget *, gpointer);
void OnEditProj(GtkWidget *, gpointer);
void OnViewFit(GtkWidget *, gpointer);
void OnViewActual(GtkWidget *, gpointer);
void OnViewX(GtkWidget *, gpointer);
void OnMouseScroll(GtkScrolledWindow *, GdkEventScroll *, gpointer);
gboolean OnSWBtnPress(GtkScrolledWindow *, GdkEvent *, gpointer);
gboolean OnSWBtnRelease(GtkScrolledWindow *, GdkEvent *, gpointer);
gboolean OnMouseDrag(GtkScrolledWindow *, GdkEvent *, gpointer);
void OnImageSize(GtkWidget *, GdkRectangle *, gpointer);
void OnImageSelect(GtkTreeSelection *, gpointer);
void OnBaseToggle(GtkCellRendererToggle *, gchar *, gpointer);
void OnPrefs(GtkWidget *, gpointer);
void OnViewLog(GtkWidget *, gpointer);
void OnAbout(GtkWidget *, gpointer);
void OnQuit(GtkWidget *, gpointer);


extern void edit_project_main(ProjectData *, MainUi *);
extern void open_project_main(MainUi *);
extern int proj_close_check_save(ProjectData *, MainUi *);
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
extern int show_image(char *, MainUi *);
extern int show_meta(char *, int, gchar *, MainUi *);
extern void img_fit_win(GdkPixbuf *, int, int, MainUi *);
extern void img_actual_sz(MainUi *);
extern void img_scale_sz(MainUi *, int);
extern void zoom_image(double, MainUi *);
extern void mouse_drag_check(MainUi *);
extern void drag_move_sw(gdouble, gdouble, gdouble, gdouble, MainUi *);


/* Globals */

static const char *debug_hdr = "DEBUG-callbacks.c ";
static gdouble last_x, last_y;


/* Main UI Callbacks */


/* Callback - Open a new project */

void OnNewProj(GtkWidget *menu_item, gpointer user_data)
{  
    GtkWidget *window;
    MainUi *m_ui;

    /* Data */
    window = (GtkWidget *) user_data;
    m_ui = (MainUi *) g_object_get_data (G_OBJECT (window), "ui");

    /* Check if a project is open */
    if (m_ui->proj != NULL)
    {
    	if (proj_close_check_save(m_ui->proj, m_ui) == FALSE)
    	    return;
    }

    edit_project_main(NULL, m_ui);

    return;
}  


/* Callback - Edit project settings */

void OnEditProj(GtkWidget *menu_item, gpointer user_data)
{  
    GtkWidget *window;
    MainUi *m_ui;

    /* Data */
    window = (GtkWidget *) user_data;
    m_ui = (MainUi *) g_object_get_data (G_OBJECT (window), "ui");

    edit_project_main(m_ui->proj, m_ui);

    return;
}  


/* Callback - Close a project */

void OnCloseProj(GtkWidget *menu_item, gpointer user_data)
{  
    GtkWidget *window;
    MainUi *m_ui;

    /* Data */
    window = (GtkWidget *) user_data;
    m_ui = (MainUi *) g_object_get_data (G_OBJECT (window), "ui");

    /* Check if a project is open */
    if (m_ui->proj != NULL)
    {
    	if (proj_close_check_save(m_ui->proj, m_ui) == FALSE)
    	    return;
    }

    return;
}  


/* Callback - Open a project */

void OnOpenProj(GtkWidget *menu_item, gpointer user_data)
{  
    GtkWidget *window;
    MainUi *m_ui;

    /* Data */
    window = (GtkWidget *) user_data;
    m_ui = (MainUi *) g_object_get_data (G_OBJECT (window), "ui");

    /* Check if a project is already open */
    if (m_ui->proj != NULL)
    {
    	proj_close_check_save(m_ui->proj, m_ui);
    }

    /* Open selection window */
    open_project_main(m_ui);

    return;
}  


/* Callback - Fit image to window */

void OnViewFit(GtkWidget *menu_item, gpointer user_data)
{  
    int sw_h, sw_w;
    GtkWidget *window;
    MainUi *m_ui;

    /* Data */
    window = (GtkWidget *) user_data;
    m_ui = (MainUi *) g_object_get_data (G_OBJECT (window), "ui");

    /* Reset image */
    sw_w = gtk_widget_get_allocated_width (m_ui->img_scroll_win);
    sw_h = gtk_widget_get_allocated_height (m_ui->img_scroll_win);
    img_fit_win(m_ui->base_pixbuf, sw_w, sw_h, m_ui);

    return;
}  


/* Callback - View full size image */

void OnViewActual(GtkWidget *menu_item, gpointer user_data)
{  
    GtkWidget *window;
    MainUi *m_ui;

    /* Data */
    window = (GtkWidget *) user_data;
    m_ui = (MainUi *) g_object_get_data (G_OBJECT (window), "ui");

    /* Reset image */
    img_actual_sz(m_ui);

    return;
}  


/* Callback - Set image scale */

void OnViewX(GtkWidget *menu_item, gpointer user_data)
{  
    GtkWidget *window;
    MainUi *m_ui;
    int multx;

    /* Data */
    window = (GtkWidget *) user_data;
    m_ui = (MainUi *) g_object_get_data (G_OBJECT (window), "ui");

    /* Reset image */
    multx = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (menu_item), "view_x"));
    img_scale_sz(m_ui, multx);

    return;
}  


/* Callback - Mouse wheel scrolling to zoom in or out */

void OnMouseScroll(GtkScrolledWindow *sw, GdkEventScroll *ev, gpointer user_data)
{  
    gdouble delta_x, delta_y;
    GtkWidget *window;
    MainUi *m_ui;
    const double step_up = 1.05;
    const double step_down = 0.95;

    /* Data */
    m_ui = (MainUi *) user_data;

    /* Zoom in or out */
    if (ev->direction == GDK_SCROLL_UP)	
    {
	zoom_image(step_up, m_ui);
	g_print("OnMouseScroll: scroll up\n");
    }
    else if (ev->direction == GDK_SCROLL_DOWN)	
    {
	zoom_image(step_down, m_ui);
	g_print("OnMouseScroll: scroll down\n");
    }
    else if (ev->direction == GDK_SCROLL_SMOOTH)
    {
	gdk_event_get_scroll_deltas ((const GdkEvent *) ev, &delta_x, &delta_y);
	g_print("OnMouseScroll: scroll smooth  d_x %0.2f   d_y %0.2f\n", delta_x, delta_y);
 
	if (delta_y > 0)
	{
	    zoom_image(step_up, m_ui);
	} 
	else if (delta_y < 0)
	{
	    zoom_image(step_down, m_ui);
	}
	else
	{
	    //g_print("ERROR: scroll event no zoom\n");
	}
    }
    else
	g_print("ERROR: unknown scroll event\n");

    return;
}  


/* Callback - Enable mouse dragging for OnMouseDrag function when enabled */

gboolean OnSWBtnPress(GtkScrolledWindow *sw, GdkEvent *event, gpointer user_data)
{  
    MainUi *m_ui;
    gdouble x;
    gdouble y;

    /* Data */
    m_ui = (MainUi *) user_data;
    last_x = event->button.x;
    last_y = event->button.y;

    m_ui->mouse_drag_mode = TRUE;
    //printf("%s OnSWBtnPress  x %0.2f  y %0.2f\n", debug_hdr, last_x, last_y); fflush(stdout);

    return TRUE;
}  


/* Callback - Disable mouse dragging for OnMouseDrag function */

gboolean OnSWBtnRelease(GtkScrolledWindow *sw, GdkEvent *event, gpointer user_data)
{  
    MainUi *m_ui;
    gdouble x;
    gdouble y;

    /* Data */
    m_ui = (MainUi *) user_data;
    x = event->button.x;
    y = event->button.y;

    m_ui->mouse_drag_mode = FALSE;
    //printf("%s OnSWBtnRelease  x %0.2f  y %0.2f\n", debug_hdr, x, y); fflush(stdout);

    return TRUE;
}  


/* Callback - Mouse dragging to move image around within scroll window */

gboolean OnMouseDrag(GtkScrolledWindow *sw, GdkEvent *event, gpointer user_data)
{  
    MainUi *m_ui;
    gdouble x;
    gdouble y;

    /* Data */
    m_ui = (MainUi *) user_data;
    x = event->button.x;
    y = event->button.y;

    if (m_ui->mouse_drag_mode == FALSE)
    	return FALSE;
    //printf("%s OnMouseDrag (on) x %0.2f  y %0.2f\n", debug_hdr, x, y); fflush(stdout);

    drag_move_sw(x, y, last_x, last_y, m_ui);

    last_x = x;
    last_y = y;

    return TRUE;
}  


/* Callback - Image has had a size allocation */

void OnImageSize(GtkWidget *img, GdkRectangle *alloc, gpointer user_data)
{  
    MainUi *m_ui;

    /* Data */
    m_ui = (MainUi *) user_data;

    mouse_drag_check(m_ui);

    return;
}  


/* Callback - Image selection */

void OnImageSelect(GtkTreeSelection *selection, gpointer user_data)
{  
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreePath *path;
    gchar *img_nm, *img_type, *s_path;
    int idx;
    MainUi *m_ui;

    /* Data */
    m_ui = (MainUi *) user_data;
    idx = -1;

    /* Get selected image */
    if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
	gtk_tree_model_get (model, &iter, IMAGE_TYPE, &img_type, IMAGE_NM, &img_nm, -1);
	//g_print ("You selected an image: %s\n", img_nm);
	path = gtk_tree_model_get_path (model, &iter);
	s_path = gtk_tree_path_to_string (path);
	idx = atoi(s_path);
	gtk_tree_path_free (path);
	g_free(s_path);
    }

    /* Display the image */
    show_image(img_nm, m_ui);
    show_meta(img_nm, idx, img_type, m_ui);
    g_free(img_nm);
    g_free(img_type);

    /*
    char *s;
    s = (char *) malloc(100);			// date, w x h, iso, exposure
    sprintf(s, "ISO: %s Exp: %s W x H: %s x %s", img->img_exif.iso,
						 img->img_exif.exposure,
						 img->img_exif.width,
						 img->img_exif.height);
    gtk_label_set_text(GTK_LABEL (lst->meta_lbl), s);
    gtk_widget_show(lst->meta_lbl);

    free(s);
    */

    return;
}  


/* Callback - Base Image selection */

void OnBaseToggle(GtkCellRendererToggle *cell_renderer, gchar *path, gpointer data)
{  
    MainUi *m_ui;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gboolean state, set;
    gchar *img_type;

    m_ui = (MainUi *) data;
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (m_ui->image_list_tree));
    set = gtk_tree_model_get_iter_from_string(model, &iter, path);

    if (set)
    {
    	gtk_tree_model_get(model, &iter, BASE_IMG, &state, IMAGE_TYPE, &img_type, -1);
    	gtk_list_store_set(GTK_LIST_STORE (model), &iter, BASE_IMG, !state, -1);
    }

    if (strcmp(img_type, "I") == 0)
	set = gtk_tree_model_get_iter_from_string(model, &iter, m_ui->curr_img_base);
    else
	set = gtk_tree_model_get_iter_from_string(model, &iter, m_ui->curr_dark_base);

    if (set)
    {
    	gtk_tree_model_get(model, &iter, BASE_IMG, &state, -1);
    	gtk_list_store_set(GTK_LIST_STORE (model), &iter, 0, !state, -1);
    }

    if (strcmp(img_type, "I") == 0)
	m_ui->curr_img_base = strdup((char *) path);
    else
	m_ui->curr_dark_base = strdup((char *) path);

    g_free(img_type);

    return;
}  


/* Callback - Set up preferences */

void OnPrefs(GtkWidget *menu_item, gpointer user_data)
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

void OnAbout(GtkWidget *menu_item, gpointer user_data)
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

void OnViewLog(GtkWidget *view_log, gpointer user_data)
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

void OnQuit(GtkWidget *window, gpointer user_data)
{  
    MainUi *m_ui;

    /* Get data */
    m_ui = (MainUi *) g_object_get_data (G_OBJECT (window), "ui");

    /* Disable any events */
    if (m_ui->sel_handler_id > 0)
	g_signal_handler_block (m_ui->select_image, m_ui->sel_handler_id);

    g_signal_handler_block (m_ui->window, m_ui->close_hndlr_id);

    /* Check Project close */
    if (m_ui->proj)
    {
    	if (proj_close_check_save(m_ui->proj, m_ui) == FALSE)
    	{
	    g_signal_handler_unblock (m_ui->select_image, m_ui->sel_handler_id);
	    g_signal_handler_unblock (m_ui->window, m_ui->close_hndlr_id);
    	    return;
	}
    }

    /* Tidy up */
    if (m_ui->curr_img_base)
	free(m_ui->curr_img_base);

    if (m_ui->curr_dark_base)
	free(m_ui->curr_dark_base);

    /* Close any open windows */
    close_open_ui();
    free_window_reg();

    /* Main quit */
    gtk_main_quit();

    return;
}  


/* CALLBACK other functions */

