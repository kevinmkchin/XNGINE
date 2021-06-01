/**

    QUAKE-STYLE IN-GAME CONSOLE IMPLEMENTATION
    
    There are two parts to the in-game console:

    1.  console.h/cpp:
        console.h is the interface that the rest of the game uses to communicate with console.cpp,
        console.cpp handles the console visuals, inputs, outputs, and logic related to console messages

    2.  command.h/cpp:
        command.h/cpp handles the actual invocation of console commands

*/

#define CONSOLE_MAX_PRINT_MSGS 8096
#define CONSOLE_SCROLL_SPEED 2000.f
#define CONSOLE_COLS_MAX 124        // char columns in line
#define CONSOLE_ROWS_MAX 27         // we can store more messages than this, but this is just rows that are being displayed
enum console_state_t
{
    CONSOLE_HIDING,
    CONSOLE_HIDDEN,
    CONSOLE_SHOWING,
    CONSOLE_SHOWN
};
GLuint console_background_vao_id = 0;
GLuint console_background_vbo_id = 0;
GLfloat console_background_vertex_buffer[] = {
    0.f, 0.f, 0.f, 0.f,
    0.f, 400.f, 0.f, 1.f,
    1280.f, 400.f, 1.f, 1.f,
    1280.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 0.f,
    1280.f, 400.f, 1.f, 1.f
};
GLuint console_line_vao_id = 0;
GLuint console_line_vbo_id = 0;
GLfloat console_line_vertex_buffer[] = {
    0.f, 400.f,
    1280.f, 400.f
};

bool        console_b_initialized = false;
console_state_t    console_state = CONSOLE_HIDDEN;
real32      console_y;

float       CONSOLE_HEIGHT = 400.f;
uint8       CONSOLE_TEXT_SIZE = 20;
uint8       CONSOLE_TEXT_PADDING_BOTTOM = 4;
uint16      CONSOLE_INPUT_DRAW_X = 4;
uint16      CONSOLE_INPUT_DRAW_Y = (uint16) (CONSOLE_HEIGHT - (float) CONSOLE_TEXT_PADDING_BOTTOM);

// Input character buffer
char        console_input_buffer[CONSOLE_COLS_MAX];
bool        console_b_input_buffer_dirty = false;
uint8       console_input_cursor = 0;
uint8       console_input_buffer_count = 0;

// Hidden character buffer
char        console_messages[CONSOLE_MAX_PRINT_MSGS] = {};
uint16      console_messages_read_cursor = 0;
uint16      console_messages_write_cursor = 0;
bool        console_b_messages_dirty = false;

// Text visuals
tta_font_t*   console_font_handle;
texture_t     console_font_atlas;
// Input text & Messages VAOs
mesh_t        console_inputtext_vao; // console_inputtext_vao gets added to console_messages_vaos if user "returns" command
mesh_t        console_messages_vaos[CONSOLE_ROWS_MAX] = {}; // one vao is one line

// TODO buffer to hold previous commands (max 20 commands)

