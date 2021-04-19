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
** Description: Image related functions.
**
** Author:	Anthony Buckley
**
** History
**	30-Aug-2020	Initial code
**
*/



/* Defines */

#define MAX_SCALE 400

/* Includes */
#include <stdio.h>  
#include <sys/stat.h>  
#include <sys/types.h>  
#include <unistd.h>  
#include <stdlib.h>  
#include <string.h>  
#include <gtk/gtk.h>  
#include <libexif/exif-data.h>
#include <libexif/exif-tag.h>
#include <libexif/exif-loader.h>
#include <project_ui.h>


/* Prototypes */

char * image_type(char *, GtkWidget *); 
Image * setup_image(char *, char *, char *, ProjectUi *);
int load_exif_data(Image *, char *, GtkWidget *);
static char * get_exif_tag(ExifData *, ExifIfd, ExifTag);
int show_image(char *, MainUi *);
void show_meta(char *, int, gchar *, MainUi *);
void show_scale(double, MainUi *);
void img_fit_win(GdkPixbuf *, int, int, MainUi *);
void img_scale_sz(MainUi *, int);
void zoom_image(double, MainUi *);
void scale_pixmap(double, MainUi *);
void mouse_drag_check(MainUi *);
void drag_move_sw(gdouble, gdouble, gdouble, gdouble, MainUi *);
void mouse_drag_on(MainUi *);
void mouse_drag_off(MainUi *);
static void nudge_loader(MainUi *);
static void init_loader(MainUi *);
gboolean pulse_bar(gpointer data);

static void OnAreaPrepared(GdkPixbufLoader *, gpointer);
static void OnAreaUpdated(GdkPixbufLoader *, gint, gint, gint, gint, gpointer);
static void OnSizePrepared(GdkPixbufLoader *, gint, gint, gpointer);
static void OnClosed(GdkPixbufLoader *, gpointer);
	
extern void log_msg(char*, char*, char*, GtkWidget*);
extern void trim_spaces(char *);
extern void view_menu_sensitive(MainUi *, int);


/* Globals */

static const char *debug_hdr = "DEBUG-image.c ";
static double px_scale = 0;

/* Pixbuf Loader variables */
static GdkPixbufLoader *loader = NULL;
static GdkPixbuf *loader_pixbuf = NULL;
static guchar *filebuf = NULL;
static guint filesize = 0;
static guint curpos = 0;


/* Determine image type */
/*
 * (Probably overkill, but interesting !)
 *
 * Image Type	Bytes	  Value
 * ----------   -----     -----
 * JPG	        1,2,3     0xFF 0xD8 0xFF
 * TIFF		1,2,3     0x49 0x49 0x2A 0x00 (Intel)   or   0x4D 0x4D 0x00 0x2A (Mac)
 * BMP		1,2       0x42 0x4D
 * GIF87a	1 - 6     0x47 0x49 0x46 0x38 0x37 0x61
 * GIF89a	1 - 6     0x47 0x49 0x46 0x38 0x39 0x61
 * PNG		1 - 8     0x89 0x50 0x4E 0x47 0x0D 0x0A 0x1A 0x0A
 * CR2		9,10,11   0x52 0x43 0x02
*/

