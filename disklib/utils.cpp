/*

Utility functions
=================

This file contains utility functions (See: disklib.h)

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

#include "disklib.h"
#include <stdio.h>

void format_size(char *out, uint64 size){
	static char *b, *kb, *mb, *gb, *tb, *pb;

	if (b == NULL) {
		kb = "KiB";
		mb = "MiB";
		gb = "GiB";
		tb = "TiB";
		pb = "PiB";
		b = "bytes";
	}

	uint32 kilo = 1024;
	uint64 kiloI64 = kilo;
	float64 kilod = kilo;

	if (size > kiloI64 * kilo * kilo * kilo * kilo * 99)
		sprintf (out, "%I64d %s", size/ kilo / kilo /kilo/kilo/kilo, pb);
	else if (size > kiloI64*kilo*kilo*kilo*kilo)
		sprintf (out, "%.1f %s",(float64)(size/kilod/kilo/kilo/kilo/kilo), pb);
	else if (size > kiloI64*kilo*kilo*kilo*99)
		sprintf (out, "%I64d %s",size/kilo/kilo/kilo/kilo, tb);
	else if (size > kiloI64*kilo*kilo*kilo)
		sprintf (out, "%.1f %s",(float64)(size/kilod/kilo/kilo/kilo), tb);
	else if (size > kiloI64*kilo*kilo*99)
		sprintf (out, "%I64d %s",size/kilo/kilo/kilo, gb);
	else if (size > kiloI64*kilo*kilo)
		sprintf (out, "%.1f %s",(float64)(size/kilod/kilo/kilo), gb);
	else if (size > kiloI64*kilo*99)
		sprintf (out, "%I64d %s", size/kilo/kilo, mb);
	else if (size > kiloI64*kilo)
		sprintf (out, "%.1f %s",(float64)(size/kilod/kilo), mb);
	else if (size > kiloI64)
		sprintf (out, "%I64d %s", size/kilo, kb);
	else
		sprintf (out, "%I64d %s", size, b);
}
