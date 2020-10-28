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
** Description: Open Project user interface and management.
**
** Author:	Anthony Buckley
**
** History
**	20-Oct-2020	Initial code
**
*/


/* Includes */

#include <gtk/gtk.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <defs.h>
#include <main.h>
#include <project.h>
#include <preferences.h>


/* Defines */


/* Types */

typedef struct _SelectProjUi
{
    GtkWidget *window;
    GtkWidget *main_vbox;
    GtkWidget *selection_lbl, *scroll_win, *list_box, *btn_hbox;
    GtkWidget *hdr_hbox, *info_hbox;
    GtkWidget *nm_hdr, *desc_hdr, *date_hdr;
    GtkWidget *cnt_lbl;
    GtkWidget *open_btn, *cancel_btn;
    int close_handler_id, sel_handler_id;
    ProjectData *proj;
} SelectProjUi;

typedef struct _ProjListEnt
{
    char *nm;
    char *desc;
    char *last_mod;
} ProjListEnt;


/* Prototypes */

int open_project_main(ProjectData *, MainUi *);
int sel_proj_init(GtkWidget *);
SelectProjUi * new_sel_proj_ui();
static void sel_proj_ui(SelectProjUi *, MainUi *);
static void select_proj_cntr(SelectProjUi *);
static int project_list(GtkWidget *, SelectProjUi *);
static ProjListEnt * new_list_entry(char *, char *, struct stat *, SelectProjUi *);
static void free_list_ent(gpointer);
static GtkWidget * create_lstbox_row(ProjListEnt *);
static void create_info(int, SelectProjUi *);
static void window_cleanup(GtkWidget *, SelectProjUi *);

static void OnProjSelect(GtkListBox*, GtkListBoxRow*, gpointer);
static void OnOpen(GtkWidget*, gpointer);
static void OnCancel(GtkWidget*, gpointer);
gboolean OnOpenDelete(GtkWidget *, GdkEvent *, gpointer);

extern ProjectData * new_proj_data();
extern void create_label(GtkWidget **, char *, char *, GtkWidget *);
extern char * get_tag_val(char **, const char *, const char *, int, GtkWidget *);
extern FILE * open_proj_file(char *, char *, GtkWidget *);
extern int read_proj_file(FILE *, char *, int, GtkWidget *);
extern int get_user_pref(char *, char **);
extern void log_msg(char*, char*, char*, GtkWidget*);
extern void app_msg(char*, char*, GtkWidget*);
extern void register_window(GtkWidget *);
extern void deregister_window(GtkWidget *);


/* Globals */

static const char *debug_hdr = "DEBUG-project_ui.c ";
static int save_indi;
static char *proj_dir;
static int proj_dir_len;


/* Project selection */

int open_project_main(ProjectData *proj, MainUi *m_ui)
{
    SelectProjUi *ui;

    /* Initial */
    if (! sel_proj_init(m_ui->window))
    	return FALSE;

    /* Initialise project */
    proj = new_proj_data();
    ui = new_sel_proj_ui();
    ui->proj = proj;

    /* Create the interface */
    sel_proj_ui(ui, m_ui);

    /* Register the window */
    register_window(ui->window);

    return TRUE;
}


/* Initial checks and values */

int sel_proj_init(GtkWidget *window)
{
    char *p;

    /* Get the application working directory */
    get_user_pref(PROJ_DIR, &p);

    if (p == NULL)
    {
	log_msg("APP0009", "", "APP0009", window);
    	return FALSE;
    }

    proj_dir = p;
    proj_dir_len = strlen(p);

    return TRUE;
}


// USER INTERFACE


/* Create new screen data structure */

SelectProjUi * new_sel_proj_ui()
{
    SelectProjUi *ui = (SelectProjUi *) malloc(sizeof(SelectProjUi));
    memset(ui, 0, sizeof(SelectProjUi));

    return ui;
}


/* Create the user interface and set the CallBacks */

