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
** Description: Project user interface and management.
**
** Author:	Anthony Buckley
**
** History
**	12-Jul-2021	Initial code
**
*/


/* Includes */

#include <gtk/gtk.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libexif/exif-data.h>
#include <libexif/exif-tag.h>
#include <libexif/exif-loader.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <defs.h>
#include <project.h>
#include <preferences.h>


/* Defines */


/* Types */


/* Prototypes */

int project_main(ProjectData *, GtkWidget *);
int project_init(GtkWidget *);
ProjectUi * new_proj_ui();
ProjectData * new_proj_data();
void project_ui(ProjectData *, ProjectUi *);
void close_project(ProjectData *);
void free_img(gpointer);
void proj_data(ProjectData *, ProjectUi *);
void select_images(SelectListUi *, ProjectUi *, char *);
void show_list(SelectListUi *, GSList *, ProjectUi *p_ui);
GtkWidget * create_lstbox_row(char *, char *);
void remove_image_list_row(SelectListUi *, GtkWidget *, GtkListBoxRow *);
void clear_image_list(SelectListUi *, GtkWidget *);
Image * setup_image(char *, char *, char *, ProjectUi *);
int load_exif_data(Image *, char *, ProjectUi *);
static char * get_tag(ExifData *, ExifIfd, ExifTag);
int proj_save_reqd(ProjectData *, ProjectUi *);
int setup_proj_validate(ProjectData *, ProjectUi *);
int validate_images(GList *, ProjectUi *);
int convert_exif(ImgExif *, int *, int *, int *, GtkWidget *);
void check_arr_int(int, int *, int *);
void check_arr_s(char *, char **, int *);
void setup_proj(ProjectData *, const gchar *, ProjectUi *p_ui);
void copy_glist(GList *, GList *);
int save_proj(ProjectData *, ProjectUi *);
static void window_cleanup(GtkWidget *, ProjectUi *);

static void OnProjCancel(GtkWidget*, gpointer);
static gboolean OnProjDelete(GtkWidget*, GdkEvent *, gpointer);
static void OnProjSave(GtkWidget*, gpointer);
static void OnDirBrowse(GtkWidget*, gpointer);
static void OnListClear(GtkWidget*, gpointer);
static void OnListRemove(GtkWidget*, gpointer);
static void OnRowSelect(GtkListBox*, GtkListBoxRow*, gpointer);

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
extern int check_dir(char *);
extern int make_dir(char *);
extern char * image_type(char *, GtkWidget *);
extern int val_str2numb(char *, int *, char *, GtkWidget *);


/* Globals */

static const char *debug_hdr = "DEBUG-project_ui.c ";
static int save_indi;
static char *proj_dir;
static int proj_dir_len;


/* Display and maintenance of project details */

int project_main(ProjectData *proj_in, GtkWidget *window)
{
    ProjectData *proj;
    ProjectUi *ui;

    /* Initial */
    if (! project_init(window))
    	return FALSE;

    /* Load and/or initialise project */
    if (! proj_in)
    	proj = new_proj_data();
    else
    	proj = proj_in;

    ui = new_proj_ui();

    /* Create the interface */
    project_ui(proj, ui);

    /* Register the window */
    register_window(ui->window);

    return TRUE;
}


/* Initial checks and values */

