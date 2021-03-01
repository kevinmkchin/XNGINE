/** mkc_ttassembler - MKC TrueType Assembler

	Single-header library to generate vertices and texture coordinates arrays for
	creating Vertex Array Objects to render text onto the screen.

	This library's purpose is to return an array of vertices and texture coordinates:
		[ x, y, u, v, x, y, u, v, ......, x, y, u, v ]
	You can feed this into a Vertex Buffer Object with a Stride of 4.

	TODO support indexed drawing (return indices as well)

	This library is not responsible for rendering text. You can do that on your own in your
	preferred graphics API, a quad/ui rendering shader, and an orthogonal projection matrix.
*/

// TODO copy stb_truetype into this header
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define mkctta_internal static		// mkctta internal function
#define mkctta_local_persist static // mkctta local static variable


mkctta_local_persist int active_font_size = 10;	// in pixels

// font bitmap unsigned char*

// Buffers for vertices and texture_coords before they are written to GPU memory.
// If you have a pointer to these buffers, DO NOT let these buffers be overwritten
// before you bind the data to GPU memory.
mkctta_local_persist float mkctta_assembly_buffer[9600]; 		// 400 characters * 6 vertices * (2 xy + 2 uv)
mkctta_local_persist int mkctta_vertex_count;	// Each vertex takes up 4 places in the assembly_buffer
mkctta_local_persist int mkctta_drawbuffer_width; // drawable buffer width in pixels
mkctta_local_persist int mkctta_drawbuffer_height; // drawable buffer height in pixels
mkctta_local_persist int mkctta_cursor_x;
mkctta_local_persist int mkctta_cursor_y;

/** mkctta must know the underlying drawable size of the screen in pixels. */
mkctta_internal void
mkctta_set_drawable_buffer_size(int drawable_width, int drawable_height)
{
	mkctta_drawbuffer_width = drawable_width;
	mkctta_drawbuffer_height = drawable_height;
}

// mkctta_internal void 
// mkctta_set_attributes(int x, int y, int font_size)
// {
// 	active_font_size = font_size;
// }

mkctta_internal void
mkctta_init_font()
{
	// load glyph bitmap for every character we want to display

	// store width, height, minUV, maxUV into a data structure
	// maybe 
	// array of struct containing minuv maxuv etc, and the index is the glyph

	// combine each glyph bitmap into a larger bitmap
}

// TODO return the font bitmap as a texture atlas?

mkctta_internal void
mkctta_append_glyph(char glyph)
{
	// get minUV and maxUV for given glyph
	// that tells us the texture uvs

	// now we need vertices
	// find minXY and maxXY and get vertices

	// for each of the 6 vertices, fill in the mckctta_assembly_buffer in the order x y u v

	// then once we are done the 6 vertices, increment mkctta_vertex_count by 6
}

mkctta_internal void
mkctta_append_text(const char* text)
{
	while(*text != '\0')
	{
		mkctta_append_glyph(*text);
		++text;
	}
}