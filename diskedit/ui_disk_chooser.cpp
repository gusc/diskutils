/*

UI Disk Chooser dialog
======================

This is an UI dialog that displays available physical disks.

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

#include "ui_disk_chooser.h"

#include <disklib.h>
#include <stdio.h>

static UIDiskChooser *ch;

static void ok_cb(Fl_Widget *w, void *o){
	UIDiskChooser *d = (UIDiskChooser *)o;
	d->hide();
}
static void cancel_cb(Fl_Widget *w, void *o){
	UIDiskChooser *d = (UIDiskChooser *)o;
	d->diskidx(-1);
	d->hide();
}

UIDiskChooser::UIDiskChooser(const char* title):Fl_Double_Window(400, 200, title){
	uint32 i = 0;
	uint32 dcount = disk_count();
	disk_info_t info;
	handle_t h;
	char *fmt_name = "\\\\.\\PhysicalDrive%d";
	char *name;
	char *size;

	_list = new UIDiskList(0, 0, 400, 160);
	_list->begin();
	for (i = 0; i < dcount; i ++){
		h = disk_open(i);
		if (h != null){
			name = (char*)calloc(64, sizeof(char));
			size = (char*)calloc(32, sizeof(char));
			
			disk_info(h, &info);
			format_size(size, info.size);
			sprintf(name, fmt_name, i);
			
			_list->add_disk(i, name, size, info.mbr.gpt_protect ? info.gpt.part_count : info.mbr.part_count);
		}
		disk_close(h);
	}
	_list->end();
	add(_list);
	
	_ok = new Fl_Button(260, 170, 60, 24, "OK");
	_ok->callback(ok_cb, this);
	add(_ok);
	_cancel = new Fl_Button(330, 170, 60, 24, "Cancel");
	_cancel->callback(cancel_cb, this);
	add(_cancel);

}
UIDiskChooser::~UIDiskChooser(){
	remove(_list);
	remove(_ok);
	remove(_cancel);
	delete _ok;
	delete _cancel;
	delete _list;
}

int UIDiskChooser::diskidx(){
	for (int i = 0; i < _list->rows(); i ++){
		if (_list->row_selected(i)){
			return i;
		}
	}
	return -1;
}
void UIDiskChooser::diskidx(int diskidx){
	_list->select_all_rows(0);
	if (diskidx >= 0){
		_list->select_row(diskidx + 1, 1);
	}
}

int disk_chooser(char *title){
	if (!ch){
		ch = new UIDiskChooser(title);
	}
	ch->show();

	while (ch->shown()){
		Fl::wait();
	}

	return ch->diskidx();
}
