#include "debug_drawer.h"
#include "../gamedefine.h"
#include "../renderer/mesh.h"
#include "../renderer/light.h"
#include "../renderer/shader.h"
#include "../runtime/camera.h"

internal int debugger_level = 1;
int debug_drawer_get_level()
{
    return debugger_level;
}

internal bool debugger_b_debug_pointlights = true;
internal point_light_t* debugger_point_lights = nullptr;
internal uint8 debugger_point_lights_count = 0;
internal bool debugger_b_debug_spotlights = true;
internal spot_light_t* debugger_spot_lights = nullptr;
internal uint8 debugger_spot_lights_count = 0;
internal real32 ATTENUATION_FACTOR_TO_CALC_RANGE_FOR = 0.1f;

internal mesh_t debug_sphere_mesh;
internal mesh_t debug_cone_mesh;

// TODO debug show vertex normals
// TODO debug forward vector (i.e. show direction of object)

void create_circle_vertex_buffer(real32* vertices, uint32* indices,
                                          uint32* vertex_count, uint32* indices_count,
                                          real32 x, real32 y, real32 z,
                                          real32 radius, uint8 axis)
{
    const real32 rad_upper_bound = 1.5f;
    const real32 rad_lower_bound = 0.5f;
    real32 rad_bound_percent = clamp((radius - rad_lower_bound) / (rad_upper_bound - rad_lower_bound), 0.f, 1.f);
    const real32 vertices_upper_bound = 128.f;
    const real32 vertices_lower_bound = 32.f;
    real32 angle_increment = (2.f*KC_PI)
        / (((vertices_upper_bound - vertices_lower_bound) * rad_bound_percent) + vertices_lower_bound);

    for(real32 angle = 0.f; angle < 2.f*KC_PI + angle_increment; angle += angle_increment)
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

mesh_t create_circle_mesh(real32 pos_x, real32 pos_y, real32 pos_z, real32 radius, uint8 axis)
{
    real32 vertices[387]; // 129 vertices * 3 floats
    uint32 vertex_count = 0;
    uint32 indices[256]; // (129 - 1) * 2 indices
    uint32 indices_count = 0;
    create_circle_vertex_buffer(vertices, indices, &vertex_count, &indices_count, pos_x, pos_y, pos_z, radius, axis);
    mesh_t created_mesh;
    mesh_t::gl_create_mesh(created_mesh, vertices, indices, vertex_count * 3, indices_count, 3, 0, 0);
    return created_mesh;
}

mesh_t create_cone_mesh(real32 apex_x, real32 apex_y, real32 apex_z,
                                 real32 height, real32 base_radius)
{
    real32 vertices[411]; // 129 vertices * 3 floats + 8 vertices * 3 floats
    uint32 vertex_count = 0;
    uint32 indices[264]; // (129 - 1) * 2 indices + 8 indices
    uint32 indices_count = 0;
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

void debug_render_sphere(shader_t& shader, real32 x, real32 y, real32 z, real32 radius)
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
                                real32 x, real32 y, real32 z,
                                real32 height, real32 base_radius,
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

real32 debug_calculate_attenuation_range(real32 c, real32 l, real32 q)
{
    c -= 1.f / ATTENUATION_FACTOR_TO_CALC_RANGE_FOR;
    real32 discriminant = l * l - 4 * q * c;
    if (discriminant >= 0) 
    {
        real32 root1 = (-l + sqrt(discriminant)) / (2 * q);
        real32 root2 = (-l - sqrt(discriminant)) / (2 * q);
        real32 att_range = max(root1, root2);
        return att_range;
    }
    else
    {
        return 0.f;
    }
}

void debug_render_pointlight(shader_t& shader, point_light_t& plight)
{
    real32 att_radius = debug_calculate_attenuation_range(plight.att_constant, plight.att_linear, plight.att_quadratic);
    shader.gl_bind_4f("frag_colour", 1.f, 1.f, 1.f, 1.f);
    debug_render_sphere(shader, plight.position.x, plight.position.y, plight.position.z, att_radius);
    shader.gl_bind_4f("frag_colour", 1.f, 1.f, 0.f, 1.f);
    debug_render_sphere(shader, plight.position.x, plight.position.y, plight.position.z, 0.05f);
}

void debug_render_spotlight(shader_t& shader, spot_light_t& slight)
{
    real32 att_radius = debug_calculate_attenuation_range(slight.att_constant, slight.att_linear, slight.att_quadratic);
    shader.gl_bind_4f("frag_colour", 1.f, 1.f, 1.f, 1.f);
    real32 base_radius = att_radius * tanf(acosf(slight.cosine_cutoff()));
    real32 height = att_radius / slight.cosine_cutoff();
    debug_render_cone(shader, slight.position.x, slight.position.y, slight.position.z,
                      height, base_radius, slight.orientation);
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
                    debug_render_pointlight(debug_shader, debugger_point_lights[i]);
                }
            }

            if(debugger_b_debug_spotlights)
            {
                for(size_t i = 0; i < debugger_spot_lights_count; ++i)
                {
                    debug_render_spotlight(debug_shader, debugger_spot_lights[i]);
                }
            }

        }

    glUseProgram(0);
}

void debug_set_pointlights(point_light_t* point_lights_array, uint8 count)
{
    debugger_point_lights = point_lights_array;
    debugger_point_lights_count = count;
}

void debug_toggle_debug_pointlights()
{
    debugger_b_debug_pointlights = !debugger_b_debug_pointlights;
}

void debug_set_spotlights(spot_light_t* spot_lights_array, uint8 count)
{
    debugger_spot_lights = spot_lights_array;
    debugger_spot_lights_count = count;
}

void debug_toggle_debug_spotlights()
{
    debugger_b_debug_spotlights = !debugger_b_debug_spotlights;
}

void debug_set_debug_level(int level)
{
    debugger_level = level;
}