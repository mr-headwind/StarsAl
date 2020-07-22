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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <defs.h>
#include <project.h>


/* Defines */


/* Types */


/* Prototypes */

int project_main(GtkWidget *);
int project_init(ProjectData *, GtkWidget *);
ProjectUi * new_proj_ui();
void project_ui(ProjectData *, ProjectUi *);
void proj_data(ProjectData *, ProjectUi *);
void select_images(ImageListUi *, ProjectUi *, char *);
int proj_save_reqd(ProjectUi *);
void set_proj(ProjectData *, ProjectUi *);
int save_proj(ProjectData *, ProjectUi *);
int validate_proj(ProjectUi *);

void OnProjCancel(GtkWidget*, gpointer);
gboolean OnProjDelete(GtkWidget*, GdkEvent *, gpointer);
void OnProjSave(GtkWidget*, gpointer);
void OnDirBrowse(GtkWidget*, gpointer);
void OnListClear(GtkWidget*, gpointer);
void OnListRemove(GtkWidget*, gpointer);

extern void create_label2(GtkWidget **, char *, char *, GtkWidget *, int, int, int, int);
extern int get_user_pref(char *, char **);
extern void log_msg(char*, char*, char*, GtkWidget*);
extern void register_window(GtkWidget *);
extern void deregister_window(GtkWidget *);
extern void string_trim(char *);





void prefs_control(PrefUi *);
void project_dir(PrefUi *);

char find_active_by_parent(GtkWidget *, char);
void get_file_name(char *, char *, char *, char *, char, char, char);
void file_name_item(char *, char, char, char, char, char *, char *, char *); 

int read_user_prefs(GtkWidget *);
int write_user_prefs(GtkWidget *);
void set_default_prefs();
void default_dir_pref();
void set_user_prefs(PrefUi *);
void get_user_pref_idx(int, char *, char **);
void get_pref_key(int, char *);
void get_pref_val(int, char **);
int set_user_pref(char *, char *);
int add_user_pref(char *, char *);
int add_user_pref_idx(char *, char *, int);
void delete_user_pref(char *);
int pref_changed(char *, char *);
void free_prefs();

extern void create_label(GtkWidget **, char *, char *, GtkWidget *);
extern char * app_dir_path();
extern char * home_dir();
extern GtkWidget * find_widget_by_name(GtkWidget *, char *);
extern void info_dialog(GtkWidget *, char *, char *);
extern gint query_dialog(GtkWidget *, char *, char *);
extern int check_dir(char *);
extern int make_dir(char *);
extern int is_ui_reg(char *, int);
extern int close_ui(char *);
extern int val_str2numb(char *, int *, char *, GtkWidget *);


/* Globals */

static const char *debug_hdr = "DEBUG-project_ui.c ";
static int save_indi;


/* Display and maintenance of project details */

int project_main(GtkWidget *window)
{
    ProjectUi *ui;
    ProjectData *proj;

    /* Initial */
    if (! project_init(proj, window))
    	return FALSE;

    ui = new_proj_ui();

    /* Create the interface */
    project_ui(proj, ui);

    /* Register the window */
    register_window(ui->window);

    return TRUE;
}


/* Initial checks and values */

int project_init(ProjectData *proj, GtkWidget *window)
{
    char *p;

    /* Initial */
    save_indi = FALSE;

    /* Get the application working directory */
    get_user_pref(WORK_DIR, &p);

    if (p == NULL)
    {
	log_msg("APP0009", "", "APP0009", window);
    	return FALSE;
    }

    proj->proj_path = p;

    return TRUE;
}


// USER INTERFACE


/* Create new screen data variable */

ProjectUi * new_proj_ui()
{
    ProjectUi *ui = (ProjectUi *) malloc(sizeof(ProjectUi));
    memset(ui, 0, sizeof(ProjectUi));

    return ui;
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
    gtk_window_set_default_size(GTK_WINDOW(p_ui->window), 475, 400);
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
    p_ui->nm_grid = gtk_grid_new();

    create_label2(&(p_ui->proj_nm_lbl), "title_4", "Project Name", p_ui->nm_grid, 0, 0, 1, 1);

    create_entry(&(p_ui->proj_nm), "proj_nm", p_ui->nm_grid, 0, 1);
    gtk_widget_set_halign(GTK_WIDGET (p_ui->proj_nm), GTK_ALIGN_START);
    gtk_entry_set_max_length(GTK_ENTRY (p_ui->proj_nm), 256);
    gtk_entry_set_width_chars(GTK_ENTRY (p_ui->proj_nm), 40);

    create_label2(&(p_ui->proj_path_lbl), "title_4", proj->proj_path, p_ui->nm_grid, 1, 1, 1, 1);

    gtk_box_pack_start (GTK_BOX (p_ui->proj_cntr), p_ui->nm_gridl, FALSE, FALSE, 0);

    /* Images selection */
    select_images(p_ui->images, p_ui, "Select Images");

    /* Darks */
    select_images(p_ui->darks, p_ui, "Select Darks");

    return;
}


