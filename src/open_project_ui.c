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
    GtkWidget *hdr_hbox, *proj_hbox;
    GtkWidget *nm_hdr, *desc_hdr, *date_hdr;
    GtkWidget *proj_nm, *proj_desc, *mod_date;
    GtkWidget *open_btn, *cancel_btn;
    int close_handler_id, sel_handler_id;
    ProjectData *proj;
} SelectProjUi;


/* Prototypes */

void open_project_main(MainUi *, GtkWidget *);
int sel_proj_init(GtkWidget *);
ProjectUi * new_sel_proj_ui();
void sel_proj_ui(,SelectProjUi *, MainUi *);
void select_proj_cntr(SelectProjUi *);
void project_list(GtkWidget *, SelectProjUi *);

void select_images(SelectListUi *, ProjectUi *, char *);
void show_list(SelectListUi *, GSList *, ProjectUi *s_ui);
GtkWidget * create_lstbox_row(char *, char *);
Image * setup_image(char *, char *, char *, ProjectUi *);
static char * get_tag(ExifData *, ExifIfd, ExifTag);
void setup_proj(ProjectData *, ProjectUi *s_ui);
static void window_cleanup(GtkWidget *, ProjectUi *);

static void OnProjSelect(GtkListBox*, GtkListBoxRow*, gpointer);
static void OnOpen(GtkWidget*, gpointer);
static void OnCancel(GtkWidget*, gpointer);

extern ProjectData * new_proj_data();
extern void create_label2(GtkWidget **, char *, char *, GtkWidget *, int, int, int, int);
extern void create_label3(GtkWidget **, char *, char *);
extern void create_label4(GtkWidget **, char *, char *, gint, gint, GtkAlign);
extern void create_entry(GtkWidget **, char *, GtkWidget *, int, int);
extern int get_user_pref(char *, char **);
extern void basename_dirname(char *, char **, char **);
extern void log_msg(char*, char*, char*, GtkWidget*);
extern void app_msg(char*, char*, GtkWidget*);
extern void register_window(GtkWidget *);
extern void deregister_window(GtkWidget *);
extern void string_trim(char *);
extern void trim_spaces(char *);
extern char * image_type(char *, GtkWidget *);


/* Globals */

static const char *debug_hdr = "DEBUG-project_ui.c ";
static int save_indi;
static char *proj_dir;
static int proj_dir_len;


/* Project selection */

