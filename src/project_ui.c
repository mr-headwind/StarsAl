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
void show_list(SelectListUi *, GSList *);
int proj_save_reqd(ProjectData *, ProjectUi *);
void setup_proj(ProjectData *, const gchar *, GList *, GList *);
int save_proj(ProjectData *, ProjectUi *);
int setup_proj_validate(ProjectData *, ProjectUi *);
void img_list(GList *, GList **, ProjectUi *);
Image * setup_image(char *, ProjectUi *);
int load_exif_data(Image *, ProjectUi *);
static char * get_tag(ExifData *, ExifIfd, ExifTag);
void window_cleanup(GtkWidget *, ProjectUi *);

static void OnProjCancel(GtkWidget*, gpointer);
static gboolean OnProjDelete(GtkWidget*, GdkEvent *, gpointer);
static void OnProjSave(GtkWidget*, gpointer);
static void OnDirBrowse(GtkWidget*, gpointer);
static void OnListClear(GtkWidget*, gpointer);
static void OnListRemove(GtkWidget*, gpointer);

extern void create_label2(GtkWidget **, char *, char *, GtkWidget *, int, int, int, int);
extern void create_entry(GtkWidget **, char *, GtkWidget *, int, int);
extern int get_user_pref(char *, char **);
extern void basename_dirname(char *, char **, char **);
extern void log_msg(char*, char*, char*, GtkWidget*);
extern void register_window(GtkWidget *);
extern void deregister_window(GtkWidget *);
extern void string_trim(char *);
extern void trim_spaces(char *);
extern int check_dir(char *);
extern int make_dir(char *);


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
    /* Set up containers */
    lst->sel_fr = gtk_frame_new(desc);
    gtk_widget_set_name(lst->sel_fr, "clr1");

    lst->sel_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);

    lst->btn_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_halign(GTK_WIDGET (lst->btn_vbox), GTK_ALIGN_END);
    gtk_widget_set_margin_bottom (GTK_WIDGET (lst->btn_vbox), 5);
    gtk_widget_set_margin_top (GTK_WIDGET (lst->btn_vbox), 5);

    /* List maintenance buttons */
    lst->sel_btn = gtk_button_new_with_label("Browse...");
    g_object_set_data (G_OBJECT (lst->sel_btn), "list", lst);
    g_object_set_data_full (G_OBJECT (lst->sel_btn), "heading", g_strdup (desc), (GDestroyNotify) g_free);
    gtk_widget_set_valign (lst->sel_btn, GTK_ALIGN_END);
    gtk_box_pack_start (GTK_BOX (lst->btn_vbox), lst->sel_btn, FALSE, FALSE, 0);
    g_signal_connect(lst->sel_btn, "clicked", G_CALLBACK(OnDirBrowse), (gpointer) p_ui);

    lst->clear_btn = gtk_button_new_with_label("Clear");
    gtk_widget_set_valign (lst->clear_btn, GTK_ALIGN_END);
    gtk_box_pack_start (GTK_BOX (lst->btn_vbox), lst->clear_btn, FALSE, FALSE, 0);
    g_signal_connect(lst->clear_btn, "clicked", G_CALLBACK(OnListClear), (gpointer) p_ui);

    lst->remove_btn = gtk_button_new_with_label("Remove");
    gtk_widget_set_valign (lst->remove_btn, GTK_ALIGN_END);
    gtk_box_pack_start (GTK_BOX (lst->btn_vbox), lst->remove_btn, FALSE, FALSE, 0);
    g_signal_connect(lst->remove_btn, "clicked", G_CALLBACK(OnListRemove), (gpointer) p_ui);

    /* Images or Darks list */
    lst->list_box = gtk_list_box_new();
    lst->scroll_win = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (lst->scroll_win), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER (lst->scroll_win), lst->list_box);

    /* List heading */

    /* Pack them up */
    gtk_box_pack_start (GTK_BOX (lst->sel_hbox), lst->scroll_win, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (lst->sel_hbox), lst->btn_vbox, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER (lst->sel_fr), lst->sel_hbox);
    gtk_box_pack_start (GTK_BOX (p_ui->proj_cntr), lst->sel_fr, FALSE, FALSE, 0);

    return;
}


