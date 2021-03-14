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
** Description:
**  StarsAl project maintenance functions
**
** Author:	Anthony Buckley
**
** History
**	22-Sep-2020	Initial code
**
*/


/* Defines */



/* Includes */

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <defs.h>
#include <project.h>
#include <main.h>
#include <preferences.h>


/* Prototypes */

ProjectData * new_proj_data();
int convert_exif(ImgExif *, int *, int *, int *, GtkWidget *);
void free_img(gpointer);
int load_proj_from_file(ProjectData *, char *, GtkWidget *);
int load_files(GList **, char **, const char *, const char *, GtkWidget *);
char * get_xmltag_val(char **, const char *, const char *, int, GtkWidget *);
int proj_close_check_save(ProjectData *, MainUi *);
void close_main_display(MainUi *);
void close_project(ProjectData *);
int save_proj_init(ProjectData *, GtkWidget *);
ProjectData * open_project(char *, GtkWidget *);
FILE * open_proj_file(char *, char *, GtkWidget *);
int write_proj_file(FILE *, const char *, GtkWidget *);
int read_proj_file(FILE *, char *, int, GtkWidget *);
int get_hdr_sz();
int get_image_sz(int, GList *);
void set_image_xml(char **, GList *, int);

extern int load_exif_data(Image *, char *, GtkWidget *);
extern int get_user_pref(char *, char **);
extern int val_str2numb(char *, int *, char *, GtkWidget *);
extern int check_dir(char *);
extern int make_dir(char *);
extern int get_file_stat(char *, struct stat *);
extern void log_msg(char*, char*, char*, GtkWidget*);
extern void view_menu_sensitive(MainUi *, int);


/* Globals */

static const char *proj_tags[][2] = 
{ 
  { "<?xml version=\"1.0\"?>", "" },
    { "<StarsAl>", "</StarsAl>" },
      { "<Project>", "</Project>" },
      { "<Description>", "</Description>" },
      { "<Path>", "</Path>" },
      { "<Status>", "</Status>" },
      { "<Images>", "</Images>" },
        { "<File>", "</File>" },
      { "<Darks>", "</Darks>" },
        { "<File>", "</File>" }
};

static const int Tag_Count = 10;
static const char *debug_hdr = "DEBUG-project.c ";

static const int starsal_idx = 1;
static const int name_idx = 2;
static const int desc_idx = 3;
static const int path_idx = 4;
static const int status_idx = 5;
static const int img_idx = 6;
static const int dark_idx = 8;
static const int file1_idx = 7;
static const int file2_idx = 9;




/* Create new project data structure*/

