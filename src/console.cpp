#define CON_HEIGHT 400.f
#define CON_SCROLL_SPEED 1000.f
#define CON_TEXT_SIZE 26
#define CON_COLS_IN_LINE 128        // char columns in line
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

bool con_b_initialized = false;
char con_io_buffer[CON_COLS_IN_LINE]; // line of text buffer used for both input and output
uint8 con_cursor = 0;
uint8 con_io_buffer_count = 0;
real32 con_y;
ConState con_state = CON_HIDDEN;
TTAFont* con_font;

INTERNAL void con_initialize(TTAFont* console_font)
{   
    con_cursor = 0;
    con_io_buffer_count = 0;
    assert(CON_COLS_IN_LINE - 1 <= (--con_cursor));
    assert(CON_COLS_IN_LINE - 1 <= (--con_io_buffer_count));
    con_cursor = 0;
    con_io_buffer_count = 0;


    con_font = console_font;

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

    con_line_vertex_buffer[1] = CON_HEIGHT - (float) CON_TEXT_SIZE;
    con_line_vertex_buffer[2] = (float) g_buffer_width;
    con_line_vertex_buffer[3] = CON_HEIGHT - (float) CON_TEXT_SIZE;

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
}

INTERNAL void con_print(char* text)
{
    
}

INTERNAL void con_printf(char* text, ...)
{
    // format
    // then call con_print
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
        con_state = CON_SHOWING;
    }
    else if(con_state == CON_SHOWN)
    {
        b_is_update_running = true;
        con_state = CON_HIDING;
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
            printf("%s\n", con_io_buffer);
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
        } break;
    }
}

INTERNAL void con_render(ShaderProgram ui_shader, ShaderProgram text_shader)
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
        GLint id_uniform_b_use_colour = glGetUniformLocation(ui_shader.id_shader_program, "b_use_colour");
        GLint id_uniform_ui_element_colour = glGetUniformLocation(ui_shader.id_shader_program, "ui_element_colour");
        glUniform1i(id_uniform_b_use_colour, true);
        glUniformMatrix4fv(ui_shader.id_uniform_model, 1, GL_FALSE, glm::value_ptr(con_transform));
        glUniformMatrix4fv(ui_shader.id_uniform_projection, 1, GL_FALSE, glm::value_ptr(g_matrix_projection_ortho));
        glBindVertexArray(con_id_vao);
            glUniform4f(id_uniform_ui_element_colour, 0.1f, 0.1f, 0.1f, 0.7f);
            glDrawArrays(GL_TRIANGLES, 0, 6); // Last param could be pointer to indices but no need cuz IBO is already bound
        glBindVertexArray(con_line_id_vao);
            glUniform4f(id_uniform_ui_element_colour, 0.8f, 0.8f, 0.8f, 1.f);
            glDrawArrays(GL_LINES, 0, 2);
        glBindVertexArray(0);
        glUniform1i(id_uniform_b_use_colour, false);
    gl_use_shader(text_shader);
        // RENDER CONSOLE TEXT
    glUseProgram(0);
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
        case SDLK_RETURN:
        {
            memset(con_io_buffer, 0, con_io_buffer_count);
            con_cursor = 0;
            con_io_buffer_count = 0;
            // take current input buffer and use that as command
        }break;
        case SDLK_BACKSPACE:
        {
            if(con_cursor > 0)
            {
                --con_cursor;
                con_io_buffer[con_cursor] = 0;
                --con_io_buffer_count;
            }
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
                case 48:
                {
                    keycode = 41;
                }break;
                case 54:
                {
                    keycode = 94;
                }break;
                case 55:
                {
                    keycode = 38;
                }break;
                case 56:
                {
                    keycode = 42;
                }break;
                case 57:
                {
                    keycode = 40;
                }break;
                case 45:
                {
                    keycode = 95;
                }break;
                case 61:
                {
                    keycode = 43;
                }break;
                case 39:
                {
                    keycode = 34;
                }break;
                case 59:
                {
                    keycode = 58;
                }break;
                case 44:
                {
                    keycode = 60;
                }break;
                case 46:
                {
                    keycode = 62;
                }break;
                case 47:
                {
                    keycode = 63;
                }break;
            }   
        }
    }
    
    // CHECK INPUT
    if((ASCII_SPACE <= keycode && keycode <= ASCII_TILDE))
    {
        if(con_io_buffer_count < CON_COLS_IN_LINE)
        {
            con_io_buffer[con_cursor] = keycode;
            ++con_cursor;
            ++con_io_buffer_count;
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