void sel_proj_ui(SelectProjUi *s_ui, MainUi *m_ui)
{
    /* Set up the UI window */
    s_ui->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(s_ui->window), PROJ_SEL_UI);
    gtk_window_set_position(GTK_WINDOW(s_ui->window), GTK_WIN_POS_CENTER);
    gtk_window_set_transient_for(GTK_WINDOW(s_ui->window), GTK_WINDOW(m_ui->window));
    gtk_window_set_default_size(GTK_WINDOW(s_ui->window), 400, 600);
    gtk_container_set_border_width(GTK_CONTAINER(s_ui->window), 10);
    g_object_set_data (G_OBJECT (s_ui->window), "ui", s_ui);

    /* Main view */
    s_ui->main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    /* Main update or view grid */
    select_proj_cntr(s_ui);

    /* Info box */
    s_ui->info_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_set_halign(GTK_WIDGET (s_ui->info_hbox), GTK_ALIGN_START);

    /* Box container for action buttons */
    s_ui->btn_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_set_halign(GTK_WIDGET (s_ui->btn_hbox), GTK_ALIGN_CENTER);

    /* Cancel button */
    s_ui->cancel_btn = gtk_button_new_with_label("  Cancel  ");
    g_signal_connect_swapped(s_ui->cancel_btn, "clicked", G_CALLBACK(OnCancel), s_ui->window);
    gtk_box_pack_end (GTK_BOX (s_ui->btn_hbox), s_ui->cancel_btn, FALSE, FALSE, 0);

    /* Open button */
    s_ui->open_btn = gtk_button_new_with_label("  Open  ");
    g_signal_connect(s_ui->open_btn, "clicked", G_CALLBACK(OnOpen), (gpointer) s_ui);
    gtk_box_pack_end (GTK_BOX (s_ui->btn_hbox), s_ui->open_btn, FALSE, FALSE, 0);

    /* Combine everything onto the window */
    gtk_box_pack_start (GTK_BOX (s_ui->main_vbox), s_ui->info_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (s_ui->main_vbox), s_ui->btn_hbox, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(s_ui->window), s_ui->main_vbox);

    /* Exit when window closed */
    s_ui->close_handler_id = g_signal_connect(s_ui->window, "delete-event", G_CALLBACK(OnOpenDelete), NULL);

    /* Show */
    gtk_widget_show_all(s_ui->window);

    /* Load the available projects list */
    project_list(s_ui->list_box, s_ui);

    return;
}


/* Control container and widgets for project selection */

void select_proj_cntr(SelectProjUi *s_ui)
{  
    GtkWidget *row;

    /* Selection header */
    create_label(&(s_ui->selection_lbl), "title_4", "Please select a project", s_ui->main_vbox); 

    /* Project list */
    s_ui->list_box = gtk_list_box_new();
    s_ui->sel_handler_id = g_signal_connect(s_ui->list_box, "row-selected", G_CALLBACK(OnProjSelect), (gpointer) s_ui);

    s_ui->scroll_win = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_margin_start (GTK_WIDGET (s_ui->scroll_win), 5);
    gtk_widget_set_margin_top (GTK_WIDGET (s_ui->scroll_win), 5);
    gtk_widget_set_margin_bottom (GTK_WIDGET (s_ui->scroll_win), 5);
    gtk_widget_set_halign(GTK_WIDGET (s_ui->scroll_win), GTK_ALIGN_START);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (s_ui->scroll_win), 
    				    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_min_content_width (GTK_SCROLLED_WINDOW (s_ui->scroll_win), 250);
    gtk_container_add(GTK_CONTAINER (s_ui->scroll_win), s_ui->list_box);

    /* List column headers */
    s_ui->hdr_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    create_label(&(s_ui->nm_hdr), "head_1", "Name", s_ui->hdr_hbox);
    create_label(&(s_ui->desc_hdr), "head_1", "Description", s_ui->hdr_hbox);
    create_label(&(s_ui->date_hdr), "head_1", "Date Modified", s_ui->hdr_hbox);

    /* List heading */
    row = gtk_list_box_row_new();
    gtk_list_box_row_set_selectable(GTK_LIST_BOX_ROW (row), FALSE);
    gtk_list_box_prepend(GTK_LIST_BOX (s_ui->list_box), row);
    gtk_container_add(GTK_CONTAINER (row), s_ui->hdr_hbox);
    gtk_list_box_row_set_header (GTK_LIST_BOX_ROW (row), NULL);

    gtk_box_pack_start (GTK_BOX (s_ui->main_vbox), s_ui->scroll_win, FALSE, FALSE, 0);

    return;
}


/* Search for projects to select */

int project_list(GtkWidget *list_box, SelectProjUi *s_ui)
{  
    DIR *dp = NULL;
    struct dirent *ep;
    struct stat fileStat;
    int err, cnt;
    char *xml;
    ProjListEnt *list_ent;
    GtkWidget *row;

    /* Open project directory */
    if((dp = opendir(proj_dir)) == NULL)
    {
	log_msg("SYS9015", "Open Project directory", "SYS9015", s_ui->window);
        return FALSE;
    }

    /* Iterate thru the each project */
    cnt = 0;

    while (ep = readdir(dp))
    {
	/* Each project name must be a directory */
    	if ((err = stat(ep->d_name, &fileStat)) < 0)
	    continue;

    	if ((fileStat.st_mode & S_IFMT) != S_IFDIR)
	    continue;

	/* Each project directory must contain a 'proj_name_data.xml' file */
	xml = (char *) malloc((strlen(ep->d_name) * 2) + proj_dir_len + 12);
	sprintf(xml, "%s/%s/%s_data.xml", proj_dir, ep->d_name, ep->d_name);

    	if ((err = stat(xml, &fileStat)) < 0)
	    continue;

	list_ent = new_list_entry(xml, ep->d_name, &fileStat, s_ui);

	if (list_ent == NULL)
	{
	    free(xml);
	    continue;
	}

	cnt++;
	row = create_lstbox_row(list_ent);
	gtk_list_box_insert(GTK_LIST_BOX (list_box), row, -1);
	free(xml);
    }

    create_info(cnt, s_ui);
    gtk_widget_show_all(list_box);
    closedir(dp);

    return TRUE;
}


