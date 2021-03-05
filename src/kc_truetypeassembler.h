/** kc_truetypeassembler.h 

 - KC TrueType Assembler -

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

Backlog:
    - Kerning
    - Top to bottom text
*/

#define kctta_internal static       // kctta internal function
#define kctta_local_persist static // kctta local static variable

#define KCTTA_ASCII_FROM ' '
#define KCTTA_ASCII_TO '~'
#define KCTTA_MAX_CHAR_IN_BUFFER 800
#define KCTTA_GLYPH_COUNT KCTTA_ASCII_TO - KCTTA_ASCII_FROM + 1

/** Stores a pointer to the vertex buffer assembly array and the count of vertices in the 
    array (total length of array would be count of vertices * 4).
*/
typedef struct
{
    int             vertex_count;           // count of vertices in vertex buffer array (4 elements per vertex, so vertices_array_count / 4 = vertex_count)
    int             vertices_array_count;   // count of elements in vertex buffer array
    int             indices_array_count;    // count of elements in index buffer array
    float*          vertex_buffer;          // pointer to vertex buffer array
    unsigned int*   index_buffer;           // pointer to index buffer array
} TTAVertexBuffer;

/** TTABitmap is a handle to hold a pointer to an unsigned byte bitmap in memory. Length/count
    of bitmap elements = width * height.
*/
typedef struct 
{
    int             width;      //
    int             height;
    unsigned char*pixels; // TTABitmap pixels are only 1 byte long and contain ONLY the alpha data
} TTABitmap;

typedef struct 
{
    float           advance;
    int             width;
    int             height;
    float           offset_x;
    float           offset_y;
    float           min_u;
    float           min_v;
    float           max_u;
    float           max_v;
    char            codepoint;
} TTAGlyph;

/** TTAFont is a handle to hold font information. It's around ~4KB, so don't copy it around all the time.
*/
typedef struct 
{
    float           ascender;
    float           descender;
    float           linegap;
    TTABitmap       font_atlas;
    TTAGlyph        glyphs[KCTTA_GLYPH_COUNT];
} TTAFont;


/** 
*/
kctta_internal TTAFont kctta_init_font(TTAFont*         font_handle,
                                       unsigned char*   font_buffer,
                                       int              font_height_in_pixels);

/** Move cursor location
*/
kctta_internal void kctta_move_cursor(int x, 
                                      int y);

/** Go to new line and set X location of cursor
*/
kctta_internal void kctta_new_line(int x, 
                                   TTAFont* font);

/** Assemble quad for a glyph and append to vertex buffer.
*/
kctta_internal void kctta_append_glyph(const char in_glyph, TTAFont* font);

/** Assemble quads for a line of text and append to vertex buffer.
*/
kctta_internal void kctta_append_line(const char* line_of_text, TTAFont* font);

/** Get TTAVertexBuffer with a pointer to the vertex buffer array
    and vertex buffer information.
*/
kctta_internal TTAVertexBuffer kctta_grab_buffer();

/** Call before starting to push new text.
    Clears the vertex buffer that text is being appended to. 
    If you called kctta_grab_buffer and want to use the buffer you received,
    make sure you pass the buffer to OpenGL (glBufferData) or make a copy of
    the buffer before calling kctta_clear_buffer.
*/
kctta_internal void kctta_clear_buffer();

/** Sets the TrueTypeAssembler to use indexed vertices and return a index buffer
    as well when grabbing buffers. Clears vertex buffer if you switch the flag while
    there are already vertices in the vertex buffer. */
kctta_internal void kctta_use_index_buffer(unsigned int b_use);


///////////////////// IMPLEMENTATION //////////////////////////

kctta_internal int
kctta_ceil(float num) 
{
    int inum = (int)num;
    if (num == (float)inum) 
    {
        return inum;
    }
    return inum + 1;
}

// Buffers for vertices and texture_coords before they are written to GPU memory.
// If you have a pointer to these buffers, DO NOT let these buffers be overwritten
// before you bind the data to GPU memory.
kctta_local_persist float kctta_vertex_buffer[KCTTA_MAX_CHAR_IN_BUFFER * 6 * 4]; // 800 characters * 6 vertices * (2 xy + 2 uv)
kctta_local_persist int kctta_vertex_count = 0;           // Each vertex takes up 4 places in the assembly_buffer
kctta_local_persist unsigned int kctta_index_buffer[KCTTA_MAX_CHAR_IN_BUFFER * 6];
kctta_local_persist int kctta_index_count = 0;

kctta_local_persist unsigned int kctta_b_use_indexed_draw_flag = 0; // Flag to indicate whether to use indexed draws [https://www.learnopengles.com/tag/index-buffer-object/]

kctta_local_persist int kctta_cursor_x = 0;             // top left of the screen is pixel (0, 0), bot right of the screen is pixel (screen buffer width, screen buffer height)
kctta_local_persist int kctta_cursor_y = 100;           // cursor points to the base line at which to start drawing the glyph


