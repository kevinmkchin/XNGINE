#include <string>
#include "profiler.h"
#include "../../game_defines.h"
#include "../console.h"
#include <kc_truetypeassembler.h>
#include "../../renderer/texture.h"
#include "../../renderer/mesh.h"
#include "../../renderer/shader.h"
#include "../../core/timer.h"
#include "../../core/kc_math.h"
#include "../../renderer/deferred_renderer.h"
#include "../../game_statics.h"

INTERNAL int    perf_profiler_level = 0;
INTERNAL u8  PERF_TEXT_SIZE = 17;
INTERNAL u16 PERF_DRAW_X = 4;
INTERNAL u16 PERF_DRAW_Y = PERF_TEXT_SIZE + 3;

// Font
INTERNAL tta_font_t*   perf_font_handle;
INTERNAL texture_t     perf_font_atlas;

// Meshes
INTERNAL mesh_t        perf_frametime_vao;

void profiler_set_level(int level)
{
    perf_profiler_level = level;
}

int profiler_get_level()
{
    return perf_profiler_level;
}

void profiler_initialize(tta_font_t* in_perf_font_handle, texture_t in_perf_font_atlas)
{
    perf_font_handle = in_perf_font_handle;
    perf_font_atlas = in_perf_font_atlas;
    mesh_t::gl_create_mesh(perf_frametime_vao, nullptr, nullptr,
                           0, 0, 2,
                           2, 0, GL_DYNAMIC_DRAW);

    get_console().bind_cmd("profiler", profiler_set_level);
}

void profiler_render(shader_t* ui_shader, shader_t* text_shader)
{
    if(!perf_profiler_level)
    {
        return;
    }

    if(1 <= perf_profiler_level)
    {
        std::string frametime_temp = std::to_string(1000.f*timer::delta_time);
        std::string perf_frametime_string = "LAST FRAME TIME: " 
            + frametime_temp.substr(0, frametime_temp.find(".")+3)
            + "ms   FPS: "
            + std::to_string((i16)(1.f / timer::delta_time))
            + "hz";
        kctta_clear_buffer();
        kctta_move_cursor(PERF_DRAW_X, PERF_DRAW_Y);
        kctta_append_line(perf_frametime_string.c_str(), perf_font_handle, PERF_TEXT_SIZE);
        tta_vertex_buffer_t vb = kctta_grab_buffer();
        perf_frametime_vao.gl_rebind_buffer_objects(vb.vertex_buffer, vb.index_buffer,
                                                    vb.vertices_array_count, vb.indices_array_count);

        mat4 perf_frametime_transform = identity_mat4();
        mat4& matrix_projection_ortho = game_statics::the_renderer->matrix_projection_ortho;

        shader_t::gl_use_shader(*text_shader);
            text_shader->gl_bind_matrix4fv("matrix_proj_orthographic", 1, matrix_projection_ortho.ptr());
            perf_font_atlas.gl_use_texture();
            text_shader->gl_bind_1i("font_atlas_sampler", 1);
            text_shader->gl_bind_3f("text_colour", 1.f, 1.f, 1.f);
            text_shader->gl_bind_matrix4fv("matrix_model", 1, perf_frametime_transform.ptr());
            if(perf_frametime_vao.indices_count > 0)
            {
                perf_frametime_vao.gl_render_mesh();
            }
        glUseProgram(0);
    }
}