char * image_type(char *path, GtkWidget *window) 
{  
    FILE *fd = NULL;
    int i, j, c, err;
    struct stat filestat;
    char buf[10];
    char *s;
    
    const int max_types = 8;
    const int max_cols = 11;

    int candidates[8] = { 1, 1, 1, 1, 1, 1, 1, 1 };
    const char *img_types[] = { "JPG", "TIFF", "TIFF", "BMP", "GIF87a", "GIF89a", "PNG", "CR2" };
    const int byte_vals[8][11] = 
    	{
	    { 0xFF, 0xD8, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },	// JPG 
	    { 0x49, 0x49, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 	// TIFF Intel
	    { 0x4D, 0x4D, 0x00, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 	// TIFF Mac
	    { 0x42, 0x4D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 	// BMP
	    { 0x47, 0x49, 0x46, 0x38, 0x37, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00 }, 	// GIF87a
	    { 0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00 }, 	// GIF89a
	    { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00 }, 	// PNG
	    { 0x49, 0x49, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x43, 0x02 } 	// CR2
	};

    /* Set default unknown */
    s = (char *) malloc(9);
    strcpy(s, "Unknown");

    /* Check file */
    err = stat(path, &filestat);

    if ((err < 0) || (filestat.st_size == 0))
    {
    	log_msg("SYS9006", path, "SYS9006", window);
	return s;
    }

    if ((fd = fopen(path, "r")) == (FILE *) NULL)
    {
    	log_msg("SYS9006", path, "SYS9006", window);
	return s;
    }

    /* This is just a 'last man standing' approach */
    for(i = 0; i < max_cols; i++)
    {
	if ((c = fgetc(fd)) == EOF)
	{
	    i = max_cols;
	    break;
	}
	 
	for(j = 0; j < max_types; j++)
	{
	    if (candidates[j] == 0)
	    	continue;

	    if (byte_vals[j][i] == 0x00)
	    	continue;

	    if (c != byte_vals[j][i])
		candidates[j] = 0;
	};
    };

    /* Should only be one still standing */
    for(i = 0; i < max_types; i++)
    {
    	if (candidates[i] == TRUE)
    	{
	    strcpy(s, img_types[i]);
	    break;
    	}
    };

    fclose(fd);

    return s;
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

    if (! load_exif_data(img, image_full_path, p_ui->window))
    {
    	free(img->nm);
    	free(img->path);
    	free(img);
    	return NULL;
    }

    return img;
}


/* Extract the image Exif data (if any) */

int load_exif_data(Image *img, char *full_path, GtkWidget *window)
{  
    ExifData *ed;
    ExifEntry *entry;

    /* Load an ExifData object from an EXIF file */
    ed = exif_data_new_from_file(full_path);

    if (!ed)
    {
	log_msg("APP0010", full_path, "APP0010", window);
        return FALSE;
    }

    img->img_exif.make = get_exif_tag(ed, EXIF_IFD_0, EXIF_TAG_MAKE);
    img->img_exif.model = get_exif_tag(ed, EXIF_IFD_0, EXIF_TAG_MODEL);
    img->img_exif.date = get_exif_tag(ed, EXIF_IFD_0, EXIF_TAG_DATE_TIME);
    img->img_exif.width = get_exif_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_PIXEL_X_DIMENSION);
    img->img_exif.height = get_exif_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_PIXEL_Y_DIMENSION);
    img->img_exif.iso = get_exif_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_ISO_SPEED_RATINGS);
    img->img_exif.exposure = get_exif_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_TIME);
    img->img_exif.f_stop = get_exif_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_FNUMBER);
    /*
    img->img_exif.make = get_exif_tag(ed, EXIF_IFD_0, EXIF_TAG_MAKE);
    img->img_exif.model = get_exif_tag(ed, EXIF_IFD_0, EXIF_TAG_MODEL);
    img->img_exif.type = get_exif_tag(ed, EXIF_IFD_0, STARSAL_EXIF_TAG_RECORD_MODE);
    img->img_exif.date = get_exif_tag(ed, EXIF_IFD_0, EXIF_TAG_DATE_TIME);
    img->img_exif.width = get_exif_tag(ed, EXIF_IFD_0, STARSAL_EXIF_IMAGE_WIDTH);
    img->img_exif.height = get_exif_tag(ed, EXIF_IFD_0, STARSAL_EXIF_IMAGE_HEIGHT);
    img->img_exif.iso = get_exif_tag(ed, EXIF_IFD_0, STARSAL_EXIF_TAG_ISO);
    img->img_exif.exposure = get_exif_tag(ed, EXIF_IFD_0, STARSAL_EXIF_TAG_EXP);
    img->img_exif.f_stop = get_exif_tag(ed, EXIF_IFD_0, STARSAL_EXIF_TAG_FSTOP);
    */
    
    /* Free the EXIF */
    exif_data_unref(ed);
    
    /* Not really exif data, but as far as possible, get the image type here */
    img->img_exif.type = image_type(full_path, window);
    //printf("%s - Type: %s\n", debug_hdr, img->img_exif.type); fflush(stdout);

    return TRUE;
}


