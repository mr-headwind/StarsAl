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
** Description: Preferences user interface and management.
**
** Author:	Anthony Buckley
**
** History
**	xx-xxx-2021	Initial code
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
#include <preferences.h>


/* Defines */


/* Types */

typedef struct _prefs_ui
{
    GtkWidget *window;
    GtkWidget *main_vbox, *btn_hbox;
    GtkWidget *prefs_cntr;

    GtkWidget *save_btn, *close_btn;

    GtkWidget *proj_dir;
    GtkWidget *browse_btn;

    int close_handler;
    int fn_err;
    GList *hide_list;
} PrefUi;


/* Prototypes */

int user_prefs_main(GtkWidget *);
int user_prefs_init(GtkWidget *);
PrefUi * new_pref_ui();
void user_prefs_ui(PrefUi *);
void prefs_control(PrefUi *);
void app_work_dir(PrefUi *);

void get_file_name(char *, char *, char *, char *, char, char, char);
void file_name_item(char *, char, char, char, char, char *, char *, char *); 

int read_user_prefs(GtkWidget *);
int write_user_prefs(GtkWidget *);
void set_default_prefs();
void default_dir_pref();
void default_backup_pref();
void set_user_prefs(PrefUi *);
int get_user_pref(char *, char **);
void get_user_pref_idx(int, char *, char **);
int match_key_val_combo(char *, char *, int, char **);
void get_pref_key(int, char *);
void get_pref_val(int, char **);
int set_user_pref(char *, char *);
int add_user_pref(char *, char *);
int add_user_pref_idx(char *, char *, int);
void delete_user_pref(char *);
int pref_save_reqd(PrefUi *);
int pref_changed(char *, char *);
int validate_pref(PrefUi *);
void free_prefs();
static void OnDirBrowse(GtkWidget*, gpointer);
static void OnPrefClose(GtkWidget*, gpointer);
static gboolean OnPrefDelete(GtkWidget*, GdkEvent *, gpointer);
static void OnPrefSave(GtkWidget*, gpointer);


extern char find_active_by_parent(GtkWidget *, char);
extern void create_label(GtkWidget **, char *, char *, GtkWidget *);
extern void log_msg(char*, char*, char*, GtkWidget*);
extern void register_window(GtkWidget *);
extern void deregister_window(GtkWidget *);
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
extern void string_trim(char *);


/* Globals */

static const char *debug_hdr = "DEBUG-prefs_ui.c ";
static int save_indi;
static GList *pref_list = NULL;
static GList *pref_list_head = NULL;
static int pref_count;


/* Display and maintenance of user preferences */

int user_prefs_main(GtkWidget *window)
{
    PrefUi *ui;

    /* Initial */
    if (! user_prefs_init(window))
    	return FALSE;

    ui = new_pref_ui();

    /* Create the interface */
    user_prefs_ui(ui);

    /* Register the window */
    register_window(ui->window);

    return TRUE;
}


/* Initial checks and values */

int user_prefs_init(GtkWidget *window)
{
    /* Initial */
    save_indi = FALSE;

    /* Load the user preferences */
    read_user_prefs(window);

    /* Should at least be a default set of user preferences */
    if (pref_count == 0)
    {
	log_msg("APP0005", "No user preferences", "APP0005", window);
    	return FALSE;
    }

    return TRUE;
}


/* Create new screen data variable */

PrefUi * new_pref_ui()
{
    PrefUi *ui = (PrefUi *) malloc(sizeof(PrefUi));
    memset(ui, 0, sizeof(PrefUi));

    return ui;
}


/* Create the user interface and set the CallBacks */

