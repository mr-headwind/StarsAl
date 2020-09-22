
#  Copyright (C) 2021 Anthony Buckley
# 
#  This file is part of StarsAl.
# 
#  StarsAl is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
# 
#  StarsAl is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with StarsAl.  If not, see <http://www.gnu.org/licenses/>.

CC=cc
CXX=g++
CFLAGS=-I. `pkg-config --cflags gtk+-3.0 libexif` 
CXXFLAGS=-I. `pkg-config --cflags gtk+-3.0` 
CFLAGS2=-Wno-deprecated-declarations
DEPS = defs.h main.h user_data.h version.h project.h preferences.h
OBJ = starsal.o callbacks.o main_ui.o project_ui.o prefs_ui.o date_util.o utility.o about_ui.o view_file_ui.o css.o gtk_common.o image.o project.o
LIBS = `pkg-config --libs gtk+-3.0 libexif`
#LIBS2 = -lxxxx

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(CFLAGS2)

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

StarsAl: $(OBJ)
	$(CXX) -o $@ $^ $(LIBS) $(LIBS2) $(LIBS3)

clean:
	rm -f $(OBJ)