void open_project_main(ProjectData *proj, MainUi *m_ui)
{
    ProjectOpenUi *ui;

    /* Initial */
    if (! sel_proj_init(window))
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
    gtk_box_pack_end (GTK_BOX (s_ui->btn_hbox), s_ui->ok_btn, FALSE, FALSE, 0);

    /* Combine everything onto the window */
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
    int row;

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

void project_list(GtkWidget *list_box, SelectProjUi *s_ui)
{  
    char *path1, *path2, *nm, *dir;
    GtkWidget *row;
    GList *l;
    GSList *sl;
    Image *image, *tmpimg;

    /*
    get project directory
    get the sub-dir names (projects)
    check each directory has .xml starsal file
    get title / desc tags
    stat to get mod date
    add to list
    */

    for(sl = gsl; sl != NULL; sl = sl->next)
    {
	path1 = (char *) sl->data;

	for(l = lst->img_files; l != NULL; l = l->next)
	{
	    tmpimg = (Image *) l->data;
	    path2 = (char *) malloc(strlen(tmpimg->path) + strlen(tmpimg->nm) + 2);
	    sprintf(path2, "%s/%s", tmpimg->path, tmpimg->nm);

	    if (strcmp(path1, path2) == 0)
	    {
	    	free(path2);
	    	break;
	    }

	    free(path2);
	}

	if (l == NULL)
	{
	    basename_dirname(path1, &nm, &dir);
	    row = create_lstbox_row(nm, dir);
	    gtk_list_box_insert(GTK_LIST_BOX (lst->list_box), row, -1);

	    image = setup_image(nm, dir, path1, s_ui);
	    lst->img_files = g_list_prepend(lst->img_files, image);
	    g_object_set_data (G_OBJECT (row), "image", image);

	    save_indi = TRUE;
	    free(nm);
	    free(dir);
	}
    }

    gtk_widget_show_all(lst->list_box);
    lst->img_files = g_list_reverse(lst->img_files);
    g_slist_free_full(gsl, (GDestroyNotify) g_free);

/*
printf("%s show_list 1\n", debug_hdr); fflush(stdout);
printf("%s show_list 2  %s\n", debug_hdr, path1); fflush(stdout);
printf("%s show_list 3\n", debug_hdr); fflush(stdout);
printf("%s show_list 5  %s  %s\n", debug_hdr, nm, dir); fflush(stdout);
printf("%s show_list 6  %s \n", debug_hdr, path1); fflush(stdout);
for(l = lst->img_files; l != NULL; l = l->next)
{
printf("%s show_list 7  files: %s\n", debug_hdr, (char *) l->data); fflush(stdout);
}
*/

    return;
}


/* Create a row with a label to include in a list box */

GtkWidget * create_lstbox_row(char *nm, char *dir)
{  
    GtkWidget *row, *lbl;

    row = gtk_list_box_row_new();
    gtk_list_box_row_set_selectable(GTK_LIST_BOX_ROW (row), TRUE);

    create_label4(&lbl, "data_4", nm, 4, 3, GTK_ALIGN_START);
    g_object_set_data_full (G_OBJECT (lbl), "dir", g_strdup (dir), (GDestroyNotify) g_free);
    gtk_container_add(GTK_CONTAINER (row), lbl);

    return row;
}


/* Extract tag and contents if exists */

static char * get_tag(ExifData *d, ExifIfd ifd, ExifTag tag)
{
    char buf[1024];
    char *s;

    /* See if this tag exists */
    ExifEntry *entry = exif_content_get_entry(d->ifd[ifd], tag);

    if (entry) 
    {
        /* Get the contents of the tag in human-readable form */
        exif_entry_get_value(entry, buf, sizeof(buf));

        /* Don't bother printing it if it's entirely blank */
        trim_spaces(buf);

        if (*buf)
        {
            printf("%s - %s: %s\n", debug_hdr, exif_tag_get_name_in_ifd(tag,ifd), buf); fflush(stdout);
	    s = (char *) malloc(strlen(buf) + 1);
	    strcpy(s, buf);
	    return s;
        }
    }

    s = (char *) malloc(4);
    sprintf(s, "N/A");

    return s;
}


/* Set up the new or edited project data - for edits, backup the old one and start again */

void setup_proj(ProjectData *proj, ProjectUi *s_ui)
{
    const gchar *nm;
    int len;
    char *s;

    /* Project name */
    nm = gtk_entry_get_text (GTK_ENTRY (s_ui->proj_nm));
    string_trim((char *) nm);
    len = strlen(nm);

    if (proj->project_name == NULL)			// New project
    {
    	proj->project_name = (char *) malloc(len + 1);
	proj->project_name[0] = '\0';
    }
    else 						// Edit
    {
	s = (char*) malloc(strlen(proj->project_path) + 5);
	sprintf(s, "%s.bak", proj->project_path);

	if (rename(proj->project_path, s) < 0)
	{
	    sprintf(app_msg_extra, "Error: (%d) %s", errno, strerror(errno));
	    log_msg("SYS9009", proj->project_name, "SYS9009", s_ui->window);
	    return;
	}
	else
	{
	    log_msg("SYS9010", proj->project_name, NULL, NULL);
	}

	proj->project_name = (char *) realloc(proj->project_name, len + 1);
	free(s);
    }

    strcpy(proj->project_name, nm);
    proj->project_path = (char *) malloc(proj_dir_len + len + 2);
    sprintf(proj->project_path, "%s/%s", proj_dir, nm);

    /* Project status */
    proj->status = 0;

    /* Images and Darks */
    proj->images_gl = g_list_copy(s_ui->images.img_files);
    proj->darks_gl = g_list_copy(s_ui->darks.img_files);

    return;
}


/* CALLBACKS */


/* Callback - Select row */

void OnProjSelect(GtkListBox *lstbox, GtkListBoxRow *row, gpointer user_data)
{  
    char *s;
    SelectListUi *s_ui;
    Image *img;

    /* Get data */
    s_ui = (SelectListUi *) user_data;
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


/* Callback - Open project */

void OnOpen(GtkWidget *lstboxgpointer user_data)
{  

    return;
}


// Callback for window close
// Destroy the window and de-register the window 
// Check for changes

void OnCancel(GtkWidget *window, gpointer user_data)
{ 
    GtkWidget *dialog;
    ProjectUi *ui;
    ProjectData *proj;
    gint response;

    /* Get data */
    ui = (ProjectUi *) g_object_get_data (G_OBJECT (window), "ui");
    proj = (ProjectData *) g_object_get_data (G_OBJECT (window), "proj");

    /* Check for changes */
    if ((save_indi = proj_save_reqd(proj, ui)) == TRUE)
    {
	/* Ask if OK to close without saving */
	dialog = gtk_message_dialog_new (GTK_WINDOW (window),
					 GTK_DIALOG_MODAL,
					 GTK_MESSAGE_QUESTION,
					 GTK_BUTTONS_OK_CANCEL,
					 "Close without saving changes?");

	response = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);

	if (response == GTK_RESPONSE_CANCEL)
	    return;
    }

    /* Close the window, free the screen data and block any secondary close signal */
    window_cleanup(window, ui);

    return;
}


/* Window delete event */

gboolean OnProjDelete(GtkWidget *window, GdkEvent *ev, gpointer user_data)
{
    OnProjCancel(window, user_data);

    return TRUE;
}


/* Window cleanup */

static void window_cleanup(GtkWidget *window, ProjectUi *ui)
{
    /* Unwanted callback action */
    g_signal_handler_block (ui->images.list_box, ui->images.sel_handler_id);
    g_signal_handler_block (ui->darks.list_box, ui->darks.sel_handler_id);
    
    /* Free the images and darks lists, but not the images attached as they are now attached to the project */
    g_list_free(ui->images.img_files);
    g_list_free(ui->darks.img_files);
    /*
    g_list_free_full(ui->images.img_files, (GDestroyNotify) free_img);
    
    if (ui->darks.img_files)
	g_list_free_full(ui->darks.img_files, (GDestroyNotify) free_img);
    else
    	g_list_free(ui->darks.img_files);
    */

    /* Close the window, free the screen data and block any secondary close signal */
    g_signal_handler_block (window, ui->close_handler_id);

    deregister_window(window);
    gtk_window_close(GTK_WINDOW(window));

    free(ui);

    return;
}