void user_prefs_ui(PrefUi *p_ui)
{
    int init;
    GtkWidget *tmp;

    /* Set up the UI window */
    p_ui->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(p_ui->window), USER_PREFS_UI);
    gtk_window_set_position(GTK_WINDOW(p_ui->window), GTK_WIN_POS_NONE);
    gtk_window_set_default_size(GTK_WINDOW(p_ui->window), 475, 400);
    gtk_container_set_border_width(GTK_CONTAINER(p_ui->window), 10);
    g_object_set_data (G_OBJECT (p_ui->window), "ui", p_ui);

    /* Main view */
    p_ui->main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    /* Main update or view grid */
    prefs_control(p_ui);

    /* Box container for action buttons */
    p_ui->btn_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_set_halign(GTK_WIDGET (p_ui->btn_hbox), GTK_ALIGN_CENTER);

    /* Close button */
    p_ui->close_btn = gtk_button_new_with_label("  Close  ");
    g_signal_connect_swapped(p_ui->close_btn, "clicked", G_CALLBACK(OnPrefClose), p_ui->window);
    gtk_box_pack_end (GTK_BOX (p_ui->btn_hbox), p_ui->close_btn, FALSE, FALSE, 0);

    /* Save button */
    p_ui->save_btn = gtk_button_new_with_label("  Apply  ");
    g_signal_connect(p_ui->save_btn, "clicked", G_CALLBACK(OnPrefSave), (gpointer) p_ui);
    gtk_box_pack_end (GTK_BOX (p_ui->btn_hbox), p_ui->save_btn, FALSE, FALSE, 0);

    /* Combine everything onto the window */
    gtk_box_pack_start (GTK_BOX (p_ui->main_vbox), p_ui->prefs_cntr, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (p_ui->main_vbox), p_ui->btn_hbox, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(p_ui->window), p_ui->main_vbox);

    /* Exit when window closed */
    p_ui->close_handler = g_signal_connect(p_ui->window, "delete-event", G_CALLBACK(OnPrefDelete), NULL);

    /* Show (or not) */
    gtk_widget_show_all(p_ui->window);

    p_ui->hide_list = g_list_first(p_ui->hide_list);

    while(p_ui->hide_list != NULL)
    {
	tmp = (GtkWidget *) p_ui->hide_list->data;
	gtk_widget_hide (tmp);
	p_ui->hide_list = g_list_next(p_ui->hide_list);
    }

    g_list_free(p_ui->hide_list);
    gtk_window_resize(GTK_WINDOW(p_ui->window), 475, 350);

    return;
}


/* Control container for user preference settings */

void prefs_control(PrefUi *p_ui)
{  
    GtkWidget *label;  
    int row;

    p_ui->prefs_cntr = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_name(p_ui->prefs_cntr, "pref_cntr");
    gtk_container_set_border_width (GTK_CONTAINER (p_ui->prefs_cntr), 10);

    /* Add each user preference setting */

    /* Application project working directory */
    app_work_dir(p_ui);

    return;
}


/* Project working directory - default */

