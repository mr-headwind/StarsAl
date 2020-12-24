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
** Description: Functions to create the main user interface for the application.
**
** Author:	Anthony Buckley
**
** History
**	02-Jun-2020	Initial code
**
*/



/* Defines */
#define MAIN_UI
#define TEXT_COL 1
#define TOGGLE_COL 2


/* Includes */
#include <stdlib.h>  
#include <string.h>  
#include <libgen.h>  
#include <gtk/gtk.h>  
#include <gdk/gdkkeysyms.h>  
#include <main.h>
#include <project.h>
#include <defs.h>
#include <preferences.h>


/* Types */

/*
enum ImageCol
    {
       IMAGE_TYPE,
       IMAGE_NM,
       BASE_IMG,
       IMG_TOOL_TIP,
       IMG_N_COLUMNS
    };
*/


/* Prototypes */

void main_ui(MainUi *);
void create_menu(MainUi *);
void create_main_view(MainUi *);
void image_area(MainUi *);
void image_list(MainUi *);
void display_proj(ProjectData *, MainUi *);
void set_image_list(ProjectData *, MainUi *);
void new_image_col(int, char *, enum ImageCol, MainUi *);

extern void log_msg(char*, char*, char*, GtkWidget*);
extern void app_msg(char*, char *, GtkWidget *);
extern int get_user_pref(char *, char **);

extern void OnNewProj(GtkWidget*, gpointer);
extern void OnOpenProj(GtkWidget*, gpointer);
extern void OnCloseProj(GtkWidget*, gpointer);
extern void OnEditProj(GtkWidget*, gpointer);
extern void OnImageSelect (GtkTreeSelection *, gpointer);
extern void OnPrefs(GtkWidget*, gpointer);
extern void OnAbout(GtkWidget*, gpointer);
extern void OnViewLog(GtkWidget*, gpointer);
extern void OnQuit(GtkWidget*, gpointer);

extern void set_css();
extern void create_label(GtkWidget **, char *, char *, GtkWidget *);
extern void create_label2(GtkWidget **, char *, char *, GtkWidget *, int, int, int, int);
extern void create_label3(GtkWidget **, char *, char *);
extern void create_entry(GtkWidget **, char *, GtkWidget *, int, int);
extern void create_radio(GtkWidget **, GtkWidget *, char *, char *, GtkWidget *, int, char *, char *);
extern void create_cbox(GtkWidget **, char *, const char *[], int, int, GtkWidget *, int, int);
extern GtkWidget * debug_cntr(GtkWidget *);
extern GtkWidget * find_widget_by_name(GtkWidget *, char *);


/* Globals */

static const char *debug_hdr = "DEBUG-main_ui.c ";


/* Create the user interface and set the CallBacks */

void main_ui(MainUi *m_ui)
{  
    /* Set up the UI window */
    m_ui->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);  
    g_object_set_data (G_OBJECT (m_ui->window), "ui", m_ui);
    gtk_window_set_title(GTK_WINDOW(m_ui->window), TITLE);
    gtk_window_set_position(GTK_WINDOW(m_ui->window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(m_ui->window), 900, 600);
    gtk_container_set_border_width(GTK_CONTAINER(m_ui->window), 10);

    /* Overall view container */
    m_ui->cntl_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
    gtk_widget_set_halign(GTK_WIDGET (m_ui->cntl_box), GTK_ALIGN_START);

    /* Menu */
    create_menu(m_ui);

    /* Application main area */
    create_main_view(m_ui);

    /* Information area */
    m_ui->status_info = gtk_label_new(NULL);
    gtk_widget_set_margin_top(GTK_WIDGET (m_ui->status_info), 5);
    gtk_label_set_text(GTK_LABEL (m_ui->status_info), " ");
    gtk_widget_set_halign(GTK_WIDGET (m_ui->status_info), GTK_ALIGN_START);

    /* Combine everything onto the window */
    gtk_box_pack_start (GTK_BOX (m_ui->cntl_box), m_ui->menu_bar, FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX (m_ui->cntl_box), m_ui->app_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (m_ui->cntl_box), m_ui->status_info, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(m_ui->window), m_ui->cntl_box);  

    /* Exit when window closed */
    g_signal_connect(m_ui->window, "destroy", G_CALLBACK(OnQuit), m_ui->window);  

    /* Show window */
    set_css();
    gtk_widget_show_all(m_ui->window);

    return;
}


