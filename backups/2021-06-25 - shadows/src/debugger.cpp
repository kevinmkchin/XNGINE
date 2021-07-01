global_var int debugger_level = 0;
bool debugger_b_debug_pointlights = true;
light_point_t* debugger_point_lights = 0x0;
uint8 debugger_point_lights_count = 0;
bool debugger_b_debug_spotlights = true;
light_spot_t* debugger_spot_lights = 0x0;
uint8 debugger_spot_lights_count = 0;
real32 ATTENUATION_FACTOR_TO_CALC_RANGE_FOR = 0.1f;

mesh_t debug_sphere_mesh;
mesh_t debug_cone_mesh;

// TODO debug show vertex normals

// TODO debug forward vector (i.e. show direction of object)

// TODO create circle in x y z axis
internal void create_circle_vertex_buffer(real32* vertices, uint32* indices,
                                          uint32* vertex_count, uint32* indices_count,
                                          real32 x, real32 y, real32 z,
                                          real32 radius, uint8 axis=0)
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

internal mesh_t create_circle_mesh(real32 pos_x, real32 pos_y, real32 pos_z, real32 radius, uint8 axis=0)
{
    real32 vertices[387]; // 129 vertices * 3 floats
    uint32 vertex_count = 0;
    uint32 indices[256]; // (129 - 1) * 2 indices
    uint32 indices_count = 0;
    create_circle_vertex_buffer(vertices, indices, &vertex_count, &indices_count, pos_x, pos_y, pos_z, radius, axis);
    return gl_create_mesh_array(vertices, indices, vertex_count * 3, indices_count, 3, 0, 0);
}

internal mesh_t create_cone_mesh(real32 apex_x, real32 apex_y, real32 apex_z,
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
    return gl_create_mesh_array(vertices, indices, vertex_count * 3, indices_count, 3, 0, 0);
}

internal void debug_render_sphere(shader_perspective_t& shader, real32 x, real32 y, real32 z, real32 radius)
{
    mat4 sphere_transform = identity_mat4();
    sphere_transform *= translation_matrix(x, y, z);
    sphere_transform *= scale_matrix(radius, radius, radius);
    gl_bind_model_matrix(shader, sphere_transform.ptr());
    gl_render_mesh(debug_sphere_mesh, GL_LINES);
    sphere_transform *= rotation_matrix(make_quaternion_deg(90.f, make_vec3(1.f, 0.f, 0.f)));
    gl_bind_model_matrix(shader, sphere_transform.ptr());
    gl_render_mesh(debug_sphere_mesh, GL_LINES);
    sphere_transform *= rotation_matrix(make_quaternion_deg(90.f, make_vec3(0.f, 0.f, 1.f)));
    gl_bind_model_matrix(shader, sphere_transform.ptr());
    gl_render_mesh(debug_sphere_mesh, GL_LINES);
}

internal void debug_render_cone(shader_perspective_t& shader,
                                real32 x, real32 y, real32 z,
                                real32 height, real32 base_radius,
                                quaternion orientation)
{
    mat4 cone_transform = identity_mat4();
    cone_transform *= translation_matrix(make_vec3(x, y, z));
    quaternion rot = rotation_from_to(make_vec3(0.f, -1.f, 0.f), orientation_to_direction(orientation));
    cone_transform *= rotation_matrix(rot);
    cone_transform *= scale_matrix(base_radius, height, base_radius);

    gl_bind_model_matrix(shader, cone_transform.ptr());
    gl_render_mesh(debug_cone_mesh, GL_LINES);
}

internal void debug_render_line()
{
    // TODO
}

internal real32 debug_calculate_attenuation_range(real32 c, real32 l, real32 q)
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

internal void debug_render_pointlight(shader_perspective_t& shader, light_point_t& plight)
{
    real32 att_radius = debug_calculate_attenuation_range(plight.att_constant,
                                                          plight.att_linear,
                                                          plight.att_quadratic);
    GLint id_uni_frag_colour = shader.uniform_location("frag_colour");
    if(id_uni_frag_colour != 0xffffffff)
    {
        glUniform4f(id_uni_frag_colour, 1.f, 1.f, 1.f, 1.f);
        debug_render_sphere(shader, plight.position.x, plight.position.y, plight.position.z, att_radius);
        glUniform4f(id_uni_frag_colour, 1.f, 1.f, 0.f, 1.f);
        debug_render_sphere(shader, plight.position.x, plight.position.y, plight.position.z, 0.05f);
    }
}

internal void debug_render_spotlight(shader_perspective_t& shader, light_spot_t& slight)
{
    real32 att_radius = debug_calculate_attenuation_range(slight.att_constant,
                                                      slight.att_linear,
                                                      slight.att_quadratic);
    GLint id_uni_frag_colour = shader.uniform_location("frag_colour");
    if(id_uni_frag_colour != 0xffffffff)
    {
        glUniform4f(id_uni_frag_colour, 1.f, 1.f, 1.f, 1.f);
        real32 base_radius = att_radius * tanf(acosf(slight.cosine_cutoff()));
        real32 height = att_radius / slight.cosine_cutoff();
        debug_render_cone(shader, slight.position.x, slight.position.y, slight.position.z,
            height, base_radius, slight.orientation);
        glUniform4f(id_uni_frag_colour, 1.f, 1.f, 0.f, 1.f);
        debug_render_sphere(shader, slight.position.x, slight.position.y, slight.position.z, 0.05f);
    }
}

internal void debug_initialize()
{
    debug_sphere_mesh = create_circle_mesh(0.f, 0.f, 0.f, 1.f);
    debug_cone_mesh = create_cone_mesh(0.f, 0.f, 0.f, 1.f, 1.f);
}

internal void debug_render(shader_perspective_t& debug_shader)
{
    if(!debugger_level)
    {
        return;
    }

    gl_use_shader(debug_shader);
        gl_bind_view_matrix(debug_shader, g_camera.matrix_view.ptr());
        gl_bind_projection_matrix(debug_shader, g_camera.matrix_perspective.ptr());

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

internal void debug_set_pointlights(light_point_t* point_lights_array, uint8 count)
{
    debugger_point_lights = point_lights_array;
    debugger_point_lights_count = count;
}

internal void debug_toggle_debug_pointlights()
{
    debugger_b_debug_pointlights = !debugger_b_debug_pointlights;
}

internal void debug_set_spotlights(light_spot_t* spot_lights_array, uint8 count)
{
    debugger_spot_lights = spot_lights_array;
    debugger_spot_lights_count = count;
}

internal void debug_toggle_debug_spotlights()
{
    debugger_b_debug_spotlights = !debugger_b_debug_spotlights;
}

internal void debug_set_debug_level(int level)
{
    debugger_level = level;
}