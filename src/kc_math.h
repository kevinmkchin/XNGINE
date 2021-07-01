/** kc_math.h

 - kevinmkchin's 3D Math Library -

    Do this:
        #define KC_MATH_IMPLEMENTATION
    before you include this file in *one* C++ file to create the implementation.
        // i.e. it should look like this:
        #include ...
        #include ...
        #include ...
        #define KC_MATH_IMPLEMENTATION
        #include "kc_math.h"

INTRO & PURPOSE:
    Single-header C++ library for math operations in 3D as well as some other common
    math operations used in game development.

    Right now, provides:
        
        - Vectors & vector operations
        - Matrices & matrix operations
        - Quaternions & quaternion operations
        - Methods to create transformation matrices
        - Methods to create projection matrices
        - Method to create view matrix
        - Spherical linear interpolation & vector linear interpolation

STANDARDS:
    Positive X axis is forward vector. Positive Y axis is up vector. Positive Z
    axis is right vector. Therefore, roll is rotation around X axis, pitch is
    rotation around Z axis, and yaw is rotation around Y axis.
    Quaternions: Quaternions are used to represent rotations. They are compact, 
    don't suffer from gimbal lock and can easily be interpolated. kc_math uses
    Quaternions to represent all rotations.
*/
#ifndef _INCLUDE_KC_MATH_H_
#define _INCLUDE_KC_MATH_H_

#include <cstdlib>

#define WORLD_FORWARD_VECTOR make_vec3(1.f,0.f,0.f)
#define WORLD_BACKWARD_VECTOR (-WORLD_FORWARD_VECTOR)
#define WORLD_UP_VECTOR make_vec3(0.f,1.f,0.f)
#define WORLD_DOWN_VECTOR (-WORLD_UP_VECTOR)
#define WORLD_RIGHT_VECTOR make_vec3(0.f,0.f,1.f)
#define WORLD_LEFT_VECTOR (-WORLD_RIGHT_VECTOR)

#define KC_PI 3.1415926535f
#define KC_TWOPI 6.28318530718f
#define KC_DEG2RAD 0.0174532925f  // in_degrees * KC_DEG2RAD = in_radians
#define KC_RAD2DEG 57.2958f       // in_radians * KC_RAD2DEG = in_degrees

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define abs(a) ((a) < 0.f ? (-(a)) : (a))
#define clamp(x, lower, upper) max((lower), min((upper), (x)))

/** - Vectors -
    just floats

    todo use unions

    union vec3
    {
        struct
        {
            float x;
            float y;
            float z;
        }
        struct
        {
            float roll;
            float yaw;
            float pitch;
        }
    };
*/

struct vec2i
{
    int x = 0;
    int y = 0;
};

struct vec3
{
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;

    float& operator[] (int row)
    {
        float* address = (float*)this;
        return address[row];
    }
    const float& operator[] (int row) const
    {
        float* address = (float*)this;
        return address[row];
    }
};

struct vec4
{
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;
    float w = 0.f;

    float& operator[] (int row)
    {
        float* address = (float*)this;
        return address[row];
    }
    const float& operator[] (int row) const
    {
        float* address = (float*)this;
        return address[row];
    }
};

/** - Matrices -
    Column-major order
    Access like so: mat4[col][row]
    Laid in memory like so:
    0x????0000  col 0 row 0 : float
    0x????0004  col 0 row 1 : float
    0x????0008  col 0 row 2 : float
    0x????000c  col 0 row 3 : float
    0x????0010  col 1 row 0 : float
    0x????0014  col 1 row 1 : float
    0x????0018  col 1 row 2 : float
    0x????001c  col 1 row 3 : float
    0x????0020  col 2 row 0 : float
    0x????0024  col 2 row 1 : float
    0x????0028  col 2 row 2 : float
    0x????002c  col 2 row 3 : float
    0x????0030  col 3 row 0 : float
    0x????0034  col 3 row 1 : float
    0x????0038  col 3 row 2 : float
    0x????003c  col 3 row 3 : float
    Get float array address through ptr()
    Can use initializer list like mat4 m = { 00, 01, ... , 33 };
*/
struct mat3
{
    vec3 e[3] = { 0.f };

    float* ptr() { return((float*)this); }

    vec3& operator[] (int col) { return e[col]; }
    const vec3& operator[] (int col) const { return e[col]; }
};

struct mat4
{
    vec4 e[4] = { 0.f };

    float* ptr() { return((float*)this); }

    vec4& operator[] (int col) { return e[col]; }
    const vec4& operator[] (int col) const { return e[col]; }
};

/** - Quaternions -
    You shouldn't modify quaternions values directly--use functions
*/
struct quaternion
{
    float w = 0.f;
    float x = 0.f;
    float y = 0.f;
    float z = 0.f; 
};

