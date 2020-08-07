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


/* Prototypes */

void main_ui(MainUi *);
void create_menu(MainUi *);
void create_main_view(MainUi *);

extern void log_msg(char*, char*, char*, GtkWidget*);
extern void app_msg(char*, char *, GtkWidget *);
extern int get_user_pref(char *, char **);

extern void OnNewProj(GtkWidget*, gpointer);
extern void OnOpenProj(GtkWidget*, gpointer);
extern void OnCloseProj(GtkWidget*, gpointer);
extern void OnEditProj(GtkWidget*, gpointer);
extern void OnPrefs(GtkWidget*, gpointer);
extern void OnAbout(GtkWidget*, gpointer);
extern void OnViewLog(GtkWidget*, gpointer);
extern void OnQuit(GtkWidget*, gpointer);

extern void set_css();
extern void create_label(GtkWidget **, char *, char *, GtkWidget *);
extern void create_label2(GtkWidget **, char *, char *, GtkWidget *, int, int, int, int);
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
    //gtk_window_set_default_size(GTK_WINDOW(m_ui->window), 200, 200);
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
    gtk_box_pack_start (GTK_BOX (m_ui->cntl_box), m_ui->app_box, FALSE, FALSE, 0);
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
**   - New Project    	 - Project settings	 - View log
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
    m_ui->edit_proj = gtk_menu_item_new_with_mnemonic ("_Project Settings...");
    m_ui->edit_prefs = gtk_menu_item_new_with_mnemonic ("_User Settings...");

    /* Add to menu */
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->edit_menu), m_ui->edit_proj);
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->edit_menu), m_ui->edit_prefs);

    /* Callbacks */
    g_signal_connect (m_ui->edit_proj, "activate", G_CALLBACK (OnEditProj), m_ui->window);
    g_signal_connect (m_ui->edit_prefs, "activate", G_CALLBACK (OnPrefs), m_ui->window);

    /* Show menu items */
    gtk_widget_show (m_ui->edit_proj);
    gtk_widget_show (m_ui->edit_prefs);


    /* HELP MENU */
    m_ui->help_menu = gtk_menu_new();

    /* Option menu items */
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
    m_ui->edit_hdr = gtk_menu_item_new_with_mnemonic ("_Options");
    gtk_widget_show (m_ui->edit_hdr);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (m_ui->edit_hdr), m_ui->edit_menu);
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->menu_bar), m_ui->edit_hdr);

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
    gtk_widget_add_accelerator(m_ui->edit_prefs, "activate", m_ui->accel_group, GDK_KEY_F10,
    			       GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);		/* Alt key */ 

    return;
}  


/* Create the application interface widgets */

void create_main_view(MainUi *m_ui)
{  
    m_ui->app_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);

    create_label(&(m_ui->dummy), "Dummy", "Application main items go here", m_ui->app_box);

    return;
}
