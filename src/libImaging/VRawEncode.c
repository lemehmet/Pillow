/*
 * The Python Imaging Library.
 * $Id$
 *
 * coder for raw data
 *
 * FIXME: This encoder will fail if the buffer is not large enough to
 * hold one full line of data.  There's a workaround for this problem
 * in ImageFile.py, but it should be solved here instead.
 *
 * history:
 * 96-04-30 fl	created
 * 97-01-03 fl	fixed padding
 *
 * Copyright (c) Fredrik Lundh 1996-97.
 * Copyright (c) Secret Labs AB 1997.
 *
 * See the README file for information on usage and redistribution.  */


#include "Imaging.h"

int
ImagingVRawEncode(Imaging im, ImagingCodecState state, UINT8* buf, int bytes)
{
    UINT8* ptr;
	// RawEncode: st: 0 c:0 xy: 0x0 xyoff: 0x0 xysize: 128x64 yst: 1 bits:1 bytes: 65536 st.bytes:16
	// fprintf(stderr, "RawEncode: st: %d c:%d xy: %dx%d xyoff: %dx%d xysize: %dx%d yst: %d bits:%d bytes: %d st.bytes:%d\n", 
	// 	state->state, state->count, state->x, state->y, state->xoff, state->yoff, state->xsize, state->ysize,
	// 	state->ystep, state->bits, bytes, state->bytes);
    if (!state->state) {
		/* The "count" field holds the stride, if specified.  Fix
		   things up so "bytes" is the full size, and "count" the
		   packed size */

		if (state->count > 0) {
			int bytes = state->count;

			/* stride must not be less than real size */
			if (state->count < state->bytes) {
				state->errcode = IMAGING_CODEC_CONFIG;
				return -1;
			}
			state->count = state->bytes;
			state->bytes = bytes;
		} else
			state->count = state->bytes;

		/* The "ystep" field specifies the orientation */
		if (state->ystep < 0) {
			state->y = state->ysize-1;
			state->ystep = -1;
		} else
			state->ystep = 1;

		state->state = 1;
    }

    if (bytes < state->bytes) {
		state->errcode = IMAGING_CODEC_CONFIG;
		return 0;
    }

    ptr = buf;

	int pages = im->ysize / 8;
	int xoffset = state->xoff * im->pixelsize;
	// TODO: ystep = -1 case
	for(int py = 0; py < pages; ++py) {
		// fprintf(stderr, "py: %d", py);
		int y = py * 8;
		int yoffset = y + state->yoff;
		for(int x = 0; x < state->xsize; ++x) {
			*ptr = (*(im->image[yoffset + 0] + xoffset + x) == 0 ? 0 : 0x01) |
					(*(im->image[yoffset + 1] + xoffset + x) == 0 ? 0 : 0x02) |
					(*(im->image[yoffset + 2] + xoffset + x) == 0 ? 0 : 0x04) |
					(*(im->image[yoffset + 3] + xoffset + x) == 0 ? 0 : 0x08) |
					(*(im->image[yoffset + 4] + xoffset + x) == 0 ? 0 : 0x10) |
					(*(im->image[yoffset + 5] + xoffset + x) == 0 ? 0 : 0x20) |
					(*(im->image[yoffset + 6] + xoffset + x) == 0 ? 0 : 0x40) |
					(*(im->image[yoffset + 7] + xoffset + x) == 0 ? 0 : 0x80);
			ptr++;
			if ((ptr - buf) >= bytes) {
				break;
			}
		}
	}
	// fprintf(stderr, "returning %d bytes\n", (ptr - buf));
	state->errcode = IMAGING_CODEC_END;
	state->y = im->ysize;
	return ptr - buf;
}