/*
** Menu function for StarsAl application.
**
**  File	        Edit		        Help
**   - New Project    	 - Project 		 - View log
**   - Open Project      - User settings         - About
**   - Close Project   
**   - Exit    	      
*/

void create_menu(MainUi *m_ui)
{
    /* Create menubar */
    m_ui->menu_bar = gtk_menu_bar_new();


    /* FILE MENU */
    m_ui->file_menu = gtk_menu_new();

    /* File menu items */
    m_ui->new_proj = gtk_menu_item_new_with_mnemonic ("New Project...");
    m_ui->open_proj = gtk_menu_item_new_with_mnemonic ("Open Project...");
    m_ui->close_proj = gtk_menu_item_new_with_mnemonic ("Close Project...");
    m_ui->sep = gtk_separator_menu_item_new();
    m_ui->file_exit = gtk_menu_item_new_with_mnemonic ("E_xit");

    /* Add to menu */
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->file_menu), m_ui->new_proj);
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->file_menu), m_ui->open_proj);
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->file_menu), m_ui->close_proj);
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->file_menu), m_ui->sep);
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->file_menu), m_ui->file_exit);

    /* Callbacks */
    g_signal_connect (m_ui->new_proj, "activate", G_CALLBACK (OnNewProj), m_ui->window);
    g_signal_connect (m_ui->open_proj, "activate", G_CALLBACK (OnOpenProj), m_ui->window);
    g_signal_connect (m_ui->close_proj, "activate", G_CALLBACK (OnCloseProj), m_ui->window);
    g_signal_connect_swapped (m_ui->file_exit, "activate", G_CALLBACK (OnQuit), m_ui->window); 

    /* Show menu items */
    gtk_widget_show (m_ui->new_proj);
    gtk_widget_show (m_ui->open_proj);
    gtk_widget_show (m_ui->close_proj);
    gtk_widget_show (m_ui->file_exit);


    /* EDIT MENU */
    m_ui->edit_menu = gtk_menu_new();

    /* Option menu items */
    m_ui->edit_proj = gtk_menu_item_new_with_mnemonic ("_Project...");

    /* Add to menu */
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->edit_menu), m_ui->edit_proj);

    /* Callbacks */
    g_signal_connect (m_ui->edit_proj, "activate", G_CALLBACK (OnEditProj), m_ui->window);

    /* Show menu items */
    gtk_widget_show (m_ui->edit_proj);


    /* OPTIONS MENU */
    m_ui->opt_menu = gtk_menu_new();

    /* Option menu items */
    m_ui->opt_prefs = gtk_menu_item_new_with_mnemonic ("_User Settings...");

    /* Add to menu */
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->opt_menu), m_ui->opt_prefs);

    /* Callbacks */
    g_signal_connect (m_ui->opt_prefs, "activate", G_CALLBACK (OnPrefs), m_ui->window);

    /* Show menu items */
    gtk_widget_show (m_ui->opt_prefs);


    /* HELP MENU */
    m_ui->help_menu = gtk_menu_new();

    /* Help menu items */
    m_ui->help_about = gtk_menu_item_new_with_mnemonic ("About...");
    m_ui->view_log = gtk_menu_item_new_with_mnemonic ("View Log...");

    /* Add to menu */
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->help_menu), m_ui->help_about);
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->help_menu), m_ui->view_log);

    /* Callbacks */
    g_signal_connect (m_ui->help_about, "activate", G_CALLBACK (OnAbout), m_ui);
    g_signal_connect (m_ui->view_log, "activate", G_CALLBACK (OnViewLog), m_ui->window);

    /* Show menu items */
    gtk_widget_show (m_ui->help_about);
    gtk_widget_show (m_ui->view_log);


    /* MENU BAR */

    /* File header menu */
    m_ui->file_hdr = gtk_menu_item_new_with_mnemonic ("_File");
    gtk_widget_show (m_ui->file_hdr);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (m_ui->file_hdr), m_ui->file_menu);
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->menu_bar), m_ui->file_hdr);

    /* Edit header menu */
    m_ui->edit_hdr = gtk_menu_item_new_with_mnemonic ("_Edit");
    gtk_widget_show (m_ui->edit_hdr);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (m_ui->edit_hdr), m_ui->edit_menu);
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->menu_bar), m_ui->edit_hdr);

    /* Option header menu */
    m_ui->opt_hdr = gtk_menu_item_new_with_mnemonic ("_Options");
    gtk_widget_show (m_ui->opt_hdr);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (m_ui->opt_hdr), m_ui->opt_menu);
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->menu_bar), m_ui->opt_hdr);

    /* Help header menu */
    m_ui->help_hdr = gtk_menu_item_new_with_mnemonic ("_Help");
    gtk_widget_show (m_ui->help_hdr);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (m_ui->help_hdr), m_ui->help_menu);
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->menu_bar), m_ui->help_hdr);


    /* Accelerators */
    m_ui->accel_group = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW (m_ui->window), m_ui->accel_group);

    gtk_widget_add_accelerator(m_ui->file_exit, "activate", m_ui->accel_group, GDK_KEY_q,
    			       GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE); 
    gtk_widget_add_accelerator(m_ui->opt_prefs, "activate", m_ui->accel_group, GDK_KEY_F10,
    			       GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);		/* Alt key */ 

    return;
}  


