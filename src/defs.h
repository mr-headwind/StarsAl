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
** Description:	Local standard constant defines
**
** Author:	Anthony Buckley
**
** History
**	xx-xxx-2021	Initial
**
*/


/* Includes */


/* General */
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#if defined (GDK_WINDOWING_X11)
#include <gdk/gdkx.h>
#elif defined (GDK_WINDOWING_WIN32)
#include <gdk/gdkwin32.h>
#elif defined (GDK_WINDOWING_QUARTZ)
#include <gdk/gdkquartz.h>
#endif

#ifndef AC_COLOURS
#define AC_COLOURS
#ifdef MAIN_UI
const GdkRGBA BLUE_GRAY = {.9, .9, 1.0, 1};
const GdkRGBA LIGHT_BLUE = {.5, .5, 1.0, 1.0};
const GdkRGBA MID_BLUE = {.5, .5, 1.0, 1.0};
const GdkRGBA DARK_BLUE = {0, 0, 0.5, 1.0};
const GdkRGBA RED1 = {.8, .1, .1, 1.0};
const GdkRGBA NIGHT = {0.57, 0.24, 0.24, 0.7};
const GdkRGBA LIGHT_GRAY = {0.4, 0.4, 0.0, 0.1};
const GdkRGBA WHITE1 = {0.0, 0.0, 0.0, 0.0};
#else
extern const GdkRGBA BLUE_GRAY;
extern const GdkRGBA LIGHT_BLUE;
extern const GdkRGBA MID_BLUE;
extern const GdkRGBA DARK_BLUE;
extern const GdkRGBA RED1;
extern const GdkRGBA NIGHT;
extern const GdkRGBA LIGHT_GRAY;
extern const GdkRGBA WHITE1;
#endif
#endif


/* Application Name et al */
#ifndef TITLE
#define TITLE "StarsAl"
#define USER_PREFS "user_preferences"
#define DEV_DIR "/dev"
#define PACKAGE_DATA_DIR "/usr/share"			// Release only
//#define PACKAGE_DATA_DIR "/home/tony/.local/share"	// Dev only
#define APP_URI "http://StarsAl.sourceforge.net"
#endif


/* Interface Names */
#ifndef UI_TITLES
#define UI_TITLES
#define ABOUT_UI "About"
#define PROJECT_UI "Project"
#define PROJ_SEL_UI "Project Selection"
#define USER_PREFS_UI "User Settings"
#define VIEW_FILE_UI "File Viewer"
#endif


/* Utility globals */
#ifndef ERR_INCLUDED
#define ERR_INCLUDED
#ifdef ERR_FILE
char app_msg_extra[1000];
#else
extern char app_msg_extra[1000];
#endif
#endif


#ifndef __STARSAL_TAG_H__
#define __STARSAL_TAG_H__

/*! EXIF tags */
typedef enum
{
    STARSAL_EXIF_TAG_RECORD_MODE	= 0x0009,
    STARSAL_EXIF_IMAGE_WIDTH		= 0xa002,
    STARSAL_EXIF_IMAGE_HEIGHT		= 0xa003,
    STARSAL_EXIF_TAG_ISO		= 0x8827,
    STARSAL_EXIF_TAG_EXP		= 0x829a,
    STARSAL_EXIF_TAG_FSTOP		= 0x829d
} StarsAlExifTag;

#endif /* __STARSAL_TAG_H__ */
