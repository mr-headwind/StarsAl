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
** Application:	StarsAl
**
** Author:	Anthony Buckley
**
** Description:
**  	Application control for StarsAl. 
**  	An application to align and stack astronomical images.
**
** History
**	02-Jun-2020	Initial code
**
*/


/* Includes */

#include <stdlib.h>  
#include <string.h>  
#include <libgen.h>  
#include <gtk/gtk.h>  
#include <main.h>
#include <defs.h>


/* Defines */


/* Prototypes */

void initialise(MainUi *);
void final(MainUi *);

extern void main_ui(MainUi *);
extern int check_app_dir();
extern int reset_log();
extern void close_log();
extern int read_user_prefs(GtkWidget *);
extern void log_msg(char*, char*, char*, GtkWidget*);
//extern void debug_session();


/* Globals */

static const char *debug_hdr = "DEBUG-StarsAl.c ";


/* Main program control */

int main(int argc, char *argv[])
{  
    MainUi m_ui;

    /* Initial work */
    initialise(&m_ui);

    /* Initialise Gtk */
    gtk_init(&argc, &argv);  

    main_ui(&m_ui);

    gtk_main();  

    final(&m_ui);

    exit(0);
}  


/* Initial work */

void initialise(MainUi *m_ui)
{
    char *p;

    /* Set variables */
    app_msg_extra[0] = '\0';
    memset(m_ui, 0, sizeof (MainUi));
    m_ui->proj = NULL;

    /* Set application directory */
    if (! check_app_dir())
    	exit(-1);

    /* Reset log file and log start */
    if (! reset_log())
    	exit(-1);

    log_msg("SYS9001", NULL, NULL, NULL);

    /* Load the user preferences */
    if (! read_user_prefs(NULL))
    	log_msg("APP0005", "No user preferences", NULL, NULL);

    return;
}


/* Final work */

void final(MainUi *m_ui)
{
    /* Close log file */
    log_msg("SYS9002", NULL, NULL, NULL);
    close_log();

    return;
}