int project_init(GtkWidget *window)
{
    char *p;

    /* Initial */
    save_indi = FALSE;

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

ProjectUi * new_proj_ui()
{
    ProjectUi *ui = (ProjectUi *) malloc(sizeof(ProjectUi));
    memset(ui, 0, sizeof(ProjectUi));

    return ui;
}


/* Create new project data structure*/

ProjectData * new_proj_data()
{
    ProjectData *proj = (ProjectData *) malloc(sizeof(ProjectData));
    memset(proj, 0, sizeof(ProjectData));

    proj->project_path = (char *) malloc(proj_dir_len + 1);
    strcpy(proj->project_path, proj_dir);

    return proj;
}


/* Create the user interface and set the CallBacks */

void project_ui(ProjectData *proj, ProjectUi *p_ui)
{
    int init;
    GtkWidget *tmp;

    /* Set up the UI window */
    p_ui->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(p_ui->window), PROJECT_UI);
    gtk_window_set_position(GTK_WINDOW(p_ui->window), GTK_WIN_POS_NONE);
    gtk_window_set_default_size(GTK_WINDOW(p_ui->window), 400, 400);
    gtk_container_set_border_width(GTK_CONTAINER(p_ui->window), 10);
    g_object_set_data (G_OBJECT (p_ui->window), "ui", p_ui);
    g_object_set_data (G_OBJECT (p_ui->window), "proj", proj);

    /* Main view */
    p_ui->main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    /* Main update or view grid */
    proj_data(proj, p_ui);

    /* Box container for action buttons */
    p_ui->btn_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_set_halign(GTK_WIDGET (p_ui->btn_hbox), GTK_ALIGN_CENTER);

    /* Cancel button */
    p_ui->cancel_btn = gtk_button_new_with_label("  Cancel  ");
    g_signal_connect_swapped(p_ui->cancel_btn, "clicked", G_CALLBACK(OnProjCancel), p_ui->window);
    gtk_box_pack_end (GTK_BOX (p_ui->btn_hbox), p_ui->cancel_btn, FALSE, FALSE, 0);

    /* Save button */
    p_ui->save_btn = gtk_button_new_with_label("  Save  ");
    g_signal_connect(p_ui->save_btn, "clicked", G_CALLBACK(OnProjSave), (gpointer) p_ui);
    gtk_box_pack_end (GTK_BOX (p_ui->btn_hbox), p_ui->save_btn, FALSE, FALSE, 0);

    /* Combine everything onto the window */
    gtk_box_pack_start (GTK_BOX (p_ui->main_vbox), p_ui->proj_cntr, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (p_ui->main_vbox), p_ui->btn_hbox, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(p_ui->window), p_ui->main_vbox);

    /* Exit when window closed */
    p_ui->close_handler = g_signal_connect(p_ui->window, "delete-event", G_CALLBACK(OnProjDelete), NULL);

    /* Show */
    gtk_widget_show_all(p_ui->window);

    return;
}


/* Control container and widgets for project data */

void proj_data(ProjectData *proj, ProjectUi *p_ui)
{  
    GtkWidget *label;  
    int row;

    /* Main container */
    p_ui->proj_cntr = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_name(p_ui->proj_cntr, "proj_cntr");
    gtk_container_set_border_width (GTK_CONTAINER (p_ui->proj_cntr), 10);

    /* Project title and path */
    p_ui->title_fr = gtk_frame_new("Project Title");
    gtk_widget_set_name(p_ui->title_fr, "clr1");
    p_ui->nm_grid = gtk_grid_new();
    gtk_grid_set_row_spacing (GTK_GRID (p_ui->nm_grid), 5);
    gtk_grid_set_column_spacing (GTK_GRID (p_ui->nm_grid), 7);

    create_label2(&(p_ui->proj_path_lbl), "lbl", proj->project_path, p_ui->nm_grid, 1, 0, 1, 1);

    create_label2(&(p_ui->proj_nm_lbl), "title_4", "Name", p_ui->nm_grid, 0, 1, 1, 1);
    gtk_widget_set_halign(p_ui->proj_nm_lbl, GTK_ALIGN_END);
    gtk_widget_set_margin_start (GTK_WIDGET (p_ui->proj_nm_lbl), 20);
    gtk_widget_set_margin_bottom (GTK_WIDGET (p_ui->proj_nm_lbl), 5);

    create_entry(&(p_ui->proj_nm), "proj_nm", p_ui->nm_grid, 1, 1);
    gtk_widget_set_halign(GTK_WIDGET (p_ui->proj_nm), GTK_ALIGN_START);
    gtk_widget_set_margin_bottom (GTK_WIDGET (p_ui->proj_nm), 5);
    gtk_entry_set_max_length(GTK_ENTRY (p_ui->proj_nm), 256);
    gtk_entry_set_width_chars(GTK_ENTRY (p_ui->proj_nm), 30);

    gtk_container_add(GTK_CONTAINER (p_ui->title_fr), p_ui->nm_grid);
    gtk_box_pack_start (GTK_BOX (p_ui->proj_cntr), p_ui->title_fr, FALSE, FALSE, 0);

    /* Images selection */
    select_images(&(p_ui->images), p_ui, "Select Images");

    /* Darks */
    select_images(&(p_ui->darks), p_ui, "Select Darks");

    return;
}


/* Widgets for selecting images and darks */