/**

    Constructors and identity consturctors

*/
inline vec3 make_vec3(float inx, float iny, float inz);
inline vec4 make_vec4(float inx, float iny, float inz, float inw);

inline mat3 identity_mat3();
inline mat4 identity_mat4();
inline mat4 make_mat4(const mat3& from);

inline quaternion identity_quaternion(); // w: 1, x: 0, y: 0, z: 0
inline quaternion make_quaternion(float w, float x, float y, float z);
inline quaternion make_quaternion(float x, float y, float z);
inline quaternion make_quaternion_rad(float angle_in_rads, vec3 axis_of_rotation);
inline quaternion make_quaternion_deg(float angle_in_degs, vec3 axis_of_rotation);

/**

    Vector Operations

*/
inline vec3 add(vec3 a, vec3 b);
inline vec4 add(vec4 a, vec4 b);
inline vec3 sub(vec3 a, vec3 b);
inline vec4 sub(vec4 a, vec4 b);
inline vec3 mul(vec3 a, float b);
inline vec4 mul(vec4 a, float b);
inline vec3 div(vec3 a, float b);
inline vec4 div(vec4 a, float b);
inline float dot(vec3 a, vec3 b);
inline float dot(vec4 a, vec4 b);
inline vec3 cross(vec3 a, vec3 b);
inline float magnitude(vec3 a);
inline float magnitude(vec4 a);
inline vec3 normalize(vec3 a);
inline vec4 normalize(vec4 a);

inline vec3 operator-(vec3 a);
inline vec3 operator+(vec3 a, vec3 b);
inline vec3 operator-(vec3 a, vec3 b);
inline vec3 operator*(vec3 a, float b);
inline vec3 operator*(float b, vec3 a);
inline vec3 operator/(vec3 a, float b);
inline vec3 &operator+=(vec3& a, vec3 b);
inline vec3 &operator-=(vec3& a, vec3 b);
inline vec3 &operator*=(vec3& a, float b);
inline vec3 &operator/=(vec3& a, float b);
inline vec4 operator-(vec4 a);
inline vec4 operator+(vec4 a, vec4 b);
inline vec4 operator-(vec4 a, vec4 b);
inline vec4 operator*(vec4 a, float b);
inline vec4 operator*(float b, vec4 a);
inline vec4 operator/(vec4 a, float b);
inline vec4 &operator+=(vec4& a, vec4 b);
inline vec4 &operator-=(vec4& a, vec4 b);
inline vec4 &operator*=(vec4& a, float b);
inline vec4 &operator/=(vec4& a, float b);

inline vec3 lerp(vec3 from, vec3 to, float ratio);
inline vec4 lerp(vec4 from, vec4 to, float ratio);

/**

    Matrix Operations

*/
inline mat3 mul(const mat3& a, const mat3& b);
inline mat4 mul(const mat4& a, const mat4& b);
inline vec3 mul(const mat3& A, vec3 v);
inline vec4 mul(const mat4& A, vec4 v);

inline vec3 operator*(mat3 A, vec3 v);
inline mat3 operator*(mat3 a, mat3 b);
inline mat3 &operator*=(mat3& a, mat3& b);
inline vec4 operator*(mat4 A, vec4 v);
inline mat4 operator*(mat4 a, mat4 b);
inline mat4 &operator*=(mat4& a, mat4& b);

/** Generates translation matrix for given delta x delta y delta z
    https://en.wikipedia.org/wiki/Translation_(geometry)#Matrix_resentation */
inline mat4 translation_matrix(float x, float y, float z);
inline mat4 translation_matrix(vec3 translation);

/** Generates rotation matrix for given quaternion represented rotation
    https://en.wikipedia.org/wiki/Rotation_matrix#In_three_dimensions */
inline mat4 rotation_matrix(quaternion q);

/** Generates scaling matrix for given x y z scales
    https://en.wikipedia.org/wiki/Scaling_(geometry)#Using_homogeneous_coordinates */
inline mat4 scale_matrix(float x_scale, float y_scale, float z_scale);
inline mat4 scale_matrix(vec3 scale);

/** Creates a matrix for a symetric perspective-view frustum based on the default handedness and default near and far clip planes definition.
    fovy: Specifies the field of view angle in the y direction. Expressed in radians.
    aspect: Specifies the aspect ratio that determines the field of view in the x direction. The aspect ratio is the ratio of x (width) to y (height).
    nearclip: Specifies the distance from the viewer to the near clipping plane (always positive).
    farclip: Specifies the distance from the viewer to the far clipping plane (always positive).

    https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml
*/
inline mat4 projection_matrix_perspective(float fovy, float aspect, float nearclip, float farclip);

