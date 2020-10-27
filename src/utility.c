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
** Description:
**  Error and Message Reference functions
**  Logging functions
**  Window management
**  General usage functions
**
** Author:	Anthony Buckley
**
** History
**	xx-xxx-2021	Initial code
**
*/


/* Defines */

#define ERR_FILE


/* Includes */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <gtk/gtk.h>
#include <defs.h>


/* Prototypes */

void app_msg(char*, char *, GtkWidget*);
void log_msg(char*, char*, char*, GtkWidget*);
void info_dialog(GtkWidget *, char *, char *);
gint query_dialog(GtkWidget *, char *, char *);
int reset_log();
void close_log();
int check_app_dir();
void get_msg(char*, char*, char*);
void string_trim(char*);
void trim_spaces(char *);
void register_window(GtkWidget *);
void deregister_window(GtkWidget *);
void free_window_reg();
void close_open_ui();
int close_ui(char *);
int is_ui_reg(char *, int);
char * log_name();
char * app_dir_path();
char * home_dir();
void strlower(char *, char *);
void dttm_stamp(char *, size_t);
int check_dir(char *);
int make_dir(char *);
int get_file_stat(char *, struct stat *);
FILE * open_file(char *, char *);
int read_file(FILE *, char *, int);
int val_str2numb(char *, int *, char *, GtkWidget *);
void basename_dirname(char *, char **, char **);
int check_errno();
void print_bits(size_t const, void const * const);

extern void cur_date_str(char *, int, char *);



/* Globals */

static const char *app_messages[][2] = 
{ 
    { "APP0001", "Error: %s has an invalid value. "},
    { "APP0002", "Error: Please enter a value for %s. "},
    { "APP0003", "Debug: %s. "},
    { "APP0004", "%s does not exist. "},
    { "APP0005", "Error: No %s found. "},
    { "APP0006", "Warning: No %s found. "},
    { "APP0007", "Warning: default user settings being set up. "},
    { "APP0008", "File error: %s. "},
    { "APP0009", "Working directory not found. "},
    { "APP0010", "Warning: File %s cannot be read or has no Exif data. "},
    { "APP0011", "Please select %s. "},
    { "APP0012", "One or more images have inconsistent - \n\tISO, Exposure, Width or Height. "},
    { "APP0013", "Warning: One or more darks have been discarded. "},
    { "APP0014", "File error: Failed to find tag - %s. "},
    { "APP9999", "Application message: "},
    { "SYS9000", "Failed to start application. "},
    { "SYS9001", "Session started. "},
    { "SYS9002", "Session ends. "},
    { "SYS9003", "Failed to read $HOME variable. "},
    { "SYS9004", "Failed to create Application directory: %s "},
    { "SYS9005", "Failed to open file: %s "},
    { "SYS9006", "File %s does not exist or cannot be read. "},
    { "SYS9007", "Failed to get parent container widget. %s "},
    { "SYS9008", "Failed to find widget. %s "},
    { "SYS9009", "Error: Failed to backup Project %s. "},
    { "SYS9010", "Project %s has been backed up. "},
    { "SYS9011", "Failed to create directory: %s "},
    { "SYS9012", "Failed to write file: %s "},
    { "SYS9013", "Failed to read file: %s "},
    { "SYS9014", "Failed to find tag: %s "},
    { "SYS9015", "Project List failed: %s "},
    { "SYS9999", "Error - Unknown error message given. "}			// NB - MUST be last
};

static const int Msg_Count = 32;
static char *Home;
static char *logfile = NULL;
static FILE *lf = NULL;
static char *app_dir;
static int app_dir_len;
static const char *debug_hdr = "DEBUG-utility.c ";
static GList *open_ui_list_head = NULL;
static GList *open_ui_list = NULL;


/* Process additional application messages and error conditions */