void select_images(SelectListUi *lst, ProjectUi *p_ui, char *desc)
{  
    GtkWidget *heading_lbl;

    /* Set up containers */
    lst->sel_fr = gtk_frame_new(desc);
    gtk_widget_set_name(lst->sel_fr, "clr1");

    lst->sel_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);

    lst->btn_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_halign(GTK_WIDGET (lst->btn_vbox), GTK_ALIGN_END);
    gtk_widget_set_valign(GTK_WIDGET (lst->btn_vbox), GTK_ALIGN_END);
    gtk_widget_set_margin_bottom (GTK_WIDGET (lst->btn_vbox), 15);
    gtk_widget_set_margin_top (GTK_WIDGET (lst->btn_vbox), 15);

    lst->sel_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);

    /* List maintenance buttons */
    lst->sel_btn = gtk_button_new_with_label("Browse...");
    g_object_set_data (G_OBJECT (lst->sel_btn), "list", lst);
    g_object_set_data_full (G_OBJECT (lst->sel_btn), "heading", g_strdup (desc), (GDestroyNotify) g_free);
    gtk_widget_set_valign (lst->sel_btn, GTK_ALIGN_END);
    gtk_box_pack_start (GTK_BOX (lst->btn_vbox), lst->sel_btn, FALSE, FALSE, 0);
    g_signal_connect(lst->sel_btn, "clicked", G_CALLBACK(OnDirBrowse), (gpointer) p_ui);

    lst->clear_btn = gtk_button_new_with_label("Clear");
    g_object_set_data (G_OBJECT (lst->clear_btn), "list", lst);
    gtk_widget_set_valign (lst->clear_btn, GTK_ALIGN_END);
    gtk_box_pack_start (GTK_BOX (lst->btn_vbox), lst->clear_btn, FALSE, FALSE, 0);
    g_signal_connect(lst->clear_btn, "clicked", G_CALLBACK(OnListClear), (gpointer) p_ui);

    lst->remove_btn = gtk_button_new_with_label("Remove");
    g_object_set_data (G_OBJECT (lst->remove_btn), "list", lst);
    gtk_widget_set_valign (lst->remove_btn, GTK_ALIGN_END);
    gtk_box_pack_start (GTK_BOX (lst->btn_vbox), lst->remove_btn, FALSE, FALSE, 0);
    g_signal_connect(lst->remove_btn, "clicked", G_CALLBACK(OnListRemove), (gpointer) p_ui);

    /* Images or Darks list */
    lst->list_box = gtk_list_box_new();
    lst->sel_handler = g_signal_connect(lst->list_box, "row-selected", G_CALLBACK(OnRowSelect), (gpointer) lst);

    lst->scroll_win = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_margin_start (GTK_WIDGET (lst->scroll_win), 5);
    gtk_widget_set_margin_top (GTK_WIDGET (lst->scroll_win), 5);
    gtk_widget_set_margin_bottom (GTK_WIDGET (lst->scroll_win), 5);
    gtk_widget_set_halign(GTK_WIDGET (lst->scroll_win), GTK_ALIGN_START);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (lst->scroll_win), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    //gtk_scrolled_window_set_propagate_natural_width (GTK_SCROLLED_WINDOW (lst->scroll_win), TRUE);
    gtk_scrolled_window_set_min_content_width (GTK_SCROLLED_WINDOW (lst->scroll_win), 250);
    gtk_container_add(GTK_CONTAINER (lst->scroll_win), lst->list_box);

    /* List heading */
    create_label3(&(heading_lbl), "head_1", "Selected Files");
    gtk_list_box_prepend(GTK_LIST_BOX (lst->list_box), heading_lbl);

    /* Image meta data for selected list box row */
    create_label4(&(lst->dir_lbl), "data_4", " ", 4, 3, GTK_ALIGN_START);
    create_label4(&(lst->meta_lbl), "data_4", " ", 4, 3, GTK_ALIGN_START);

    /* Pack them up */
    gtk_box_pack_start (GTK_BOX (lst->sel_hbox), lst->scroll_win, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (lst->sel_hbox), lst->btn_vbox, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (lst->sel_vbox), lst->sel_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (lst->sel_vbox), lst->dir_lbl, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (lst->sel_vbox), lst->meta_lbl, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER (lst->sel_fr), lst->sel_vbox);
    gtk_box_pack_start (GTK_BOX (p_ui->proj_cntr), lst->sel_fr, FALSE, FALSE, 0);

    return;
}


/* Add and maintain the selected files for the list box along with full image details in the selected glist */

