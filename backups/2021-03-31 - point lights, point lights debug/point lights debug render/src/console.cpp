/**

    QUAKE-STYLE IN-GAME CONSOLE IMPLEMENTATION
    
    There are two parts to the in-game console:

    1.  console.h/cpp:
        console.h is the interface that the rest of the game uses to communicate with console.cpp,
        console.cpp handles the console visuals, inputs, outputs, and logic related to console messages

    2.  command.h/cpp:
        command.h/cpp handles the actual invocation of console commands

*/

#define CON_MAX_PRINT_MSGS 8096
#define CON_SCROLL_SPEED 2000.f
#define CON_COLS_MAX 124        // char columns in line
#define CON_ROWS_MAX 27         // we can store more messages than this, but this is just rows that are being displayed
enum ConState
{
    CON_HIDING,
    CON_HIDDEN,
    CON_SHOWING,
    CON_SHOWN
};
GLuint con_id_vao = 0;
GLuint con_id_vbo = 0;
GLfloat con_vertex_buffer[] = {
    0.f, 0.f, 0.f, 0.f,
    0.f, 400.f, 0.f, 1.f,
    1280.f, 400.f, 1.f, 1.f,
    1280.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 0.f,
    1280.f, 400.f, 1.f, 1.f
};
GLuint con_line_id_vao = 0;
GLuint con_line_id_vbo = 0;
GLfloat con_line_vertex_buffer[] = {
    0.f, 400.f,
    1280.f, 400.f
};

bool        con_b_initialized = false;
ConState    con_state = CON_HIDDEN;
real32      con_y;

float       CON_HEIGHT = 400.f;
uint8       CON_TEXT_SIZE = 20;
uint8       CON_TEXT_PADDING_BOTTOM = 4;
uint16      CON_INPUT_DRAW_X = 4;
uint16      CON_INPUT_DRAW_Y = (uint16) (CON_HEIGHT - (float) CON_TEXT_PADDING_BOTTOM);

// Input character buffer
char        con_input_buffer[CON_COLS_MAX]; // line of text buffer used for both input and output
bool        con_b_input_buffer_dirty = false;
uint8       con_input_cursor = 0;
uint8       con_input_buffer_count = 0;

// Hidden character buffer
char        con_messages[CON_MAX_PRINT_MSGS] = {};
uint16      con_messages_read_cursor = 0;
uint16      con_messages_write_cursor = 0;
bool        con_b_messages_dirty = false;

// Text visuals
TTAFont*    con_font_handle;
Texture     con_font_atlas;
// Input text & Messages VAOs
Mesh        con_input_vao; // con_input_vao gets added to con_text_vaos (after eviction) if user "returns" command
Mesh        con_text_vaos[CON_ROWS_MAX] = {}; // one vao is one line

// TODO buffer to hold previous commands