void app_msg(char *msg_id, char *opt_str, GtkWidget *window)
{
    char msg[512];
    int i;

    /* Lookup the error */
    get_msg(msg, msg_id, opt_str);
    strcat(msg, " \n\%s");

    /* Display the error */
    info_dialog(window, msg, app_msg_extra);

    /* Reset global error details */
    app_msg_extra[0] = '\0';

    return;
}


/* Add a message to the log file and optionally display a popup */

void log_msg(char *msg_id, char *opt_str, char *sys_msg_id, GtkWidget *window)
{
    char msg[512];
    char date_str[50];

    /* Lookup the error */
    get_msg(msg, msg_id, opt_str);

    /* Log the message */
    cur_date_str(date_str, sizeof(date_str), "%d-%b-%Y %I:%M:%S %p");

    /* This may before anything has been set up (chicken & egg !). Use stderr if required */
    if (lf == NULL)
    	lf = stderr;

    fprintf(lf, "%s - %s\n", date_str, msg);

    if (strlen(app_msg_extra) > 0)
	fprintf(lf, "\t%s\n", app_msg_extra);

    fflush(lf);

    /* Reset global error details */
    app_msg_extra[0] = '\0';

    /* Optional display */
    if (sys_msg_id && window && logfile)
    {
    	sprintf(app_msg_extra, "\nLog file (%s) may contain more details.", logfile);
    	app_msg(sys_msg_id, opt_str, window);
    }

    return;
}


/* General prupose information dialog */

void info_dialog(GtkWidget *window, char *msg, char *opt)
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new (GTK_WINDOW (window),
				     GTK_DIALOG_MODAL,
				     GTK_MESSAGE_ERROR,
				     GTK_BUTTONS_CLOSE,
				     msg,
				     opt);

    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    return;
}


/* General prupose query dialog */

gint query_dialog(GtkWidget *window, char *msg, char *opt)
{
    GtkWidget *dialog;
    gint res;

    GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

    dialog = gtk_message_dialog_new (GTK_WINDOW (window),
				     flags,
				     GTK_MESSAGE_QUESTION,
				     GTK_BUTTONS_YES_NO,
				     msg,
				     opt);

    res = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    return res;
}


/* Reset the log file */

int reset_log()
{
    logfile = (char *) malloc(strlen(Home) + (strlen(TITLE) * 2) + 10);
    sprintf(logfile, "%s/.%s/%s.log", Home, TITLE, TITLE);

    if ((lf = fopen(logfile, "w")) == (FILE *) NULL)
    {
	log_msg("SYS9003", logfile, NULL, NULL);
	free(logfile);
	return FALSE;
    }
    else
    {
    	g_print("%s: See Log file - %s for all details.\n", TITLE, logfile);
    }

    return TRUE;
}


/* Close the log file and free any memory */

void close_log()
{
    fclose(lf);
    free(logfile);
    free(app_dir);

    return;
}


/* Return the logfile name */

char * log_name()
{
    return logfile;
}


/* Return the application directory path */

char * app_dir_path()
{
    return app_dir;
}


/* Return the Home directory path */

char * home_dir()
{
    return Home;
}


/* Set up application directory(s) for the user if necessary */

int check_app_dir()
{
    struct stat fileStat;
    int err;

    if ((Home = getenv("HOME")) == NULL)
    {
    	log_msg("SYS9003", NULL, NULL, NULL);
    	return FALSE;
    }

    app_dir = (char *) malloc(strlen(Home) + strlen(TITLE) + 5);
    sprintf(app_dir, "%s/.%s", Home, TITLE);
    app_dir_len = strlen(app_dir);

    if ((err = stat(app_dir, &fileStat)) < 0)
    {
	if ((err = mkdir(app_dir, 0700)) != 0)
	{
	    log_msg("SYS9004", app_dir, NULL, NULL);
	    free(app_dir);
	    return FALSE;
	}
    }

    return TRUE;
}


/* Error lookup and optional string argument substitution */