/* Widgets for selecting images and darks */

void select_images(ImageListUi *lst, ProjectUi *p_ui, char *desc)
{  
    lst->img_grid = gtk_grid_new();

    create_label2(&(lst->img_lbl), "title_4", desc, lst->img_grid, 0, 0, 1, 1);

    lst->btn_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);

    lst->sel_btn = gtk_button_new_with_label("Browse...");
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

    gtk_grid_attach(GTK_GRID (lst->img_grid), lst->btn_vbox, 1, 1, 1, 1);

    lst->list_box = gtk_list_box_new();
    lst->scroll_win = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOw (lst->scroll_win), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER (lst->scroll_win), lst->list_box);
    gtk_grid_attach(GTK_GRID (lst->img_grid), lst->scroll_win, 0, 1, 1, 1);

    gtk_box_pack_start (GTK_BOX (p_ui->proj_cntr), p_ui->img_grid, FALSE, FALSE, 0);

    return;
}


// MAINTENANCE


/* Check if changes have been made */

int proj_save_reqd(ProjectUi *p_ui)
{
    gint res;
    char cc;
    char s[3];
    int idx;
    const gchar *proj_dir;

    /* Initial */
    s[1] = '\0';

    /* Project directory */
    proj_dir = gtk_entry_get_text(GTK_ENTRY (p_ui->proj_dir));

    /* Project directory exists */
    if (! check_dir((char *) proj_dir))
    {
	res = query_dialog(p_ui->window, "Location (%s) does not exist. Create it?", (char *) proj_dir);

	if (res == GTK_RESPONSE_YES)
	    make_dir((char *) proj_dir);
    }

    if (pref_changed(PROJECT_DIR, (char *) proj_dir))
    	return TRUE;

    return FALSE;
}


/* Validate screen contents */

int validate_proj(ProjectUi *p_ui)
{
    const gchar *s;

    /* Project name must be present */
    s = gtk_entry_get_text (GTKENTRY (p_ui->proj_nm));

    if (s == NULL)
    {
	log_msg("APP0005", "Project Name", "APP0005", window);
    	return FALSE;
    }

    if (string_trim((char *) s) == "")
    {
	log_msg("APP0005", "Project Name", "APP0005", window);
    	return FALSE;
    }

    /* Check consistency of image data */

    return TRUE;
}


/* Set up the project data */

void set_proj(ProjectData *proj, ProjectUi *p_ui)
{
    const gchar *s;

    /* Project name */
    s = gtk_entry_get_text (GTKENTRY (p_ui->proj_nm));

    if (proj->project_name == NULL)
    {
    	proj->project_name = (char *) malloc(strlen(s) + 1);
	proj->project_name[0] = '\0';
	proj->status = 0;
    }

    /* Make sure it's changed */
    if (strcmp(s, proj->project_nm) != 0)
    	strcpy(proj->project_name, s);

    /* Load the image list */

    /* Load the darks list */

    return;
}


/* Write the project file */

int save_proj(ProjectData *proj, ProjectUi *p_ui)
{
    FILE *fd = NULL;
    char buf[256];
    char *prefs_fn;
    char *app_dir;
    int app_dir_len;

    /* Project name */
    s = gtk_entry_get_text (GTKENTRY (p_ui->proj_nm));

    app_dir = app_dir_path();
    app_dir_len = strlen(app_dir);
    prefs_fn = (char *) malloc(app_dir_len + 19);
    sprintf(prefs_fn, "%s/app_settings", app_dir);

    /* New or overwrite file */
    if ((fd = fopen(prefs_fn, "w")) == (FILE *) NULL)
    {
	free(prefs_fn);
	return FALSE;
    }

    /* Write new values */
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

    /* Close off */
    fclose(fd);
    free(prefs_fn);
    save_indi = FALSE;

    return TRUE;
}