void show_list(SelectListUi *lst, GSList *gsl, ProjectUi *p_ui)
{  
    char *path1, *path2, *nm, *dir;
    GtkWidget *row;
    GList *l;
    GSList *sl;
    Image *image, *tmpimg;

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

	    image = setup_image(nm, dir, path1, p_ui);
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


/* Remove a row from a list box of selected images and the associated GList */

void remove_image_list_row(SelectListUi *lst, GtkWidget *list_box, GtkListBoxRow *row)
{  
    Image *img, *tmp;
    GList *l;

    /* Remove the image from the associated GList */
    img = (Image *) g_object_get_data (G_OBJECT (row), "image");

    for(l = lst->img_files; l != NULL; l = l->next)
    {
	tmp = (Image *) l->data;

	if (strcmp(tmp->nm, img->nm) == 0 && strcmp(tmp->path, img->path) == 0)
	{
	    free_img(img);
	    lst->img_files = g_list_delete_link(l, l);
	    break;
	}
    };

    /* Destroy the row container and show the new list box */
    g_signal_handler_block (lst->list_box, lst->sel_handler);
    gtk_widget_destroy (GTK_WIDGET (row));
    gtk_widget_show_all(list_box);
    gtk_label_set_text(GTK_LABEL (lst->dir_lbl), "");
    gtk_label_set_text(GTK_LABEL (lst->meta_lbl), "");

    g_signal_handler_unblock (lst->list_box, lst->sel_handler);
    save_indi = TRUE;

    return;
}


/* Remove all rows from a list box of selected images and the associated GList */

void clear_image_list(SelectListUi *lst, GtkWidget *list_box)
{  
    int i, rows;
    GtkListBoxRow *row;

    /* Clear the associated GList */
    rows = g_list_length(lst->img_files);
    g_list_free_full(lst->img_files, (GDestroyNotify) free_img);
    lst->img_files = NULL;

    /* Destroy each row container */
    g_signal_handler_block (lst->list_box, lst->sel_handler);

    for(i = rows; i > 0; i--)
    {
	row = gtk_list_box_get_row_at_index (GTK_LIST_BOX (lst->list_box), i);
	gtk_widget_destroy (GTK_WIDGET (row));
    };

    g_signal_handler_unblock (lst->list_box, lst->sel_handler);
    gtk_widget_show_all(list_box);
    gtk_label_set_text(GTK_LABEL (lst->dir_lbl), "");
    gtk_label_set_text(GTK_LABEL (lst->meta_lbl), "");
    save_indi = TRUE;

    return;
}


/* Set up all Image data */

Image * setup_image(char *nm, char *dir, char *image_full_path, ProjectUi *p_ui)
{  
    Image *img;

    img = (Image *) malloc(sizeof(Image));
    img->nm = (char *) malloc(strlen(nm) + 1);
    img->path = (char *) malloc(strlen(dir) + 1);
    strcpy(img->nm, nm);
    strcpy(img->path, dir);

    load_exif_data(img, image_full_path, p_ui);

    return img;
}


/* Extract the image Exif data (if any) */

int load_exif_data(Image *img, char *full_path, ProjectUi *p_ui)
{  
    ExifData *ed;
    ExifEntry *entry;

    /* Load an ExifData object from an EXIF file */
    ed = exif_data_new_from_file(full_path);

    if (!ed)
    {
	log_msg("APP0010", full_path, "APP0010", p_ui->window);
        free(full_path);
        return FALSE;
    }

    img->img_exif.make = get_tag(ed, EXIF_IFD_0, EXIF_TAG_MAKE);
    img->img_exif.model = get_tag(ed, EXIF_IFD_0, EXIF_TAG_MODEL);
    img->img_exif.date = get_tag(ed, EXIF_IFD_0, EXIF_TAG_DATE_TIME);
    img->img_exif.width = get_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_PIXEL_X_DIMENSION);
    img->img_exif.height = get_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_PIXEL_Y_DIMENSION);
    img->img_exif.iso = get_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_ISO_SPEED_RATINGS);
    img->img_exif.exposure = get_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_TIME);
    img->img_exif.f_stop = get_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_FNUMBER);
    /*
    img->img_exif.make = get_tag(ed, EXIF_IFD_0, EXIF_TAG_MAKE);
    img->img_exif.model = get_tag(ed, EXIF_IFD_0, EXIF_TAG_MODEL);
    img->img_exif.type = get_tag(ed, EXIF_IFD_0, STARSAL_EXIF_TAG_RECORD_MODE);
    img->img_exif.date = get_tag(ed, EXIF_IFD_0, EXIF_TAG_DATE_TIME);
    img->img_exif.width = get_tag(ed, EXIF_IFD_0, STARSAL_EXIF_IMAGE_WIDTH);
    img->img_exif.height = get_tag(ed, EXIF_IFD_0, STARSAL_EXIF_IMAGE_HEIGHT);
    img->img_exif.iso = get_tag(ed, EXIF_IFD_0, STARSAL_EXIF_TAG_ISO);
    img->img_exif.exposure = get_tag(ed, EXIF_IFD_0, STARSAL_EXIF_TAG_EXP);
    img->img_exif.f_stop = get_tag(ed, EXIF_IFD_0, STARSAL_EXIF_TAG_FSTOP);
    */
    
    /* Free the EXIF */
    exif_data_unref(ed);
    
    /* Not really exif data, but as far as possible, get the image type here */
    img->img_exif.type = image_type(full_path, p_ui->window);
    printf("%s - Type: %s\n", debug_hdr, img->img_exif.type); fflush(stdout);

    return TRUE;
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