/** Creates a matrix for projecting two-dimensional coordinates onto the screen.
    https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluOrtho2D.xml

    left, right: Specify the coordinates for the left and right vertical clipping planes.
    bottom, top: Specify the coordinates for the bottom and top horizontal clipping planes.
    e.g. projection_matrix_orthographic(0.f, 1920.f, 1080.f, 0.f);
*/
inline mat4 projection_matrix_orthographic_2d(float left, float right, float bottom, float top);

/** Creates a matrix for an orthographic parallel viewing volume, using right-handed coordinates.
    https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glOrtho.xml

    The near and far clip planes correspond to z normalized device coordinates of -1 and +1 respectively.(OpenGL clip volume definition)
*/
inline mat4 projection_matrix_orthographic(float left, float right, float bottom, float top, float z_near, float z_far);

/**
*/
inline mat4 view_matrix_look_at(vec3 const& eye, vec3 const& target, vec3 const& in_up);

/**

    Quaternion Operations

*/
inline quaternion add(quaternion a, quaternion b);
inline quaternion sub(quaternion a, quaternion b);
inline float dot(quaternion a, quaternion b);
inline quaternion mul(quaternion a, quaternion b);
inline quaternion mul(quaternion a, float scale);
inline quaternion div(quaternion a, float scale);
inline float magnitude(quaternion a);
inline quaternion normalize(quaternion a);

/** Checks if dot product of a and b is within 1 +/- tolerance */
inline bool similar(quaternion a, quaternion b, float tolerance = 0.001f);

/** Combines rotations represented by quaternions. Equivalent to second * first. */
inline quaternion cumulate_rotations(quaternion first_rotation, quaternion second_rotation);

/** Gets the conjugate of given quaternion. Doesn't check that magnitude is 1. */
inline quaternion conjugate(quaternion a);

/** Identical to conjugate */
inline quaternion inverse(quaternion a);

/** Gets the conjugate of the given quaternion with magnitude 1 */
inline quaternion inverse_unit(quaternion a);

/** Convert Quaternion to Euler angles IN RADIANS. When you read the .eulerAngles property,
    Unity converts the Quaternion's internal representation of the rotation to Euler angles.
    Because, there is more than one way to represent any given rotation using Euler angles,
    the values you read back out may be quite different from the values you assigned. This
    can cause confusion if you are trying to gradually increment the values to produce animation.
    To avoid these kinds of problems, the recommended way to work with rotations is to avoid
    relying on consistent results when reading .eulerAngles particularly when attempting to
    gradually increment a rotation to produce animation.
    This will not work when the Z euler angle is within [90, 270] degrees. This is a
    limitation with euler angles: euler angles (of any type) have a singularity. Unity's
    Quaternion.eulerAngle also experiences the same limitation, so I don't think there is
    anything I can do about it. Just whenever possible, avoid using euler angles.
*/
inline vec3 quat_to_euler(quaternion q);

/** Convert Euler angles IN RADIANS to a rotation Quaternion representing a rotation
    x/roll degrees around the x-axis, z/pitch degrees around the z-axis, and y/yaw degrees
    around the y-axis; applied in that order.
    See https://ntrs.nasa.gov/api/citations/19770024290/downloads/19770024290.pdf
    The following wikipedia page uses a different order of rotation, but still helpful:
    https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles#Euler_angles_to_quaternion_conversion
*/
inline quaternion euler_to_quat(float roll, float yaw, float pitch);
inline quaternion euler_to_quat(vec3 euler_angles);

/** Returns an orientation that faces the direction as given direction. The return value
    represents the rotation from the world's forward direction (forward vector) in order
    to face the same direction as the given direction. */
inline quaternion direction_to_orientation(vec3 direction);

/** Returns the direction of this orientation. The world's forward direction (forward vector)
    rotated by the orientation is the direction. */
inline vec3 orientation_to_direction(quaternion orientation);

/** Creates a rotation which rotates from from_direction to to_direction
    Similar to https://docs.unity3d.com/ScriptReference/Quaternion.FromToRotation.html
*/
inline quaternion rotation_from_to(vec3 from_direction, vec3 to_direction);

/** Finds the difference such that b = difference * a */
inline quaternion rotation_difference(quaternion a, quaternion b);

/** Rotates given vector by given quaternion represented rotation.
    The center of rotation is the origin. If you want to rotate around another point,
    translate the vector before calling rotate_vector, then inverse translate (translate back). */
inline vec3 rotate_vector(vec3 vector, quaternion rotation);

/** Converts quaternion to a 3x3 matrix representing the rotation */
inline mat3 quaternion_to_mat3(quaternion q);

