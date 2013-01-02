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

#include <Windows.h>

#include "main.h"

handle_t _h;
eType _type = kTypeNone;

Fl_Double_Window *_win;
Fl_Menu_Item _menutable[] = {
	{"&File", 0, 0, 0, FL_SUBMENU},
		{"&Open Disk...", FL_ALT+'d', open_disk_cb},
		{"&Open Image...", FL_ALT+'o', open_image_cb},
		{"&Save Image...", FL_ALT+'s', save_image_cb},
		{"&Quit", FL_ALT+'q', quit_cb},
		{0},
	{0}
};
UIDiskLayout *_layout;
UIPartList *_partitions;
disk_info_t _info;


int main (int argc, char *argv){
	_win = new Fl_Double_Window(800, 580, "Disk Editor");

	Fl_Menu_Bar *menu = new Fl_Menu_Bar(0, 0, 800, 22);
	menu->box(FL_FLAT_BOX);
	menu->menu(_menutable);

	_layout = new UIDiskLayout(10, 30, 780, 60);
	_win->add(_layout);

	_partitions = new UIPartList(10, 130, 780, 400);
	_partitions->callback(part_sel_cb);
	_win->add(_partitions);

	Fl_Button *disk_mbr = new Fl_Button(10, 100, 90, 24, "Show MBR");
	disk_mbr->callback(disk_mbr_cb);
	_win->add(disk_mbr);

	Fl_Button *disk_gpth = new Fl_Button(110, 100, 90, 24, "Show GPT");
	disk_gpth->callback(disk_gpth_cb);
	_win->add(disk_gpth);

	Fl_Button *disk_hex = new Fl_Button(210, 100, 90, 24, "Show Hex");
	disk_hex->callback(disk_hex_cb);
	_win->add(disk_hex);

	_win->end();
	_win->show();
	int res = Fl::run();

	return res;
}

void open_disk_cb(Fl_Widget *w, void *o){
	int diskidx = disk_chooser("Open Disk...");
	int i;
	if (diskidx >= 0){
		if (_h != null){
			disk_close(_h);
			_partitions->clear_disk();
		}
		_h = disk_open(diskidx);
		if (_h != null){
			_type = kTypeDisk;
			if (disk_info(_h, &_info)){
				_layout->set_info(&_info);
				_partitions->set_disk(diskidx, &_info);
			}
		}
	}
}
void open_image_cb(Fl_Widget *w, void *o){
	int i;
	Fl_Native_File_Chooser choose;
	choose.title("Open Image...");
	choose.type(Fl_Native_File_Chooser::BROWSE_FILE);
	choose.filter(
		"Image file\t*.{img,bin}\n"
		"All files\t*.*"
	);
	if (choose.show() == 0){
		if (_h != null){
			image_close(_h);
			_partitions->clear_disk();
		}
		_h = image_open(choose.filename());
		if (_h != null){
			_type = kTypeImage;
			if (image_info(_h, &_info)){
				_layout->set_info(&_info);
				_partitions->set_disk(0, &_info);
			}
		}
	}
}
void save_image_cb(Fl_Widget *w, void *o){
	Fl_Native_File_Chooser choose;
	choose.title("Save Image...");
	choose.type(Fl_Native_File_Chooser::BROWSE_FILE);
	choose.filter(
		"Image file\t*.{img,bin}\n"
		"All files\t*.*"
	);
	if (choose.show() == 0){
		//handle_t h = image_open(choose.filename());
	}
}
void write_disk_cb(Fl_Widget *w, void *o){

}

void part_sel_cb(Fl_Widget *w, void *o){
	if (_partitions->callback_context() == Fl_Table::CONTEXT_CELL){
		_layout->set_selected(_partitions->callback_row());
	} else if (_partitions->callback_context() == Fl_Table::CONTEXT_TABLE){
		_layout->set_selected(-1);
	}
}

void disk_mbr_cb(Fl_Widget *w, void *o){

}
void disk_gpth_cb(Fl_Widget *w, void *o){

}
void disk_hex_cb(Fl_Widget *w, void *o){

}
void part_vbr_cb(Fl_Widget *w, void *o){

}
void part_gpta_cb(Fl_Widget *w, void *o){

}

void quit_cb(Fl_Widget *w, void *o){
	exit(0);
}