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
printf("%s p 1 \n", debug_hdr);fflush(stdout);
	if ((c = fgetc(fd)) == EOF)
	{
	    i = max_cols;
	    break;
	}
	 
	for(j = 0; j < max_types; j++)
	{
	    if (candidates[j] == 0)
	    	continue;

	    if (*byte_vals[i, j] == 0x00)
	    	continue;

printf("%s p 2  i %d  j %d  c %x  byte_val %x\n", debug_hdr, i, j, c, *(byte_vals[i, j]));fflush(stdout);
	    if (c != *(byte_vals[i, j]))
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