/* Set up the list box with the selected files and maintain a selected glist */

void show_list(SelectListUi *lst, GSList *gsl)
{  
    char *path1, *path2, *nm, *dir;
    GList *l;
    GSList *sl;

    lst->list_box = gtk_list_box_new();

    for(sl = gsl; sl != NULL; sl = sl->next)
    {
	path1 = (char *) sl->data;

	for(l = lst->files; l != NULL; l = l->next)
	{
	    path2 = (char *) l->data;

	    if (strcmp(path1, path2) == 0)
	    	break;
	}

	if (l == NULL)
	{
	    basename_dirname(path1, &nm, &dir);

	    GtkWidget *lbl = gtk_label_new(nm);
	    g_object_set_data_full (G_OBJECT (lbl), "dir", g_strdup (dir), (GDestroyNotify) g_free);
	    gtk_list_box_insert(GTK_LIST_BOX (lst->list_box), lbl, -1);
	    lst->files = g_list_prepend(lst->files, path1);

	    free(nm);
	    free(dir);
	}
    }

    lst->files = g_list_reverse(lst->files);
    g_slist_free_full(gsl, (GDestroyNotify) g_free);

    return;
}


// MAINTENANCE


/* Check if changes have been made */

int proj_save_reqd(ProjectData *proj, ProjectUi *p_ui)
{
    const gchar *s;

    /* Project name */
    s = gtk_entry_get_text(GTK_ENTRY (p_ui->proj_nm));

    if (proj->project_name == NULL)
    	return TRUE;

    if (s == NULL)			// Error - but continue to validate
    	return TRUE;

    if (strcmp(s, proj->project_name) != 0)
    	return TRUE;
    	
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

    if (nm == NULL)
    {
	log_msg("APP0005", "Project Name", "APP0005", p_ui->window);
    	return FALSE;
    }

    string_trim((char *) nm);
    if (*nm == '\0')
    {
	log_msg("APP0005", "Project Name", "APP0005", p_ui->window);
    	return FALSE;
    }

    /* Need to compile a list of Images and exif data */
    img_list(p_ui->images.files, &images_gl, p_ui);

    /* Need to compile a list of Darks and exif data */
    img_list(p_ui->darks.files, &darks_gl, p_ui);

    /* Check all the images for exposure consistency */

    /* Discard and warn of unusable darks */

    /* Set up project */
    setup_proj(proj, nm, images_gl, darks_gl);

    return TRUE;
}


/* Compile a list of images and exif data */

void img_list(GList *in_gl, GList **out_gl, ProjectUi *p_ui)
{
    char *f;
    Image *img;
    GList *l;

    *out_gl = NULL;

    for(l = in_gl; l != NULL; l = l->next)
    {
    	f = (char *) l->data;
    	img = setup_image(f, p_ui);
    	*out_gl = g_list_prepend(*out_gl, img);
    }

    *out_gl = g_list_reverse(*out_gl);

    return;
}


/* Set up all Image data */

Image * setup_image(char *image_path, ProjectUi *p_ui)
{  
    Image *img;

    img = (Image *) malloc(sizeof(Image));
    basename_dirname(image_path, &(img->nm), &(img->path));

    load_exif_data(img, p_ui);

    return img;
}


/* Extract the image Exif data (if any) */