/* Extract tag and contents if exists */

static char * get_exif_tag(ExifData *d, ExifIfd ifd, ExifTag tag)
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
            //printf("%s - %s: %s\n", debug_hdr, exif_tag_get_name_in_ifd(tag,ifd), buf); fflush(stdout);
	    s = (char *) malloc(strlen(buf) + 1);
	    strcpy(s, buf);
	    return s;
        }
    }

    s = (char *) malloc(4);
    sprintf(s, "N/A");

    return s;
}


/* Show an image */

int show_image(char *img_fn, MainUi *m_ui)
{
    int sw_h, sw_w;
    GError *err = NULL;

    m_ui->base_pixbuf = gdk_pixbuf_new_from_file(img_fn, &err);
    m_ui->img_fn = strdup(img_fn);
    sw_w = gtk_widget_get_allocated_width (m_ui->img_scroll_win);
    sw_h = gtk_widget_get_allocated_height (m_ui->img_scroll_win);

    img_fit_win(m_ui->base_pixbuf, sw_w, sw_h, m_ui);
    view_menu_sensitive(m_ui, TRUE);

    return TRUE;
}


/* Show image meta data */

void show_meta(char *img_fn, int idx, gchar *img_type, MainUi *m_ui)
{
    GtkTextBuffer *txt_buffer;
    GtkTextIter iter;
    char buf[200];
    GList *l;
    Image *img;

    if (idx < 0)
    	return;

    /* Find the image in the project image glist */
    if (strcmp(img_type, "I") == 0)
    	l = g_list_nth (m_ui->proj->images_gl, idx);
    else
    	l = g_list_nth (m_ui->proj->darks_gl, idx);

    img = (Image *) l->data;

    /* Format the text from the meta data */
    txt_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (m_ui->txt_view));
    gtk_text_buffer_set_text (txt_buffer, "", -1);
    gtk_text_view_set_editable (GTK_TEXT_VIEW (m_ui->txt_view), FALSE);
    gtk_text_buffer_get_end_iter (txt_buffer, &iter);
    
    sprintf(buf, "Make:  %s\n"
    		 "Model:  %s\n"
    		 "Type:  %s\n"
    		 "Date:  %s\n"
    		 "Width:  %s\n"
    		 "Height:  %s\n"
    		 "ISO:  %s\n"
    		 "Exposure:  %s\n"
    		 "F-Stop:  %s\n", 
	     img->img_exif.make, img->img_exif.model, img->img_exif.type, img->img_exif.date, img->img_exif.width,
	     img->img_exif.height, img->img_exif.iso, img->img_exif.exposure, img->img_exif.f_stop);

    gtk_text_buffer_insert (txt_buffer, &iter, buf, -1);

    if (! gtk_widget_get_visible (m_ui->img_info_vbox))
    {
    	gtk_widget_set_visible (m_ui->img_info_vbox, TRUE);
    	gtk_widget_set_visible (m_ui->img_progress_bar, FALSE);
    }

    return;
}


/* Show image current scale */

void show_scale(double img_scale, MainUi *m_ui)
{
    char s[30];

    sprintf(s, "Image scale: %0.2f%%", img_scale);
    gtk_label_set_text( GTK_LABEL(m_ui->img_scale_lbl), s);
    gtk_widget_show(m_ui->img_scale_lbl);

    return;
}


/* Fit an image to the window, but keep aspect */

void img_fit_win(GdkPixbuf *pixbuf, int win_w, int win_h, MainUi *m_ui)
{
    int px_h, px_w, in_px_h;
    GdkPixbuf *pxbscaled;

    in_px_h = gdk_pixbuf_get_height(pixbuf);

    px_h = win_h;
    px_w = (gdk_pixbuf_get_width(pixbuf) * win_h) / in_px_h;
    pxbscaled = gdk_pixbuf_scale_simple (pixbuf, px_w, px_h, GDK_INTERP_BILINEAR);
    gtk_image_set_from_pixbuf (GTK_IMAGE (m_ui->image_area), pxbscaled);

    px_scale = (double) ((px_h * 100) / in_px_h);
    show_scale(px_scale, m_ui);

    g_object_unref (pxbscaled);
    gtk_widget_show(m_ui->image_area);

    return;
}