void app_work_dir(PrefUi *p_ui)
{  
    char *p;

    /* Heading */
    create_label(&(p_ui->proj_dir), "data_1", "Project Directory", p_ui->prefs_cntr); 

    /* Put in horizontal box */
    p_ui->proj_dir = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);

    /* Directory */
    p_ui->proj_dir = gtk_entry_new();
    gtk_widget_set_name(p_ui->proj_dir, "proj_dir");
    gtk_widget_set_halign(GTK_WIDGET (p_ui->proj_dir), GTK_ALIGN_START);
    gtk_entry_set_max_length(GTK_ENTRY (p_ui->proj_dir), 256);
    gtk_entry_set_width_chars(GTK_ENTRY (p_ui->proj_dir), 40);
    gtk_box_pack_start (GTK_BOX (p_ui->proj_dir), p_ui->proj_dir, FALSE, FALSE, 3);

    get_user_pref(PROJ_DIR, &p);
    gtk_entry_set_text (GTK_ENTRY (p_ui->proj_dir), p);

    /* Browse button */
    p_ui->browse_btn = gtk_button_new_with_label("Browse...");
    g_signal_connect(p_ui->browse_btn, "clicked", G_CALLBACK(OnDirBrowse), (gpointer) p_ui);
    gtk_box_pack_start (GTK_BOX (p_ui->proj_dir), p_ui->browse_btn, FALSE, FALSE, 0);

    gtk_box_pack_start (GTK_BOX (p_ui->prefs_cntr), p_ui->proj_dir, FALSE, FALSE, 0);


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
	    log_msg("APP0008", "Invalid Preference format (key|value)", "APP0008", window);
	    return FALSE;
	}

	if ((p - buf) > (PREF_KEY_SZ - 1))
	{
	    free(prefs_fn);
	    sprintf(app_msg_extra, "%s\nMax key size: %d", buf, PREF_KEY_SZ);
	    log_msg("APP0008", "Invalid Preference Key size", "APP0008", window);
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


/* Write the user preferences file */

int write_user_prefs(GtkWidget *window)
{
    FILE *fd = NULL;
    char buf[256];
    char *prefs_fn;
    char *app_dir;
    int app_dir_len;

    /* Get the full path for the user settings file */
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


/* Set up default user preferences. All preferences may not be present */

void set_default_prefs()
{
    char *p;

    /* Default working directory */
    get_user_pref(PROJ_DIR, &p);

    if (p == NULL)
	default_dir_pref();

    /* Default backup directory */
    get_user_pref(BACKUP_DIR, &p);

    if (p == NULL)
	default_backup_pref();

    /* Save to file */
    write_user_prefs(NULL);

    /* Save to file */
    write_user_prefs(NULL);

    return;
}


/* Default working directory preference - $HOME/StarsAl */

void default_dir_pref()
{
    char *home_str, *val;
    int len;

    home_str = home_dir();
    len = strlen(home_str) + strlen(TITLE) + 2;
    val = (char *) malloc(len);
    sprintf(val, "%s/%s", home_str, TITLE);

    add_user_pref(PROJ_DIR, val);

    if (! check_dir(val))
	make_dir(val);

    free(val);

    return;
}


/* Default backup directory preference - $HOME/StarsAl/BACKUP */

void default_backup_pref()
{
    char *home_str, *val;
    int len;

    home_str = home_dir();
    len = strlen(home_str) + strlen(TITLE) + 9;
    val = (char *) malloc(len);
    sprintf(val, "%s/%s/BACKUP", home_str, TITLE);

    add_user_pref(BACKUP_DIR, val);

    if (! check_dir(val))
	make_dir(val);

    free(val);

    return;
}


/* Update all user preferences */

void set_user_prefs(PrefUi *p_ui)
{
    char cc;
    char s[3];
    int idx;
    const gchar *proj_dir;

    /* Initial */
    s[1] = '\0';

    /* Capture duration */
    proj_dir = gtk_entry_get_text(GTK_ENTRY (p_ui->proj_dir));
    set_user_pref(PROJ_DIR, (char *) proj_dir);

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


// Return a pointer to a user preference value for a key or NULL
// String match the key and part value

int match_key_val_combo(char *key, char *s_val, int s_len, char **val)
{
    int i, k_len;
    UserPrefData *Preference;

    i = 0;
    *val = NULL;
    k_len = strlen(key);

    pref_list = g_list_first(pref_list_head);

    while(pref_list != NULL)
    {
    	Preference = (UserPrefData *) pref_list->data;

    	if ((strncmp(Preference->key, key, k_len) == 0) &&
	    (strncmp(Preference->val, s_val, s_len) == 0))
    	{
	    *val = Preference->val;
	    break;
    	}

	pref_list = g_list_next(pref_list);
	i++;
    }

    return i;
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

int pref_save_reqd(PrefUi *p_ui)
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

    if (pref_changed(PROJ_DIR, (char *) proj_dir))
    	return TRUE;

    return FALSE;
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


/* Validate screen contents */

int validate_pref(PrefUi *p_ui)
{
    const gchar *s;
    int i;

    return TRUE;
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


/* Callback - Set capture directory */

void OnDirBrowse(GtkWidget *browse_btn, gpointer user_data)
{  
    GtkWidget *dialog;
    PrefUi *p_ui;
    gchar *dir_name;
    gint res;

    /* Get data */
    p_ui = (PrefUi *) user_data;

    /* Selection */
    dialog = gtk_file_chooser_dialog_new ("Project Location",
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


/* Callback for apply changes and close */

void OnPrefSave(GtkWidget *btn, gpointer user_data)
{
    PrefUi *ui;

    /* Get data */
    ui = (PrefUi *) user_data;

    /* Check for changes */
    if ((save_indi = pref_save_reqd(ui)) == FALSE)
    {
    	info_dialog(ui->window, "There are no changes to save!", "");
    	return;
    }

    /* Error check */
    if (validate_pref(ui) == FALSE)
    	return;

    if (ui->fn_err == TRUE)
    {
	log_msg("APP0001", NULL, "APP0001", ui->window);
    	return;
    }

    /* Store preferences */
    set_user_prefs(ui);

    /* Save to file */
    write_user_prefs(ui->window);

    return;
}


// Callback for window close
// Destroy the window and de-register the window 
// Check for changes

void OnPrefClose(GtkWidget *window, gpointer user_data)
{ 
    GtkWidget *dialog;
    PrefUi *ui;
    gint response;

    /* Get data */
    ui = (PrefUi *) g_object_get_data (G_OBJECT (window), "ui");

    /* Check for changes */
    if ((save_indi = pref_save_reqd(ui)) == TRUE)
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

gboolean OnPrefDelete(GtkWidget *window, GdkEvent *ev, gpointer user_data)
{
    OnPrefClose(window, user_data);

    return TRUE;
}
