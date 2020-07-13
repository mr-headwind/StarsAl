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
** Description:	User Preferences
**
** Author:	Anthony Buckley
**
** History
**	xx-xxx-2021	Initial
**
*/


// Structure to contain all user preferences.
// This list is loaded on startup if it exists. If not a default set is created.
// Items are updated on the preferences (settings) screen.
// The format of the settings file in the application directory is as follows:-
//	name_key|setting_value		- leading and trailing spaces are ignored
//					- no spaces in key

#ifndef USER_PREFS_H
#define USER_PREFS_H

#define PREF_KEY_SZ 25

typedef struct _UserPrefData
{
    char key[PREF_KEY_SZ];
    char *val;
} UserPrefData;

// Key values for each preference setting detail stored

#define SAMPLE_KEY "SAMPLEKEY1"
#define WORK_DIR "WORKDIR"

#endif
