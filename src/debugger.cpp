Mesh debug_sphere_mesh;

INTERNAL Mesh create_circle_mesh(real32 pos_x, real32 pos_y, real32 pos_z, real32 radius, uint8 axis=0)
{
    real32 vertices[387]; // 129 vertices * 3 floats
    uint32 vertex_count = 0;
    uint32 indices[256]; // (129 - 1) * 2 indices
    uint32 indices_count = 0;

    const real32 rad_upper_bound = 1.5f;
    const real32 rad_lower_bound = 0.5f;
    real32 rad_bound_percent = clamp((radius - rad_lower_bound) / (rad_upper_bound - rad_lower_bound), 0.f, 1.f);
    const real32 vertices_upper_bound = 128.f; //+1
    const real32 vertices_lower_bound = 32.f; //+1
    real32 angle_increment = (2.f*KC_PI)
        / (((vertices_upper_bound - vertices_lower_bound) * rad_bound_percent) + vertices_lower_bound);

    for(real32 angle = 0.f; angle < 2.f*KC_PI + angle_increment; angle += angle_increment)
    {
        float x_add = radius * sin(angle);
        float z_add = radius * cos(angle);

        vertices[(vertex_count * 3)] = pos_x + x_add;
        vertices[(vertex_count * 3)+1] = pos_y;
        vertices[(vertex_count * 3)+2] = pos_z + z_add;
        if(vertex_count > 0)
        {
            indices[indices_count] = vertex_count - 1;
            indices[indices_count+1] = vertex_count;
            indices_count += 2;
        }
        ++vertex_count;
    }

    return gl_create_mesh_array(vertices, indices, vertex_count * 3, indices_count, 3, 0, 0, GL_DYNAMIC_DRAW);
}

INTERNAL void debug_render_sphere(PerspectiveShader& shader, real32 x, real32 y, real32 z, real32 radius)
{
    glm::mat4 sphere_transform = glm::mat4(1.f);

    sphere_transform = glm::translate(sphere_transform, glm::vec3(x, y, z));
    sphere_transform = glm::scale(sphere_transform, glm::vec3(radius, radius, radius));

    gl_bind_model_matrix(shader, glm::value_ptr(sphere_transform));
    gl_render_mesh(debug_sphere_mesh, GL_LINES);
    sphere_transform = glm::rotate(sphere_transform, KC_PI/2.f, glm::vec3(1.f, 0.f, 0.f));
    gl_bind_model_matrix(shader, glm::value_ptr(sphere_transform));
    gl_render_mesh(debug_sphere_mesh, GL_LINES);
    sphere_transform = glm::rotate(sphere_transform, KC_PI/2.f, glm::vec3(0.f, 0.f, 1.f));
    gl_bind_model_matrix(shader, glm::value_ptr(sphere_transform));
    gl_render_mesh(debug_sphere_mesh, GL_LINES);
}

INTERNAL void debug_render_pointlight(PerspectiveShader& shader, PointLight& plight)
{
    real32 attenuation_factor = 1.f / 0.4f;
    real32 a = plight.att_quadratic;
    real32 b = plight.att_linear;
    real32 c = plight.att_constant - attenuation_factor;
    real32 discriminant = b * b - 4 * a * c;
    if (discriminant >= 0) {
        real32 root1 = (-b + sqrt(discriminant)) / (2 * a);
        real32 root2 = (-b - sqrt(discriminant)) / (2 * a);
        real32 att_radius = max(root1, root2);

        GLint id_uni_frag_colour = shader.uniform_location("frag_colour");
        if(id_uni_frag_colour != 0xffffffff)
        {
            glUniform4f(id_uni_frag_colour, 1.f, 1.f, 1.f, 1.f);
            debug_render_sphere(shader, plight.position.x, plight.position.y, plight.position.z, att_radius);
            glUniform4f(id_uni_frag_colour, 1.f, 1.f, 0.f, 1.f);
            debug_render_sphere(shader, plight.position.x, plight.position.y, plight.position.z, 0.05f);
        }
    }
    else
    {
        con_printf("Attenuation coefficients are messed up for a point light we are trying to debug render\n");
    }
}

int debugger_level = 0;
bool debugger_b_debug_pointlights = true;
PointLight* debugger_point_lights = 0x0;
uint8 debugger_point_lights_count;

INTERNAL void debugger_initialize()
{
    debug_sphere_mesh = create_circle_mesh(0.f, 0.f, 0.f, 1.f);
}

INTERNAL void debugger_render(PerspectiveShader& debug_shader)
{
    if(!debugger_level)
    {
        return;
    }

    gl_use_shader(debug_shader);
        gl_bind_view_matrix(debug_shader, glm::value_ptr(g_camera.matrix_view));
        gl_bind_projection_matrix(debug_shader, glm::value_ptr(g_camera.matrix_perspective));

        if(1 <= debugger_level)
        {
            if(debugger_b_debug_pointlights)
            {
                for(mi i = 0; i < debugger_point_lights_count; ++i)
                {
                    debug_render_pointlight(debug_shader, debugger_point_lights[i]);
                }
            }


        }

    glUseProgram(0);
}

INTERNAL void debugger_set_pointlights(PointLight* point_lights_array, uint8 count)
{
    debugger_point_lights = point_lights_array;
    debugger_point_lights_count = count;
}

INTERNAL void debugger_set_debug_level(int level)
{
    debugger_level = level;
}

INTERNAL void debugger_toggle_debug_pointlights()
{
    debugger_b_debug_pointlights = !debugger_b_debug_pointlights;
}