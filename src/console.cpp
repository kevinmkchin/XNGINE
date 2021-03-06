#define CON_HEIGHT 400
#define CON_TEXT_SIZE 26
#define CON_TEXT_BUFFER_SIZE 800
enum ConState
{
    CON_HIDING,
    CON_HIDDEN,
    CON_SHOWING,
    CON_SHOWN
};

GLOBAL_VAR GLuint con_id_vao = 0;
GLOBAL_VAR GLuint con_id_vbo = 0;
// GLOBAL_VAR GLfloat con_vertex_buffer[] = {
//     0.f,                        CON_HEIGHT, 0.f, 0.f,
//     0.f,                        0.f,        0.f, 1.f,
//     (GLfloat)g_buffer_width,    0.f,        1.f, 1.f,
//     (GLfloat)g_buffer_width,    CON_HEIGHT, 1.f, 0.f,
//     0.f,                        CON_HEIGHT, 0.f, 0.f,
//     (GLfloat)g_buffer_width,    0.f,        1.f, 1.f
// };

// GLfloat con_vertex_buffer[] = {
//     0.f, 0.f, 0.f, 0.f,
//     0.f, CON_HEIGHT, 0.f, 1.f,
//     (GLfloat)g_buffer_width, CON_HEIGHT, 1.f, 1.f,
//     (GLfloat)g_buffer_width, 0.f, 1.f, 0.f,
//     0.f, 0.f, 0.f, 0.f,
//     (GLfloat)g_buffer_width, CON_HEIGHT, 1.f, 1.f
// };

GLfloat con_vertex_buffer[] = {
    0.f, 0.f, 0.f, 0.f,
    0.f, 400.f, 0.f, 1.f,
    1280.f, 400.f, 1.f, 1.f,
    1280.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 0.f,
    1280.f, 400.f, 1.f, 1.f
};

GLOBAL_VAR bool con_b_initialized = false;
GLOBAL_VAR char* con_chars;
GLOBAL_VAR uint16 con_cursor;
GLOBAL_VAR real32 con_y;
GLOBAL_VAR ConState con_state = CON_HIDDEN;

INTERNAL void con_initialize()
{
    con_vertex_buffer[8] = (float) g_buffer_width;
    con_vertex_buffer[12] = (float) g_buffer_width;
    con_vertex_buffer[20] = (float) g_buffer_width;
    con_vertex_buffer[5] = (float) CON_HEIGHT;
    con_vertex_buffer[9] = (float) CON_HEIGHT;
    con_vertex_buffer[21] = (float) CON_HEIGHT;

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
    glBindVertexArray(0);

    con_chars = (char*) calloc(CON_TEXT_BUFFER_SIZE, 1);
    con_b_initialized = true;
}

INTERNAL void con_print(char* text);

INTERNAL void con_printf(char* text, ...);

INTERNAL void con_toggle()
{
    if(con_state == CON_HIDING || con_state == CON_SHOWING)
    {
        return;
    }

    if(con_state == CON_HIDDEN)
    {
        // show con_state = CON_SHOWING;
        printf("show console\n");

        con_state = CON_SHOWN;
    }
    else if(con_state == CON_SHOWN)
    {
        // hide con_state = CON_HIDING;
        printf("hide console\n");

        con_state = CON_HIDDEN;
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
            con_y = 400.f;
        } break;
        case CON_HIDING: 
        {
            con_y -= 200.f * dt;
        } break;
        case CON_SHOWING: 
        {
            con_y += 200.f * dt;
        } break;
    }
}

INTERNAL void con_render(ShaderProgram ui_shader, ShaderProgram text_shader)
{
    if(!con_b_initialized || con_state == CON_HIDDEN)
    {
        return;
    }

    // render console
    float console_translation_y = con_y - (float) CON_HEIGHT;
    glm::mat4 con_transform = glm::mat4(1.f);
    con_transform = glm::translate(con_transform, glm::vec3(0.f, console_translation_y, 0.f));

    gl_use_shader(ui_shader);
        glUniformMatrix4fv(ui_shader.id_uniform_model, 1, GL_FALSE, glm::value_ptr(con_transform));
        glUniformMatrix4fv(ui_shader.id_uniform_projection, 1, GL_FALSE, glm::value_ptr(g_matrix_projection_ortho));
        //gl_use_texture(tex_brick);
        glBindVertexArray(con_id_vao);
            glDrawArrays(GL_TRIANGLES, 0, 6); // Last param could be pointer to indices but no need cuz IBO is already bound
        glBindVertexArray(0);
    //gl_use_shader(text_shader);
        // RENDER CONSOLE TEXT
    glUseProgram(0);
}

INTERNAL void con_keydown()
{
    // CHECK INPUT
}

INTERNAL bool con_shown()
{
    return con_b_initialized && con_state == CON_SHOWN;
}