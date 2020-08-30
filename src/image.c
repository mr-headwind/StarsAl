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


/* Includes */
#include <stdio.h>  
#include <sys/stat.h>  
#include <sys/types.h>  
#include <unistd.h>  
#include <stdlib.h>  
#include <string.h>  
#include <gtk/gtk.h>  


/* Prototypes */

char * image_type(char *, GtkWidget *); 
	
extern void log_msg(char*, char*, char*, GtkWidget*);


/* Globals */

static const char *debug_hdr = "DEBUG-image.c ";


/* Determine image type */
/*
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
    int err;
    struct stat filestat;
    char buf[10];
    char *s;

    /* Set default unknown */
    s = (char *) malloc(7);
    strcpy(s, "N/A");

    /* Check file */
    err = stat(path, &filestat);

    if ((err < 0) || (filestat.st_size == 0))
    {
    	log_msg("SYS9006", path, "SYS9006", window);
	return NULL;
    }

    if ((fd = fopen(path, "r")) == (FILE *) NULL)
    {
    	log_msg("SYS9006", path, "SYS9006", window);
	return NULL;
    }

    if (fgets(buf, sizeof(buf), fd) == NULL)
    {
    	log_msg("SYS9006", path, "SYS9006", window);
	return NULL;
    }

    fclose(fd);

    /* Use the table above to deetermine type */
    switch(buf[0])
    {
	case 0xFF:		// JPG
	    if (buf[1] == 0xD8 && buf[2] == 0xFF)
	    	strcpy(s, "JPG");
	    break;

	case 0x49:		// TIFF
	case 0x4D:	
	    if ((buf[1] == 0x49 && buf[2] == 0x2A && buf[3] == 0x00) ||
	        (buf[1] == 0x4D && buf[2] == 0x00 && buf[3] == 0x2A))
	    	strcpy(s, "TIFF");
	    break;

	case 0x42:		// BMP
	    if (buf[1] == 0x4D)
	    	strcpy(s, "BMP");
	    break;

	case 0x47:		// GIF87 or GIF89
	    if (buf[1] == 0x49 && buf[2] == 0x46 && buf[3] == 0x38 && buf[5] == 0x61)
	    	if (buf[4] == 0x37)
		    strcpy(s, "GIF87a");
		else if (buf[4] == 0x39)
		    strcpy(s, "GIF89a");
	    break;

	case 0x89:		// PNG
	    if ((buf[1] == 0x50 && buf[2] == 0x4E && buf[3] == 0x47 && buf[4] == 0x0D) &&
	        (buf[5] == 0x0A && buf[6] == 0x1A && buf[7] == 0x0A))
	    	strcpy(s, "PNG");
	    break;

	default:
	    if (buf[8] == 0x52 && buf[9] == 0x43 && buf[10] == 0x02)		 // CR2
	    	strcpy(s, "CR2");
	    break;
    }

    return s;
}  
