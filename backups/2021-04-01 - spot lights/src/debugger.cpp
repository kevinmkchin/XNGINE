GLOBAL_VAR int debugger_level = 1;
bool debugger_b_debug_pointlights = true;
PointLight* debugger_point_lights = 0x0;
uint8 debugger_point_lights_count = 0;
bool debugger_b_debug_spotlights = true;
SpotLight* debugger_spot_lights = 0x0;
uint8 debugger_spot_lights_count = 0;

Mesh debug_sphere_mesh;
Mesh debug_cone_mesh;

// TODO create circle in x y z axis

INTERNAL void create_circle_vertex_buffer(real32* vertices, uint32* indices,
                                          uint32* vertex_count, uint32* indices_count,
                                          real32 x, real32 y, real32 z,
                                          real32 radius, uint8 axis=0)
{
    const real32 rad_upper_bound = 1.5f;
    const real32 rad_lower_bound = 0.5f;
    real32 rad_bound_percent = clamp((radius - rad_lower_bound) / (rad_upper_bound - rad_lower_bound), 0.f, 1.f);
    const real32 vertices_upper_bound = 128.f; //+1
    const real32 vertices_lower_bound = 32.f; //+1
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

INTERNAL Mesh create_circle_mesh(real32 pos_x, real32 pos_y, real32 pos_z, real32 radius, uint8 axis=0)
{
    real32 vertices[387]; // 129 vertices * 3 floats
    uint32 vertex_count = 0;
    uint32 indices[256]; // (129 - 1) * 2 indices
    uint32 indices_count = 0;
    create_circle_vertex_buffer(vertices, indices, &vertex_count, &indices_count, pos_x, pos_y, pos_z, radius, axis);
    return gl_create_mesh_array(vertices, indices, vertex_count * 3, indices_count, 3, 0, 0);
}

INTERNAL Mesh create_cone_mesh(real32 apex_x, real32 apex_y, real32 apex_z,
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

INTERNAL void debug_render_cone(PerspectiveShader& shader,
                                real32 x, real32 y, real32 z,
                                real32 height, real32 base_radius,
                                real32 dir_x, real32 dir_y, real32 dir_z)
{
    glm::mat4 cone_transform = glm::mat4(1.f);
    cone_transform = glm::translate(cone_transform, glm::vec3(x, y, z));

    glm::quat rotq;
    {
        using namespace glm;
        // Conversion from Euler angles (in radians) to Quaternion
        vec3 EulerAngles(90.f, 45.f, 0.f);
        quat MyQuaternion = quat(EulerAngles);

        vec3 start = vec3(0.f, -1.f, 0.f);
        vec3 dest = normalize(vec3(dir_x, dir_y, dir_z));
        start = normalize(start);
        dest = normalize(dest);

        float cosTheta = dot(start, dest);
        vec3 rotationAxis;

        if (cosTheta < -1 + 0.001f) {
            // special case when vectors in opposite directions:
            // there is no "ideal" rotation axis
            // So guess one; any will do as long as it's perpendicular to start
            rotationAxis = cross(vec3(0.0f, 0.0f, 1.0f), start);
            if (length2(rotationAxis) < 0.01) // bad luck, they were parallel, try again!
                rotationAxis = cross(vec3(1.0f, 0.0f, 0.0f), start);

            rotationAxis = normalize(rotationAxis);
            rotq = angleAxis(glm::radians(180.0f), rotationAxis);
        }
        else
        {
            rotationAxis = cross(start, dest);

            float s = sqrt((1 + cosTheta) * 2);
            float invs = 1 / s;

            rotq = quat(
                s * 0.5f,
                rotationAxis.x * invs,
                rotationAxis.y * invs,
                rotationAxis.z * invs
            );
        }
        cone_transform *= toMat4(rotq);
    }
    cone_transform = glm::scale(cone_transform, glm::vec3(base_radius, height, base_radius));

    gl_bind_model_matrix(shader, glm::value_ptr(cone_transform));
    gl_render_mesh(debug_cone_mesh, GL_LINES);
}

//INTERNAL void debug_render_line();

INTERNAL real32 debug_calculate_attenuation_range(real32 c, real32 l, real32 q)
{
    real32 ATTENUATION_FACTOR_TO_CALC_RANGE_FOR = 0.4f;
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

INTERNAL void debug_render_pointlight(PerspectiveShader& shader, PointLight& plight)
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

INTERNAL void debug_render_spotlight(PerspectiveShader& shader, SpotLight& slight)
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
            height, base_radius, slight.direction.x, slight.direction.y, slight.direction.z);
        glUniform4f(id_uni_frag_colour, 1.f, 1.f, 0.f, 1.f);
        debug_render_sphere(shader, slight.position.x, slight.position.y, slight.position.z, 0.05f);
    }
}

INTERNAL void debugger_initialize()
{
    debug_sphere_mesh = create_circle_mesh(0.f, 0.f, 0.f, 1.f);
    debug_cone_mesh = create_cone_mesh(0.f, 0.f, 0.f, 1.f, 1.f);
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

            if(debugger_b_debug_spotlights)
            {
                for(mi i = 0; i < debugger_spot_lights_count; ++i)
                {
                    debug_render_spotlight(debug_shader, debugger_spot_lights[i]);
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

INTERNAL void debugger_toggle_debug_pointlights()
{
    debugger_b_debug_pointlights = !debugger_b_debug_pointlights;
}

INTERNAL void debugger_set_spotlights(SpotLight* spot_lights_array, uint8 count)
{
    debugger_spot_lights = spot_lights_array;
    debugger_spot_lights_count = count;
}

INTERNAL void debugger_toggle_debug_spotlights()
{
    debugger_b_debug_spotlights = !debugger_b_debug_spotlights;
}

INTERNAL void debugger_set_debug_level(int level)
{
    debugger_level = level;
}