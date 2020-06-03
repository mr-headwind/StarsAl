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
** Description:	Show application version, license, credits etc.
**
** Author:	Anthony Buckley
**
** History
**	xx-xxx-2021	Initial code
**
*/



/* Defines */


/* Includes */

#include <stdlib.h>  
#include <string.h>  
#include <libgen.h>  
#include <gtk/gtk.h>  
#include <stdio.h>
#include <defs.h>
#include <version.h>


/* Types */

typedef struct _about_ui
{
    /* Main view */
    GtkWidget *window;
    GtkWidget *parent_window;
    GtkWidget *main_box, *btn_box;  
    GtkWidget *close_btn;  

    /* Title and version */
    GtkWidget *tv_box, *tvi_box;  
    GtkWidget *title_lbl, *version_lbl;
    GtkWidget *icon;

    /* Info */
    GtkWidget *info_box, *txt_box, *url_box;  
    GtkWidget *txt_lbl[3];
    GtkWidget *home_page;

    /* License and credits */
    GtkWidget *lc_tab_nb;

    int close_hndlr_id;
} AboutUi;


/* Prototypes */

int about_main(GtkWidget *);
void about_ui(AboutUi *);
AboutUi * new_about_ui();
void about_ui_hdr(AboutUi *);
void about_ui_info(AboutUi *);
void about_ui_lc(AboutUi *);
GtkWidget * new_page(int);
void add_lic_link(GtkTextBuffer **, GtkWidget **);
void OnAboutClose(GtkWidget*, gpointer);

extern void register_window(GtkWidget *);
extern void deregister_window(GtkWidget *);
extern void strlower(char *, char *);


/* Globals */

static const char *debug_hdr = "DEBUG-about_ui.c ";

static const char *about_text[][2] =
{
    { "License", "Copyright (C) 2021  Anthony Buckley\n\n"
    		 "This program comes with ABSOLUTELY NO WARRANTY.\n"
		 "See the GNU General Public License, version 3 or later for details.\n" },
    { "Credits", "Tony Buckley\t (tony.buckley0000@gmail.com)\n" }
};
static const int txt_max = 2;

static const char *license_url[] =
{
    "http://www.gnu.org/licenses/gpl.html",
    "http://www.gnu.org/licenses/gpl-3.0.html"
};
static const int url_max = 2;



/* Application 'About' Information display control */

int about_main(GtkWidget *parent_window)
{
    AboutUi *ui;  

    /* Initial */
    ui = new_about_ui();
    ui->parent_window = parent_window;

    /* Create the interface */
    about_ui(ui);
    gtk_widget_show_all(ui->window);

    /* Register the window */
    register_window(ui->window);

    return TRUE;
}


/* Create new screen data variable */

AboutUi * new_about_ui()
{
    AboutUi *ui = (AboutUi *) malloc(sizeof(AboutUi));
    memset(ui, 0, sizeof(AboutUi));

    return ui;
}


/* Create the user interface and set the CallBacks */

void about_ui(AboutUi *a_ui)
{  
    char buffer[500];
    int rc;

    /* Set up the UI window */
    a_ui->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);  
    gtk_window_set_title(GTK_WINDOW(a_ui->window), ABOUT_UI);
    gtk_window_set_position(GTK_WINDOW(a_ui->window), GTK_WIN_POS_NONE);
    gtk_window_set_default_size(GTK_WINDOW(a_ui->window), 360, 370);
    gtk_container_set_border_width(GTK_CONTAINER(a_ui->window), 10);
    gtk_window_set_transient_for (GTK_WINDOW (a_ui->window), GTK_WINDOW (a_ui->parent_window));
    gtk_window_set_modal (GTK_WINDOW (a_ui->window), TRUE);

    /* Header */
    about_ui_hdr(a_ui);

    /* General information */
    about_ui_info(a_ui);

    /* License and Credits */
    about_ui_lc(a_ui);
    
    /* Close button */
    a_ui->close_btn = gtk_button_new_with_label("  Close  ");
    g_signal_connect(a_ui->close_btn, "clicked", G_CALLBACK(OnAboutClose), a_ui);
    a_ui->btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_halign (a_ui->btn_box, GTK_ALIGN_CENTER);
    gtk_box_pack_end (GTK_BOX (a_ui->btn_box), a_ui->close_btn, FALSE, FALSE, 0);

    /* Combine everything onto the window */
    a_ui->main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    gtk_box_pack_start (GTK_BOX (a_ui->main_box), a_ui->tvi_box, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (a_ui->main_box), a_ui->info_box, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (a_ui->main_box), a_ui->lc_tab_nb, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (a_ui->main_box), a_ui->btn_box, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(a_ui->window), a_ui->main_box);  

    /* Exit when window closed */
    a_ui->close_hndlr_id = g_signal_connect(a_ui->window, "destroy", G_CALLBACK(OnAboutClose), a_ui);  

    return;
}


/* Header information - icon, application, version */