/* Create the application interface widgets */

void create_main_view(MainUi *m_ui)
{  
    /* Control containers */
    m_ui->app_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
    m_ui->algn_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
    m_ui->proc_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);

    /* Create drawing area for graphs */
    image_area(m_ui);

    /* Listbox for project images and darks */
    image_list(m_ui);

    /* Combine together */
    gtk_box_pack_start (GTK_BOX (m_ui->algn_vbox), m_ui->img_scroll_win, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (m_ui->algn_vbox), m_ui->lst_scroll_win, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (m_ui->app_hbox), m_ui->proc_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (m_ui->app_hbox), m_ui->algn_vbox, FALSE, FALSE, 0);

    //create_label(&(m_ui->dummy), "Dummy", "Application main items go here", m_ui->app_vbox);

    return;
}


/* Image drawing area */

void image_area(MainUi *m_ui)
{  
    /* Create drawing area for graphs */
    m_ui->image_area = gtk_drawing_area_new();
    gtk_widget_set_margin_top (m_ui->image_area, 10);
    gtk_widget_set_size_request (m_ui->image_area, 600, 400);
    gtk_widget_set_halign (m_ui->image_area, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (m_ui->image_area, GTK_ALIGN_CENTER);

    /* Place in scrolled window */
    m_ui->img_scroll_win = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_margin_start (GTK_WIDGET (m_ui->img_scroll_win), 5);
    gtk_widget_set_margin_top (GTK_WIDGET (m_ui->img_scroll_win), 5);
    gtk_widget_set_margin_bottom (GTK_WIDGET (m_ui->img_scroll_win), 5);
    gtk_widget_set_halign(GTK_WIDGET (m_ui->img_scroll_win), GTK_ALIGN_START);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (m_ui->img_scroll_win), 
				    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    //gtk_scrolled_window_set_min_content_width (GTK_SCROLLED_WINDOW (m_ui->img_scroll_win), 550);
    gtk_container_add(GTK_CONTAINER (m_ui->img_scroll_win), m_ui->image_area);

    return;
}


/* Image list */

void image_list(MainUi *m_ui)
{  
    /* Image list will be in a scrolled window */
    m_ui->lst_scroll_win = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_margin_start (GTK_WIDGET (m_ui->lst_scroll_win), 5);
    gtk_widget_set_margin_top (GTK_WIDGET (m_ui->lst_scroll_win), 5);
    gtk_widget_set_margin_bottom (GTK_WIDGET (m_ui->lst_scroll_win), 5);
    gtk_widget_set_halign(GTK_WIDGET (m_ui->lst_scroll_win), GTK_ALIGN_START);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (m_ui->lst_scroll_win), 
				    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    //gtk_scrolled_window_set_min_content_width (GTK_SCROLLED_WINDOW (m_ui->lst_scroll_win), 550);


    //GtkWidget *heading_lbl, *row;

    /* Listbox for project images and darks **
    m_ui->list_box = gtk_list_box_new();
    m_ui->sel_handler_id = g_signal_connect(m_ui->list_box, "row-selected", G_CALLBACK(OnImageSelect), (gpointer) m_ui);

    ** Place in scrolled window **
    m_ui->lst_scroll_win = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_margin_start (GTK_WIDGET (m_ui->lst_scroll_win), 5);
    gtk_widget_set_margin_top (GTK_WIDGET (m_ui->lst_scroll_win), 5);
    gtk_widget_set_margin_bottom (GTK_WIDGET (m_ui->lst_scroll_win), 5);
    gtk_widget_set_halign(GTK_WIDGET (m_ui->lst_scroll_win), GTK_ALIGN_START);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (m_ui->lst_scroll_win), 
				    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_min_content_width (GTK_SCROLLED_WINDOW (m_ui->lst_scroll_win), 250);
    gtk_container_add(GTK_CONTAINER (m_ui->lst_scroll_win), m_ui->list_box);

    ** List heading **
    row = gtk_list_box_row_new();
    gtk_list_box_row_set_selectable(GTK_LIST_BOX_ROW (row), FALSE);
    create_label3(&(heading_lbl), "head_1", "Project Images and Darks");
    gtk_container_add(GTK_CONTAINER (row), heading_lbl);
    gtk_list_box_prepend(GTK_LIST_BOX (m_ui->list_box), row);
    gtk_list_box_row_set_header (GTK_LIST_BOX_ROW (row), NULL);
    */

    return;
}


/* Display project */

void display_proj(ProjectData *proj, MainUi *m_ui)
{  
    char *msg;

    if (proj == NULL)
    	return;

    /* Create images list */
    set_image_list(proj, m_ui);

    /* Open 1st image */

    /* Info status */
    msg = (char *) malloc(strlen(proj->project_name) + 25);
    sprintf(msg, "Currently viewing project: %s", proj->project_name);
    gtk_label_set_text(GTK_LABEL (m_ui->status_info), msg);
    free(msg);

    gtk_scrolled_window_set_min_content_width (GTK_SCROLLED_WINDOW (m_ui->lst_scroll_win), 650);
    gtk_scrolled_window_set_min_content_height (GTK_SCROLLED_WINDOW (m_ui->lst_scroll_win), 250);

    gtk_widget_show_all(m_ui->window);

    return;
}


/* Display project */

void set_image_list(ProjectData *proj, MainUi *m_ui)
{  
    char *s;
    GList *l;
    Image *img;
    gboolean base;
    GtkListStore *store;
    GtkTreeIter iter;

    /* Build a list view for images */
    store = gtk_list_store_new (IMG_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_STRING);

    /* Iterate through the images and add the store */
    base = TRUE;

    for(l = proj->images_gl; l != NULL; l = l->next)
    {
    	img = (Image *) l->data;
    	s = (char *) malloc(strlen(img->nm) + strlen(img->path) + 2);
    	sprintf(s, "%s/%s", img->path, img->nm);

    	/* Acquire an iterator and load the data*/
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    IMAGE_TYPE, "I",
			    IMAGE_NM, s,
			    BASE_IMG, base,
			    IMG_TOOL_TIP, img->nm,
			    -1);
	base = FALSE;
	free(s);
    }

    /* Iterate through the darks and add the store */
    base = TRUE;

    for(l = proj->darks_gl; l != NULL; l = l->next)
    {
    	img = (Image *) l->data;
    	s = (char *) malloc(strlen(img->nm) + strlen(img->path) + 2);
    	sprintf(s, "%s/%s", img->path, img->nm);

    	/* Acquire an iterator and load the data*/
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    IMAGE_TYPE, "D",
			    IMAGE_NM, s,
			    BASE_IMG, base,
			    IMG_TOOL_TIP, img->nm,
			    -1);
	base = FALSE;
	free(s);
    }

    /* Tree (list) view */
    m_ui->image_list_tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
    g_object_unref (G_OBJECT (store));
    m_ui->model = gtk_tree_view_get_model (GTK_TREE_VIEW (m_ui->image_list_tree));
    gtk_tree_view_set_tooltip_column (GTK_TREE_VIEW (m_ui->image_list_tree), IMG_TOOL_TIP);
    gtk_tree_view_set_activate_on_single_click (GTK_TREE_VIEW (m_ui->image_list_tree), FALSE);

    /* Selection */
    m_ui->select_image = gtk_tree_view_get_selection (GTK_TREE_VIEW (m_ui->image_list_tree));
    gtk_tree_selection_set_mode (m_ui->select_image, GTK_SELECTION_SINGLE);
    m_ui->sel_handler_id = g_signal_connect (G_OBJECT (m_ui->select_image), "changed", G_CALLBACK (OnImageSelect), m_ui);

    /* Column and headers for Name, Description & Date Modified */
    new_image_col(TEXT_COL, "Type", IMAGE_TYPE, m_ui);
    new_image_col(TEXT_COL, "Image", IMAGE_NM, m_ui);
    new_image_col(TOGGLE_COL, "Base", BASE_IMG, m_ui);

    /* Add to the window container */
    gtk_container_add (GTK_CONTAINER (m_ui->lst_scroll_win), m_ui->image_list_tree);
    gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (m_ui->image_list_tree), GTK_TREE_VIEW_GRID_LINES_NONE);

    return;
}