/* CALLBACKS */


/* Callback for apply changes and close */

void OnProjSave(GtkWidget *btn, gpointer user_data)
{
    ProjectUi *p_ui;
    ProjectData *proj;

    /* Get data */
    p_ui = (ProjectUi *) user_data;
    proj = (ProjectData *) g_object_get_data (G_OBJECT (p_ui->window), "proj");

    /* Check for changes */
    if ((save_indi = proj_save_reqd(p_ui)) == FALSE)
    {
    	info_dialog(p_ui->window, "There are no changes to save!", "");
    	return;
    }

    /* Error check */
    if (validate_proj(p_ui) == FALSE)
    	return;

    /* Set up the project data */
    set_proj(proj, p_ui);

    /* Save to file */
    save_proj(proj, p_ui);

    return;
}


/* Callback - Image selection */

void OnDirBrowse(GtkWidget *browse_btn, gpointer user_data)
{  
    GtkWidget *dialog;
    ProjectUi *p_ui;
    gchar *dir_name;
    gint res;

    /* Get data */
    p_ui = (ProjectUi *) user_data;

    /* Selection */
    dialog = gtk_file_chooser_dialog_new ("Select Images",
					  GTK_WINDOW (p_ui->window),
					  GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER,
					  "_Cancel", GTK_RESPONSE_CANCEL,
					  "_Apply", GTK_RESPONSE_APPLY,
					  NULL);

    res = gtk_dialog_run (GTK_DIALOG (dialog));

    if (res == GTK_RESPONSE_APPLY)
    {
	GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
	dir_name = gtk_file_chooser_get_filename (chooser);

	if (dir_name)
	{
	    gtk_entry_set_text (GTK_ENTRY (p_ui->proj_dir), dir_name);

	    if (! check_dir(dir_name))
	    {
		res = query_dialog(p_ui->window, "Location (%s) does not exist. Create it?", dir_name);

		if (res == GTK_RESPONSE_YES)
		    make_dir(dir_name);
	    }
	}

	g_free (dir_name);
    }

    gtk_widget_destroy (dialog);

    return;
}


/* Callback - Clear Image List */

void OnListClear(GtkWidget *browse_btn, gpointer user_data)
{  
    ProjectUi *p_ui;

    /* Get data */
    p_ui = (ProjectUi *) user_data;

    return;
}


/* Callback - Clear Image List */

void OnListRemove(GtkWidget *browse_btn, gpointer user_data)
{  
    ProjectUi *p_ui;

    /* Get data */
    p_ui = (ProjectUi *) user_data;

    return;
}


// Callback for window close
// Destroy the window and de-register the window 
// Check for changes

void OnProjCancel(GtkWidget *window, gpointer user_data)
{ 
    GtkWidget *dialog;
    ProjectUi *ui;
    gint response;

    /* Get data */
    ui = (ProjectUi *) g_object_get_data (G_OBJECT (window), "ui");

    /* Check for changes */
    if ((save_indi = proj_save_reqd(ui)) == TRUE)
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
    g_signal_handler_block (window, ui->close_handler);

    deregister_window(window);
    gtk_window_close(GTK_WINDOW(window));

    free(ui);

    return;
}


/* Window delete event */

gboolean OnProjDelete(GtkWidget *window, GdkEvent *ev, gpointer user_data)
{
    OnProjCancel(window, user_data);

    return TRUE;
}










/* Project working directory - default */

void project_dir(PrefUi *p_ui)
{  
    char *p;

    /* Heading */
    create_label(&(p_ui->proj_dir_lbl), "data_1", "Working Directory", p_ui->prefs_cntr); 

    /* Put in horizontal box */
    p_ui->proj_dir_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);

    /* Directory */
    p_ui->proj_dir = gtk_entry_new();
    gtk_widget_set_name(p_ui->proj_dir, "proj_dir");
    gtk_widget_set_halign(GTK_WIDGET (p_ui->proj_dir), GTK_ALIGN_START);
    gtk_entry_set_max_length(GTK_ENTRY (p_ui->proj_dir), 256);
    gtk_entry_set_width_chars(GTK_ENTRY (p_ui->proj_dir), 40);
    gtk_box_pack_start (GTK_BOX (p_ui->proj_dir_box), p_ui->proj_dir, FALSE, FALSE, 3);

    get_user_pref(WORK_DIR, &p);
    gtk_entry_set_text (GTK_ENTRY (p_ui->proj_dir), p);

    /* Browse button */
    p_ui->browse_btn = gtk_button_new_with_label("Browse...");
    g_signal_connect(p_ui->browse_btn, "clicked", G_CALLBACK(OnDirBrowse), (gpointer) p_ui);
    gtk_box_pack_start (GTK_BOX (p_ui->proj_dir_box), p_ui->browse_btn, FALSE, FALSE, 0);

    gtk_box_pack_start (GTK_BOX (p_ui->prefs_cntr), p_ui->proj_dir_box, FALSE, FALSE, 0);


    return;
}


