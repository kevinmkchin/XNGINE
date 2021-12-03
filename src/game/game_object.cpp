#include "game_object.h"
#include "../renderer/shader.h"
#include "../renderer/material.h"

void game_object::update()
{

}

void game_object::render(const shader_t* render_shader, const mat4* parent_model_matrix)
{
    bind_material_data(render_shader);
    mat4 model_matrix = *parent_model_matrix;
    model_matrix *= translation_matrix(pos);
    model_matrix *= rotation_matrix(orient);
    model_matrix *= scale_matrix(scale);
    bind_model_matrix_data(render_shader, &model_matrix);
    render();

    for(auto& child : children)
    {
        if(child)
        {
            child->render(render_shader, &model_matrix);
        }
    }
}

INTERNAL material_t temp_material_shiny = {4.f, 128.f };
INTERNAL material_t temp_material_dull = {0.5f, 1.f };

void game_object::bind_material_data(const shader_t* render_shader)
{
    if(render_shader->get_cached_uniform_location("material.specular_intensity") >= 0)
    {
        render_shader->gl_bind_1f("material.specular_intensity", temp_material_dull.specular_intensity);
        render_shader->gl_bind_1f("material.shininess", temp_material_dull.shininess);
    }
}

void game_object::bind_model_matrix_data(const shader_t* render_shader, const mat4* model_matrix)
{
    render_shader->gl_bind_matrix4fv("matrix_model", 1, (*model_matrix).ptr());
}

void game_object::render() const
{
    mesh_group_t* model = get_render_model();
    if(model)
    {
        model->render();
    }
}

game_object* game_object::get_parent() const
{
    return parent;
}

void game_object::set_parent(game_object* new_parent)
{
    if(parent != new_parent)
    {
        if(parent) { parent->remove_child(this); }
        parent = new_parent;
        if(parent) { parent->add_child(this); }
    }
}

std::vector<game_object*>& game_object::get_children()
{
    return children;
}

void game_object::add_child(game_object* new_child)
{
    if(!has_child(new_child))
    {
        children.push_back(new_child);
        new_child->set_parent(this);
    }
}

void game_object::remove_child(game_object* new_child)
{
    if(has_child(new_child))
    {
        auto end_iter = std::remove(children.begin(), children.end(), new_child);
        if(new_child->get_parent() == this)
        {
            new_child->set_parent(nullptr);
        }
    }
}

bool game_object::has_child(game_object *child)
{
    for (auto& actual_child : children)
    {
        if(actual_child == child)
        {
            return true;
        }
    }
    return false;
}

void game_object::set_render_model(mesh_group_t* new_model)
{
    render_model = new_model;
}

mesh_group_t* game_object::get_render_model() const
{
    return render_model;
}