int load_exif_data(Image *img, ProjectUi *p_ui)
{  
    char *s;
    ExifData *ed;
    ExifEntry *entry;

    /* Load an ExifData object from an EXIF file */
    s = (char *) malloc(strlen(img->path) + strlen(img->nm) + 1);
    sprintf(s, "%s/%s", img->path, img->nm);

    ed = exif_data_new_from_file(s);

    if (!ed)
    {
	log_msg("APP0010", s, "APP0010", p_ui->window);
        free(s);
        return FALSE;
    }

    img->img_exif.make = get_tag(ed, EXIF_IFD_0, EXIF_TAG_MAKE);
    img->img_exif.model = get_tag(ed, EXIF_IFD_0, EXIF_TAG_MODEL);
    img->img_exif.type = get_tag(ed, EXIF_IFD_0, EXIF_TAG_NEW_SUBFILE_TYPE);
    img->img_exif.date = get_tag(ed, EXIF_IFD_0, EXIF_TAG_DATE_TIME);
    img->img_exif.width = get_tag(ed, EXIF_IFD_0, EXIF_TAG_PIXEL_X_DIMENSION);
    img->img_exif.height = get_tag(ed, EXIF_IFD_0, EXIF_TAG_PIXEL_Y_DIMENSION);
    img->img_exif.iso = get_tag(ed, EXIF_IFD_0, EXIF_TAG_ISO_SPEED_RATINGS);
    img->img_exif.exposure = get_tag(ed, EXIF_IFD_0, EXIF_TAG_EXPOSURE_TIME);
    img->img_exif.f_stop = get_tag(ed, EXIF_IFD_0, EXIF_TAG_FNUMBER);
    
    /* Free the EXIF and other data */
    free(s);
    exif_data_unref(ed);

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
            printf("%s - %s: %s\n", debug_hdr, exif_tag_get_name_in_ifd(tag,ifd), buf);
	    s = (char *) malloc(strlen(buf) + 1);
	    strcpy(s, buf);
        }
	else
	{
	    s = NULL;
	}
    }

    return s;
}


/* Set up the project data */

void setup_proj(ProjectData *proj, const gchar *nm, GList *images_gl, GList *darks_gl)
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
    proj->images_gl = images_gl;
    proj->darks_gl = darks_gl;

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
    g_object_set_data (G_OBJECT (lst->sel_btn), "list", lst);
    lst = (SelectListUi *) g_object_get_data (G_OBJECT (browse_btn), "list");
    heading = (char *) g_object_get_data (G_OBJECT (browse_btn), "heading");

    /* Selection */
    dialog = gtk_file_chooser_dialog_new (heading,
					  GTK_WINDOW (p_ui->window),
					  GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER,
					  "_Cancel", GTK_RESPONSE_CANCEL,
					  "_Apply", GTK_RESPONSE_APPLY,
					  NULL);

    res = gtk_dialog_run (GTK_DIALOG (dialog));

    if (res == GTK_RESPONSE_APPLY)
    {
	GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
	gtk_file_chooser_set_select_multiple (chooser, TRUE);
	gtk_file_chooser_set_action (chooser, GTK_FILE_CHOOSER_ACTION_OPEN);
	gsl = gtk_file_chooser_get_filenames (chooser);

	if (lst->files != NULL)
	    save_indi = TRUE;
    }

    gtk_widget_destroy (dialog);

    /* Add the selected files to the List widget */
    show_list(lst, gsl);

    return;
}


/* Callback - Clear Image List */

void OnListClear(GtkWidget *browse_btn, gpointer user_data)
{  
    ProjectUi *p_ui;

    /* Get data */
    p_ui = (ProjectUi *) user_data;

    save_indi = TRUE;

    return;
}


/* Callback - Clear Image List */

void OnListRemove(GtkWidget *browse_btn, gpointer user_data)
{  
    ProjectUi *p_ui;

    /* Get data */
    p_ui = (ProjectUi *) user_data;

    save_indi = TRUE;

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

void window_cleanup(GtkWidget *window, ProjectUi *ui)
{
    /* Free any list and filenames */
    g_list_free_full(ui->images.files, (GDestroyNotify) g_free);
    g_list_free_full(ui->darks.files, (GDestroyNotify) g_free);

    /* Close the window, free the screen data and block any secondary close signal */
    g_signal_handler_block (window, ui->close_handler);

    deregister_window(window);
    gtk_window_close(GTK_WINDOW(window));

    free(ui);

    return;
}