kctta_internal TTAFont
kctta_init_font(unsigned char* font_buffer, int font_height_in_pixels)
{
    TTAFont font;

    int desired_atlas_width = 400;

    int font_size = font_height_in_pixels;

    stbtt_fontinfo stb_font_info;
    stbtt_InitFont(&stb_font_info, font_buffer, 0);
    
    float stb_scale = stbtt_ScaleForPixelHeight(&stb_font_info, (float)font_size);
    
    int stb_ascender;
    int stb_descender;
    int stb_linegap;
    stbtt_GetFontVMetrics(&stb_font_info, &stb_ascender, &stb_descender, &stb_linegap);
    font.ascender = (float)stb_ascender * stb_scale;
    font.descender = (float)stb_descender * stb_scale;
    font.linegap = (float)stb_linegap * stb_scale;

    // LOAD GLYPH BITMAP AND INFO FOR EVERY CHARACTER WE WANT IN THE FONT
    TTABitmap temp_glyph_bitmaps[KCTTA_GLYPH_COUNT];
    int tallest_glyph_height = 0;
    int aggregate_glyph_width = 0;
    // load glyph data
    for(char char_index = KCTTA_ASCII_FROM; char_index <= KCTTA_ASCII_TO; ++char_index) // ASCII
    {
        // get shit from stbtt
        TTAGlyph glyph;
        
        int stb_advance;
        int stb_leftbearing;
        stbtt_GetCodepointHMetrics(&stb_font_info, 
                                   char_index, 
                                   &stb_advance, 
                                   &stb_leftbearing);
        glyph.codepoint = char_index;
        glyph.advance = (float)stb_advance * stb_scale;
        
        int stb_width, stb_height;
        int stb_offset_x, stb_offset_y;
        unsigned char* stb_bitmap_temp = stbtt_GetCodepointBitmap(&stb_font_info,
                                                                0, stb_scale,
                                                                char_index,
                                                                &stb_width,
                                                                &stb_height,
                                                                &stb_offset_x,
                                                                &stb_offset_y);
        glyph.width = stb_width;
        glyph.height = stb_height;
        glyph.offset_x = (float)stb_offset_x;
        glyph.offset_y = (float)stb_offset_y;

        // Copy stb_bitmap_temp bitmap into glyph's pixels bitmap so we can free stb_bitmap_temp
        int iter = char_index - KCTTA_ASCII_FROM;
        temp_glyph_bitmaps[iter].pixels = (unsigned char*) calloc((size_t)glyph.width * (size_t)glyph.height, 1);
        for(int row = 0; row < glyph.height; ++row)
        {
            for(int col = 0; col < glyph.width; ++col)
            {
                temp_glyph_bitmaps[iter].pixels[row * glyph.width + col] = stb_bitmap_temp[(glyph.height - row - 1) * glyph.width + col];
            }
        }

        temp_glyph_bitmaps[iter].width = glyph.width;
        temp_glyph_bitmaps[iter].height = glyph.height;
        aggregate_glyph_width += glyph.width;
        if(tallest_glyph_height < glyph.height)
        {
            tallest_glyph_height = glyph.height;
        }
        stbtt_FreeBitmap(stb_bitmap_temp, 0);

        font.glyphs[iter] = glyph;
    }

    int desired_atlas_height = tallest_glyph_height
        * kctta_ceil((float)aggregate_glyph_width / (float)desired_atlas_width);
    // Build font atlas bitmap based on these parameters
    TTABitmap atlas;
    atlas.pixels = (unsigned char*) calloc(desired_atlas_width * desired_atlas_height, 1);
    atlas.width = desired_atlas_width;
    atlas.height = desired_atlas_height;
    // COMBINE ALL GLYPH BITMAPS INTO FONT ATLAS
    int glyph_count = KCTTA_GLYPH_COUNT;
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
        font.glyphs[i].min_u = (float) atlas_x / (float) atlas.width;
        font.glyphs[i].min_v = (float) atlas_y / (float) atlas.height;
        font.glyphs[i].max_u = (float) (atlas_x + glyph_bitmap.width) / (float) atlas.width;
        font.glyphs[i].max_v = (float) (atlas_y + glyph_bitmap.height) / (float) atlas.height;

        atlas_x += glyph_bitmap.width; // move the atlas bitmap cursor by glyph bitmap width

        free(glyph_bitmap.pixels);
    }
    font.font_atlas = atlas;

    return font;
}

kctta_internal void
kctta_move_cursor(int x, int y)
{
    kctta_cursor_x = x;
    kctta_cursor_y = y;
}

kctta_internal void
kctta_new_line(int x, TTAFont* font)
{
    kctta_cursor_x = x;
    kctta_cursor_y += (int) (font->descender + font->linegap + font->ascender);
}

