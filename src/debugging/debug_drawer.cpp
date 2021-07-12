#include "debug_drawer.h"
#include "../gamedefine.h"
#include "../renderer/mesh.h"
#include "../renderer/light.h"
#include "../renderer/shader.h"
#include "../renderer/camera.h"

internal int debugger_level = 0;
int debug_drawer_get_level()
{
    return debugger_level;
}

internal bool debugger_b_debug_pointlights = true;
internal point_light_t* debugger_point_lights = nullptr;
internal u8 debugger_point_lights_count = 0;

internal mesh_t debug_sphere_mesh;
internal mesh_t debug_cone_mesh;

// TODO debug show vertex normals
// TODO debug forward vector (i.e. show direction of object)

void create_circle_vertex_buffer(float* vertices, u32* indices,
                                 u32* vertex_count, u32* indices_count,
                                 float x, float y, float z,
                                 float radius, u8 axis)
{
    const float rad_upper_bound = 1.5f;
    const float rad_lower_bound = 0.5f;
    float rad_bound_percent = clamp((radius - rad_lower_bound) / (rad_upper_bound - rad_lower_bound), 0.f, 1.f);
    const float vertices_upper_bound = 128.f;
    const float vertices_lower_bound = 32.f;
    float angle_increment = (2.f*KC_PI)
        / (((vertices_upper_bound - vertices_lower_bound) * rad_bound_percent) + vertices_lower_bound);

    for(float angle = 0.f; angle < 2.f*KC_PI + angle_increment; angle += angle_increment)
    {
        float x_add = radius * sinf(angle);
        float z_add = radius * cosf(angle);

        vertices[(*vertex_count * 3)] = x + x_add;
        vertices[(*vertex_count * 3)+1] = y;
        vertices[(*vertex_count * 3)+2] = z + z_add;
        if(*vertex_count > 0)
        {
            indices[*indices_count] = *vertex_count - 1;
            indices[*indices_count+1] = *vertex_count;
            *indices_count += 2;
        }
        ++(*vertex_count);
    }
}

mesh_t create_circle_mesh(float pos_x, float pos_y, float pos_z, float radius, u8 axis)
{
    float vertices[387]; // 129 vertices * 3 floats
    u32 vertex_count = 0;
    u32 indices[256]; // (129 - 1) * 2 indices
    u32 indices_count = 0;
    create_circle_vertex_buffer(vertices, indices, &vertex_count, &indices_count, pos_x, pos_y, pos_z, radius, axis);
    mesh_t created_mesh;
    mesh_t::gl_create_mesh(created_mesh, vertices, indices, vertex_count * 3, indices_count, 3, 0, 0);
    return created_mesh;
}

mesh_t create_cone_mesh(float apex_x, float apex_y, float apex_z,
                                 float height, float base_radius)
{
    float vertices[411]; // 129 vertices * 3 floats + 8 vertices * 3 floats
    u32 vertex_count = 0;
    u32 indices[264]; // (129 - 1) * 2 indices + 8 indices
    u32 indices_count = 0;
    create_circle_vertex_buffer(vertices, indices, &vertex_count, &indices_count,
        apex_x, apex_y - height, apex_z, base_radius);
    for(int i = 0; i < 4; ++i)
    {
        vertices[(vertex_count * 3)] = apex_x;
        vertices[(vertex_count * 3)+1] = apex_y;
        vertices[(vertex_count * 3)+2] = apex_z;
        vertices[(vertex_count * 3)+3] = apex_x + (i < 2 ? (i % 2 == 0 ? base_radius : -base_radius) : 0);
        vertices[(vertex_count * 3)+4] = apex_y - height;
        vertices[(vertex_count * 3)+5] = apex_z + (i >= 2 ? (i % 2 == 0 ? base_radius : -base_radius) : 0);
        vertex_count += 2;
        indices[indices_count] = vertex_count-2;
        indices[indices_count+1] = vertex_count-1;
        indices_count += 2;
    }
    mesh_t created_mesh;
    mesh_t::gl_create_mesh(created_mesh, vertices, indices, vertex_count * 3, indices_count, 3, 0, 0);
    return created_mesh;
}