void about_ui_hdr(AboutUi *a_ui)
{  
    char *app_icon;
    char *s;

    /* Title and version */
    a_ui->tv_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_halign (a_ui->tv_box, GTK_ALIGN_START);

    a_ui->title_lbl = gtk_label_new(TITLE);
    gtk_widget_set_name(a_ui->title_lbl, "title_2");

    s = (char *) malloc(strlen(VERSION) + 12);
    sprintf(s, "(Version: %s)", VERSION);
    a_ui->version_lbl = gtk_label_new(s);
    gtk_widget_set_name(a_ui->version_lbl, "title_3");
    free(s);
    gtk_widget_set_halign (a_ui->version_lbl, GTK_ALIGN_END);
    gtk_widget_set_valign (a_ui->version_lbl, GTK_ALIGN_END);

    gtk_box_pack_start (GTK_BOX (a_ui->tv_box), a_ui->title_lbl, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (a_ui->tv_box), a_ui->version_lbl, FALSE, FALSE, 0);

    /* Icon */
    s = (char *) malloc(sizeof(TITLE) + 1);
    strlower(TITLE, s);
    app_icon = g_strconcat (PACKAGE_DATA_DIR, "/pixmaps/", s, "/StarsAl.png",NULL);
    free(s);
    a_ui->icon = gtk_image_new_from_file(app_icon);
    g_free(app_icon);
    gtk_widget_set_margin_end(GTK_WIDGET (a_ui->icon), 20);

    /* Pack */
    a_ui->tvi_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign (a_ui->tvi_box, GTK_ALIGN_START);
    gtk_box_pack_start (GTK_BOX (a_ui->tvi_box), a_ui->icon, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (a_ui->tvi_box), a_ui->tv_box, FALSE, FALSE, 0);

    return;
}


/* General information - decsription, homepage */

void about_ui_info(AboutUi *a_ui)
{  
    int i;
    const char *desc[] = 
    	{
	    "xxxxx",
	    "xxxxx",
	    "xxxxx"
    	};
    const int desc_max = 3;

    /* Set up */
    a_ui->info_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_top(GTK_WIDGET (a_ui->info_box), 10);

    /* Description */
    a_ui->txt_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_halign (a_ui->txt_box, GTK_ALIGN_START);
    gtk_widget_set_margin_start(GTK_WIDGET (a_ui->txt_box), 20);

    for(i = 0; i < desc_max; i++)
    {
	a_ui->txt_lbl[i] = gtk_label_new(desc[i]);
	gtk_widget_set_name( a_ui->txt_lbl[i], "data_3");
	gtk_widget_set_halign (a_ui->txt_lbl[i], GTK_ALIGN_START);
	gtk_box_pack_start (GTK_BOX (a_ui->txt_box), a_ui->txt_lbl[i], FALSE, FALSE, 0);
    }

    /* Web page */
    a_ui->home_page = gtk_link_button_new_with_label (APP_URI, "Web page");
    a_ui->url_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_halign (a_ui->url_box, GTK_ALIGN_CENTER);
    gtk_box_pack_start (GTK_BOX (a_ui->url_box), a_ui->home_page, FALSE, FALSE, 0);

    /* Pack */
    gtk_box_pack_start (GTK_BOX (a_ui->info_box), a_ui->txt_box, TRUE, TRUE, 5);
    gtk_box_pack_start (GTK_BOX (a_ui->info_box), a_ui->url_box, FALSE, FALSE, 5);

    return;
}


/* License and Credits */

void about_ui_lc(AboutUi *a_ui)
{  
    int i;

    /* Setup */
    a_ui->lc_tab_nb = gtk_notebook_new();
    gtk_notebook_set_scrollable (GTK_NOTEBOOK(a_ui->lc_tab_nb), TRUE);

    /* Tab pages */
    for(i = 0; i < txt_max; i++)
    {
	if (gtk_notebook_append_page (GTK_NOTEBOOK (a_ui->lc_tab_nb), 
				      new_page(i), 
				      gtk_label_new (about_text[i][0])) == -1)
	    return;
    }

    return;
}


/* New tabbed notebook page */

GtkWidget * new_page(int i)
{  
    GtkWidget *scroll_win;
    GtkWidget *txt_view;  
    GtkTextBuffer *txt_buffer;  

    /* TextView */
    txt_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (txt_view), GTK_WRAP_WORD);
    txt_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (txt_view));
    gtk_text_buffer_set_text (txt_buffer, about_text[i][1], -1);

    /* Page specific additions */
    switch(i)
    {
    	case 0:
	    add_lic_link(&txt_buffer, &txt_view);
	    break;

	default:
	    break;
    };

    gtk_text_view_set_editable (GTK_TEXT_VIEW (txt_view), FALSE);

    /* Scrolled window for TextView */
    scroll_win = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scroll_win),
    				   GTK_POLICY_AUTOMATIC,
    				   GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request (scroll_win, 200, 135);
    gtk_container_add(GTK_CONTAINER(scroll_win), txt_view);
    gtk_container_set_border_width(GTK_CONTAINER(scroll_win), 3);

    return scroll_win;
}


/* Add lins to the License page */

void add_lic_link(GtkTextBuffer **txt_buffer, GtkWidget **txt_view)
{  
    int i;
    GtkTextChildAnchor *anchor_lnk;
    GtkTextIter iter;

    for(i = 0; i < url_max; i++)
    {
	GtkWidget *lic_url = gtk_link_button_new (license_url[i]);
	gtk_text_buffer_get_end_iter (*txt_buffer, &iter);
	anchor_lnk = gtk_text_buffer_create_child_anchor (*txt_buffer, &iter);
	gtk_text_view_add_child_at_anchor (GTK_TEXT_VIEW (*txt_view), lic_url, anchor_lnk);
	gtk_text_iter_forward_to_end (&iter);
	gtk_text_buffer_insert (*txt_buffer, &iter, "\n", -1);
    }

    return;
}


// Callback for window close
// Destroy the window and de-register the window 

void OnAboutClose(GtkWidget *w, gpointer user_data)
{ 
    AboutUi *a_ui;

    a_ui = (AboutUi *) user_data;

    g_signal_handler_block (a_ui->window, a_ui->close_hndlr_id);
    deregister_window(a_ui->window);
    gtk_window_close(GTK_WINDOW(a_ui->window));

    return;
}