kctta_internal void
kctta_append_glyph(const char in_glyph, TTAFont* font)
{
    int iter = in_glyph - KCTTA_ASCII_FROM;
    TTAGlyph glyph = font->glyphs[iter];

    // Make sure we are not exceeding the array size
    if(KCTTA_MAX_CHAR_IN_BUFFER * 6 < kctta_vertex_count + 6)
    {
        return;
    }

    // For each of the 6 vertices, fill in the kctta_vertex_buffer in the order x y u v
    int STRIDE = 4;

    if(kctta_b_use_indexed_draw_flag)
    {
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 0] = kctta_cursor_x + glyph.offset_x;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 1] = kctta_cursor_y + glyph.offset_y + glyph.height;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 2] = glyph.min_u;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 3] = glyph.min_v;

        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 4] = kctta_cursor_x + glyph.offset_x;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 5] = kctta_cursor_y + glyph.offset_y;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 6] = glyph.min_u;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 7] = glyph.max_v;

        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 8] = kctta_cursor_x + glyph.offset_x + glyph.width;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 9] = kctta_cursor_y + glyph.offset_y;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 10] = glyph.max_u;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 11] = glyph.max_v;

        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 12] = kctta_cursor_x + glyph.offset_x + glyph.width;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 13] = kctta_cursor_y + glyph.offset_y + glyph.height;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 14] = glyph.max_u;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 15] = glyph.min_v;

        kctta_index_buffer[kctta_index_count + 0] = kctta_vertex_count + 0;
        kctta_index_buffer[kctta_index_count + 1] = kctta_vertex_count + 1;
        kctta_index_buffer[kctta_index_count + 2] = kctta_vertex_count + 2;
        kctta_index_buffer[kctta_index_count + 3] = kctta_vertex_count + 0;
        kctta_index_buffer[kctta_index_count + 4] = kctta_vertex_count + 2;
        kctta_index_buffer[kctta_index_count + 5] = kctta_vertex_count + 3;

        kctta_vertex_count += 4;
        kctta_index_count += 6;
    }
    else
    {
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 0] = kctta_cursor_x + glyph.offset_x;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 1] = kctta_cursor_y + glyph.offset_y + glyph.height;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 2] = glyph.min_u;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 3] = glyph.min_v;

        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 4] = kctta_cursor_x + glyph.offset_x;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 5] = kctta_cursor_y + glyph.offset_y;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 6] = glyph.min_u;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 7] = glyph.max_v;

        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 8] = kctta_cursor_x + glyph.offset_x + glyph.width;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 9] = kctta_cursor_y + glyph.offset_y;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 10] = glyph.max_u;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 11] = glyph.max_v;

        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 12] = kctta_cursor_x + glyph.offset_x + glyph.width;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 13] = kctta_cursor_y + glyph.offset_y + glyph.height;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 14] = glyph.max_u;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 15] = glyph.min_v;

        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 16] = kctta_cursor_x + glyph.offset_x;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 17] = kctta_cursor_y + glyph.offset_y + glyph.height;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 18] = glyph.min_u;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 19] = glyph.min_v;

        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 20] = kctta_cursor_x + glyph.offset_x + glyph.width;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 21] = kctta_cursor_y + glyph.offset_y;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 22] = glyph.max_u;
        kctta_vertex_buffer[kctta_vertex_count * STRIDE + 23] = glyph.max_v;

        kctta_vertex_count += 6;
    }

    // Advance the cursor
    kctta_cursor_x += (int) glyph.advance;
}

kctta_internal void
kctta_append_line(const char* line_of_text, TTAFont* font)
{
    int line_start_x = kctta_cursor_x;
    while(*line_of_text != '\0')
    {
        if(*line_of_text == '\n')
        {
            kctta_new_line(line_start_x, font);
        }
        kctta_append_glyph(*line_of_text, font);
        ++line_of_text;
    }
}

kctta_internal TTAVertexBuffer
kctta_grab_buffer()
{
    TTAVertexBuffer retval;
    retval.vertex_buffer = kctta_vertex_buffer;
    if(kctta_b_use_indexed_draw_flag)
    {
        retval.index_buffer = kctta_index_buffer;
        retval.vertices_array_count = kctta_vertex_count * 4;
        retval.indices_array_count = kctta_index_count;
    }
    else
    {
        retval.index_buffer = NULL;
        retval.vertices_array_count = kctta_vertex_count * 6;
        retval.indices_array_count = 0;
    }
    retval.vertex_count = kctta_vertex_count;
    return retval;
}

kctta_internal void
kctta_clear_buffer()
{
    for(int i = 0; i < kctta_vertex_count; ++i)
    {   
        int STRIDE = 4;
        kctta_vertex_buffer[i * STRIDE + 0] = 0;
        kctta_vertex_buffer[i * STRIDE + 1] = 0;
        kctta_vertex_buffer[i * STRIDE + 2] = 0;
        kctta_vertex_buffer[i * STRIDE + 3] = 0;
    }
    for(int i = 0; i < kctta_index_count; ++i)
    {   
        kctta_vertex_buffer[i] = 0;
    }
    kctta_vertex_count = 0;
    kctta_index_count = 0;
}

kctta_internal void
kctta_use_index_buffer(unsigned int b_use)
{
    if(b_use)
    {
        if(!kctta_b_use_indexed_draw_flag)
        {
            kctta_clear_buffer();
        }
        kctta_b_use_indexed_draw_flag = 1;
    }
    else
    {
        if(kctta_b_use_indexed_draw_flag)
        {
            kctta_clear_buffer();
        }
        kctta_b_use_indexed_draw_flag = 0;
    }
}