ProjectData * new_proj_data()
{
    ProjectData *proj = (ProjectData *) malloc(sizeof(ProjectData));
    memset(proj, 0, sizeof(ProjectData));

    return proj;
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


/* Open a project */

ProjectData * open_project(char *nm, GtkWidget *window)
{
    int count, sz;
    char *fn, *buf, *p;
    FILE *fd = NULL;
    ProjectData *proj;
    struct stat fileStat;

    /* Open the project file */
    get_user_pref(PROJ_DIR, &p);
    fn = (char *) malloc(strlen(p) + (strlen(nm) * 2) + 12);
    sprintf(fn, "%s/%s/%s_data.xml", p, nm, nm);

    if ((fd = open_proj_file(fn, "r", window)) == FALSE)
    {
	free(fn);
    	return NULL;
    }

    /* Stat filename to get size */
    get_file_stat(fn, &fileStat);
    sz = fileStat.st_size + 1;
    buf = (char *) malloc(sz);
    free(fn);

    /* read file */
    if ((count = read_proj_file(fd, buf, sz - 1, window)) < 0)
    	return NULL;

    /* Set project and load the data from the buffer */
    proj = new_proj_data();

    if (load_proj_from_file(proj, buf, window) == FALSE)
    {
	close_project(proj);
	free(buf);
    	return NULL;
    }

    return proj;
}


/* Load the project details from the buffer */

int load_proj_from_file(ProjectData *proj, char *buf, GtkWidget *window)
{
    char *buf_ptr, *tmp_status;

    /* Check that it's a StarsAl file */
    if ((buf_ptr = strstr(buf, proj_tags[starsal_idx][0])) == NULL)
    {
	log_msg("SYS9014", (char *) proj_tags[starsal_idx][0], "SYS9014", window);
    	return FALSE;
    }

    /* Title, Description, Path and Status */
    proj->project_name = get_xmltag_val(&buf_ptr, proj_tags[name_idx][0], proj_tags[name_idx][1], TRUE, window);
    proj->project_desc = get_xmltag_val(&buf_ptr, proj_tags[desc_idx][0], proj_tags[desc_idx][1], TRUE, window);
    proj->project_path = get_xmltag_val(&buf_ptr, proj_tags[path_idx][0], proj_tags[path_idx][1], TRUE, window);
    tmp_status = get_xmltag_val(&buf_ptr, proj_tags[status_idx][0], proj_tags[status_idx][1], TRUE, window);
    proj->status = atoi(tmp_status);
    free(tmp_status);

    /* Images and Darks */
    load_files(&(proj->images_gl), &buf_ptr, proj_tags[img_idx][0], proj_tags[img_idx][1], window);
    load_files(&(proj->darks_gl), &buf_ptr, proj_tags[dark_idx][0], proj_tags[dark_idx][1], window);

    return TRUE;
}


/* Extract the files from the buffer */

int load_files(GList **gl, char **buf_ptr, const char *start_tag, const char *end_tag, GtkWidget *window)
{
    char *ptr, *end_ptr, *fn, *p;

    /* Start pointer */
    if ((ptr = strstr(*buf_ptr, start_tag)) == NULL)
    {
	log_msg("SYS9014", (char *) start_tag, "SYS9014", window);
    	return FALSE;
    }

    /* End pointer */
    if ((end_ptr = strstr(ptr, end_tag)) == NULL)
    {
	log_msg("SYS9014", (char *) end_tag, "SYS9014", window);
    	return FALSE;
    }

    /* Iterate all the File tags in the range */
    end_ptr--;

    while(end_ptr > ptr)
    {
	fn = get_xmltag_val(&ptr, proj_tags[file1_idx][0], proj_tags[file1_idx][1], FALSE, NULL);
	
	//if (fn == NULL || ptr > end_ptr)
	if (fn == NULL)
	    break;

	/* Set up an image */
	Image *img;
	img = (Image *) malloc(sizeof(Image));

	if ((p = strrchr(fn, '/')) == NULL)
	    p = fn;
	else
	    p++;

	img->nm = strdup(p);

	if (p == fn)
	{
	    img->path = NULL;
	}
	else
	{
	    img->path = strndup(fn, p - fn - 1);
	    load_exif_data(img, fn, window);
	}

	/* Add to list */
	*gl = g_list_prepend(*gl, img);
	free(fn);
    };

    *gl = g_list_reverse(*gl);
    *buf_ptr = ptr;

    return TRUE;
}


/* Extract the value of a xml tag from the buffer */

char * get_xmltag_val(char **buf_ptr, const char *start_tag, const char *end_tag, int err, GtkWidget *window)
{
    int len, sz;
    char *tag_val, *ptr, *end_ptr;

    len = strlen(start_tag);

    /* Position of both tags */
    if ((ptr = strstr(*buf_ptr, start_tag)) == NULL)
    {
    	if (err == TRUE)
	    log_msg("SYS9014", (char *) start_tag, "SYS9014", window);

	return NULL;
    }

    if ((end_ptr = strstr(ptr, end_tag)) == NULL)
    {
    	if (err == TRUE)
	    log_msg("SYS9014", (char *) end_tag, "SYS9014", window);

	return NULL;
    }

    /* Value length */
    sz = end_ptr - (ptr + len);
    tag_val = (char *) malloc(sz + 1);

    /* Extract value */
    memcpy(tag_val, (ptr + len), sz);
    tag_val[sz] = '\0';
    end_ptr += (len + 1);
    *buf_ptr = end_ptr;

    if (*(buf_ptr)[0] == '\n')
	(*buf_ptr)++;

    return tag_val;
}


/* Check for save and close */

int proj_close_check_save(ProjectData *proj, MainUi *m_ui)
{
    int action = FALSE;     // To be coded later with dialog

    if (action == FALSE)
    {
	close_project(proj);
	close_main_display(m_ui);
	m_ui->proj = NULL;
    }
    else if (action == TRUE)
    {
	// SAVE
	close_project(proj);
	close_main_display(m_ui);
	m_ui->proj = NULL;
    }
    else if (action == -1)
    {
	return FALSE;
    }

    return TRUE;
}


/* Close the project on main screen */

void close_main_display(MainUi *m_ui)
{
    g_signal_handler_block (m_ui->select_image, m_ui->sel_handler_id);
    gtk_widget_destroy(m_ui->image_list_tree);
    m_ui->sel_handler_id = 0;
    gtk_label_set_text(GTK_LABEL (m_ui->status_info), "");
    gtk_widget_set_sensitive(m_ui->edit_proj, FALSE);
    gtk_widget_set_sensitive(m_ui->close_proj, FALSE);
    g_object_unref (m_ui->base_pixbuf);
    gtk_image_clear(GTK_IMAGE (m_ui->image_area));
    view_menu_sensitive(m_ui, FALSE);
    gtk_label_set_text(GTK_LABEL (m_ui->proj_name_lbl), "");
    gtk_label_set_text(GTK_LABEL (m_ui->proj_desc_lbl), "");
    gtk_widget_set_visible (m_ui->img_info_vbox, FALSE);

    return;
}


/* Close (not delete) the current project */

void close_project(ProjectData *proj)
{
    /* Free the listed images and darks */
    g_list_free_full(proj->images_gl, (GDestroyNotify) free_img);
    g_list_free_full(proj->darks_gl, (GDestroyNotify) free_img);

    proj->images_gl = NULL;
    proj->darks_gl = NULL;

    /* Free remaining */
    free(proj->project_name);
    free(proj->project_desc);
    free(proj->project_path);

    /* Free project */
    free(proj);

    return;
}


/* Write the initial project file */

int save_proj_init(ProjectData *proj, GtkWidget *window)
{
    int nml, pathl, descl, buf_sz, err;
    FILE *fd = NULL;
    char *buf;
    char *proj_fn;

    /* Create Project directory if necessary */
    if (check_dir((char *) proj->project_path) == FALSE)
	if (make_dir((char *) proj->project_path) == FALSE)
	    return FALSE;

    /* New or overwrite file */
    nml = strlen(proj->project_name);
    descl = strlen(proj->project_desc);
    pathl = strlen(proj->project_path);
    proj_fn = (char *) malloc(pathl + nml + 11);
    sprintf(proj_fn, "%s/%s_data.xml", proj->project_path, proj->project_name);

    if ((fd = open_proj_file(proj_fn, "w", window)) == FALSE)
    {
	free(proj_fn);
    	return FALSE;
    }

    /* Determine the buffer size required for headers */
    buf_sz = 0;
    buf_sz += get_hdr_sz();

    /* Add size for project title, path and status */
    buf_sz += (nml + descl + pathl + 4);

    /* Add size for images and darks */
    buf_sz += get_image_sz(strlen(proj_tags[file1_idx][0]), proj->images_gl);
    buf_sz += get_image_sz(strlen(proj_tags[file2_idx][0]), proj->darks_gl);

    /* Prepare the project header details and tags */
    buf = (char *) malloc(buf_sz);

    sprintf(buf, "%s\n%s\n%s%s%s\n%s%s%s\n%s%s%s\n%s%d%s\n", proj_tags[0][0],		// XML header tag
							     proj_tags[starsal_idx][0],	// StarsAl header tag
							     proj_tags[name_idx][0],	// Project tag
							     proj->project_name,	// Project name
							     proj_tags[name_idx][1],	// End Project tag
							     proj_tags[desc_idx][0],	// Description tag
							     proj->project_desc,	// Description
							     proj_tags[desc_idx][1],	// End Description tag
							     proj_tags[path_idx][0],	// Path tag
							     proj->project_path,	// Path
							     proj_tags[path_idx][1],	// End Path tag
							     proj_tags[status_idx][0],	// Status tag
							     proj->status,		// Status
							     proj_tags[status_idx][1]); // End Status tag

    
    /* Prepare the Prepare the file names and tags */
    set_image_xml(&buf, proj->images_gl, img_idx);
    set_image_xml(&buf, proj->darks_gl, dark_idx);

    /* Prepare the project header end tag */
    sprintf(buf, "%s%s\n", buf, proj_tags[starsal_idx][1]);		// End StarsAl tag

    /* Write to file */
    if (write_proj_file(fd, buf, window) == FALSE)
    	return FALSE;

    /* Close off */
    fclose(fd);
    free(proj_fn);

    return TRUE;
}


/* Get the total size of header tags */

int get_hdr_sz()
{
    int i, len;

    len = 0;

    for(i = 0; i < Tag_Count; i++)
    {
    	if (strcmp(proj_tags[i][0], "<File>") == 0)
	    continue;

    	len += ((strlen(proj_tags[i][0]) * 2) + 2);
    }

    return len;
}


/* Get the total size of all the image file names */

int get_image_sz(int tag_len, GList *gl)
{
    int len;
    GList *l;
    Image *img;

    len = 0;

    for(l = gl; l != NULL; l = l->next)
    {
	img = (Image *) l->data;
	len += ((tag_len * 2) + strlen(img->nm) + strlen(img->path) + 3);
    };

    return len;
}


/* Set up the image files xml */

void set_image_xml(char **buf, GList *gl, int idx)
{
    int i;
    GList *l;
    Image *img;

    sprintf(*buf, "%s%s\n", *buf, proj_tags[idx][0]);		// Images start tag
    i = idx + 1;

    for(l = gl; l != NULL; l = l->next)
    {
	img = (Image *) l->data;
	sprintf(*buf, "%s%s%s/%s%s\n", *buf, proj_tags[i][0], img->path, img->nm, proj_tags[i][1]);
    };

    sprintf(*buf, "%s%s\n", *buf, proj_tags[idx][1]);		// Images end tag

    return;
}


/* General open function */

FILE * open_proj_file(char *fn, char *action, GtkWidget *window)
{
    FILE *fd = NULL;

    if ((fd = fopen(fn, action)) == (FILE *) NULL)
    {
	sprintf(app_msg_extra, "Action: %s  Error: (%d) %s", action, errno, strerror(errno));
	log_msg("SYS9005", fn, "SYS9005", window);
	return NULL;
    }

    return fd;
}


/* General read function */

int read_proj_file(FILE *fd, char *buf, int sz, GtkWidget *window)
{
    int count = 0;

    count = (fread(buf, 1, sz, fd));

    if (ferror(fd) != 0)
    {
	log_msg("SYS9013", "Project file", "SYS9013", window);
	clearerr(fd);
	return -1;
    }
    else
    {
	buf[sz - 1] = '\0';
	return count;
    }
}


/* General write function */

int write_proj_file(FILE *fd, const char *s, GtkWidget *window)
{
    if ((fputs(s, fd)) == EOF)
    {
	sprintf(app_msg_extra, "Error: (%d) %s", errno, strerror(errno));
	log_msg("SYS9012", "Project file", "SYS9012", window);
	return FALSE;
    }

    return TRUE;
}
