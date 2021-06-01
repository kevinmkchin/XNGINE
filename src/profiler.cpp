global_var int      perf_profiler_level = 0;
global_var real32   perf_frametime_secs = 0.f;

uint8   PERF_TEXT_SIZE = 17;
uint16  PERF_DRAW_X = 4;
uint16  PERF_DRAW_Y = PERF_TEXT_SIZE + 3;

// Font
TTAFont*    perf_font_handle;
texture_t     perf_font_atlas;

// Meshes
mesh_t        perf_frametime_vao;

internal void profiler_set_level(int level)
{
    perf_profiler_level = level;
}

internal void profiler_initialize(TTAFont* in_perf_font_handle, texture_t in_perf_font_atlas)
{
    perf_font_handle = in_perf_font_handle;
    perf_font_atlas = in_perf_font_atlas;
    perf_frametime_vao = gl_create_mesh_array(0, 0, 0, 0, 2, 2, 0, GL_DYNAMIC_DRAW);
}

internal void profiler_render(shader_orthographic_t ui_shader, shader_orthographic_t text_shader)
{
    if(!perf_profiler_level)
    {
        return;
    }

    if(1 <= perf_profiler_level)
    {
        std::string frametime_temp = std::to_string(1000.f*perf_frametime_secs);
        std::string perf_frametime_string = "LAST FRAME TIME: " 
            + frametime_temp.substr(0, frametime_temp.find(".")+3)
            + "ms   FPS: "
            + std::to_string((int16)(1.f/perf_frametime_secs))
            + "hz";
        kctta_clear_buffer();
        kctta_move_cursor(PERF_DRAW_X, PERF_DRAW_Y);
        kctta_append_line(perf_frametime_string.c_str(), perf_font_handle, PERF_TEXT_SIZE);
        TTAVertexBuffer vb = kctta_grab_buffer();
        gl_rebind_buffers(perf_frametime_vao, 
            vb.vertex_buffer, vb.index_buffer, 
            vb.vertices_array_count, vb.indices_array_count);

        mat4 perf_frametime_transform = identity_mat4();

        gl_use_shader(text_shader);
            gl_bind_projection_matrix(text_shader, g_matrix_projection_ortho.ptr());
            gl_use_texture(perf_font_atlas);
            // TODO(Kevin): get uniform location for arbitrary uniform names? or create function to bind this colour
            glUniform3f(text_shader.uniform_location("text_colour"), 1.f, 1.f, 1.f);
            gl_bind_model_matrix(text_shader, perf_frametime_transform.ptr());
            if(perf_frametime_vao.index_count > 0)
            {
                gl_render_mesh(perf_frametime_vao);
            }
        glUseProgram(0);
    }
}