/** Converts quaternion to a 4x4 matrix representing the rotation */
inline mat4 quaternion_to_mat4(quaternion q);

/** Converts quaternion to a 3x3 matrix representing the rotation */
inline mat3 make_mat3(quaternion q);

/** Converts quaternion to a 4x4 matrix representing the rotation */
inline mat4 make_mat4(quaternion q);

/** Spherically interpolates between quaternions from and to by ratio. The parameter ratio is clamped to the range [0, 1].
    Use this to create a rotation which smoothly interpolates between the first quaternion a to the second quaternion b,
    based on the value of the interpolation ratrio.
    from : Start value, returned when t = 0.
    to : End value, returned when t = 1.
    ratio : Interpolation ratio.
    https://www.youtube.com/watch?v=x1aCcyD0hqE&ab_channel=JorgeRodriguez
*/
inline quaternion slerp(const quaternion from, const quaternion to, const float ratio);


/**

    Other Operations

*/
inline float lerp(float from, float to, float ratio);



/**

    IMPLEMENTATION

*/

inline vec3 make_vec3(float inx, float iny, float inz)
{
    vec3 ret = {inx,iny,inz};
    return ret;
}

inline vec4 make_vec4(float inx, float iny, float inz, float inw)
{
    vec4 ret = {inx,iny,inz,inw};
    return ret;
}

inline mat3 identity_mat3()
{
    mat3 ret;
    ret[0][0] = 1.f;
    ret[1][1] = 1.f;
    ret[2][2] = 1.f;
    return ret;
}

inline mat4 identity_mat4()
{
    mat4 ret;
    ret[0][0] = 1.f;
    ret[1][1] = 1.f;
    ret[2][2] = 1.f;
    ret[3][3] = 1.f;
    return ret;
}

inline mat4 make_mat4(const mat3& from)
{
    mat4 ret;
    ret[0][0] = from[0][0];
    ret[0][1] = from[0][1];
    ret[0][2] = from[0][2];
    ret[1][0] = from[1][0];
    ret[1][1] = from[1][1];
    ret[1][2] = from[1][2];
    ret[2][0] = from[2][0];
    ret[2][1] = from[2][1];
    ret[2][2] = from[2][2];
    ret[3][3] = 1.f;
    return ret;
}

inline quaternion identity_quaternion()
{
    quaternion ret = { 1.f, 0.f, 0.f, 0.f };
    return ret;
}

inline quaternion make_quaternion(float w, float x, float y, float z)
{
    quaternion ret;
    ret.w = w;
    ret.x = x;
    ret.y = y;
    ret.z = z;
    ret = normalize(ret);
    return ret;
}

inline quaternion make_quaternion(float x, float y, float z)
{
    quaternion ret;
    ret.w = 0.f;
    ret.x = x;
    ret.y = y;
    ret.z = z;
    ret = normalize(ret);
    return ret;
}

inline quaternion make_quaternion_rad(float angle_in_rads, vec3 axis_of_rotation)
{
    axis_of_rotation = normalize(axis_of_rotation);
    quaternion ret;
    float half_angle = angle_in_rads * 0.5f;
    float s = sinf(half_angle);
    ret.w = cosf(half_angle);
    ret.x = axis_of_rotation.x * s;
    ret.y = axis_of_rotation.y * s;
    ret.z = axis_of_rotation.z * s;
    return ret;
}

inline quaternion make_quaternion_deg(float angle_in_degs, vec3 axis_of_rotation)
{
    return make_quaternion_rad(angle_in_degs * KC_DEG2RAD, axis_of_rotation);
}

