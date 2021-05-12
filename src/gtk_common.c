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
** Description: Common GTK functions.
**
** Author:	Anthony Buckley
**
** History
**	14-Jul-2020	Initial code
**
*/



/* Defines */


/* Includes */
#include <stdlib.h>  
#include <string.h>  
#include <gtk/gtk.h>  
#include <gdk/gdkkeysyms.h>  


/* Prototypes */

void create_label(GtkWidget **, char *, char *, GtkWidget *);
void create_label2(GtkWidget **, char *, char *, GtkWidget *, int, int, int, int);
void create_label3(GtkWidget **, char *, char *);
void create_label4(GtkWidget **, char *, char *, gint, gint, GtkAlign);
void create_entry(GtkWidget **, char *, GtkWidget *, int, int);
void create_radio(GtkWidget **, GtkWidget *, char *, char *, GtkWidget *, int, char *, char *);
void create_cbox(GtkWidget **, char *, const char *[], int, int, GtkWidget *, int, int);
void setup_btnbx(GtkWidget **, char *, int, GtkWidget **, char *, int);
	
char find_active_by_parent(GtkWidget *, char);
GtkWidget * find_widget_by_name(GtkWidget *, char *);

GtkWidget * debug_cntr(GtkWidget *);

extern void log_msg(char*, char*, char*, GtkWidget*);


/* Globals */

static const char *debug_hdr = "DEBUG-gtk_common.c ";


/* Create standard label */

