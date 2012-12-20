/*

UI Disk table (list)
====================

This is an UI widget that displays available physical disks

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

#include "ui_disk_list.h"

#include <Fl.H>
#include <fl_draw.H>

UIDiskList::UIDiskList(int x, int y, int w, int h):Fl_Table_Row(x, y, w, h){
	_list_disk_count = 0;
	_list_size = 16;
	_list = (disk_list_entry_t *)calloc(_list_size, sizeof(disk_list_entry_t));
	cols(4);
	col_width(0, 40);
	col_width(1, 150);
	col_width(2, 110);
	col_width(3, 60);
	col_header(1);
}
UIDiskList::~UIDiskList(){
	for (int i = 0; i < _list_disk_count; i ++){
		free(_list[i].idx);
		free(_list[i].name);
		free(_list[i].size);
		free(_list[i].part_count);
	}
	free(_list);
}
void UIDiskList::add_disk(int idx, char *name, char *size, int part_count){
	if (_list_disk_count < _list_size){
		char *id = (char *)calloc(4, sizeof(char));
		char *pt = (char *)calloc(4, sizeof(char));
		itoa(idx, id, 10);
		itoa(part_count, pt, 10);
		_list[_list_disk_count].idx = id;
		_list[_list_disk_count].name = name;
		_list[_list_disk_count].size = size;
		_list[_list_disk_count].part_count = pt;
		_list_disk_count ++;
		rows(_list_disk_count);
	}
}
void UIDiskList::draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0, int W = 0, int H = 0){
	switch ( context ) {
        case CONTEXT_STARTPAGE:             // Fl_Table telling us its starting to draw page
            fl_font(FL_HELVETICA, 16);
            return;
       
        //case CONTEXT_ROW_HEADER:            // Fl_Table telling us it's draw row/col headers
        case CONTEXT_COL_HEADER:
            fl_push_clip(X, Y, W, H);
            {
                fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, color());
                fl_color(FL_BLACK);
				switch (C){
					case 0:
						fl_draw("Num", X, Y, W, H, FL_ALIGN_CENTER);
						break;
					case 1:
						fl_draw("Name", X, Y, W, H, FL_ALIGN_CENTER);
						break;
					case 2:
						fl_draw("Size", X, Y, W, H, FL_ALIGN_CENTER);
						break;
					case 3:
						fl_draw("Part", X, Y, W, H, FL_ALIGN_CENTER);
						break;
				}
                //
            }
            fl_pop_clip();
            return;
           
        case CONTEXT_CELL:                  // Fl_Table telling us to draw cells
            fl_push_clip(X, Y, W, H);
            {
                // BG COLOR
                fl_color( row_selected(R) ? selection_color() : FL_WHITE);
                fl_rectf(X, Y, W, H);
                   
                // TEXT
                fl_color(FL_BLACK);
				switch (C){
					case 0:
						fl_draw(_list[R].idx, X, Y, W, H, FL_ALIGN_CENTER);
						break;
					case 1:
						fl_draw(_list[R].name, X + 5, Y, W - 10, H, FL_ALIGN_LEFT);
						break;
					case 2:
						fl_draw(_list[R].size, X + 5, Y, W - 10, H, FL_ALIGN_LEFT);
						break;
					case 3:
						fl_draw(_list[R].part_count, X, Y, W, H, FL_ALIGN_CENTER);
						break;
				}
                   
                // BORDER
                fl_color(FL_LIGHT2);
                fl_rect(X, Y, W, H);
            }
            fl_pop_clip();
            return;
           
    default:
        return;
    }
}