// MAINTENANCE


/* Check if changes have been made */

int proj_save_reqd(ProjectData *proj, ProjectUi *p_ui)
{
    const gchar *s;
    int len;

    /* Project name */
    s = gtk_entry_get_text(GTK_ENTRY (p_ui->proj_nm));
    len = (int) gtk_entry_get_text_length(GTK_ENTRY (p_ui->proj_nm));

    if (proj->project_name == NULL && len == 0)
	return save_indi;

    if (proj->project_name == NULL)
    {
	save_indi = TRUE;
    }
    else
    {
	if (strcmp(s, proj->project_name) != 0)
	    save_indi = TRUE;
    }
    	
    return save_indi;
}


/* Validate screen contents */

int proj_setup_validate(ProjectData *proj, ProjectUi *p_ui)
{
    const gchar *nm;
    SelectListUi images;
    SelectListUi darks;
    GList *images_gl;
    GList *darks_gl;

    /* Project name must be present */
    nm = gtk_entry_get_text (GTK_ENTRY (p_ui->proj_nm));
    string_trim((char *) nm);

    if (*nm == '\0')
    {
	log_msg("APP0005", "Project Name", "APP0005", p_ui->window);
    	return FALSE;
    }

    /* Check all the images for exposure consistency */
    if ((validate_images(p_ui->images.img_files, p_ui)) == FALSE)
    	return FALSE;

    /* Discard and warn of unusable darks */

    /* Set up project */
    setup_proj(proj, nm, p_ui);

    return TRUE;
}


/* Validate images for exposure consistency (iso, exposure, width, height) */