/* Create a new column and header */

void new_image_col(int col_type, char *col_title, enum ImageCol image_col, MainUi *m_ui)
{  
    GtkWidget *colhdr;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;

    switch(col_type)
    {
    	case TEXT_COL:
	    renderer = gtk_cell_renderer_text_new ();
	    column = gtk_tree_view_column_new_with_attributes (col_title, renderer, 
							       "text", image_col, NULL);

	    gtk_tree_view_append_column (GTK_TREE_VIEW (m_ui->image_list_tree), column);
	    gtk_cell_renderer_set_sensitive (GTK_CELL_RENDERER (renderer), TRUE);
	    g_object_set (G_OBJECT (renderer), "foreground-rgba", &DARK_BLUE,
					       "font", "Sans 10", NULL);

	    if (strcmp(col_title, "Type") == 0)
		g_object_set (G_OBJECT (renderer), "xpad", 10, NULL);

            break;

    	case TOGGLE_COL:
	    renderer = gtk_cell_renderer_toggle_new ();
	    column = gtk_tree_view_column_new_with_attributes (col_title, renderer,
							   "active", image_col, NULL);
	    gtk_tree_view_append_column (GTK_TREE_VIEW (m_ui->image_list_tree), column);
	    gtk_cell_renderer_set_sensitive (GTK_CELL_RENDERER (renderer), TRUE);
	    g_object_set (G_OBJECT (renderer), "xpad", 10, NULL);
            break;

    	default:
            break;
    }

    colhdr = gtk_tree_view_column_get_button (column);
    gtk_widget_set_name (colhdr, "list_col_1");

    return;
}