void debug_render_sphere(shader_t& shader, float x, float y, float z, float radius)
{
    mat4 sphere_transform = identity_mat4();
    sphere_transform *= translation_matrix(x, y, z);
    sphere_transform *= scale_matrix(radius, radius, radius);
    shader.gl_bind_matrix4fv("matrix_model", 1, sphere_transform.ptr());
    debug_sphere_mesh.gl_render_mesh(GL_LINES);
    sphere_transform *= rotation_matrix(make_quaternion_deg(90.f, make_vec3(1.f, 0.f, 0.f)));
    shader.gl_bind_matrix4fv("matrix_model", 1, sphere_transform.ptr());
    debug_sphere_mesh.gl_render_mesh(GL_LINES);
    sphere_transform *= rotation_matrix(make_quaternion_deg(90.f, make_vec3(0.f, 0.f, 1.f)));
    shader.gl_bind_matrix4fv("matrix_model", 1, sphere_transform.ptr());
    debug_sphere_mesh.gl_render_mesh(GL_LINES);
}

void debug_render_cone(shader_t& shader,
                                float x, float y, float z,
                                float height, float base_radius,
                                quaternion orientation)
{
    mat4 cone_transform = identity_mat4();
    cone_transform *= translation_matrix(make_vec3(x, y, z));
    quaternion rot = rotation_from_to(make_vec3(0.f, -1.f, 0.f), orientation_to_direction(orientation));
    cone_transform *= rotation_matrix(rot);
    cone_transform *= scale_matrix(base_radius, height, base_radius);

    shader.gl_bind_matrix4fv("matrix_model", 1, cone_transform.ptr());
    debug_cone_mesh.gl_render_mesh(GL_LINES);
}

void debug_render_line()
{
    // TODO
}

void debug_render_pointlight(shader_t& shader, point_light_t& plight)
{
    float att_radius = plight.get_radius() / 2.5f;
    shader.gl_bind_4f("frag_colour", 1.f, 1.f, 1.f, 1.f);
    debug_render_sphere(shader, plight.position.x, plight.position.y, plight.position.z, att_radius);
    shader.gl_bind_4f("frag_colour", 1.f, 1.f, 0.f, 1.f);
    debug_render_sphere(shader, plight.position.x, plight.position.y, plight.position.z, 0.05f);
}

void debug_render_spotlight(shader_t& shader, point_light_t& slight)
{
    float att_radius = slight.get_radius();
    shader.gl_bind_4f("frag_colour", 1.f, 1.f, 1.f, 1.f);
    float base_radius = att_radius * tanf(acosf(slight.cosine_cutoff()));
    float height = att_radius / slight.cosine_cutoff();
    debug_render_cone(shader, slight.position.x, slight.position.y, slight.position.z,
                      height, base_radius, direction_to_orientation(slight.get_direction()));
    shader.gl_bind_4f("frag_colour", 1.f, 1.f, 0.f, 1.f);
    debug_render_sphere(shader, slight.position.x, slight.position.y, slight.position.z, 0.05f);
}

void debug_initialize()
{
    debug_sphere_mesh = create_circle_mesh(0.f, 0.f, 0.f, 1.f);
    debug_cone_mesh = create_cone_mesh(0.f, 0.f, 0.f, 1.f, 1.f);
}

void debug_render(shader_t& debug_shader, camera_t camera)
{
    if(!debugger_level)
    {
        return;
    }

    shader_t::gl_use_shader(debug_shader);
        debug_shader.gl_bind_matrix4fv("matrix_view", 1, camera.matrix_view.ptr());
        debug_shader.gl_bind_matrix4fv("matrix_proj_perspective", 1, camera.matrix_perspective.ptr());

        if(1 <= debugger_level)
        {
            if(debugger_b_debug_pointlights)
            {
                for(size_t i = 0; i < debugger_point_lights_count; ++i)
                {
                    if(debugger_point_lights[i].is_b_spotlight())
                    {
                        debug_render_spotlight(debug_shader, debugger_point_lights[i]);
                    }
                    else
                    {
                        debug_render_pointlight(debug_shader, debugger_point_lights[i]);
                    }
                }
            }
        }

    glUseProgram(0);
}

void debug_set_pointlights(point_light_t* point_lights_array, u8 count)
{
    debugger_point_lights = point_lights_array;
    debugger_point_lights_count = count;
}

void debug_toggle_debug_pointlights()
{
    debugger_b_debug_pointlights = !debugger_b_debug_pointlights;
}

void debug_set_debug_level(int level)
{
    debugger_level = level;
}