/* Set a particular scale - this is likely to result in delays with large images so use a loader */

void img_scale_sz(MainUi *m_ui, int multx)
{
    px_scale = 100 * multx;
    init_loader(m_ui);

    /*
    double px_h, px_w, d_scale;
    GdkPixbuf *pxbscaled;

    px_h = (double) (gdk_pixbuf_get_height(m_ui->base_pixbuf) * multx);
    px_w = (double) (gdk_pixbuf_get_width(m_ui->base_pixbuf) * multx);
    pxbscaled = gdk_pixbuf_scale_simple (m_ui->base_pixbuf, (int) px_w, (int) px_h, GDK_INTERP_BILINEAR);

    if (pxbscaled == NULL)
	return;

    gtk_image_set_from_pixbuf (GTK_IMAGE (m_ui->image_area), pxbscaled);

    px_scale = 100 * multx;
    show_scale(px_scale, m_ui);
    g_object_unref (pxbscaled);
    */

    return;
}


/* Set up the loader for progressive load, progress shown with a pulse bar */

static void init_loader(MainUi *m_ui)
{
    FILE *file;

    /* Prepare the loader and local callbacks */
    loader = gdk_pixbuf_loader_new();
    g_signal_connect (G_OBJECT(loader), "area_prepared", G_CALLBACK (OnAreaPrepared), m_ui);
    g_signal_connect (G_OBJECT(loader), "area_updated", G_CALLBACK (OnAreaUpdated), m_ui);
    g_signal_connect (G_OBJECT(loader), "size-prepared", G_CALLBACK (OnSizePrepared), m_ui);
    g_signal_connect (G_OBJECT(loader), "closed", G_CALLBACK (OnClosed), m_ui);

    /* Load file into memory for easier cycling */
    file = fopen (m_ui->img_fn, "r");

    if (file)
    {
	struct stat statbuf;

	if (stat(m_ui->img_fn, &statbuf) == 0)
	{
	    filesize = statbuf.st_size;
	    filebuf = g_malloc(filesize);
	    fread(filebuf, sizeof(char), filesize, file);
	}

	fclose(file);
    }
    else
    {
	log_msg("SYS9006", m_ui->img_fn, "SYS9006", m_ui->window);
	return;
    }

    /* Load the first small chunk, i.e., enough to trigger an area_prepared event */
    nudge_loader(m_ui);

    return;
}


/* Control writing to the loader */

static void nudge_loader(MainUi *m_ui)
{
    guint bitesize = 262144;  //65536;
    guint writesize = bitesize;
    GError *err = NULL;

    if (curpos >= filesize)
	return;

    while(curpos < filesize)
    {
	/* Trim writesize if it extends past the end of the file */
	if (curpos + bitesize >= filesize)
	    writesize = filesize - curpos;

	/* Send next chunk of image */
	if (!gdk_pixbuf_loader_write(loader, &filebuf[curpos], writesize, &err))
	{
	    log_msg("SYS9012", "GdkPixbufLoader", "SYS9012", m_ui->window);
	    return;
	}

	curpos += writesize;
     
	/* 
	** Clean up loader when we're finished writing the entire file; 
	** loader_pixbuf is still around because we referenced it in OnAreaPrepared
	*/
	if (curpos >= filesize)
	    gdk_pixbuf_loader_close(loader, &err);
    }

    return;
}


/* Show pulsing to indicate image loading in progress */

gboolean pulse_bar(gpointer user_data)
{
    MainUi *m_ui;

    /* Data */
    m_ui = (MainUi *) user_data;

    gtk_progress_bar_pulse(GTK_PROGRESS_BAR (m_ui->img_progress_bar));

    if (m_ui->pulse_status == FALSE)
	gtk_widget_set_visible (m_ui->img_progress_bar, FALSE);
 
    return m_ui->pulse_status;
}


