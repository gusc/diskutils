/*

UI Partition table (list)
=========================

This is an UI widget that displays available GPT or MBR partitions

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

#include "ui_part_list.h"

#include <Fl.H>
#include <fl_draw.H>

UIPartList::UIPartList(int x, int y, int w, int h):Fl_Table_Row(x, y, w, h){
	_info = null;
	cols(7);
	col_header(1);
	col_header_height(24);
}
UIPartList::~UIPartList(){
	
}

void UIPartList::set_disk(int diskidx, disk_info_t *info){
	_diskidx = diskidx;
	_info = info;
	if (_info->mbr.gpt_protect){
		col_width(0, 40);
		col_width(1, 200);
		col_width(2, 40);
		col_width(3, 360);
		col_width(4, 120);
		col_width(5, 120);
		col_width(6, 90);
		rows(_info->gpt.part_count);
	} else {
		col_width(0, 40);
		col_width(1, 300);
		col_width(2, 40);
		col_width(3, 40);
		col_width(4, 120);
		col_width(5, 120);
		col_width(6, 90);
		rows(_info->mbr.part_count);
	}
	redraw();
}
void UIPartList::clear_disk(){
	_diskidx = 0;
	_info = null;
	rows(0);
	redraw();
}

void UIPartList::draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0, int W = 0, int H = 0){
	static char *cols_mbr[7] = {"Num","Path","Boot","Type","Start LBA","End LBA","Size"};
	static char *cols_gpt[7] = {"Num","Name","Boot","GUID","Start LBA","End LBA","Size"};
	static char *cols_empty[7] = {"","","","","","",""};
	char **cols;
	switch ( context ) {
        case CONTEXT_STARTPAGE:             // Fl_Table telling us its starting to draw page
            fl_font(FL_HELVETICA, 16);
            return;
       
        //case CONTEXT_ROW_HEADER:            // Fl_Table telling us it's draw row/col headers
        case CONTEXT_COL_HEADER:
            fl_push_clip(X, Y, W, H);
            {
				if (_info != null){
					cols = _info->mbr.gpt_protect ? cols_gpt : cols_mbr;
				} else {
					cols = cols_empty;
				}
                fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, color());
                fl_color(FL_BLACK);
				fl_draw(cols[C], X, Y, W, H, FL_ALIGN_CENTER);
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
				if (_info != null){
					char *fmt_name = "\\Device\\Harddisk%d\\Partition%d";
					char text[128] = "";
					fl_color(FL_BLACK);
					if (_info->mbr.gpt_protect){
						switch (C){
							case 0:
								itoa(R + 1, text, 10);
								fl_draw(text, X, Y, W, H, FL_ALIGN_CENTER);
								break;
							case 1:
								fl_draw(_info->gpt.part[R].name, X + 5, Y, W - 10, H, FL_ALIGN_LEFT);
								break;
							case 2:
								fl_draw(_info->gpt.part[R].bios_bootable ? "Yes" : "", X, Y, W, H, FL_ALIGN_CENTER);
								break;
							case 3:
								sprintf(text, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
									_info->gpt.part[R].part_guid.data1,
									_info->gpt.part[R].part_guid.data2,
									_info->gpt.part[R].part_guid.data3,
									_info->gpt.part[R].part_guid.data4[0],
									_info->gpt.part[R].part_guid.data4[1],
									_info->gpt.part[R].part_guid.data4[2],
									_info->gpt.part[R].part_guid.data4[3],
									_info->gpt.part[R].part_guid.data4[4],
									_info->gpt.part[R].part_guid.data4[5],
									_info->gpt.part[R].part_guid.data4[6],
									_info->gpt.part[R].part_guid.data4[7]
								);
								fl_draw(text, X, Y, W, H, FL_ALIGN_CENTER);
								break;
							case 4:
								itoa(_info->gpt.part[R].lba_start, text, 10);
								fl_draw(text, X + 5, Y, W - 10, H, FL_ALIGN_LEFT);
								break;
							case 5:
								itoa(_info->gpt.part[R].lba_end, text, 10);
								fl_draw(text, X + 5, Y, W - 10, H, FL_ALIGN_LEFT);
								break;
							case 6:
								format_size(text, _info->gpt.part[R].lba_size * ((uint64)_info->chs.bps));
								fl_draw(text, X + 5, Y, W - 10, H, FL_ALIGN_LEFT);
								break;
						
						}
					} else {
						switch (C){
							case 0:
								itoa(R + 1, text, 10);
								fl_draw(text, X, Y, W, H, FL_ALIGN_CENTER);
								break;
							case 1:
								sprintf(text, fmt_name, _diskidx, R);
								fl_draw(text, X + 5, Y, W - 10, H, FL_ALIGN_LEFT);
								break;
							case 2:
								fl_draw(_info->mbr.part[R].bootable ? "Yes" : "", X, Y, W, H, FL_ALIGN_CENTER);
								break;
							case 3:
								sprintf(text, "%02X", _info->mbr.part[R].type);
								fl_draw(text, X, Y, W, H, FL_ALIGN_CENTER);
								break;
							case 4:
								itoa(_info->mbr.part[R].lba_start, text, 10);
								fl_draw(text, X + 5, Y, W - 10, H, FL_ALIGN_LEFT);
								break;
							case 5:
								itoa(_info->mbr.part[R].lba_end, text, 10);
								fl_draw(text, X + 5, Y, W - 10, H, FL_ALIGN_LEFT);
								break;
							case 6:
								format_size(text, ((uint64)_info->mbr.part[R].lba_size) * ((uint64)_info->chs.bps));
								fl_draw(text, X + 5, Y, W - 10, H, FL_ALIGN_LEFT);
								break;
						
						}
					}
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