void create_label(GtkWidget **lbl, char *nm, char *txt, GtkWidget *cntr) 
{  
    *lbl = gtk_label_new(txt);  
    gtk_widget_set_name(*lbl, nm);

    gtk_widget_set_halign(*lbl, GTK_ALIGN_START);
    gtk_widget_set_valign(*lbl, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top (*lbl, 5);
    gtk_widget_set_margin_start (*lbl, 10);
    gtk_box_pack_start (GTK_BOX (cntr), *lbl, FALSE, FALSE, 0);

    return;
}  


/* Create standard label */

void create_label2(GtkWidget **lbl, char *nm, char *txt, GtkWidget *cntr, 
		   int col, int row, int c_spn, int r_spn)
{  
    *lbl = gtk_label_new(txt);  
    gtk_widget_set_name(*lbl, nm);

    gtk_widget_set_halign(*lbl, GTK_ALIGN_START);
    gtk_widget_set_valign(*lbl, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top (*lbl, 5);
    gtk_grid_attach(GTK_GRID (cntr), *lbl, col, row, c_spn, r_spn);

    return;
}


/* Create standard label */

void create_label3(GtkWidget **lbl, char *nm, char *txt)
{  
    *lbl = gtk_label_new(txt);  
    gtk_widget_set_name(*lbl, nm);
    gtk_widget_set_valign(*lbl, GTK_ALIGN_CENTER);

    return;
}


/* Create standard label */

void create_label4(GtkWidget **lbl, char *nm, char *txt, gint start, gint bottom, GtkAlign h_align)
{  
    *lbl = gtk_label_new(txt);
    gtk_widget_set_name (GTK_WIDGET (*lbl), nm);
    gtk_widget_set_halign (GTK_WIDGET (*lbl), h_align);
    gtk_widget_set_margin_start (GTK_WIDGET (*lbl), start);
    gtk_widget_set_margin_bottom (GTK_WIDGET (*lbl), bottom);

    return;
}


/* Create standard entry */

void create_entry(GtkWidget **ent, char *nm, GtkWidget *cntr, int col, int row) 
{  
    *ent = gtk_entry_new();  
    gtk_widget_set_name(*ent, nm);

    gtk_widget_set_valign(GTK_WIDGET (*ent), GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID (cntr), *ent, col, row, 1, 1);

    return;
}


/* Create standard radio */

void create_radio(GtkWidget **rad, GtkWidget *grp, char *txt, char *nm, GtkWidget *cntr, 
		  int active, char *obj_nm, char *obj_data_str)
{  
    if (grp == NULL)
	*rad = gtk_radio_button_new_with_label (NULL, txt);
    else
	*rad = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (grp), txt);

    gtk_widget_set_name(*rad, "rad_1");
    gtk_widget_set_halign(*rad, GTK_ALIGN_START);
    gtk_widget_set_valign(*rad, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top (*rad, 0);
    gtk_widget_set_margin_start (*rad, 0);
    gtk_box_pack_start (GTK_BOX (cntr), *rad, FALSE, FALSE, 0);

    if (active == TRUE)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (*rad), TRUE);

    if (obj_nm != NULL && obj_data_str != NULL)
	g_object_set_data_full (G_OBJECT (*rad), obj_nm, g_strdup (obj_data_str), (GDestroyNotify) g_free);

    return;
}


/* Create standard combobox */

void create_cbox(GtkWidget **cbox, char *nm, const char *arr[], int max, int active, 
		 GtkWidget *cntr, int col, int row)
{  
    int i;
    char s[max];

    *cbox = gtk_combo_box_text_new();  
    gtk_widget_set_name(*cbox, nm);

    for(i = 0; i < max; i++)
    {
    	sprintf(s, "%d", i);
    	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT(*cbox), s, arr[i]);
    }

    gtk_combo_box_set_active (GTK_COMBO_BOX(*cbox), active);

    gtk_widget_set_halign(*cbox, GTK_ALIGN_START);
    gtk_widget_set_valign(*cbox, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top (*cbox, 0);
    gtk_grid_attach(GTK_GRID (cntr), *cbox, col, row, 1, 1);

    return;
}


/* Set up a button inside a colour coded box */

void setup_btnbx(GtkWidget **btnbx, char *btnbx_nm, int btnbx_top,
		 GtkWidget **btn, char *btn_lbl, int margin)
{  
    /* Button box */
    *btnbx = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_margin_top (*btnbx, btnbx_top);
    gtk_widget_set_name (*btnbx, btnbx_nm);
    gtk_widget_set_margin_top(*btnbx, 10);
    gtk_widget_set_margin_bottom(*btnbx, 10);
    gtk_widget_set_margin_start(*btnbx, 10);
    gtk_widget_set_margin_end(*btnbx, 10);

    /* Button */
    *btn = gtk_button_new_with_label(btn_lbl);
    gtk_widget_set_margin_top(*btn, margin);
    gtk_widget_set_margin_bottom(*btn, margin);
    gtk_widget_set_margin_start(*btn, margin);
    gtk_widget_set_margin_end(*btn, margin);
    gtk_box_pack_start (GTK_BOX (*btnbx), *btn, TRUE, TRUE, 0);

    return;
}


/* Return the parent of a widget */

GtkWidget * find_parent(GtkWidget *init_widget)
{
    GtkWidget *parent_contnr;

    parent_contnr = gtk_widget_get_parent(init_widget);

    if (! GTK_IS_CONTAINER(parent_contnr))
    {
	log_msg("SYS9011", "From initial widget", NULL, NULL);
    	return NULL;
    }

    return parent_contnr;
}


/* Search for a child widget using the widget name */

GtkWidget * find_widget_by_name(GtkWidget *parent_contnr, char *nm)
{
    GtkWidget *widget;
    const gchar *widget_name;

    if (! GTK_IS_CONTAINER(parent_contnr))
    {
	log_msg("SYS9011", "By name", NULL, NULL);
    	return NULL;
    }

    GList *child_widgets = gtk_container_get_children(GTK_CONTAINER (parent_contnr));

    child_widgets = g_list_first(child_widgets);

    while (child_widgets != NULL)
    {
	widget = child_widgets->data;
	widget_name = gtk_widget_get_name (widget);

	if (widget_name != NULL)
	{
	    if (strcmp(widget_name, nm) == 0)
	    {
		g_list_free (child_widgets);
		return widget;
	    }
	}

	child_widgets = g_list_next(child_widgets);
    }

    g_list_free (child_widgets);

    return NULL;
}


/* Search for a widget using the parent of an initiating widget */

GtkWidget * find_widget_by_parent(GtkWidget *init_widget, char *nm)
{
    GtkWidget *widget;
    GtkWidget *parent_contnr;
    const gchar *widget_name;

    parent_contnr = gtk_widget_get_parent(init_widget);

    if (! GTK_IS_CONTAINER(parent_contnr))
    {
	log_msg("SYS9011", "By parent", NULL, NULL);
    	return NULL;
    }

    GList *child_widgets = gtk_container_get_children(GTK_CONTAINER (parent_contnr));

    child_widgets = g_list_first(child_widgets);

    while (child_widgets != NULL)
    {
	widget = child_widgets->data;
	widget_name = gtk_widget_get_name (widget);

	if (strcmp(widget_name, nm) == 0)
	{
	    g_list_free (child_widgets);
	    return widget;
	}

	child_widgets = g_list_next(child_widgets);
    }

    g_list_free (child_widgets);

    return NULL;
}


/* Find all the control widgets in a container */

GList * ctrl_widget_list(GtkWidget *contr, GtkWidget *window)
{
    GtkWidget *widget;
    GList *ctl_list = NULL;
    GList *tmp_list = NULL;

    if (! GTK_IS_CONTAINER(contr))
    {
	log_msg("SYS9011", "Get Next Control", "SYS9011", window);
	return NULL;
    }

    GList *child_widgets = gtk_container_get_children(GTK_CONTAINER (contr));
    child_widgets = g_list_first(child_widgets);

    while (child_widgets != NULL)
    {
	widget = child_widgets->data;

	if ((GTK_IS_RANGE (widget)) || (GTK_IS_COMBO_BOX_TEXT (widget)) || (GTK_IS_RADIO_BUTTON (widget)))
	{
	    ctl_list = g_list_prepend(ctl_list, widget);
	}
	else if (GTK_IS_CONTAINER (widget))
	{
	    tmp_list = ctrl_widget_list(widget, window);
	    ctl_list = g_list_concat(ctl_list, tmp_list);
	}

	child_widgets = g_list_next(child_widgets);
    }

    return ctl_list;
}


/* Delete items in a menu by name */

void delete_menu_items(GtkWidget *menu, char *nm)
{
    int len;
    GtkWidget *widget;
    const gchar *s;

    len = strlen(nm);

    GList *child_widgets = gtk_container_get_children(GTK_CONTAINER (menu));
    child_widgets = g_list_first(child_widgets);

    while (child_widgets != NULL)
    {
	widget = child_widgets->data;
	s = gtk_widget_get_name(widget);

	if (strncmp(s, nm, len) == 0)
	    gtk_widget_destroy(widget);

	child_widgets = g_list_next(child_widgets);
    }

    g_list_free(child_widgets);

    return;
}


/* Set a field based on a radio button name */

char find_active_by_parent(GtkWidget *parent, char nm)
{
    GtkWidget *radio;
    const gchar *widget_name;

    GList *child_widgets = gtk_container_get_children(GTK_CONTAINER (parent));

    child_widgets = g_list_first(child_widgets);

    while (child_widgets != NULL)
    {
	radio = (GtkWidget *) child_widgets->data;

	if (GTK_IS_TOGGLE_BUTTON (radio))
	{
	    widget_name = gtk_widget_get_name (radio);

	    if (widget_name[1] == nm)
	    {
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (radio)) == TRUE)
		{
		    g_list_free (child_widgets);
		    return widget_name[3];
		}
	    }
	}

	child_widgets = g_list_next(child_widgets);
    }

    g_list_free (child_widgets);

    return '\0';
}


