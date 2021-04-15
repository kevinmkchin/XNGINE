namespace profiler
{
    global_var int perf_profiler_level = 0;
    global_var real32 perf_frametime_secs = 0.f;

    uint8       perf_text_size = 17;
    uint16      perf_draw_x = 4;
    uint16      perf_draw_y = perf_text_size + 3;

    TTAFont*    perf_font_handle;
    Texture     perf_font_atlas;
    Mesh        perf_frametime_vao;

    internal void set_level(int level)
    {
        perf_profiler_level = level;
    }

    internal void initialize(TTAFont* in_perf_font_handle, Texture in_perf_font_atlas)
    {
        perf_font_handle = in_perf_font_handle;
        perf_font_atlas = in_perf_font_atlas;
        perf_frametime_vao = gl::create_mesh_array(0, 0, 0, 0, 2, 2, 0, GL_DYNAMIC_DRAW);
    }

    internal void render(OrthographicShader ui_shader, OrthographicShader text_shader)
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
            kctta_move_cursor(perf_draw_x, perf_draw_y);
            kctta_append_line(perf_frametime_string.c_str(), perf_font_handle, perf_text_size);
            TTAVertexBuffer vb = kctta_grab_buffer();
            gl::rebind_buffers(perf_frametime_vao, 
                vb.vertex_buffer, vb.index_buffer, 
                vb.vertices_array_count, vb.indices_array_count);

            mat4 perf_frametime_transform = identity_mat4();

            gl::use_shader(text_shader);
                gl::bind_projection_matrix(text_shader, g_matrix_projection_ortho.ptr());
                gl::use_texture(perf_font_atlas);
                // TODO(Kevin): get uniform location for arbitrary uniform names? or create function to bind this colour
                glUniform3f(text_shader.uniform_location("text_colour"), 1.f, 1.f, 1.f);
                gl::bind_model_matrix(text_shader, perf_frametime_transform.ptr());
                if(perf_frametime_vao.index_count > 0)
                {
                    gl::render_mesh(perf_frametime_vao);
                }
            glUseProgram(0);
        }
    }
}