/*

Disk Editor
===========

This program allows the user to edit physical disks or image files
using GUI.

License (BSD-3)
===============

Copyright (c) 2012, Gusts 'gusC' Kaksis <gusts.kaksis@gmail.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef __main_h
#define __main_h

#include <disklib.h>

#include <Fl.H>
#include <Fl_Double_Window.H>
#include <Fl_Native_File_Chooser.H>
#include <Fl_Menu_Item.H>
#include <Fl_Menu_Bar.H>
#include <Fl_Button.H>
#include <Fl_Table.H>

#include "ui_disk_chooser.h"
#include "ui_disk_layout.h"
#include "ui_part_list.h"

enum eType {
	kTypeNone,
	kTypeDisk,
	kTypePart,
	kTypeImage
};

void open_disk_cb(Fl_Widget *w, void *o);
void open_image_cb(Fl_Widget *w, void *o);
void save_image_cb(Fl_Widget *w, void *o);
void write_disk_cb(Fl_Widget *w, void *o);

void part_sel_cb(Fl_Widget *w, void *o);

void disk_mbr_cb(Fl_Widget *w, void *o);
void disk_gpth_cb(Fl_Widget *w, void *o);
void disk_hex_cb(Fl_Widget *w, void *o);
void part_vbr_cb(Fl_Widget *w, void *o);
void part_gpta_cb(Fl_Widget *w, void *o);

void quit_cb(Fl_Widget *w, void *o);

#endif
