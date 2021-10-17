#ifndef XNGINE_GAME_OBJECT_H
#define XNGINE_GAME_OBJECT_H

#include "../core/kc_math.h"
#include "../renderer/mesh_group.h"

struct shader_t;

class game_object
{
public:
    vec3        pos = {0.f};
    quaternion  orient = identity_quaternion();
    vec3        scale = {1.f,1.f,1.f};
    // int32 flags
    // Tags tags[4];

public:
    game_object() = default;

    virtual void update();

    virtual void render(const shader_t* render_shader, const mat4* parent_model_matrix);


    /** Get reference to parent object */
    game_object* get_parent() const;
    /** Sets new parent-child relationship */
    void set_parent(game_object* new_parent);
    /** Get reference to children vector */
    std::vector<game_object*>& get_children();
    /** Sets new parent-child relationship */
    void add_child(game_object* new_child);
    /** Sets new_child's parent to null ONLY if new_child's parent is this */
    void remove_child(game_object* new_child);
    bool has_child(game_object* child);

    void set_render_model(mesh_group_t* new_model);
    mesh_group_t* get_render_model() const;


private:
    game_object* parent = nullptr;

    /** When a game object renders, every child of that game object gets rendered too. */
    std::vector<game_object*> children;

    mesh_group_t* render_model = nullptr;

private:
    static void bind_material_data(const shader_t* render_shader);
    static void bind_model_matrix_data(const shader_t* render_shader, const mat4* model_matrix);
    void render() const;

};

#endif //XNGINE_GAME_OBJECT_H