void get_msg(char *s, char *msg_id, char *opt_str)
{
    int i;
    char *p, *p2;

    /* Find message */
    for(i = 0; i < Msg_Count; i++)
    {
    	if ((strcmp(msg_id, app_messages[i][0])) == 0)
	    break;
    }

    if (i >= Msg_Count)
    	i--;

    /* Check substitution. If none, show message as is with any '%s' blanked out. */
    p = (char *) app_messages[i][1];
    p2 = strstr(p, "%s");

    if ((! opt_str) || (strlen(opt_str) == 0) || (p2 == NULL))
    {
	sprintf(s, "(%s) %s", app_messages[i][0], app_messages[i][1]);

	if (p2 != NULL)
	{
	    p2 = strstr(s, "%s");
	    *p2++ = ' ';
	    *p2 = ' ';
	}

    	return;
    }

    /* Add substitution string */
    *s = '\0';
    sprintf(s, "(%s) ", app_messages[i][0]);

    for(s = (s + strlen(app_messages[i][0]) + 3); p < p2; p++)
    	*s++ = *p;

    *s = '\0';

    strcat(s, opt_str);
    strcat(s, p2 + 2);

    return;
}


/* Remove leading and trailing spaces from a string */

void string_trim(char *s)
{
    int i;
    char *p;

    /* Trailing */
    for(i = strlen(s) - 1; i >= 0; i--)
    {
	if (isspace(s[i]))
	    s[i] = '\0';
	else
	    break;
    }

    /* Empty - all spaces */
    if (*s == '\0')
    	return;

    /* Leading */
    p = s;

    while(isspace(*p))
    {
    	p++;
    }

    while(*p != '\0')
    {
    	*s++ = *p++;
    }

    *s = '\0';

    return;
}


/* Remove spaces on the right of the string */

void trim_spaces(char *buf)
{
    char *s = buf - 1;

    for(; *buf; ++buf) 
    {
        if (*buf != ' ')
            s = buf;
    }

    *++s = 0; /* null terminate the string on the first of the final spaces */
}


/* Regiser the window as open */

void register_window(GtkWidget *window)
{
    open_ui_list = g_list_append (open_ui_list_head, window);

    if (open_ui_list_head == NULL)
    	open_ui_list_head = open_ui_list;

    return;
}


/* De-register the window as closed */

void deregister_window(GtkWidget *window)
{
    open_ui_list_head = g_list_remove (open_ui_list_head, window);

    return;
}


/* Check if a window title is registered (open) and present it to the user if reguired */

int is_ui_reg(char *s, int present)
{
    GtkWidget *window;
    const gchar *title;

    open_ui_list = g_list_first(open_ui_list_head);

    while(open_ui_list != NULL)
    {
	window = GTK_WIDGET (open_ui_list->data);
	title = gtk_window_get_title(GTK_WINDOW (window));

	if (strcmp(s, title) == 0)
	{
	    if (present == TRUE)
	    {
	    	gtk_window_present (GTK_WINDOW (window));
	    }

	    return TRUE;
	}

	open_ui_list = g_list_next(open_ui_list);
    }

    return FALSE;
}


/* Close any open windows */

void close_open_ui()
{
    GtkWidget *window;

    open_ui_list = g_list_first(open_ui_list_head);

    while(open_ui_list != NULL)
    {
	window = GTK_WIDGET (open_ui_list->data);
	gtk_window_close(GTK_WINDOW (window));
	open_ui_list = g_list_next(open_ui_list);
    }

    return;
}


/* Close a window */

int close_ui(char *s)
{
    GtkWidget *window;
    const gchar *title;

    open_ui_list = g_list_first(open_ui_list_head);

    while(open_ui_list != NULL)
    {
	window = GTK_WIDGET (open_ui_list->data);
	title = gtk_window_get_title(GTK_WINDOW (window));

	if (strcmp(s, title) == 0)
	{
	    gtk_window_close(GTK_WINDOW (window));
	    return TRUE;
	}

	open_ui_list = g_list_next(open_ui_list);
    }

    return FALSE;
}