/* Create a new list entry for the list box */

ProjListEnt * new_list_entry(char *xml, char *proj_nm, struct stat *fileStat, SelectProjUi *s_ui)
{  
    ProjListEnt *list_ent;
    FILE *fd;
    struct tm mod_time, *tp;
    int count, sz;
    char *buf, *buf_ptr;
    const char *desc_start_tag = "<Description>";
    const char *desc_end_tag = "</Description>";

    /* New entry */
    list_ent = (ProjListEnt *) malloc(sizeof(ProjListEnt));

    /* Project name */
    list_ent->nm = strdup(proj_nm);

    /* Description */
    sz = fileStat->st_size + 1;
    buf = (char *) malloc(sz);

    fd = open_proj_file(xml, "r", s_ui->window);

    if ((count = read_proj_file(fd, buf, sz - 1, s_ui->window)) < 0)
    	return NULL;

    buf_ptr = buf;
    list_ent->desc = get_tag_val(&buf_ptr, desc_start_tag, desc_end_tag, TRUE, s_ui->window);
    free(buf);

    /* Last mod */
    tp = &mod_time;
    tp = localtime((const time_t) fileStat->st_mtime);
    list_ent->last_mod = (char *) malloc(18);
    strftime(list_ent->last_mod, 18, "%d-%b-%Y %H:%M", tp);

    return list_ent;
}


/* Create a row with project details to include in a list box */

GtkWidget * create_lstbox_row(ProjListEnt *list_ent)
{  
    GtkWidget *row, *nm_lbl, *desc_lbl, *mod_lbl, *row_hbox;

    row = gtk_list_box_row_new();
    gtk_list_box_row_set_selectable(GTK_LIST_BOX_ROW (row), TRUE);
    g_object_set_data_full (G_OBJECT (row), "list_ent", list_ent, (GDestroyNotify) free_list_ent);

    row_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

    create_label(&nm_lbl, "data_2", list_ent->nm, row_hbox);
    create_label(&desc_lbl, "data_2", list_ent->desc, row_hbox);
    create_label(&nm_lbl, "data_2", list_ent->last_mod, row_hbox);

    gtk_container_add(GTK_CONTAINER (row), row_hbox);

    return row;
}


/* Free a list entry */

void free_list_ent(gpointer data)
{
    ProjListEnt *ent;

    ent = (ProjListEnt *) data;

    free(ent->nm);
    free(ent->desc);
    free(ent->last_mod);
    free(ent);

    return;
}


/* Set up info line */

void create_info(int proj_cnt, SelectProjUi *s_ui)
{  
    char txt[25];

    sprintf(txt, "Projects found: %d", proj_cnt);
    create_label(&(s_ui->cnt_lbl), "title_5", txt, s_ui->info_hbox); 

    gtk_widget_show_all(s_ui->info_hbox);

    return;
}


/* CALLBACKS */


/* Callback - Select row */

void OnProjSelect(GtkListBox *lstbox, GtkListBoxRow *row, gpointer user_data)
{  
    SelectProjUi *s_ui;
    ProjListEnt *list_ent;

    /* Get data */
    s_ui = (SelectProjUi *) user_data;
    list_ent = (ProjListEnt *) g_object_get_data (G_OBJECT (row), "list_ent");

    return;
}


/* Callback - Open project */

void OnOpen(GtkWidget *lstbox, gpointer user_data)
{  

    return;
}


// Callback for window close
// Destroy the window and de-register the window 

void OnCancel(GtkWidget *window, gpointer user_data)
{ 
    SelectProjUi *ui;

    /* Get data */
    ui = (SelectProjUi *) g_object_get_data (G_OBJECT (window), "ui");

    /* Close the window, free the screen data and block any secondary close signal */
    window_cleanup(window, ui);

    return;
}


/* Window delete event */

gboolean OnOpenDelete(GtkWidget *window, GdkEvent *ev, gpointer user_data)
{
    OnCancel(window, user_data);

    return TRUE;
}


/* Window cleanup */

void window_cleanup(GtkWidget *window, SelectProjUi *ui)
{
    /* Unwanted callback action */
    g_signal_handler_block (ui->list_box, ui->sel_handler_id);
    
    /* Close the window, free the screen data and block any secondary close signal */
    g_signal_handler_block (window, ui->close_handler_id);

    deregister_window(window);
    gtk_window_close(GTK_WINDOW(window));

    free(ui);

    return;
}