internal void console_initialize(tta_font_t* in_console_font_handle, texture_t in_console_font_atlas)
{
    // ADD COMMANDS
    console_register_commands();

    console_font_handle = in_console_font_handle;
    console_font_atlas = in_console_font_atlas;

    // INIT TEXT mesh_t OBJECTS
    kctta_clear_buffer();
    kctta_move_cursor(CONSOLE_INPUT_DRAW_X, CONSOLE_INPUT_DRAW_Y);
    kctta_append_glyph('>', console_font_handle, CONSOLE_TEXT_SIZE);
    tta_vertex_buffer_t vb = kctta_grab_buffer();
    console_inputtext_vao = gl_create_mesh_array(vb.vertex_buffer, vb.index_buffer,
                                                 vb.vertices_array_count, vb.indices_array_count, 2, 2, 0, GL_DYNAMIC_DRAW);
    // INIT MESSAGES mesh_t OBJECTS
    for(int i = 0; i < CONSOLE_ROWS_MAX; ++i)
    {
        console_messages_vaos[i] = gl_create_mesh_array(NULL, NULL, 0, 0, 2, 2, 0, GL_DYNAMIC_DRAW);
    }

    // INIT CONSOLE GUI
    console_background_vertex_buffer[8] = (float) g_buffer_width;
    console_background_vertex_buffer[12] = (float) g_buffer_width;
    console_background_vertex_buffer[20] = (float) g_buffer_width;
    console_background_vertex_buffer[5] = CONSOLE_HEIGHT;
    console_background_vertex_buffer[9] = CONSOLE_HEIGHT;
    console_background_vertex_buffer[21] = CONSOLE_HEIGHT;
    glGenVertexArrays(1, &console_background_vao_id);
    glBindVertexArray(console_background_vao_id);
        glGenBuffers(1, &console_background_vbo_id);
        glBindBuffer(GL_ARRAY_BUFFER, console_background_vbo_id);
            glBufferData(GL_ARRAY_BUFFER, sizeof(console_background_vertex_buffer), console_background_vertex_buffer, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(real32) * 4, 0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(real32) * 4, (void*)(sizeof(real32) * 2));
            glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    console_line_vertex_buffer[1] = CONSOLE_HEIGHT - (float) CONSOLE_TEXT_SIZE - CONSOLE_TEXT_PADDING_BOTTOM;
    console_line_vertex_buffer[2] = (float) g_buffer_width;
    console_line_vertex_buffer[3] = CONSOLE_HEIGHT - (float) CONSOLE_TEXT_SIZE - CONSOLE_TEXT_PADDING_BOTTOM;
    glGenVertexArrays(1, &console_line_vao_id);
    glBindVertexArray(console_line_vao_id);
        glGenBuffers(1, &console_line_vbo_id);
        glBindBuffer(GL_ARRAY_BUFFER, console_line_vbo_id);
            glBufferData(GL_ARRAY_BUFFER, sizeof(console_line_vertex_buffer), console_line_vertex_buffer, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    console_b_initialized = true;
    console_print("Console initialized.\n");
}

/** logs the message into the messages buffer */
internal void console_print(const char* message)
{

#if internal_BUILD
    printf(message);
#endif

    // commands get con_printed when returned
    int i = 0;
    while(*(message + i) != '\0')
    {
        console_messages[console_messages_write_cursor] = *(message + i);
        ++console_messages_write_cursor;
        if(console_messages_write_cursor >= CONSOLE_MAX_PRINT_MSGS)
        {
            console_messages_write_cursor = 0;
        }
        ++i;
    }
    console_messages_read_cursor = console_messages_write_cursor;
    console_b_messages_dirty = true;
}

internal void console_printf(const char* fmt, ...)
{
    va_list argptr;

    char message[1024];
    va_start(argptr, fmt);
    stbsp_vsprintf(message, fmt, argptr);
    va_end(argptr);

    console_print(message);
}

internal void console_command(char* text_command)
{
    char text_command_buffer[CONSOLE_COLS_MAX];
    strcpy_s(text_command_buffer, CONSOLE_COLS_MAX, text_command);//because text_command might point to read-only data

    if(*text_command_buffer == '\0')
    {
        return;
    }

    std::string cmd = std::string(text_command_buffer);
    cmd = ">" + cmd + "\n";
    console_print(cmd.c_str());

    char* token_buff;
    const char delim = ' ';
    token_buff = strtok(text_command_buffer, &delim);
    cmd = std::string(token_buff);
    if (con_commands.find(cmd) != con_commands.end()) 
    {
        console_command_meta_t cmd_meta = con_commands.at(cmd);

        // get list of args
        int argcount = 0;
        std::vector<std::string> argslist;
        token_buff = strtok(NULL, &delim);
        while(token_buff != NULL && argcount < 4)
        {
            std::string arg = std::string(token_buff);
            argslist.push_back(arg);
            ++argcount;
            token_buff = strtok(NULL, &delim);
        }

        // invoke command
        if(cmd_meta.arg_types.size() == argcount)
        {
            COMMAND_INVOKE(cmd_meta, argslist);
        }
        else
        {
            console_printf("%s takes %zd arguments...\n", cmd.c_str(), cmd_meta.arg_types.size());
        }
    }
    else
    {
        console_printf("'%s' is not a recognized command...\n", cmd.c_str());
    }
}

internal void console_toggle()
{
    if(console_state == CONSOLE_HIDING || console_state == CONSOLE_SHOWING)
    {
        return;
    }

    if(console_state == CONSOLE_HIDDEN)
    {
        b_is_update_running = false;
        SDL_SetRelativeMouseMode(SDL_FALSE);
        console_state = CONSOLE_SHOWING;
    }
    else if(console_state == CONSOLE_SHOWN)
    {
        b_is_update_running = true;
        SDL_SetRelativeMouseMode(SDL_TRUE);
        console_state = CONSOLE_HIDING;
    }
}

internal void console_update_messages()
{
    if(console_b_messages_dirty)
    {
        int msg_iterator = console_messages_read_cursor - 1;
        for(int row = 0;
            row < CONSOLE_ROWS_MAX;
            ++row)
        {
            // get line
            int line_len = 0;
            if(console_messages[msg_iterator] == '\n')
            {
                ++line_len;
                --msg_iterator;
            }
            for(char c = console_messages[msg_iterator];
                c != '\n' && c != '\0'; 
                c = console_messages[msg_iterator])
            {
                ++line_len;
                --msg_iterator;
                if(msg_iterator < 0)
                {
                    msg_iterator = CONSOLE_MAX_PRINT_MSGS - 1;
                }
            }
            // rebind vao
            {
                kctta_clear_buffer();
                kctta_move_cursor(CONSOLE_INPUT_DRAW_X, CONSOLE_INPUT_DRAW_Y);
                for(int i = 0; i < line_len; ++i)
                {
                    int j = msg_iterator + i + 1;
                    if(j >= CONSOLE_MAX_PRINT_MSGS)
                    {
                        j -= CONSOLE_MAX_PRINT_MSGS;
                    }
                    char c = console_messages[j];
                    if(c != '\n')
                    {
                        kctta_append_glyph(c, console_font_handle, CONSOLE_TEXT_SIZE);
                    }
                    else
                    {
                        kctta_new_line(CONSOLE_INPUT_DRAW_X, console_font_handle);
                    }
                }
                tta_vertex_buffer_t vb = kctta_grab_buffer();
                gl_rebind_buffers(console_messages_vaos[row], vb.vertex_buffer, vb.index_buffer,
                                  vb.vertices_array_count, vb.indices_array_count);
            }
        }

        console_b_messages_dirty = false;
    }
}

internal void console_update(real32 dt)
{
    if(!console_b_initialized || console_state == CONSOLE_HIDDEN)
    {
        return;
    }

    switch(console_state)
    {
        case CONSOLE_SHOWN:
        {
            if(console_b_input_buffer_dirty)
            {
                // update input vao
                kctta_clear_buffer();
                kctta_move_cursor(CONSOLE_INPUT_DRAW_X, CONSOLE_INPUT_DRAW_Y);
                std::string input_text = ">" + std::string(console_input_buffer);
                kctta_append_line(input_text.c_str(), console_font_handle, CONSOLE_TEXT_SIZE);
                tta_vertex_buffer_t vb = kctta_grab_buffer();
                gl_rebind_buffers(console_inputtext_vao, vb.vertex_buffer, vb.index_buffer,
                                  vb.vertices_array_count, vb.indices_array_count);
                console_b_input_buffer_dirty = false;
            }

            console_update_messages();
        } break;
        case CONSOLE_HIDING:
        {
            console_y -= CONSOLE_SCROLL_SPEED * dt;
            if(console_y < 0.f)
            {
                console_y = 0.f;
                console_state = CONSOLE_HIDDEN;
            }
        } break;
        case CONSOLE_SHOWING:
        {
            console_y += CONSOLE_SCROLL_SPEED * dt;
            if(console_y > CONSOLE_HEIGHT)
            {
                console_y = CONSOLE_HEIGHT;
                console_state = CONSOLE_SHOWN;
            }

            console_update_messages();
        } break;
    }
}

internal void console_render(shader_orthographic_t ui_shader, shader_orthographic_t text_shader)
{
    if(!console_b_initialized || console_state == CONSOLE_HIDDEN)
    {
        return;
    }

    float console_translation_y = console_y - (float) CONSOLE_HEIGHT;
    mat4 con_transform = identity_mat4();
    con_transform *= translation_matrix(0.f, console_translation_y, 0.f);
    // render console
    gl_use_shader(ui_shader);
        GLint id_uniform_b_use_colour = ui_shader.uniform_location("b_use_colour");
        GLint id_uniform_ui_element_colour = ui_shader.uniform_location("ui_element_colour");
        glUniform1i(id_uniform_b_use_colour, true);
        gl_bind_model_matrix(ui_shader, con_transform.ptr());
        gl_bind_projection_matrix(ui_shader, g_matrix_projection_ortho.ptr());
        glBindVertexArray(console_background_vao_id);
            glUniform4f(id_uniform_ui_element_colour, 0.1f, 0.1f, 0.1f, 0.7f);
            glDrawArrays(GL_TRIANGLES, 0, 6); // Last param could be pointer to indices but no need cuz IBO is already bound
        glBindVertexArray(console_line_vao_id);
            glUniform4f(id_uniform_ui_element_colour, 0.8f, 0.8f, 0.8f, 1.f);
            glDrawArrays(GL_LINES, 0, 2);
        glBindVertexArray(0);
    gl_use_shader(text_shader);
        // RENDER CONSOLE TEXT
        gl_bind_projection_matrix(text_shader, g_matrix_projection_ortho.ptr());
        gl_use_texture(console_font_atlas);

        // Input text visual
        glUniform3f(text_shader.uniform_location("text_colour"), 1.f, 1.f, 1.f);
        gl_bind_model_matrix(text_shader, con_transform.ptr());
        if(console_inputtext_vao.index_count > 0)
        {
            gl_render_mesh(console_inputtext_vao);
        }
        // move transform matrix up a lil
        con_transform[3][1] -= 30.f;

        // Messages text visual
        glUniform3f(text_shader.uniform_location("text_colour"), 0.8f, 0.8f, 0.8f);
        for(int i = 0; i < CONSOLE_ROWS_MAX; ++i)
        {
            mesh_t m = console_messages_vaos[i];
            if(m.index_count > 0)
            {
                gl_bind_model_matrix(text_shader, con_transform.ptr());
                con_transform[3][1] -= (float) CONSOLE_TEXT_SIZE + 3.f;
                gl_render_mesh(m);
            }
        }
    glUseProgram(0);
}

internal void console_scroll_up()
{
    int temp_cursor = console_messages_read_cursor - 1;
    char c = console_messages[temp_cursor];
    if(c == '\n')
    {
        --temp_cursor;
        if(temp_cursor < 0)
        {
            temp_cursor += CONSOLE_MAX_PRINT_MSGS;
        }
        c = console_messages[temp_cursor];
    }
    while(c != '\n' && c != '\0' && temp_cursor != console_messages_write_cursor)
    {
        --temp_cursor;
        if(temp_cursor < 0)
        {
            temp_cursor += CONSOLE_MAX_PRINT_MSGS;
        }
        c = console_messages[temp_cursor];
    }
    console_messages_read_cursor = temp_cursor + 1;
    if(console_messages_read_cursor < 0)
    {
        console_messages_read_cursor += CONSOLE_MAX_PRINT_MSGS;
    }
    console_b_messages_dirty = true;
}

internal void console_scroll_down()
{
    if(console_messages_read_cursor != console_messages_write_cursor)
    {
        int temp_cursor = console_messages_read_cursor;
        char c = console_messages[temp_cursor];
        while(c != '\n' && c != '\0' && temp_cursor != console_messages_write_cursor - 1)
        {
            ++temp_cursor;
            if(temp_cursor >= CONSOLE_MAX_PRINT_MSGS)
            {
                temp_cursor = 0;
            }
            c = console_messages[temp_cursor];
        }
        console_messages_read_cursor = temp_cursor + 1;
        if(console_messages_read_cursor > CONSOLE_MAX_PRINT_MSGS)
        {
            console_messages_read_cursor = 0;
        }
        console_b_messages_dirty = true;
    }
}

internal void console_keydown(SDL_KeyboardEvent& keyevent)
{
    SDL_Keycode keycode = keyevent.keysym.sym;

    // SPECIAL KEYS
    switch(keycode)
    {
        case SDLK_ESCAPE:
        {
            console_toggle();
            return;
        }
        // COMMAND
        case SDLK_RETURN:
        {
            // take current input buffer and use that as command
            console_command(console_input_buffer);
            memset(console_input_buffer, 0, console_input_buffer_count);
            console_input_cursor = 0;
            console_input_buffer_count = 0;
            console_b_input_buffer_dirty = true;
        }break;
        // Delete char before cursor
        case SDLK_BACKSPACE:
        {
            if(console_input_cursor > 0)
            {
                --console_input_cursor;
                console_input_buffer[console_input_cursor] = 0;
                --console_input_buffer_count;
                console_b_input_buffer_dirty = true;
            }
        }break;
        case SDLK_PAGEUP:
        {
            for(int i=0;i<10;++i)
            {
                console_scroll_up();
            }
        }break;
        case SDLK_PAGEDOWN:
        {
            loop(10)
            {
                console_scroll_down();
            }
        }break;
        // TODO Move cursor left right
        case SDLK_LEFT:
        {

        }break;
        case SDLK_RIGHT:
        {

        }break;
        // TODO Flip through previously entered commands and fill command buffer w previous command
        case SDLK_UP:
        {

        }break;
        case SDLK_DOWN:
        {

        }break;
    }

    // CHECK MODIFIERS
    if(keyevent.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT))
    {
        if(97 <= keycode && keycode <= 122)
        {
            keycode -= 32;
        }
        else if(keycode == 50)
        {
            keycode = 64;
        }
        else if(49 <= keycode && keycode <= 53)
        {
            keycode -= 16;
        }
        else if(91 <= keycode && keycode <= 93)
        {
            keycode += 32;
        }
        else
        {
            switch(keycode)
            {
                case 48: { keycode = 41; } break;
                case 54: { keycode = 94; } break;
                case 55: { keycode = 38; } break;
                case 56: { keycode = 42; } break;
                case 57: { keycode = 40; } break;
                case 45: { keycode = 95; } break;
                case 61: { keycode = 43; } break;
                case 39: { keycode = 34; } break;
                case 59: { keycode = 58; } break;
                case 44: { keycode = 60; } break;
                case 46: { keycode = 62; } break;
                case 47: { keycode = 63; } break;
            }   
        }
    }
    
    // CHECK INPUT
    if((ASCII_SPACE <= keycode && keycode <= ASCII_TILDE))
    {
        if(console_input_buffer_count < CONSOLE_COLS_MAX)
        {
            console_input_buffer[console_input_cursor] = keycode;
            ++console_input_cursor;
            ++console_input_buffer_count;
            console_b_input_buffer_dirty = true;
        }
    }
}

internal bool console_is_shown()
{
    return console_b_initialized && console_state == CONSOLE_SHOWN;
}

internal bool console_is_hidden()
{
    return console_state == CONSOLE_HIDDEN;
}