/* Debug widget container */

GtkWidget * debug_cntr(GtkWidget *cntr)
{
    const gchar *widget_name;
    GtkWidget *widget;
    GtkWidgetPath *w_path;

    if (! GTK_IS_CONTAINER(cntr))
    {
	log_msg("SYS9011", "btn children", NULL, NULL);
    	return NULL;
    }

    widget_name = gtk_widget_get_name (cntr);
    printf("%s widget structure for %s\n", debug_hdr, widget_name);

    GList *child_widgets = gtk_container_get_children(GTK_CONTAINER (cntr));
    //printf("%s \tno of children %d\n", debug_hdr, g_list_length(child_widgets));

    child_widgets = g_list_first(child_widgets);

    while (child_widgets != NULL)
    {
	widget = (GtkWidget *) child_widgets->data;
	widget_name = gtk_widget_get_name (widget);
	printf("%s \tname %s\n", debug_hdr, widget_name);

	w_path = gtk_container_get_path_for_child (GTK_CONTAINER (cntr), widget);
	printf("%s \tpath %s\n", debug_hdr, gtk_widget_path_to_string (w_path));

	if (GTK_IS_CONTAINER(widget))
	    debug_cntr(widget);

	if (GTK_IS_LABEL (widget))
	    break;

	child_widgets = g_list_next(child_widgets);
    }

    g_list_free (child_widgets);

    return widget;
}
