/*

UI Disk layout
==============

This is an UI widget that displays partition distribution on the disk

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

#include "ui_disk_layout.h"

#include <Fl.H>
#include <fl_draw.H>

UIDiskLayout::UIDiskLayout(int x, int y, int w, int h):Fl_Group(x, y, w, h){
	_info = null;
	_selected = -1;
}
UIDiskLayout::~UIDiskLayout(){

}
void UIDiskLayout::set_info(disk_info_t *info){
	_selected = -1;
	_info = info;
	redraw();
}
void UIDiskLayout::set_selected(int selected){
	_selected = selected;
	redraw();
}
void UIDiskLayout::draw(){
	Fl_Color bg = fl_rgb_color(220, 220, 220);
	//Fl_Color part = fl_rgb_color(255, 255, 255);
	fl_draw_box(FL_THIN_DOWN_BOX, x(), y(), w(), h(), bg);

	Fl_Color part[6] = {
		fl_rgb_color(255, 200, 200),
		fl_rgb_color(200, 255, 200),
		fl_rgb_color(200, 200, 255),
		fl_rgb_color(255, 200, 255),
		fl_rgb_color(200, 255, 255),
		fl_rgb_color(255, 255, 200)
	};
	Fl_Color part_sel[6] = {
		fl_rgb_color(255, 100, 100),
		fl_rgb_color(100, 255, 100),
		fl_rgb_color(100, 100, 255),
		fl_rgb_color(255, 100, 255),
		fl_rgb_color(100, 255, 255),
		fl_rgb_color(255, 255, 100)
	};

	if (_info != null){
		uint64 size = 0;
		uint32 i;
		int xp, wp;
		float32 ratio;
		if (_info->mbr.gpt_protect){
			size = _info->gpt.usable_lba_end - _info->gpt.usable_lba_start;
			ratio = ((float32)w()) / (float32)size;
			for (i = 0; i < _info->gpt.part_count; i ++){
				xp = x() + (((float32)_info->gpt.part[i].lba_start) * ratio) + 1;
				wp = (((float32)_info->gpt.part[i].lba_size) * ratio);
				if (i == _selected){
					fl_draw_box(FL_THIN_UP_BOX, xp, y() + 1, wp, h() - 2, part_sel[i % 6]);
				} else {
					fl_draw_box(FL_THIN_UP_BOX, xp, y() + 1, wp, h() - 2, part[i % 6]);
				}
			}
		} else {
			size = _info->size / _info->chs.bps;
			size --; // exclude MBR sector
			ratio = ((float32)w()) / (float32)size;
			for (i = 0; i < _info->mbr.part_count; i ++){
				xp = x() + (((float32)_info->mbr.part[i].lba_start) * ratio) + 1;
				wp = (((float32)_info->mbr.part[i].lba_size) * ratio);
				if (i == _selected){
					fl_draw_box(FL_THIN_UP_BOX, xp, y() + 1, wp, h() - 2, part_sel[i % 6]);
				} else {
					fl_draw_box(FL_THIN_UP_BOX, xp, y() + 1, wp, h() - 2, part[i % 6]);
				}
			}
		}


	}
}