/* Zoom image in or out by the step passed */

void zoom_image(double step, MainUi *m_ui)
{
    /* Set a lower limit for shrinkage */
    if (px_scale < 1)
    	if (step < 1)
    	    return;

    if (px_scale > MAX_SCALE)
	return;

    scale_pixmap(step, m_ui);

    return;
}


/* Scale image in or out by the step passed */

void scale_pixmap(double step, MainUi *m_ui)
{
    double px_h, px_w, d_scale;
    GdkPixbuf *pxbscaled;

g_print("scale_pixmap: step:  %0.2f\n", step);
    d_scale = (double) px_scale * step;
    
    px_h = ((double) gdk_pixbuf_get_height(m_ui->base_pixbuf)) * (d_scale / 100.0);
    px_w = ((double) gdk_pixbuf_get_width(m_ui->base_pixbuf)) * (d_scale / 100.0);
    pxbscaled = gdk_pixbuf_scale_simple (m_ui->base_pixbuf, (int) px_w, (int) px_h, GDK_INTERP_BILINEAR);

    if (pxbscaled == NULL)
	printf("%s scale_pixmap - null\n", debug_hdr); fflush(stdout);
    gtk_image_set_from_pixbuf (GTK_IMAGE (m_ui->image_area), pxbscaled);

    px_scale = d_scale;
    show_scale(px_scale, m_ui);
    g_object_unref (pxbscaled);

    return;
}
/*
printf("%s  zoom_image 1a d_scale %0.2f px_scale %0.2f\n", debug_hdr, d_scale, px_scale); fflush(stdout);
printf("%s  zoom_image 3 dpx_h %0.2f  dpx_w %0.2f\n", debug_hdr, dpx_h, dpx_w); fflush(stdout);
printf("%s  px_w %0.2f, px_h %0.2f, px_scale %0.2f\n", debug_hdr, px_w, px_h, px_scale); fflush(stdout);
printf("%s  px_w %d, px_h %d, px_scale %0.2f\n", debug_hdr, px_w, px_h, px_scale); fflush(stdout);
printf("%s  base px_w %d, base px_h %d\n", debug_hdr, 
		    gdk_pixbuf_get_width(m_ui->base_pixbuf), gdk_pixbuf_get_height(m_ui->base_pixbuf)); fflush(stdout);
*/


/* Set up mouse drag on or off for the image as required */

void mouse_drag_check(MainUi *m_ui)
{
    /* If one is blocked they all will be */
    if ((gtk_widget_get_allocated_width(m_ui->image_area) > gtk_widget_get_allocated_width(m_ui->img_scroll_win)) ||
        (gtk_widget_get_allocated_height(m_ui->image_area) > gtk_widget_get_allocated_height(m_ui->img_scroll_win)))
    {
	if (m_ui->img_drag_blocked)
	    mouse_drag_on(m_ui);
    }
    else
    {
	if (! m_ui->img_drag_blocked)
	    mouse_drag_off(m_ui);
    }

    return;
}


/* Move the scrollbars in sync with the mouse drag */

void drag_move_sw(gdouble x, gdouble y, gdouble last_x, gdouble last_y, MainUi *m_ui)
{
    gdouble h_value, v_value, h_shift, v_shift;
    GtkAdjustment *v_adj, *h_adj;

    v_adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (m_ui->img_scroll_win));
    h_adj = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (m_ui->img_scroll_win));
    h_value = gtk_adjustment_get_value (h_adj);
    v_value = gtk_adjustment_get_value (v_adj);
    //printf("%s drag_move_sw 1  h_value %0.2f  v_value %0.2f\n", debug_hdr, h_value, v_value); fflush(stdout);

    h_value = h_value + last_x - x;
    v_value = v_value + last_y - y;

    //printf("%s drag_move_sw 2  h_value %0.2f  v_value %0.2f\n", debug_hdr, h_value, v_value); fflush(stdout);
    gtk_adjustment_set_value (h_adj, h_value);
    gtk_adjustment_set_value (v_adj, v_value);
    gtk_scrolled_window_set_hadjustment (GTK_SCROLLED_WINDOW (m_ui->img_scroll_win), h_adj);
    gtk_scrolled_window_set_vadjustment (GTK_SCROLLED_WINDOW (m_ui->img_scroll_win), v_adj);

    gtk_widget_show_all(m_ui->window);

    return;
}