/* Read the user preferences file */

int read_user_prefs(GtkWidget *window)
{
    FILE *fd = NULL;
    struct stat fileStat;
    char buf[256];
    char *prefs_fn;
    char *app_dir;
    char *p, *p2;
    int app_dir_len;
    int err;

    /* Initial */
    pref_count = 0;

    /* Get the full path for the preferences file */
    app_dir = app_dir_path();
    app_dir_len = strlen(app_dir);
    prefs_fn = (char *) malloc(app_dir_len + 19);
    sprintf(prefs_fn, "%s/%s", app_dir, USER_PREFS);

    /* If no preferences exist, create a default set */
    err = stat(prefs_fn, &fileStat);

    if ((err < 0) || (fileStat.st_size == 0))
    {
	log_msg("APP0007", NULL, NULL, NULL);
    	set_default_prefs();
	free(prefs_fn);

	return TRUE;
    }
    
    /* Read existing user preference settings */
    if ((fd = fopen(prefs_fn, "r")) == (FILE *) NULL)
    {
	free(prefs_fn);
	return FALSE;
    }
    
    /* Store the preferences */
    while ((fgets(buf, sizeof(buf), fd)) != NULL)
    {
	/* Check and save key */
	if ((p = strchr(buf, '|')) == NULL)
	{
	    free(prefs_fn);
	    sprintf(app_msg_extra, "%s", buf);
	    log_msg("APP0008", "Invalid user preference key format", "APP0008", window);
	    return FALSE;
	}

	if ((p - buf) > (PREF_KEY_SZ - 1))
	{
	    free(prefs_fn);
	    sprintf(app_msg_extra, "%s", buf);
	    log_msg("APP0008", "Invalid user preference key size", "APP0008", window);
	    return FALSE;
	}

	/* Check and save value */
	if ((p2 = strchr((p), '\n')) == NULL)
	{
	    free(prefs_fn);
	    sprintf(app_msg_extra, "%s", buf);
	    log_msg("APP0008", "Invalid user preference value", "APP0008", window);
	    return FALSE;
	}

	/* Create a preference entry */
	UserPrefData *Preference = (UserPrefData *) malloc(sizeof(UserPrefData));
	memset(Preference, 0, sizeof (UserPrefData));
	strncpy(Preference->key, buf, p - buf);
	Preference->key[p - buf] = '\0';
	string_trim(Preference->key);

	p++;
	*p2 = '\0';

	Preference->val = (char *) malloc(strlen(p) + 1);
	strcpy(Preference->val, p);
	string_trim(Preference->val);
	    
	pref_list = g_list_prepend(pref_list, Preference);
	pref_count++;
    }

    /* Still may need set up some default preferences */
    pref_list_head = g_list_reverse(pref_list);
    set_default_prefs();

    /* Close off */
    fclose(fd);
    free(prefs_fn);
    save_indi = FALSE;

    return TRUE;
}


/* Set up default user preferences. All preferences may not be present */

void set_default_prefs()
{
    char *p;

    /* Default working directory */
    get_user_pref(PROJECT_DIR, &p);

    if (p == NULL)
	default_dir_pref();

    /* Save to file */
    write_user_prefs(NULL);

    return;
}


/* Default working directory preference  - $HOME/StarsAl */

void default_dir_pref()
{
    char *home_str, *val;
    int len;

    home_str = home_dir();
    len = strlen(home_str) + strlen(TITLE) + 11;
    val = (char *) malloc(len);
    sprintf(val, "%s/%s", home_str, TITLE);

    add_user_pref(PROJECT_DIR, val);

    if (! check_dir(val))
	make_dir(val);

    free(val);

    return;
}


/* Add a user preference */