/* Free the window register */

void free_window_reg()
{
    g_list_free (open_ui_list_head);

    return;
}


/* Convert a string to lowercase */

void strlower(char *s1, char *s2)
{
    for(; *s1 != '\0'; s1++, s2++)
    	*s2 = tolower(*s1);

    *s2 = *s1;

    return;
}


/* Return a date and time stamp */

void dttm_stamp(char *s, size_t max)
{
    size_t sz;
    struct tm *tm;
    time_t current_time;

    *s = '\0';
    current_time = time(NULL);
    tm = localtime(&current_time);
    sz = strftime(s, max, "%d%m%Y_%H%M%S", tm);

    return;
}


/* Check directory exists */

int check_dir(char *s)
{
    struct stat fileStat;
    int err;

    if ((err = stat(s, &fileStat)) < 0)
	return FALSE;

    if ((fileStat.st_mode & S_IFMT) == S_IFDIR)
	return TRUE;
    else
	return FALSE;
}


/* Create a directory */

int make_dir(char *s)
{
    int err;

    if ((err = mkdir(s, 0700)) != 0)
    {
	log_msg("SYS9011", s, NULL, NULL);
	return FALSE;
    }

    return TRUE;
}


/* Get file details */

int get_file_stat(char *s, struct stat *fileStat)
{
    int err;

    if ((err = stat(s, fileStat)) < 0)
	return FALSE;

    return TRUE;
}


/* Open a file */

FILE * open_file(char *fn, char *op)
{
    FILE *fd;

    if ((fd = fopen(fn, op)) == (FILE *) NULL)
    {
    	check_errno();
	return (FILE *) NULL;
    }

    return fd;
}


/* Read a file */

int read_file(FILE *fd, char *buf, int sz_len)
{
    int i, max;
    char c;

    i = 0;
    max = sz_len - 1;
    buf[0] = '\0';
    
    while((c = fgetc(fd)) != EOF)
    {
    	buf[i++] = c;

    	if (i >= max)
	    break;
    }

    buf[i] = '\0';

    if (c == EOF)
    {
	fclose(fd);
	return (int) c;
    }
    else
    {
	return i;
    }
}


/* Convert a string to a number and validate */

int val_str2numb(char *s, int *numb, char *subst, GtkWidget *window)
{
    int i;
    char *end;

    if (strlen(s) > 0)
    {
	errno = 0;
	i = strtol(s, &end, 10);

	if (errno != 0)
	{
	    app_msg("APP0001", subst, window);
	    return FALSE;
	}
	else if (*end)
	{
	    app_msg("APP0001", subst, window);
	    return FALSE;
	}
    }
    else
    {
    	i = 0;
    }

    *numb = i;

    return TRUE;
}


/* Return the filename and directory separately from a full path */

void basename_dirname(char *path, char **nm, char **dir)
{  
    int len, i;
    char *s;

    len = strlen(path);
    s = strrchr(path, '/');
    i = s - path + 1;
    *nm = (char *) malloc(len - i + 1);
    strncpy(*nm, path + i, len - i + 1);
    *dir = (char *) malloc(i);
    strncpy(*dir, path, i - 1);
    *(*dir + i - 1) = '\0';

    return;
}


/* Check and print error message */

int check_errno(char *s)
{
    int err;

    if (errno != 0)
    {
	printf("%s %s - error: (%d) %s\n", debug_hdr, s, errno, strerror(errno));
	return errno;
    }

    return 0;
}


/* Show binary representation of value (useful debug) */

void print_bits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for(i = size - 1; i >= 0; i--)
    {
        for(j = 7; j >= 0; j--)
        {
            byte = b[i] & (1<<j);
            byte >>= j;
            printf("%u", byte);
        }
    }

    printf("\n");
    fflush(stdout);
}
