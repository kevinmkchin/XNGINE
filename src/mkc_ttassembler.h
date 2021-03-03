/** mkc_ttassembler - MKC TrueTypeAssembler

	mkc_assembler
	mkc_TrueTypeAssembler
	mkc_truetypebuffer
	mkc_textbuffer
	mkc_TextPrimitiveAssembler
	mkc_TextQuadAssembler
	mkc_textquadassembly
	mkcttb_*

PURPOSE:
	Single-header library to generate vertices and texture coordinates array for
	creating Vertex Buffers to render text onto the screen. Works seamlessly with 
	both OpenGL and DirectX. Probably also works with other graphics APIs out there...

    This library strives to solve 2 problems:
    - Creating an individual vertex array / textured quad for every single character you 
    want to draw is extremely inefficient.
    - Every character/glyph has varying sizes and parameters that affect how they should
    be drawn relative to all the other characters/glyphs. These must be considered when
    drawing a line of text.

USAGE:
	This library's purpose is to return an array of vertices and texture coordinates:
		[ x, y, u, v, x, y, u, v, ......, x, y, u, v ]
	You can feed this into a Vertex Buffer with a Stride of 4.

	Since this library only returns an array of vertex and texture coordinates, you 
	should be able to feed that array into the vertex buffer of any graphics API and 
	get it working.

	This library is not responsible for rendering text. You can do that on your own in your
	preferred graphics API, a quad/ui rendering shader, and an orthogonal projection matrix.

TODO:
	- Push line of text
	- Push char
	- Pop all

Backlog:
	- Top to bottom text
    - Support indexed drawing (return indices as well)
*/

// TODO copy stb_truetype into this header
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define mkctta_internal static		// mkctta internal function
#define mkctta_local_persist static // mkctta local static variable

typedef struct
{
	float* vertex_buffer; // x y u v  x y u v
	int vertex_count;
} TTAVertexBuffer;

typedef struct 
{
	unsigned char* 	pixels; // TTABitmap pixels are only 1 byte long and contain ONLY the alpha data
    int 			width;
    int 			height;
} TTABitmap;

typedef struct 
{
	//TTABitmap 		bitmap;			// glyph width and height is contained in bitmap
	float 			advance;
	float 			width;
	float 			height;
	float 			offset_x;
	float 			offset_y;
	float 			min_u;
	float 			min_v;
	float 			max_u;
	float 			max_v;
	char 			codepoint;
} TTAGlyph;

typedef struct 
{
	TTAGlyph*		glyphs;
	int 			glyph_count;
	float 			ascender;
	float			descender;
	TTABitmap 		font_atlas;
	// font identifier
} TTAFont;

mkctta_internal int
mkctta_ceil(float num) 
{
    int inum = (int)num;
    if (num == (float)inum) 
    {
        return inum;
    }
    return inum + 1;
}

#define MKCTTA_ASCII_FROM ' '
#define MKCTTA_ASCII_TO '~'

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