/* Set mouse drag off */

void mouse_drag_on(MainUi *m_ui)
{
    g_signal_handler_unblock (m_ui->img_scroll_win, m_ui->press_handler_id);
    g_signal_handler_unblock (m_ui->img_scroll_win, m_ui->release_handler_id);
    g_signal_handler_unblock (m_ui->img_scroll_win, m_ui->motion_handler_id);
    m_ui->img_drag_blocked = FALSE;

    return;
}


/* Set mouse drag off */

void mouse_drag_off(MainUi *m_ui)
{
    g_signal_handler_block (m_ui->img_scroll_win, m_ui->press_handler_id);
    g_signal_handler_block (m_ui->img_scroll_win, m_ui->release_handler_id);
    g_signal_handler_block (m_ui->img_scroll_win, m_ui->motion_handler_id);
    m_ui->img_drag_blocked = TRUE;

    return;
}


/* Local Image Callbacks */


/* The original image is in place so don't need placeholder, but start the pulse bar */

void OnAreaPrepared(GdkPixbufLoader *loader, gpointer user_data)
{
    MainUi *m_ui;

    /* Data */
    m_ui = (MainUi *) user_data;

    /* Grab and ref the pixbuf */
    loader_pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
    g_object_ref(loader_pixbuf);

    /* Start pulse bar */
    gtk_widget_set_visible (m_ui->img_progress_bar, TRUE);
    gtk_progress_bar_pulse(GTK_PROGRESS_BAR (m_ui->img_progress_bar));
    gtk_widget_show (m_ui->img_progress_bar);

    /* Make uniform pulsing */ 
    m_ui->pulse_status = TRUE;
    gtk_widget_set_visible (m_ui->img_progress_bar, TRUE);
    g_timeout_add(300, pulse_bar, m_ui);

    return;
}


/* Progressively draw the image and check if loader is still active for removing pulse bar */

void OnAreaUpdated(GdkPixbufLoader *loader, gint x, gint y, gint width, gint height, gpointer user_data)
{
    MainUi *m_ui;

    /* Data */
    m_ui = (MainUi *) user_data;

    /* Draw latest */
    gtk_image_set_from_pixbuf (GTK_IMAGE (m_ui->image_area), loader_pixbuf);

    return;
}


/* The Pixbuf has an original size, but scaling may be applied at this point */

void OnSizePrepared(GdkPixbufLoader *loader, gint width, gint height, gpointer user_data)
{
    MainUi *m_ui;
    double px_w, px_h, d_scale;

    /* Data */
    m_ui = (MainUi *) user_data;

    /* Set scaling if any */
    px_w = (double) width * (px_scale / 100.0);
    px_h = (double) height * (px_scale / 100.0);

    gdk_pixbuf_loader_set_size (loader, (int) px_w, (int) px_h);

    return;
}


/* The loader is closed, reset variables and set pulsing off */

void OnClosed(GdkPixbufLoader *loader, gpointer user_data)
{
    MainUi *m_ui;

    /* Data */
    m_ui = (MainUi *) user_data;

// Debug
/*
const char *handle = "/tmp/test_scale.jpg";
GError *err = NULL;
gdk_pixbuf_save (loader_pixbuf, handle, "jpeg", &err, "quality", "100", NULL);
*/

    /* Reset */
    m_ui->pulse_status = FALSE;
    g_free(filebuf);
    g_object_unref(loader_pixbuf);
    loader_pixbuf = NULL;
    filebuf = NULL;
    filesize = 0;
    curpos = 0;

    show_scale(px_scale, m_ui);

    return;
}