INTERNAL void con_initialize(TTAFont* console_font_handle, Texture console_font_atlas)
{
    // ADD COMMANDS
    con_register_cmds();

    con_font_handle = console_font_handle;
    con_font_atlas = console_font_atlas;

    // INIT TEXT Mesh OBJECTS
    kctta_clear_buffer();
    kctta_move_cursor(CON_INPUT_DRAW_X, CON_INPUT_DRAW_Y);
    kctta_append_glyph('>', con_font_handle, CON_TEXT_SIZE);
    TTAVertexBuffer vb = kctta_grab_buffer();
    con_input_vao = gl_create_mesh_array(vb.vertex_buffer, vb.index_buffer, 
        vb.vertices_array_count, vb.indices_array_count, 2, 2, 0, GL_DYNAMIC_DRAW);
    // INIT MESSAGES Mesh OBJECTS
    for(int i = 0; i < CON_ROWS_MAX; ++i)
    {
        con_text_vaos[i] = gl_create_mesh_array(NULL, NULL, 0, 0, 2, 2, 0, GL_DYNAMIC_DRAW);
    }

    // INIT CONSOLE GUI
    con_vertex_buffer[8] = (float) g_buffer_width;
    con_vertex_buffer[12] = (float) g_buffer_width;
    con_vertex_buffer[20] = (float) g_buffer_width;
    con_vertex_buffer[5] = CON_HEIGHT;
    con_vertex_buffer[9] = CON_HEIGHT;
    con_vertex_buffer[21] = CON_HEIGHT;
    glGenVertexArrays(1, &con_id_vao);
    glBindVertexArray(con_id_vao);
        glGenBuffers(1, &con_id_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, con_id_vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(con_vertex_buffer), con_vertex_buffer, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(real32) * 4, 0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(real32) * 4, (void*)(sizeof(real32) * 2));
            glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    con_line_vertex_buffer[1] = CON_HEIGHT - (float) CON_TEXT_SIZE - CON_TEXT_PADDING_BOTTOM;
    con_line_vertex_buffer[2] = (float) g_buffer_width;
    con_line_vertex_buffer[3] = CON_HEIGHT - (float) CON_TEXT_SIZE - CON_TEXT_PADDING_BOTTOM;
    glGenVertexArrays(1, &con_line_id_vao);
    glBindVertexArray(con_line_id_vao);
        glGenBuffers(1, &con_line_id_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, con_line_id_vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(con_line_vertex_buffer), con_line_vertex_buffer, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    con_b_initialized = true;
    con_print("Console initialized.\n");
}

/** logs the message into the messages buffer */
INTERNAL void con_print(const char* message)
{

#if INTERNAL_BUILD
    printf(message);
#endif

    // commands get con_printed when returned
    int i = 0;
    while(*(message + i) != '\0')
    {
        con_messages[con_messages_write_cursor] = *(message + i);
        ++con_messages_write_cursor;
        if(con_messages_write_cursor >= CON_MAX_PRINT_MSGS)
        {
            con_messages_write_cursor = 0;
        }
        ++i;
    }
    con_messages_read_cursor = con_messages_write_cursor;
    con_b_messages_dirty = true;
}

INTERNAL void con_printf(const char* fmt, ...)
{
    va_list argptr;

    char message[1024];
    va_start(argptr, fmt);
    stbsp_vsprintf(message, fmt, argptr);
    va_end(argptr);

    con_print(message);
}

INTERNAL void con_command(char* text_command)
{
    if(*text_command == '\0')
    {
        return;
    }

    std::string cmd = std::string(text_command);
    cmd = ">" + cmd + "\n";
    con_print(cmd.c_str());

    char* token;
    const char delim = ' ';
    token = strtok(text_command, &delim);
    cmd = std::string(token);
    if (con_commands.find(cmd) != con_commands.end()) 
    {
        ConCommandMeta cmd_meta = con_commands.at(cmd);

        // get list of args
        int argcount = 0;
        std::vector<std::string> argslist;
        token = strtok(NULL, &delim);
        while(token != NULL && argcount < 4)
        {
            std::string arg = std::string(token);
            argslist.push_back(arg);
            ++argcount;
            token = strtok(NULL, &delim);
        }

        // invoke command
        if(cmd_meta.arg_types.size() == argcount)
        {
            COMMAND_INVOKE(cmd_meta, argslist);
        }
        else
        {
            con_printf("%s takes %zd arguments...\n", cmd, cmd_meta.arg_types.size());
        }
    }
    else
    {
        con_printf("'%s' is not a recognized command...\n", cmd);
    }
}

INTERNAL void con_toggle()
{
    if(con_state == CON_HIDING || con_state == CON_SHOWING)
    {
        return;
    }

    if(con_state == CON_HIDDEN)
    {
        b_is_update_running = false;
        SDL_SetRelativeMouseMode(SDL_FALSE);
        con_state = CON_SHOWING;
    }
    else if(con_state == CON_SHOWN)
    {
        b_is_update_running = true;
        SDL_SetRelativeMouseMode(SDL_TRUE);
        con_state = CON_HIDING;
    }
}

INTERNAL void con_update_messages()
{
    if(con_b_messages_dirty)
    {
        int msg_iterator = con_messages_read_cursor - 1;
        for(int row = 0;
            row < CON_ROWS_MAX;
            ++row)
        {
            // get line
            int line_len = 0;
            if(con_messages[msg_iterator] == '\n')
            {
                ++line_len;
                --msg_iterator;
            }
            for(char c = con_messages[msg_iterator];
                c != '\n' && c != '\0'; 
                c = con_messages[msg_iterator])
            {
                ++line_len;
                --msg_iterator;
                if(msg_iterator < 0)
                {
                    msg_iterator = CON_MAX_PRINT_MSGS - 1;
                }
            }
            // rebind vao
            {
                kctta_clear_buffer();
                kctta_move_cursor(CON_INPUT_DRAW_X, CON_INPUT_DRAW_Y);
                for(int i = 0; i < line_len; ++i)
                {
                    int j = msg_iterator + i + 1;
                    if(j >= CON_MAX_PRINT_MSGS)
                    {
                        j -= CON_MAX_PRINT_MSGS;
                    }
                    char c = con_messages[j];
                    if(c != '\n')
                    {
                        kctta_append_glyph(c, con_font_handle, CON_TEXT_SIZE);
                    }
                    else
                    {
                        kctta_new_line(CON_INPUT_DRAW_X, con_font_handle);
                    }
                }
                TTAVertexBuffer vb = kctta_grab_buffer();
                gl_rebind_buffers(con_text_vaos[row], vb.vertex_buffer, vb.index_buffer, 
                    vb.vertices_array_count, vb.indices_array_count);
            }
        }

        con_b_messages_dirty = false;
    }
}

INTERNAL void con_update(real32 dt)
{
    if(!con_b_initialized || con_state == CON_HIDDEN)
    {
        return;
    }

    switch(con_state)
    {
        case CON_SHOWN: 
        {
            if(con_b_input_buffer_dirty)
            {
                // update input vao
                kctta_clear_buffer();
                kctta_move_cursor(CON_INPUT_DRAW_X, CON_INPUT_DRAW_Y);
                std::string input_text = ">" + std::string(con_input_buffer);
                kctta_append_line(input_text.c_str(), con_font_handle, CON_TEXT_SIZE);
                TTAVertexBuffer vb = kctta_grab_buffer();
                gl_rebind_buffers(con_input_vao, vb.vertex_buffer, vb.index_buffer, 
                    vb.vertices_array_count, vb.indices_array_count);
                con_b_input_buffer_dirty = false;
            }

            con_update_messages();
        } break;
        case CON_HIDING:
        {
            con_y -= CON_SCROLL_SPEED * dt;
            if(con_y < 0.f)
            {
                con_y = 0.f;
                con_state = CON_HIDDEN;
            }
        } break;
        case CON_SHOWING: 
        {
            con_y += CON_SCROLL_SPEED * dt;
            if(con_y > CON_HEIGHT)
            {
                con_y = CON_HEIGHT;
                con_state = CON_SHOWN;
            }

            con_update_messages();
        } break;
    }
}

INTERNAL void con_render(OrthographicShader ui_shader, OrthographicShader text_shader)
{
    if(!con_b_initialized || con_state == CON_HIDDEN)
    {
        return;
    }

    float console_translation_y = con_y - (float) CON_HEIGHT;
    glm::mat4 con_transform = glm::mat4(1.f);
    con_transform = glm::translate(con_transform, glm::vec3(0.f, console_translation_y, 0.f));
    // render console
    gl_use_shader(ui_shader);
        GLint id_uniform_b_use_colour = ui_shader.uniform_location("b_use_colour");
        GLint id_uniform_ui_element_colour = ui_shader.uniform_location("ui_element_colour");
        glUniform1i(id_uniform_b_use_colour, true);
        gl_bind_model_matrix(ui_shader, glm::value_ptr(con_transform));
        gl_bind_projection_matrix(ui_shader, glm::value_ptr(g_matrix_projection_ortho));
        glBindVertexArray(con_id_vao);
            glUniform4f(id_uniform_ui_element_colour, 0.1f, 0.1f, 0.1f, 0.7f);
            glDrawArrays(GL_TRIANGLES, 0, 6); // Last param could be pointer to indices but no need cuz IBO is already bound
        glBindVertexArray(con_line_id_vao);
            glUniform4f(id_uniform_ui_element_colour, 0.8f, 0.8f, 0.8f, 1.f);
            glDrawArrays(GL_LINES, 0, 2);
        glBindVertexArray(0);
    gl_use_shader(text_shader);
        // RENDER CONSOLE TEXT
        gl_bind_projection_matrix(text_shader, glm::value_ptr(g_matrix_projection_ortho));
        gl_use_texture(con_font_atlas);

        // Input text visual
        glUniform3f(text_shader.uniform_location("text_colour"), 1.f, 1.f, 1.f);
        gl_bind_model_matrix(text_shader, glm::value_ptr(con_transform));
        if(con_input_vao.index_count > 0)
        {
            gl_render_mesh(con_input_vao);
        }
        // move transform matrix up a lil
        con_transform[3][1] -= 30.f;

        // Messages text visual
        glUniform3f(text_shader.uniform_location("text_colour"), 0.8f, 0.8f, 0.8f);
        for(int i = 0; i < CON_ROWS_MAX; ++i)
        {
            Mesh m = con_text_vaos[i];
            if(m.index_count > 0)
            {
                gl_bind_model_matrix(text_shader, glm::value_ptr(con_transform));
                con_transform[3][1] -= (float) CON_TEXT_SIZE + 3.f;
                gl_render_mesh(m);
            }
        }
    glUseProgram(0);
}

INTERNAL void con_scroll_up()
{
    int temp_cursor = con_messages_read_cursor - 1;
    char c = con_messages[temp_cursor];
    if(c == '\n')
    {
        --temp_cursor;
        if(temp_cursor < 0)
        {
            temp_cursor += CON_MAX_PRINT_MSGS;
        }
        c = con_messages[temp_cursor];
    }
    while(c != '\n' && c != '\0' && temp_cursor != con_messages_write_cursor)
    {
        --temp_cursor;
        if(temp_cursor < 0)
        {
            temp_cursor += CON_MAX_PRINT_MSGS;
        }
        c = con_messages[temp_cursor];
    }
    con_messages_read_cursor = temp_cursor + 1;
    if(con_messages_read_cursor < 0)
    {
        con_messages_read_cursor += CON_MAX_PRINT_MSGS;
    }
    con_b_messages_dirty = true;
}

INTERNAL void con_scroll_down()
{
    if(con_messages_read_cursor != con_messages_write_cursor)
    {
        int temp_cursor = con_messages_read_cursor;
        char c = con_messages[temp_cursor];
        while(c != '\n' && c != '\0' && temp_cursor != con_messages_write_cursor - 1)
        {
            ++temp_cursor;
            if(temp_cursor >= CON_MAX_PRINT_MSGS)
            {
                temp_cursor = 0;
            }
            c = con_messages[temp_cursor];
        }
        con_messages_read_cursor = temp_cursor + 1;
        if(con_messages_read_cursor > CON_MAX_PRINT_MSGS)
        {
            con_messages_read_cursor = 0;
        } 
        con_b_messages_dirty = true;   
    }
}

INTERNAL void con_keydown(SDL_KeyboardEvent& keyevent)
{
    SDL_Keycode keycode = keyevent.keysym.sym;

    // SPECIAL KEYS
    switch(keycode)
    {
        case SDLK_ESCAPE:
        {
            con_toggle();
            return;
        }
        // COMMAND
        case SDLK_RETURN:
        {
            // take current input buffer and use that as command
            con_command(con_input_buffer);
            memset(con_input_buffer, 0, con_input_buffer_count);
            con_input_cursor = 0;
            con_input_buffer_count = 0;
            con_b_input_buffer_dirty = true;
        }break;
        // Delete char before cursor
        case SDLK_BACKSPACE:
        {
            if(con_input_cursor > 0)
            {
                --con_input_cursor;
                con_input_buffer[con_input_cursor] = 0;
                --con_input_buffer_count;
                con_b_input_buffer_dirty = true;
            }
        }break;
        // Move cursor left right
        case SDLK_LEFT:
        {

        }break;
        case SDLK_RIGHT:
        {

        }break;
        // Scroll console logs
        case SDLK_UP:
        {
            con_scroll_up();
        }break;
        case SDLK_DOWN:
        {
            con_scroll_down();
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
        if(con_input_buffer_count < CON_COLS_MAX)
        {
            con_input_buffer[con_input_cursor] = keycode;
            ++con_input_cursor;
            ++con_input_buffer_count;
            con_b_input_buffer_dirty = true;
        }
    }
}

INTERNAL bool con_is_shown()
{
    return con_b_initialized && con_state == CON_SHOWN;
}

INTERNAL bool con_is_hidden()
{
    return con_state == CON_HIDDEN;
}