mkctta_internal TTABitmap
mkctta_init_font(unsigned char* font_buffer, int font_height_in_pixels)
{
	TTAFont font;

    char glyph_count = MKCTTA_ASCII_TO - MKCTTA_ASCII_FROM + 1;
    font.glyphs = (TTAGlyph*) malloc(sizeof(TTAGlyph) * glyph_count);

    int desired_atlas_width = 400;


    int FontSize = 20;
    // Result.Size = 30;
    


    stbtt_fontinfo StbFont;
    stbtt_InitFont(&StbFont, font_buffer, 0);
    
    float Scale = stbtt_ScaleForPixelHeight(&StbFont, (float)FontSize);
    

    // int StbAscent;
    // int StbDescent;
    // int StbLineGap;
    // stbtt_GetFontVMetrics(&StbFont, &StbAscent, &StbDescent, &StbLineGap);
    
    // Result.Ascent = (f32)StbAscent * Scale;
    // Result.Descent = (f32)StbDescent * Scale;
    // Result.LineGap = (f32)StbLineGap * Scale;
    
    // Result.LineAdvance = Result.Ascent - Result.Descent + Result.LineGap;


	// load glyph bitmap for every character we want to display
    TTABitmap temp_glyph_bitmaps[MKCTTA_ASCII_TO - MKCTTA_ASCII_FROM + 1] = {};
    int tallest_glyph_height = 0;
    int aggregate_glyph_width = 0;
    // load glyph data
	for(char char_index = MKCTTA_ASCII_FROM; char_index <= MKCTTA_ASCII_TO; ++char_index) // ASCII
    {
    	// get shit from stbtt
        TTAGlyph glyph = {};
        
        // int StbAdvance;
        // int StbLeftBearing;
        // stbtt_GetCodepointHMetrics(&StbFont, 
        //                            CharIndex, 
        //                            &StbAdvance, 
        //                            &StbLeftBearing);
        
        // Glyph.Codepoint = CharIndex;
        // Glyph.Advance = (f32)StbAdvance * Scale;
        // Glyph.LeftBearing = (f32)StbLeftBearing * Scale;
        
        int stb_width, stb_height;
        int StbXOffset, StbYOffset;
        unsigned char* StbBitmap = stbtt_GetCodepointBitmap(&StbFont,
                                                            0, Scale,
                                                            char_index,
                                                            &stb_width, &stb_height,
                                                            &StbXOffset, 
                                                            &StbYOffset);
        
        // glyph.Image = AllocateImageInternal(GlyphWidth, GlyphHeight, OurImageMem);
        // glyph.XOffset = StbXOffset;
        // glyph.YOffset = StbYOffset;

    	int iter = char_index - MKCTTA_ASCII_FROM;
    	temp_glyph_bitmaps[iter].pixels = (unsigned char*) calloc(stb_width * stb_height, 1);
    	// TODO copy StbBitmap into pixels
    	for(int i = 0; i < stb_width * stb_height; ++i)
    	{
    		temp_glyph_bitmaps[iter].pixels[i] = StbBitmap[i];
    	}

    	temp_glyph_bitmaps[iter].width = stb_width;
    	temp_glyph_bitmaps[iter].height = stb_height;
    	aggregate_glyph_width += stb_width;
    	if(tallest_glyph_height < stb_height)
    	{
    		tallest_glyph_height = stb_height;
    	}


        stbtt_FreeBitmap(StbBitmap, 0);
    }

    int desired_atlas_height = tallest_glyph_height
    	* mkctta_ceil((float)aggregate_glyph_width / (float)desired_atlas_width);

    TTABitmap atlas;
    atlas.pixels = (unsigned char*) calloc(desired_atlas_width * desired_atlas_height, 1);
    atlas.width = desired_atlas_width;
    atlas.height = desired_atlas_height;
    // knowing the atlas width and height before iterating through the glyphs will help us with UVs

    int atlas_x = 0;
    int atlas_y = 0;
    for(int i = 0; i < glyph_count; ++i)
    {
    	TTABitmap glyph_bitmap = temp_glyph_bitmaps[i];
    	if (atlas_x + glyph_bitmap.width > atlas.width) // check if move atlas bitmap cursor to next line
    	{
    		atlas_x = 0;
    		atlas_y += tallest_glyph_height;
    	}

    	for(int glyph_y = 0; glyph_y < glyph_bitmap.height; ++glyph_y)
    	{
    		for(int glyph_x = 0; glyph_x < glyph_bitmap.width; ++glyph_x)
    		{
    			atlas.pixels[atlas_x + glyph_x + (atlas_y + glyph_y) * atlas.width]
    				= glyph_bitmap.pixels[glyph_x + glyph_y * glyph_bitmap.width];
    		} 
    	}
    	atlas_x += glyph_bitmap.width; // move the atlas bitmap cursor by glyph bitmap width

    	// TODO set glyph UVs

    	// TODO make sure this deallocates temp glyph bitmap
    	free(glyph_bitmap.pixels);
    }

    return atlas;
}

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
mkctta_push_line(const char* line_of_text)
{
	while(*line_of_text != '\0')
	{
		mkctta_append_glyph(*line_of_text);
		++line_of_text;
	}
}

mkctta_internal void
mkctta_push_char(const char character)
{

}

mkctta_internal TTAVertexBuffer
mkcta_grab_all()
{

}

/** Call before starting to push new text. 
	Clears the vertex buffer that text is being appended to. 
*/
mkctta_internal void
mkctta_clear_buffer()
{
	for(int i = 0; i < mkctta_vertex_count; ++i)
	{
		mkctta_assembly_buffer[i] = 0;
	}
	mkctta_vertex_count = 0;
}