int add_user_pref(char *key, char *val)
{
    UserPrefData *Preference;

    Preference = (UserPrefData *) malloc(sizeof(UserPrefData));
    strcpy(Preference->key, key);
    Preference->val = (char *) malloc(strlen(val) + 1);
    strcpy(Preference->val, val);

    pref_list = g_list_append(pref_list_head, (gpointer) Preference);
    pref_count++;

    if (pref_list_head == NULL)
    	pref_list_head = pref_list;

    return TRUE;
}


/* Add a user preference at a given reference */

int add_user_pref_idx(char *key, char *val, int idx)
{
    UserPrefData *Preference;

    Preference = (UserPrefData *) malloc(sizeof(UserPrefData));
    strcpy(Preference->key, key);
    Preference->val = (char *) malloc(strlen(val) + 1);
    strcpy(Preference->val, val);

    pref_list_head = g_list_insert(pref_list_head, (gpointer) Preference, idx);
    pref_count++;

    return TRUE;
}


/* Update a user preference */

int set_user_pref(char *key, char *val)
{
    int i;
    UserPrefData *Preference;

    /* Find the key entry and set the new value */
    pref_list = g_list_first(pref_list_head);
    i = 0;

    while(pref_list != NULL)
    {
    	Preference = (UserPrefData *) pref_list->data;

    	if (strcmp(Preference->key, key) == 0)
    	{
	    i = strlen(val);

	    if (i > strlen(Preference->val))
		Preference->val = (char *) realloc(Preference->val, i + 1);

	    strcpy(Preference->val, val);
	    break;
    	}

	pref_list = g_list_next(pref_list);
    }

    if (i == 0)
    	return FALSE;

    return TRUE;
}


/* Return a pointer to a user preference value for a key or NULL */

int get_user_pref(char *key, char **val)
{
    int i;
    UserPrefData *Preference;

    *val = NULL;
    i = 0;

    pref_list = g_list_first(pref_list_head);

    while(pref_list != NULL)
    {
    	Preference = (UserPrefData *) pref_list->data;

    	if (strcmp(Preference->key, key) == 0)
    	{
	    *val = Preference->val;
	    break;
    	}

	pref_list = g_list_next(pref_list);
	i++;
    }

    return i;
}


/* Return a pointer to a user preference value for a key and index or NULL */

void get_user_pref_idx(int idx, char *key, char **val)
{
    UserPrefData *Preference;

    *val = NULL;

    pref_list = g_list_nth(pref_list_head, idx);
    Preference = (UserPrefData *) pref_list->data;

    if (strcmp(Preference->key, key) == 0)
	*val = Preference->val;

    return;
}


/* Return the key at an index */

void get_pref_key(int idx, char *key)
{
    UserPrefData *Preference;

    pref_list = g_list_nth(pref_list_head, idx);
    Preference = (UserPrefData *) pref_list->data;

    if (pref_list == NULL)
    	key = NULL;
    else
    	strcpy(key, Preference->key);

    return;
}


/* Return a pointer to the value at an index */

void get_pref_val(int idx, char **val)
{
    UserPrefData *Preference;

    pref_list = g_list_nth(pref_list_head, idx);
    Preference = (UserPrefData *) pref_list->data;

    if (pref_list == NULL)
    	*val = NULL;
    else
	*val = Preference->val;

    return;
}


/* Delete a preference setting */

void delete_user_pref(char *key)
{
    UserPrefData *Preference;
    GList *llink = NULL;

    llink = g_list_first(pref_list_head);

    while(llink != NULL)
    {
    	Preference = (UserPrefData *) llink->data;

    	if (strcmp(Preference->key, key) == 0)
    	{
	    pref_list_head = g_list_remove_link(pref_list_head, llink);
	    free(Preference->val);
	    free(Preference);
	    g_list_free(llink);
	    break;
    	}

	llink = g_list_next(llink);
    }

    return;
}


/* Check if changes have been made */

int pref_changed(char *key, char *val)
{
    char *p;

    get_user_pref(key, &p);

    if (strcmp(p, val) != 0)
    	return TRUE;

    return FALSE;
}


/* Free the user preferences */

void free_prefs()
{
    UserPrefData *Preference;

    pref_list = g_list_first(pref_list_head);

    while(pref_list != NULL)
    {
    	Preference = (UserPrefData *) pref_list->data;
    	free(Preference->val);
    	free(Preference);

	pref_list = g_list_next(pref_list);
    }

    g_list_free(pref_list_head);

    return;
}