/**

    Vector Operations
    
*/
inline vec3 add(vec3 a, vec3 b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

inline vec4 add(vec4 a, vec4 b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    a.w += b.w;
    return a;
}

inline vec3 sub(vec3 a, vec3 b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

inline vec4 sub(vec4 a, vec4 b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    a.w -= b.w;
    return a;
}

inline vec3 mul(vec3 a, float b)
{
    a.x *= b;
    a.y *= b;
    a.z *= b;
    return a;
}

inline vec4 mul(vec4 a, float b)
{
    a.x *= b;
    a.y *= b;
    a.z *= b;
    a.w *= b;
    return a;
}

inline vec3 div(vec3 a, float b)
{
    b = 1.f/b;
    a.x *= b;
    a.y *= b;
    a.z *= b;
    return a;
}

inline vec4 div(vec4 a, float b)
{
    b = 1.f/b;
    a.x *= b;
    a.y *= b;
    a.z *= b;
    a.w *= b;
    return a;
}

inline float dot(vec3 a, vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z; 
}

inline float dot(vec4 a, vec4 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

inline vec3 cross(vec3 a, vec3 b)
{
    vec3 R;
    R.x = a.y * b.z - b.y * a.z;
    R.y = a.z * b.x - b.z * a.x;
    R.z = a.x * b.y - b.x * a.y;
    return R;
}

inline float magnitude(vec3 a) { return sqrtf(dot(a, a)); }

inline float magnitude(vec4 a) { return sqrtf(dot(a, a)); }

inline vec3 normalize(vec3 a) { return div(a, magnitude(a)); }

inline vec4 normalize(vec4 a) { return div(a, magnitude(a)); }

inline vec3 operator-(vec3 a) { vec3 r = { -a.x, -a.y, -a.z }; return(r); }
inline vec3 operator+(vec3 a, vec3 b) { return add(a, b); }
inline vec3 operator-(vec3 a, vec3 b) { return sub(a, b); }
inline vec3 operator*(vec3 a, float b) { return mul(a, b); }
inline vec3 operator*(float b, vec3 a) { return mul(a, b); }
inline vec3 operator/(vec3 a, float b) { return div(a, b); }
inline vec3 &operator+=(vec3& a, vec3 b) { return(a = a + b); }
inline vec3 &operator-=(vec3& a, vec3 b) { return(a = a - b); }
inline vec3 &operator*=(vec3& a, float b) { return(a = a * b); }
inline vec3 &operator/=(vec3& a, float b) { return(a = a / b); }

inline vec4 operator-(vec4 a) { vec4 r = { -a.x, -a.y, -a.z, -a.w }; return(r); }
inline vec4 operator+(vec4 a, vec4 b) { return add(a, b); }
inline vec4 operator-(vec4 a, vec4 b) { return sub(a, b); }
inline vec4 operator*(vec4 a, float b) { return mul(a, b); }
inline vec4 operator*(float b, vec4 a) { return mul(a, b); }
inline vec4 operator/(vec4 a, float b) { return div(a, b); }
inline vec4 &operator+=(vec4& a, vec4 b) { return(a = a + b); }
inline vec4 &operator-=(vec4& a, vec4 b) { return(a = a - b); }
inline vec4 &operator*=(vec4& a, float b) { return(a = a * b); }
inline vec4 &operator/=(vec4& a, float b) { return(a = a / b); }

inline vec3 lerp(vec3 from, vec3 to, float ratio) { return((1.0f - ratio) * from + to * ratio); }
inline vec4 lerp(vec4 from, vec4 to, float ratio) { return((1.0f - ratio) * from + to * ratio); }

/**

    Matrix Operations
    
*/

inline mat3 mul(const mat3& a, const mat3& b)
{
    mat3 res;
    
    res.e[0][0] = dot(make_vec3(a[0][0], a[1][0], a[2][0]), b[0]);
    res.e[0][1] = dot(make_vec3(a[0][1], a[1][1], a[2][1]), b[0]);
    res.e[0][2] = dot(make_vec3(a[0][2], a[1][2], a[2][2]), b[0]);
    
    res.e[1][0] = dot(make_vec3(a[0][0], a[1][0], a[2][0]), b[1]);
    res.e[1][1] = dot(make_vec3(a[0][1], a[1][1], a[2][1]), b[1]);
    res.e[1][2] = dot(make_vec3(a[0][2], a[1][2], a[2][2]), b[1]);
    
    res.e[2][0] = dot(make_vec3(a[0][0], a[1][0], a[2][0]), b[2]);
    res.e[2][1] = dot(make_vec3(a[0][1], a[1][1], a[2][1]), b[2]);
    res.e[2][2] = dot(make_vec3(a[0][2], a[1][2], a[2][2]), b[2]);
    
    return res;
}

inline mat4 mul(const mat4& a, const mat4& b)
{
    mat4 res;
    
    res.e[0][0] = dot(make_vec4(a[0][0], a[1][0], a[2][0], a[3][0]), b[0]);
    res.e[0][1] = dot(make_vec4(a[0][1], a[1][1], a[2][1], a[3][1]), b[0]);
    res.e[0][2] = dot(make_vec4(a[0][2], a[1][2], a[2][2], a[3][2]), b[0]);
    res.e[0][3] = dot(make_vec4(a[0][3], a[1][3], a[2][3], a[3][3]), b[0]);
    
    res.e[1][0] = dot(make_vec4(a[0][0], a[1][0], a[2][0], a[3][0]), b[1]);
    res.e[1][1] = dot(make_vec4(a[0][1], a[1][1], a[2][1], a[3][1]), b[1]);
    res.e[1][2] = dot(make_vec4(a[0][2], a[1][2], a[2][2], a[3][2]), b[1]);
    res.e[1][3] = dot(make_vec4(a[0][3], a[1][3], a[2][3], a[3][3]), b[1]);
    
    res.e[2][0] = dot(make_vec4(a[0][0], a[1][0], a[2][0], a[3][0]), b[2]);
    res.e[2][1] = dot(make_vec4(a[0][1], a[1][1], a[2][1], a[3][1]), b[2]);
    res.e[2][2] = dot(make_vec4(a[0][2], a[1][2], a[2][2], a[3][2]), b[2]);
    res.e[2][3] = dot(make_vec4(a[0][3], a[1][3], a[2][3], a[3][3]), b[2]);
    
    res.e[3][0] = dot(make_vec4(a[0][0], a[1][0], a[2][0], a[3][0]), b[3]);
    res.e[3][1] = dot(make_vec4(a[0][1], a[1][1], a[2][1], a[3][1]), b[3]);
    res.e[3][2] = dot(make_vec4(a[0][2], a[1][2], a[2][2], a[3][2]), b[3]);
    res.e[3][3] = dot(make_vec4(a[0][3], a[1][3], a[2][3], a[3][3]), b[3]);
    
    return res;
}

inline vec3 mul(const mat3& A, vec3 v) 
{
    return A[0] * v.x + A[1] * v.y + A[2] * v.z;
}

inline vec4 mul(const mat4& A, vec4 v) 
{
    return A[0] * v.x + A[1] * v.y + A[2] * v.z + A[3] * v.w;
}

inline vec3 operator*(mat3 A, vec3 v) { return(mul(A, v)); }
inline mat3 operator*(mat3 a, mat3 b) { return(mul(a, b)); }
inline mat3 &operator*=(mat3& a, mat3& b) { a = mul(a, b); return a; }

inline vec4 operator*(mat4 A, vec4 v) { return(mul(A, v)); }
inline mat4 operator*(mat4 a, mat4 b) { return(mul(a, b)); }
inline mat4 &operator*=(mat4& a, mat4& b) { a = mul(a, b); return a; }

inline mat4 translation_matrix(float x, float y, float z)
{
    mat4 ret = identity_mat4();
    ret[3][0] = x;
    ret[3][1] = y;
    ret[3][2] = z;
    return ret;
}

inline mat4 translation_matrix(vec3 translation)
{
    return translation_matrix(translation.x, translation.y, translation.z);
}

inline mat4 rotation_matrix(quaternion q)
{
    return make_mat4(q);
}

inline mat4 scale_matrix(float x_scale, float y_scale, float z_scale)
{
    mat4 ret;
    ret[0][0] = x_scale;
    ret[1][1] = y_scale;
    ret[2][2] = z_scale;
    ret[3][3] = 1.f;
    return ret;
}

inline mat4 scale_matrix(vec3 scale)
{
    return scale_matrix(scale.x, scale.y, scale.z);
}

inline mat4 projection_matrix_perspective(float fovy, float aspect, float nearclip, float farclip)
{
    float const tanHalfFovy = tan(fovy / 2.f);

    mat4 Result;
    Result[0][0] = 1.f / (aspect * tanHalfFovy);
    Result[1][1] = 1.f / (tanHalfFovy);
    Result[2][2] = -(farclip + nearclip) / (farclip - nearclip);
    Result[2][3] = -1.f;
    Result[3][2] = -(2.f * farclip * nearclip) / (farclip - nearclip);
    return Result;
}

inline mat4 projection_matrix_orthographic_2d(float left, float right, float bottom, float top)
{
    mat4 ret = identity_mat4();
    ret[0][0] = 2.f / (right - left);
    ret[1][1] = 2.f / (top - bottom);
    ret[2][2] = -1.f;
    ret[3][0] = -(right + left) / (right - left);
    ret[3][1] = -(top + bottom) / (top - bottom);
    return ret;
}

inline mat4 projection_matrix_orthographic(float left, float right, float bottom, float top, float z_near, float z_far)
{
    mat4 ret = identity_mat4();
    ret[0][0] = 2.f / (right - left);
    ret[1][1] = 2.f / (top - bottom);
    ret[2][2] = - 2.f / (z_far - z_near);
    ret[3][0] = - (right + left) / (right - left);
    ret[3][1] = - (top + bottom) / (top - bottom);
    ret[3][2] = - (z_far + z_near) / (z_far - z_near);
    return ret;
}

inline mat4 view_matrix_look_at(vec3 const& eye, vec3 const& target, vec3 const& in_up)
{
    vec3 const direction = normalize(target - eye);
    vec3 const right = normalize(cross(direction, in_up));
    vec3 const up = cross(right, direction);

    mat4 ret = identity_mat4();

    ret[0][0] = right.x;
    ret[1][0] = right.y;
    ret[2][0] = right.z;

    ret[0][1] = up.x;
    ret[1][1] = up.y;
    ret[2][1] = up.z;

    ret[0][2] = -direction.x;
    ret[1][2] = -direction.y;
    ret[2][2] = -direction.z;

    ret[3][0] = -dot(right, eye);
    ret[3][1] = -dot(up, eye);
    ret[3][2] = dot(direction, eye);

    return ret;
}

// TODO transpose

/**

    Quaternion Operations

*/
inline quaternion add(quaternion a, quaternion b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    a.w += b.w;
    return a;
}

inline quaternion sub(quaternion a, quaternion b) 
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    a.w -= b.w;
    return a;
}

inline float dot(quaternion a, quaternion b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

inline quaternion mul(quaternion a, quaternion b)
{
    quaternion R;
    R.w = a.w * b.w - dot(make_vec3(a.x,a.y,a.z), make_vec3(b.x,b.y,b.z));
    vec3 va = make_vec3(a.x, a.y, a.z);
    vec3 vb = make_vec3(b.x, b.y, b.z);
    vec3 first = b.w * va;
    vec3 second = a.w * vb;
    vec3 third = cross(va, vb);
    R.x = first.x + second.x + third.x;
    R.y = first.y + second.y + third.y;
    R.z = first.z + second.z + third.z;
    return R;
}

inline quaternion mul(quaternion a, float scale)
{
    a.w *= scale;
    a.x *= scale;
    a.y *= scale;
    a.z *= scale;
    return a;
}

inline quaternion div(quaternion a, float scale) 
{
    float one_over_s = 1.0f / scale;
    a.w *= one_over_s;
    a.x *= one_over_s;
    a.y *= one_over_s;
    a.z *= one_over_s;
    return a;
}

inline quaternion operator*(quaternion a, quaternion b) { return(mul(a, b)); }

inline float magnitude(quaternion a)
{
    return(sqrtf(dot(a, a)));
}

inline quaternion normalize(quaternion a)
{
    return div(a, magnitude(a));
}

inline bool similar(quaternion a, quaternion b, float tolerance)
{
    return(abs(dot(a,b)-1.f) <= tolerance);
}

inline quaternion cumulate_rotations(quaternion first_rotation, quaternion second_rotation)
{
    return second_rotation * first_rotation; // order matters!
}

inline quaternion conjugate(quaternion a){
    quaternion ret;
    ret.w = a.w;
    ret.x = -a.x;
    ret.y = -a.y;
    ret.z = -a.z;
    return ret;
}

inline quaternion inverse(quaternion a)
{
    quaternion ret;
    ret.w = a.w;
    ret.x = -a.x;
    ret.y = -a.y;
    ret.z = -a.z;
    return ret;
}

inline quaternion inverse_unit(quaternion a)
{
    quaternion ret = div(conjugate(a), dot(a, a));
    return ret;
}

inline vec3 quat_to_euler(quaternion q)
{
    vec3 euler_angles;

    float x2 = q.x * q.x;
    float y2 = q.y * q.y;
    float z2 = q.z * q.z;
    float xy = q.x * q.y;
    float zw = q.z * q.w;
    float xz = q.x * q.z;
    float yw = q.y * q.w;
    float yz = q.y * q.z;
    float xw = q.x * q.w;

    float r10 = 2.0f * (xy + zw);
    float r20 = 2.0f * (xz - yw);
    float r00 = 1.0f - 2.0f * (y2 + z2);
    float r12 = 2.0f * (yz - xw);
    float r11 = 1.0f - 2.0f * (x2 + z2);
    float r21 = 2.0f * (yz + xw) ;
    float r22 = 1.0f - 2.0f * (x2 + y2);

    if(r10 < 1.f)
    {
        if(r10 > -1.f)
        {
            euler_angles.z = asinf(r10);
            euler_angles.y = atan2f(-r20, r00);
            euler_angles.x = atan2f(-r12, r11);
        }
        else // r10 = -1
        {
            euler_angles.z = -KC_PI/2.f;
            euler_angles.y = -atan2f(r21, r22);
            euler_angles.x = 0;
        }
    }
    else
    {
            euler_angles.z = KC_PI/2.f;
            euler_angles.y = atan2f(r21, r22);
            euler_angles.x = 0;
    }

    return euler_angles;
}

inline quaternion euler_to_quat(float roll, float pitch, float yaw)
{
    float cr = cosf(roll * 0.5f);
    float sr = sinf(roll * 0.5f);

    float cp = cosf(pitch * 0.5f);
    float sp = sinf(pitch * 0.5f);

    float cy = cosf(yaw * 0.5f);
    float sy = sinf(yaw * 0.5f);

    quaternion ret;

    ret.w = cr * cp * cy - sr * sp * sy;
    ret.x = sr * cp * cy + cr * sp * sy;
    ret.y = cr * cp * sy + sr * sp * cy;
    ret.z = cr * cy * sp - sr * sy * cp;

    return ret;
}

inline quaternion euler_to_quat(vec3 euler_angles)
{
    return euler_to_quat(euler_angles.x, euler_angles.z, euler_angles.y);
}

inline quaternion direction_to_orientation(vec3 direction)
{
    return rotation_from_to(WORLD_FORWARD_VECTOR, direction);
}

inline vec3 orientation_to_direction(quaternion orientation)
{
    return rotate_vector(WORLD_FORWARD_VECTOR, orientation);    
}

inline quaternion rotation_from_to(vec3 from_direction, vec3 to_direction)
{
    vec3 start = normalize(from_direction);
    vec3 dest = normalize(to_direction);

    float cos_theta = dot(start, dest);
    vec3 rotation_axis;
    quaternion rotation_quat;

    rotation_axis = cross(start, dest);
    if (cos_theta >= -1 + 0.0001f)
    {
        float s = sqrt((1 + cos_theta) * 2);
        float sin_of_half_angle = 1 / s;

        rotation_quat = make_quaternion(
            s * 0.5f, // recall cos(theta/2) trig identity
            rotation_axis.x * sin_of_half_angle,
            rotation_axis.y * sin_of_half_angle,
            rotation_axis.z * sin_of_half_angle
        );
    }
    else
    {
        // When vectors in opposite directions, there is no "ideal" rotation axis
        // So guess one; any will do as long as it's perpendicular to start
        rotation_axis = cross(make_vec3(0.0f, 0.0f, 1.0f), start);
        if (dot(rotation_axis, rotation_axis) < 0.01) // bad luck, they were parallel, try again!
            rotation_axis = cross(make_vec3(1.0f, 0.0f, 0.0f), start);
        rotation_quat = make_quaternion_rad(KC_PI, rotation_axis);
    }

    return rotation_quat;
}

inline quaternion rotation_difference(quaternion a, quaternion b)
{
    quaternion ret = mul(b, inverse_unit(a));
    return ret;
}

inline vec3 rotate_vector(vec3 vector, quaternion rotation)
{
    quaternion vector_quat = { 0.f, vector.x, vector.y, vector.z };
    quaternion rotated_vector = rotation * vector_quat * inverse(rotation);
    return make_vec3(rotated_vector.x, rotated_vector.y, rotated_vector.z);
}

inline mat3 quaternion_to_mat3(quaternion q)
{
    q = inverse_unit(q);

    mat3 ret;
    
    float x2 = q.x * q.x;
    float y2 = q.y * q.y;
    float z2 = q.z * q.z;
    
    float xy = q.x * q.y;
    float zw = q.z * q.w;
    float xz = q.x * q.z;
    float yw = q.y * q.w;
    float yz = q.y * q.z;
    float xw = q.x * q.w;
    
    ret.e[0][0] = 1.0f - 2.0f * (y2 + z2);
    ret.e[1][0] = 2.0f * (xy + zw);
    ret.e[2][0] = 2.0f * (xz - yw);
    
    ret.e[0][1] = 2.0f * (xy - zw);
    ret.e[1][1] = 1.0f - 2.0f * (x2 + z2);
    ret.e[2][1] = 2.0f * (yz + xw);
    
    ret.e[0][2] = 2.0f * (xz + yw);
    ret.e[1][2] = 2.0f * (yz - xw);
    ret.e[2][2] = 1.0f - 2.0f * (x2 + y2);
    
    return ret;
}

inline mat4 quaternion_to_mat4(quaternion q)
{
    return make_mat4(quaternion_to_mat3(q));
}

inline mat3 make_mat3(quaternion q)
{
    return quaternion_to_mat3(q);
}

inline mat4 make_mat4(quaternion q)
{
    return quaternion_to_mat4(q);
}

inline quaternion slerp(const quaternion from, const quaternion to, const float ratio)
{
    float t = clamp(ratio, 0.f, 1.f);
    quaternion start = normalize(from);
    quaternion end = normalize(to);
    quaternion d = end * inverse(from);
    float cos_theta = dot(start, end);
    float theta = acosf(cos_theta);
    theta *= t;
    quaternion d_raised_t = make_quaternion_rad(theta, make_vec3(d.x, d.y, d.z));
    return d_raised_t * start;
}

/**
    
    Other Operations

*/
inline float lerp(float from, float to, float ratio)
{
    return from + ratio * (to - from);
}

#endif // _INCLUDE_KC_MATH_H_