int validate_images(GList *img_files, ProjectUi *p_ui)
{
    typedef struct
    {
	int iso;
	char *exp;
	int width;
	int height;
    } Array;

    Array *img_arr;
    int i, img_cnt, iso, w, h, r;
    int iso_fnd, exp_fnd, w_fnd, h_fnd;
    Image *img;
    ImgExif e;
    GList *l;

    /* Set up a dynamic array as count of each element to check */
    img_cnt = g_list_length(img_files);
    img_arr = malloc(img_cnt * sizeof(Array));
    memset(img_arr, 0, img_cnt * sizeof(Array));

    /* Iterate each image file and check the exposure data */
    for(l = img_files; l != NULL; l = l->next)
    {
	img = (Image *) l->data;
	e = img->img_exif;
	iso_fnd = exp_fnd = w_fnd = h_fnd = FALSE;

	if (convert_exif(&e, &iso, &w, &h, p_ui->window) == FALSE)
	    break;
printf("%s validate_images  1  exp  %s  iso %d  w %d  h %d\n", debug_hdr, e.exposure, iso, w, h); fflush(stdout);

	for(i = 0; i < img_cnt; i++)
	{
	    check_arr_s(e.exposure, &(img_arr[i].exp), &exp_fnd);
	    check_arr_int(iso, &(img_arr[i].iso), &iso_fnd);
	    check_arr_int(w, &(img_arr[i].width), &w_fnd);
	    check_arr_int(h, &(img_arr[i].height), &h_fnd);

printf("%s validate_images  4  array exp  %s  iso %d  w %d  h %d\n", debug_hdr, 
			img_arr[i].exp, img_arr[i].iso, img_arr[i].width, img_arr[i].height); fflush(stdout);

printf("%s validate_images  5  found iso %d  exp  %d  w %d  h %d\n", debug_hdr, iso_fnd, exp_fnd, w_fnd,
									h_fnd); fflush(stdout);
	    if (iso_fnd == TRUE  &&  exp_fnd == TRUE  &&  w_fnd == TRUE  &&  h_fnd == TRUE)
	    	break;
	};
    };

    /* Check for errors - more than one row means images are not consistent */
    for(i = 0; i < img_cnt; i++)
    {
	if (img_arr[i].iso == 0  &&  img_arr[i].width == 0  &&  img_arr[i].width == 0  && !(img_arr[i].exp))
	    break;
    }

    if (i > 1)
    {
	log_msg("APP0012", "", "APP0012", p_ui->window);
    	r = FALSE;
    }
    else
    {
    	r = TRUE;
    }

    /* Free the array */
    for(i = 0; i < img_cnt; i++)
    {
    	if (img_arr[i].exp)
    	    free(img_arr[i].exp);
    }

    free(img_arr);
    
    return r;
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


/* Check if an integer exif item is present and add if necessary */

void check_arr_int(int i, int *arr_i, int *fnd)
{
    if (*arr_i == 0)
    {
	*arr_i = i;
	*fnd = TRUE;
    }
    else
    {
	if (*arr_i == i)
	    *fnd = TRUE;
    }

    return;
}


/* Check if a string exif item is present and add if necessary */

void check_arr_s(char *s, char **arr_s, int *fnd)
{
    if (! (*arr_s))
    {
printf("%s check_arr_s  1 arr s in null\n", debug_hdr); fflush(stdout);
	*arr_s = (char *) malloc(strlen(s) + 1);
	strcpy(*arr_s, s);
	*fnd = TRUE;
    }
    else
    {
printf("%s check_arr_s  2 arr s in not null  arr %s  s %s\n", debug_hdr, *arr_s, s); fflush(stdout);
	if (strcmp(*arr_s, s) == 0)
	{
	    *fnd = TRUE;
printf("%s check_arr_s  3 arr s match\n", debug_hdr); fflush(stdout);
	}
    }

    return;
}


/* Set up the project data */

void setup_proj(ProjectData *proj, const gchar *nm, ProjectUi *p_ui)
{
    /* Project name */
    if (proj->project_name == NULL)
    {
    	proj->project_name = (char *) malloc(strlen(nm) + 1);
	proj->project_name[0] = '\0';
	proj->project_path = (char *) malloc(strlen(proj_dir) + 1);
	strcpy(proj->project_path, proj_dir);
	proj->status = 0;
    }

    /* Make sure it's changed */
    if (strcmp(nm, proj->project_name) != 0)
    	strcpy(proj->project_name, nm);

    /* Images and Darks */
    copy_glist(p_ui->images.img_files, proj->images_gl);
    copy_glist(p_ui->darks.img_files, proj->darks_gl);

    return;
}


/* Compile a list of images and exif data */

void copy_glist(GList *in_gl, GList *out_gl)
{
    char *f;
    Image *img;
    GList *l;

    out_gl = NULL;

    for(l = in_gl; l != NULL; l = l->next)
    {
    	img = (Image *) l->data;
    	out_gl = g_list_prepend(out_gl, img);
    }

    out_gl = g_list_reverse(out_gl);

    return;
}


/* Write the project file */

int save_proj(ProjectData *proj, ProjectUi *p_ui)
{
    int nml, pathl;
    char *path;
    FILE *fd = NULL;
    char buf[256];
    char *proj_fn;

    /* Project directory exists */
    nml = strlen(proj->project_name);
    pathl = strlen(proj->project_path);

    path = (char *) malloc(proj_dir_len + pathl + nml + 3);
    sprintf(path, "%s/%s/%s", proj_dir, proj->project_path, proj->project_name);

    if (! check_dir((char *) path))
	make_dir((char *) path);

    /* New or overwrite file */
    proj_fn = (char *) malloc(proj_dir_len + pathl + nml + nml + 8);
    sprintf(proj_fn, "%s/%s_data", path, proj->project_name);

    if ((fd = fopen(proj_fn, "w")) == (FILE *) NULL)
    {
	free(proj_fn);
	return FALSE;
    }

    /* Write new values */
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
    free(path);
    save_indi = FALSE;

    return TRUE;
}


/* Close the current project */

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


/* CALLBACKS */


/* Callback - Image selection */

void OnDirBrowse(GtkWidget *browse_btn, gpointer user_data)
{  
    GtkWidget *dialog;
    ProjectUi *p_ui;
    SelectListUi *lst;
    GSList *gsl;
    char *heading;
    gint res;

    /* Get data */
    p_ui = (ProjectUi *) user_data;
    lst = (SelectListUi *) g_object_get_data (G_OBJECT (browse_btn), "list");
    heading = (char *) g_object_get_data (G_OBJECT (browse_btn), "heading");

    /* Selection  dialog */
    dialog = gtk_file_chooser_dialog_new (heading,
					  GTK_WINDOW (p_ui->window),
					  GTK_FILE_CHOOSER_ACTION_OPEN,
					  "_Cancel", GTK_RESPONSE_CANCEL,
					  "_Apply", GTK_RESPONSE_APPLY,
					  NULL);

    GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
    gtk_file_chooser_set_action (chooser, GTK_FILE_CHOOSER_ACTION_OPEN);
    gtk_file_chooser_set_select_multiple (chooser, TRUE);

    res = gtk_dialog_run (GTK_DIALOG (dialog));

    /* Add the selected files to the List widget */
    if (res == GTK_RESPONSE_APPLY)
    {
	gsl = gtk_file_chooser_get_filenames (chooser);

	if (gsl != NULL)
	{
	    show_list(lst, gsl, p_ui);
	}
    }

    gtk_widget_destroy (dialog);
    gtk_widget_show_all(p_ui->window);

    return;
}


/* Callback - Clear the images selected list */

void OnListClear(GtkWidget *clear_btn, gpointer user_data)
{  
    ProjectUi *p_ui;
    SelectListUi *lst;

    /* Get data */
    p_ui = (ProjectUi *) user_data;
    lst = (SelectListUi *) g_object_get_data (G_OBJECT (clear_btn), "list");

    if (lst->img_files == NULL)
    	return;

    clear_image_list(lst, lst->list_box);

    return;
}


/* Callback - Remove a row from the images selected list */

void OnListRemove(GtkWidget *remove_btn, gpointer user_data)
{  
    ProjectUi *p_ui;
    SelectListUi *lst;
    GtkListBoxRow *row;

    /* Get data */
    p_ui = (ProjectUi *) user_data;
    lst = (SelectListUi *) g_object_get_data (G_OBJECT (remove_btn), "list");
    row = gtk_list_box_get_selected_row (GTK_LIST_BOX (lst->list_box));

    if (row == NULL)
    {
    	app_msg("APP0011", "a row", p_ui->window);
    	return;
    }

    remove_image_list_row(lst, lst->list_box, row);

    return;
}


/* Callback - Select row */

void OnRowSelect(GtkListBox *lstbox, GtkListBoxRow *row, gpointer user_data)
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


/* Callback for apply changes and close */

void OnProjSave(GtkWidget *btn, gpointer user_data)
{
    ProjectUi *ui;
    ProjectData *proj;

    /* Get data */
    ui = (ProjectUi *) user_data;
    proj = (ProjectData *) g_object_get_data (G_OBJECT (ui->window), "proj");

    /* Ignore if save is not required */
    if ((save_indi = proj_save_reqd(proj, ui)) == FALSE)
    	return;

    /* Error check */
    if (proj_setup_validate(proj, ui) == FALSE)
    	return;

    /* Save to file */
    save_proj(proj, ui);

    /* Close the window, free the screen data and block any secondary close signal */
    window_cleanup(ui->window, ui);

    return;
}


// Callback for window close
// Destroy the window and de-register the window 
// Check for changes

void OnProjCancel(GtkWidget *window, gpointer user_data)
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
    g_signal_handler_block (ui->images.list_box, ui->images.sel_handler);
    g_signal_handler_block (ui->darks.list_box, ui->darks.sel_handler);
    
    /* Free any list and filenames */
    g_list_free_full(ui->images.img_files, (GDestroyNotify) free_img);
    g_list_free_full(ui->darks.img_files, (GDestroyNotify) free_img);

    /* Close the window, free the screen data and block any secondary close signal */
    g_signal_handler_block (window, ui->close_handler);

    deregister_window(window);
    gtk_window_close(GTK_WINDOW(window));

    free(ui);

    return;
}
