#define _USE_MATH_DEFINES // used for m_pi macro

//
// GLUT LIBRARY
//
#include "glut/glut.h"
#pragma comment(lib, "glut/glut32.lib")



//
// GLFW LIBRARY JOYSTICK / CONTROLLER INPUT
//
#include "glfw/glfw3.h"
#include "glfw/glfw3native.h"
#pragma comment(lib, "glfw/glfw3.lib")
#pragma comment(lib, "glfw/glfw3_mt.lib")
#pragma comment(lib, "glfw/glfw3dll.lib")



#include <stdio.h>
#include <cmath>

#include "macros.h"
#include "structs.h"
#include "loader.h"
#include "drawer.h"
#include "text.h"

#include "textures.h"



extern T_HALF_POLY terrainPolys[MAX_POLYS]; // from main.cpp
extern T_HALF_TEX  terrainTexs[MAX_POLYS]; // from main.cpp

extern st_model models[255]; // from main.cpp

extern short terrainHeights[MAX_VERTS]; // from main.cpp
extern unsigned char terrainClouds[MAX_VERTS]; // from main.cpp

extern int numTerrainTexs; // from main.cpp
extern int currentPoly; // from main.cpp
extern int currentRow; // from main.cpp
extern int currentColumn; // from main.cpp
extern int currentSquare; // from main.cpp
extern int selectedPoly; // from main.cpp

extern int brush_size; // from main.cpp

extern st_vec3f polyPoints[MAX_VERTS]; // from main.cpp

extern st_pal LBA2Palette; // from main.cpp

extern bool showHeights; // from main.cpp
extern bool showTextures; // from main.cpp
extern bool showShadows; // from main.cpp
extern bool showCollisions; // from main.cpp
extern bool showWireframe; // from main.cpp

extern bool mouse_held; // from main.cpp

extern float lerp(float a, float b, float f); // from main.cpp

void convert_3d_coord_to_2d(GLdouble model[16], GLdouble proj[16], GLint viewport[4], float winW, float winH, float aspect, st_vec3f* vec_3d, st_vec2f* vec_2d)
{
    double sx, sy, sz;

    gluProject(vec_3d->x, vec_3d->y, vec_3d->z, model, proj, viewport, &sx, &sy, &sz);

    // convert
    vec_2d->x = (sx / winW) * 2.0f - 1.0f;   // 0..w -> -1..1
    vec_2d->y = (sy / winH) * 2.0f - 1.0f;   // 0..h -> -1..1

    if (aspect >= 1.0f)
    {
        // wide -> X stretches
        vec_2d->x *= aspect;
    }
    else
    {
        // tall -> Y stretches
        vec_2d->y *= 1.0f / aspect;
    };
};

bool coord_2d_is_visible(st_vec2f p, float aspect)
{
    if (aspect >= 1.0f)
        return (p.x >= -aspect && p.x <= aspect &&
            p.y >= -1.0f && p.y <= 1.0f);
    else
        return (p.x >= -1.0f && p.x <= 1.0f &&
            p.y >= -1.0f / aspect && p.y <= 1.0f / aspect);
};

float cross(st_vec2f a, st_vec2f b, st_vec2f c)
{
    return (b.x - a.x) * (c.y - a.y) -
        (b.y - a.y) * (c.x - a.x);
};

bool pointInTriangle(st_vec2f p, st_vec2f a, st_vec2f b, st_vec2f c)
{
    bool b1 = cross(p, a, b) < 0.0f;
    bool b2 = cross(p, b, c) < 0.0f;
    bool b3 = cross(p, c, a) < 0.0f;

    return ((b1 == b2) && (b2 == b3));
};

bool pointInTriangle_2(st_vec2f p, st_vec2f a, st_vec2f b, st_vec2f c)
{
    // Compute vectors
    st_vec2f v0 = { c.x - a.x, c.y - a.y };
    st_vec2f v1 = { b.x - a.x, b.y - a.y };
    st_vec2f v2 = { p.x - a.x, p.y - a.y };

    // Dot products
    float dot00 = v0.x * v0.x + v0.y * v0.y;
    float dot01 = v0.x * v1.x + v0.y * v1.y;
    float dot02 = v0.x * v2.x + v0.y * v2.y;
    float dot11 = v1.x * v1.x + v1.y * v1.y;
    float dot12 = v1.x * v2.x + v1.y * v2.y;

    // Compute barycentric coordinates
    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    // Check if point is in triangle
    return (u >= 0) && (v >= 0) && (u + v <= 1);
};

st_vec3f cross_2(st_vec3f a, st_vec3f b)
{
    st_vec3f result;

    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;

    return result;
};

st_vec3f normalize(st_vec3f v)
{
    float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (length != 0.0f)
    {
        v.x /= length;
        v.y /= length;
        v.z /= length;
    };
    return v;
};

st_vec3f triangleNormal(st_vec3f A, st_vec3f B, st_vec3f C)
{
    st_vec3f U = { B.x - A.x, B.y - A.y, B.z - A.z };
    st_vec3f V = { C.x - A.x, C.y - A.y, C.z - A.z };

    st_vec3f N = cross_2(U, V);
    return normalize(N); // unit normal
};

void scale_poly_texture(T_HALF_TEX* poly)
{
    st_short2 center =
    {
        (poly[terrainPolys[selectedPoly].IndexTex].Tx0 +
         poly[terrainPolys[selectedPoly].IndexTex].Tx1 +
         poly[terrainPolys[selectedPoly].IndexTex].Tx2) / 3,
        (poly[terrainPolys[selectedPoly].IndexTex].Ty0 +
         poly[terrainPolys[selectedPoly].IndexTex].Ty1 +
         poly[terrainPolys[selectedPoly].IndexTex].Ty2) / 3,
    };

    st_short2 p0 =
    {
        poly[terrainPolys[selectedPoly].IndexTex].Tx0,
        poly[terrainPolys[selectedPoly].IndexTex].Ty0,
    };

    st_short2 p1 =
    {
        poly[terrainPolys[selectedPoly].IndexTex].Tx1,
        poly[terrainPolys[selectedPoly].IndexTex].Ty1,
    };

    st_short2 p2 =
    {
        poly[terrainPolys[selectedPoly].IndexTex].Tx2,
        poly[terrainPolys[selectedPoly].IndexTex].Ty2,
    };

    // apply new scaling to triangle
    poly[terrainPolys[selectedPoly].IndexTex].Tx0 = (center.x + (p0.x - center.x) * 0.95);
    poly[terrainPolys[selectedPoly].IndexTex].Ty0 = (center.y + (p0.y - center.y) * 0.95);

    poly[terrainPolys[selectedPoly].IndexTex].Tx1 = (center.x + (p1.x - center.x) * 0.95);
    poly[terrainPolys[selectedPoly].IndexTex].Ty1 = (center.y + (p1.y - center.y) * 0.95);

    poly[terrainPolys[selectedPoly].IndexTex].Tx2 = (center.x + (p2.x - center.x) * 0.95);
    poly[terrainPolys[selectedPoly].IndexTex].Ty2 = (center.y + (p2.y - center.y) * 0.95);
};

float dot(st_vec3f a, st_vec3f b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
};

float distance3D(float x1, float y1, float z1, float x2, float y2, float z2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;

    return sqrt(dx * dx + dy * dy + dz * dz);
};

void interpolatePoints(st_vec2f a, st_vec2f b, st_vec2f* out, int count)
{
    for (int i = 1; i <= count; i++)
    {
        float t = (float)i / (count + 1);

        out[i - 1].x = a.x + t * (b.x - a.x);
        out[i - 1].y = a.y + t * (b.y - a.y);
    };
};

void interpolatePoints_short(st_short2 a, st_short2 b, st_short2* out, int count)
{
    for (int i = 1; i <= count; i++)
    {
        int32_t t_num = i;
        int32_t t_den = count + 1;

        out[i - 1].x = (int16_t)(
            a.x + ((int32_t)(b.x - a.x) * t_num) / t_den
            );

        out[i - 1].y = (int16_t)(
            a.y + ((int32_t)(b.y - a.y) * t_num) / t_den
            );
    };
};

unsigned char getSmallestInArray(st_float_data values[], int length)
{
    float temp = values[0].value;
    unsigned char tempIndex = values[0].index;

    for (int i = 0; i < length; i++)
    {
        if (temp > values[i].value)
        {
            temp = values[i].value;
            tempIndex = values[i].index;
        };
    };

    return tempIndex;
};

int int_min(int a, int b)
{
    return (a < b) ? a : b;
};

int int_max(int a, int b)
{
    return (a > b) ? a : b;
};

int int_clamp(int value, int min_val, int max_val)
{
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
};

int snap_down(int value, int grid)
{
    return (value >= 0)
        ? (value / grid) * grid
        : ((value - (grid - 1)) / grid) * grid;
};

int snap_up(int value, int grid)
{
    return (value >= 0)
        ? ((value + (grid - 1)) / grid) * grid
        : (value / grid) * grid;
};

st_vec3f vertex_rotate_x(st_vec3f v, float theta)
{
    st_vec3f r;
    float cosT = cosf(theta);
    float sinT = sinf(theta);

    r.x = v.x;
    r.y = v.y * cosT - v.z * sinT;
    r.z = v.y * sinT + v.z * cosT;

    return r;
};

st_vec3f vertex_rotate_y(st_vec3f v, float theta)
{
    st_vec3f r;
    float cosT = cosf(theta);
    float sinT = sinf(theta);

    r.x = v.x * cosT + v.z * sinT;
    r.y = v.y;
    r.z = -v.x * sinT + v.z * cosT;

    return r;
};

st_vec3f vertex_rotate_z(st_vec3f v, float theta)
{
    st_vec3f r;
    float cosT = cosf(theta);
    float sinT = sinf(theta);

    r.x = v.x * cosT + v.y * sinT;
    r.y = v.x * sinT + v.y * cosT;
    r.z = v.z;

    return r;
};

void vertex_rotate_2d(st_vec2f* p, st_vec2f center, float angle)
{
    float s = sin(angle);
    float c = cos(angle);

    // Translate point to origin
    float x = p->x - center.x;
    float y = p->y - center.y;

    // Rotate
    float xnew = x * c - y * s;
    float ynew = x * s + y * c;

    // Translate back
    p->x = xnew + center.x;
    p->y = ynew + center.y;
};

void vertex_rotate_2d_short(st_short2* p, st_short2 center, float angle)
{
    float s = sin(angle);
    float c = cos(angle);

    // Translate to origin (use double for precision)
    float x = (float)p->x - center.x;
    float y = (float)p->y - center.y;

    // Rotate
    float xnew = x * c - y * s;
    float ynew = x * s + y * c;

    // Translate back and round to nearest short
    p->x = (float)lround(xnew + center.x);
    p->y = (float)lround(ynew + center.y);
};

void init_window(st_window* window, float x, float y, float size_x, float size_y, const char* text)
{
    window->p0.x = x;
    window->p0.y = y;

    window->p1.x = (x + size_x);
    window->p1.y = (y + size_y);

    window->text = text;
};

void draw_window(st_window* window)
{
    // draw back of button
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);

    glColor4f(0, 0, 0, 0.5f);
    glVertex3f(window->p0.x, window->p0.y, 0);

    glColor4f(0, 0, 0, 0.5f);
    glVertex3f(window->p1.x, window->p0.y, 0);

    glColor4f(0, 0, 0, 0.5f);
    glVertex3f(window->p1.x, window->p1.y, 0);

    glColor4f(0, 0, 0, 0.5f);
    glVertex3f(window->p0.x, window->p1.y, 0);

    glEnd();



    glColor3f(1, 1, 1);
    drawText(window->p0.x + 0.01f, window->p0.y + 0.05f, 0.02f, window->text);



    if (window->hover == true) // draw white outlines
    {
        // top line
        glBegin(GL_LINES);

        glColor3f(1, 1, 1);
        glVertex3f(window->p0.x, window->p0.y, 0);

        glColor3f(1, 1, 1);
        glVertex3f(window->p1.x, window->p0.y, 0);

        glEnd();

        // bottom line
        glBegin(GL_LINES);

        glColor3f(1, 1, 1);
        glVertex3f(window->p0.x, window->p1.y, 0);

        glColor3f(1, 1, 1);
        glVertex3f(window->p1.x, window->p1.y, 0);

        glEnd();

        // left line
        glBegin(GL_LINES);

        glColor3f(1, 1, 1);
        glVertex3f(window->p0.x, window->p0.y, 0);

        glColor3f(1, 1, 1);
        glVertex3f(window->p0.x, window->p1.y, 0);

        glEnd();

        // right line
        glBegin(GL_LINES);

        glColor3f(1, 1, 1);
        glVertex3f(window->p1.x, window->p0.y, 0);

        glColor3f(1, 1, 1);
        glVertex3f(window->p1.x, window->p1.y, 0);

        glEnd();
    }
    else // draw red outlines
    {
        // top line
        glBegin(GL_LINES);

        glColor3f(1, 0, 0);
        glVertex3f(window->p0.x, window->p0.y, 0);

        glColor3f(1, 0, 0);
        glVertex3f(window->p1.x, window->p0.y, 0);

        glEnd();

        // bottom line
        glBegin(GL_LINES);

        glColor3f(1, 0, 0);
        glVertex3f(window->p0.x, window->p1.y, 0);

        glColor3f(1, 0, 0);
        glVertex3f(window->p1.x, window->p1.y, 0);

        glEnd();

        // left line
        glBegin(GL_LINES);

        glColor3f(1, 0, 0);
        glVertex3f(window->p0.x, window->p0.y, 0);

        glColor3f(1, 0, 0);
        glVertex3f(window->p0.x, window->p1.y, 0);

        glEnd();

        // right line
        glBegin(GL_LINES);

        glColor3f(1, 0, 0);
        glVertex3f(window->p1.x, window->p0.y, 0);

        glColor3f(1, 0, 0);
        glVertex3f(window->p1.x, window->p1.y, 0);

        glEnd();
    };

    glDisable(GL_BLEND);
};

bool mouse_in_window(st_window* window, float x, float y)
{
    if (x >= window->p0.x && x <= window->p1.x &&
        y >= window->p0.y && y <= window->p1.y)
    {
        window->hover = true;
        return true;
    };

    window->hover = false;
    return false;
};

void init_button(st_button* button, float x, float y, float size_x, float size_y, const char* text)
{
    button->p0.x = x;
    button->p0.y = y;

    button->p1.x = (x + size_x);
    button->p1.y = (y + size_y);

    button->text = text;

    button->is_focused = false;

    button->is_visible = true;

    button->window = NULL;
};

void clamp_button_old(st_button* button, float screen_w, float screen_h, int clamp_type)
{
        float aspect = (float)screen_w / (float)screen_h;

        float left, right, top, bottom;

        if (aspect >= 1.0f)
        {
            left = -aspect;
            right = aspect;
            top = 1.0f;
            bottom = -1.0f;
        }
        else
        {
            left = -1.0f;
            right = 1.0f;
            top = 1.0f / aspect;
            bottom = -1.0f / aspect;
        };

        // button width and height
        float bw = (button->p1.x - button->p0.x);
        float bh = (button->p1.y - button->p0.y);

        switch (clamp_type)
        {
        case CLAMP_TOP_LEFT:
            //button->p0.x = left;
            //button->p1.x = left + bw;
            //button->p1.y = top;
            //button->p0.y = top - bh;

            // give cushion of space from corner
            button->p0.x = (left) + 0.1f;
            button->p1.x = (left + bw) + 0.1f;
            button->p1.y = (top) - 0.1f;
            button->p0.y = (top - bh) - 0.1f;
            break;

        case CLAMP_TOP_RIGHT:
            //button->p1.x = right;
            //button->p0.x = right - bw;
            //button->p1.y = top;
            //button->p0.y = top - bh;

            // give cushion of space from corner
            button->p1.x = (right) - 0.1f;
            button->p0.x = (right - bw) - 0.1f;
            button->p1.y = (top) - 0.1f;
            button->p0.y = (top - bh) - 0.1f;
            break;

        case CLAMP_BOTTOM_LEFT:
            //button->p0.x = left;
            //button->p1.x = left + bw;
            //button->p0.y = bottom;
            //button->p1.y = bottom + bh;

            // give cushion of space from corner
            button->p0.x = (left) + 0.1f;
            button->p1.x = (left + bw) + 0.1f;
            button->p0.y = (bottom) + 0.1f;
            button->p1.y = (bottom + bh) + 0.1f;
            break;

        case CLAMP_BOTTOM_RIGHT:
            //button->p1.x = right;
            //button->p0.x = right - bw;
            //button->p0.y = bottom;
            //button->p1.y = bottom + bh;

            // give cushion of space from corner
            button->p0.x = (right) - 0.1f;
            button->p1.x = (right - bw) - 0.1f;
            button->p0.y = (bottom) + 0.1f;
            button->p1.y = (bottom + bh) + 0.1f;
            break;

        case CLAMP_LEFT:
            //float midY_left;
            //midY_left = (top + bottom) * 0.5f;
            //button->p0.x = (left);
            //button->p1.x = (left + bw);
            //button->p0.y = (midY_left - bh) * 0.5f;
            //button->p1.y = (midY_left + bh) * 0.5f;

            // clamps to the left without moving on Y-axis
            float midY_left;
            midY_left = (top + bottom) * 0.5f;
            button->p0.x = (left);
            button->p1.x = (left + bw);
            break;

        case CLAMP_RIGHT:
            //float midY_right;
            //midY_right = (top + bottom) * 0.5f;
            //button->p1.x = (right);
            //button->p0.x = (right - bw);
            //button->p0.y = (midY_right - bh) * 0.5f;
            //button->p1.y = (midY_right + bh) * 0.5f;

            // clamps to the right without moving on Y-axis
            float midY_right;
            midY_right = (top + bottom) * 0.5f;
            button->p1.x = (right);
            button->p0.x = (right - bw);
            break;

        case CLAMP_TOP:
            float mid_top;
            mid_top = (left + right) * 0.5f;
            button->p0.x = (mid_top - bw) * 0.5f;
            button->p1.x = (mid_top + bw) * 0.5f;
            button->p1.y = (top);
            button->p0.y = (top - bh);
            break;

        case CLAMP_BOTTOM:
            float mid_bottom;
            mid_bottom = (left + right) * 0.5f;
            button->p0.x = (mid_bottom - bw) * 0.5f;
            button->p1.x = (mid_bottom + bw) * 0.5f;
            button->p0.y = (bottom);
            button->p1.y = (bottom + bh);
            break;
        };
};

void clamp_button(st_button* button, float screen_w, float screen_h, int clamp_type, float offset_x, float offset_y)
{
    float aspect = screen_w / screen_h;

    float left, right, top, bottom;

    if (aspect >= 1.0f)
    {
        left = -aspect;
        right = aspect;
        top = 1.0f;
        bottom = -1.0f;
    }
    else
    {
        left = -1.0f;
        right = 1.0f;
        top = 1.0f / aspect;
        bottom = -1.0f / aspect;
    }

    float bw = button->p1.x - button->p0.x;
    float bh = button->p1.y - button->p0.y;

    float midX = (left + right) * 0.5f;
    float midY = (top + bottom) * 0.5f;

    switch (clamp_type)
    {
    case CLAMP_TOP_LEFT:
        button->p0.x = left + offset_x;
        button->p1.x = left + offset_x + bw;
        button->p1.y = top - offset_y;
        button->p0.y = top - offset_y - bh;
        break;

    case CLAMP_TOP_RIGHT:
        button->p1.x = right - offset_x;
        button->p0.x = right - offset_x - bw;
        button->p1.y = top - offset_y;
        button->p0.y = top - offset_y - bh;
        break;

    case CLAMP_BOTTOM_LEFT:
        button->p0.x = left + offset_x;
        button->p1.x = left + offset_x + bw;
        button->p0.y = bottom + offset_y;
        button->p1.y = bottom + offset_y + bh;
        break;

    case CLAMP_BOTTOM_RIGHT:
        button->p1.x = right - offset_x;
        button->p0.x = right - offset_x - bw;
        button->p0.y = bottom + offset_y;
        button->p1.y = bottom + offset_y + bh;
        break;

    case CLAMP_LEFT:
        button->p0.x = left + offset_x;
        button->p1.x = left + offset_x + bw;
        button->p0.y = midY - bh * 0.5f;
        button->p1.y = midY + bh * 0.5f;
        break;

    case CLAMP_RIGHT:
        button->p1.x = right - offset_x;
        button->p0.x = right - offset_x - bw;

        button->p1.y = top - offset_y;
        button->p0.y = top - offset_y - bh;
        break;

    case CLAMP_TOP:
        button->p0.x = midX - bw * 0.5f;
        button->p1.x = midX + bw * 0.5f;
        button->p1.y = top - offset_y;
        button->p0.y = top - offset_y - bh;
        break;

    case CLAMP_BOTTOM:
        button->p0.x = midX - bw * 0.5f;
        button->p1.x = midX + bw * 0.5f;
        button->p0.y = bottom + offset_y;
        button->p1.y = bottom + offset_y + bh;
        break;
    };
};

void draw_button(st_button* button)
{
    glDisable(GL_TEXTURE_2D);

    // draw back of button
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);

    glColor4f(0, 0, 0, 0.5f);
    glVertex3f(button->p0.x, button->p0.y, 0);

    glColor4f(0, 0, 0, 0.5f);
    glVertex3f(button->p1.x, button->p0.y, 0);

    glColor4f(0, 0, 0, 0.5f);
    glVertex3f(button->p1.x, button->p1.y, 0);

    glColor4f(0, 0, 0, 0.5f);
    glVertex3f(button->p0.x, button->p1.y, 0);

    glEnd();



    glDisable(GL_BLEND);

    // draw text of button
    glColor3f(1, 1, 1);
    if (button->is_text_field == false)
        drawText(button->p0.x + 0.01f, button->p0.y + 0.05f, 0.02f, button->text);
    else
        drawText(button->p0.x + 0.05f, button->p0.y + 0.05f, 0.02f, (const char*)button->text_field_buffer);



	if (button->hover == true) // draw white outlines
	{
		// top line
		glBegin(GL_LINES);

		glColor3f(1, 1, 1);
		glVertex3f(button->p0.x, button->p0.y, 0);

		glColor3f(1, 1, 1);
		glVertex3f(button->p1.x, button->p0.y, 0);

		glEnd();

		// bottom line
		glBegin(GL_LINES);

		glColor3f(1, 1, 1);
		glVertex3f(button->p0.x, button->p1.y, 0);

		glColor3f(1, 1, 1);
		glVertex3f(button->p1.x, button->p1.y, 0);

		glEnd();

		// left line
		glBegin(GL_LINES);

		glColor3f(1, 1, 1);
		glVertex3f(button->p0.x, button->p0.y, 0);

		glColor3f(1, 1, 1);
		glVertex3f(button->p0.x, button->p1.y, 0);

		glEnd();

		// right line
		glBegin(GL_LINES);

		glColor3f(1, 1, 1);
		glVertex3f(button->p1.x, button->p0.y, 0);

		glColor3f(1, 1, 1);
		glVertex3f(button->p1.x, button->p1.y, 0);

		glEnd();
	}
    else // draw red outlines
    {
        // top line
        glBegin(GL_LINES);

        glColor3f(1, 0, 0);
        glVertex3f(button->p0.x, button->p0.y, 0);

        glColor3f(1, 0, 0);
        glVertex3f(button->p1.x, button->p0.y, 0);

        glEnd();

        // bottom line
        glBegin(GL_LINES);

        glColor3f(1, 0, 0);
        glVertex3f(button->p0.x, button->p1.y, 0);

        glColor3f(1, 0, 0);
        glVertex3f(button->p1.x, button->p1.y, 0);

        glEnd();

        // left line
        glBegin(GL_LINES);

        glColor3f(1, 0, 0);
        glVertex3f(button->p0.x, button->p0.y, 0);

        glColor3f(1, 0, 0);
        glVertex3f(button->p0.x, button->p1.y, 0);

        glEnd();

        // right line
        glBegin(GL_LINES);

        glColor3f(1, 0, 0);
        glVertex3f(button->p1.x, button->p0.y, 0);

        glColor3f(1, 0, 0);
        glVertex3f(button->p1.x, button->p1.y, 0);

        glEnd();
    };

    if (button->is_focused == true) // draw green outlines
    {
        // top line
        glBegin(GL_LINES);

        glColor3f(0, 1, 0);
        glVertex3f(button->p0.x, button->p0.y, 0);

        glColor3f(0, 1, 0);
        glVertex3f(button->p1.x, button->p0.y, 0);

        glEnd();

        // bottom line
        glBegin(GL_LINES);

        glColor3f(0, 1, 0);
        glVertex3f(button->p0.x, button->p1.y, 0);

        glColor3f(0, 1, 0);
        glVertex3f(button->p1.x, button->p1.y, 0);

        glEnd();

        // left line
        glBegin(GL_LINES);

        glColor3f(0, 1, 0);
        glVertex3f(button->p0.x, button->p0.y, 0);

        glColor3f(0, 1, 0);
        glVertex3f(button->p0.x, button->p1.y, 0);

        glEnd();

        // right line
        glBegin(GL_LINES);

        glColor3f(0, 1, 0);
        glVertex3f(button->p1.x, button->p0.y, 0);

        glColor3f(0, 1, 0);
        glVertex3f(button->p1.x, button->p1.y, 0);

        glEnd();
    };

    glEnable(GL_TEXTURE_2D);

    glDisable(GL_BLEND);
};

void draw_button_color(st_button* button, st_color3* color)
{
    // draw back of button
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);

    // cast the colors from unsigned char to float
    glColor3f((float)color->r / 255, (float)color->g / 255, (float)color->b / 255);
    glVertex3f(button->p0.x, button->p0.y, 0);

    glColor3f((float)color->r / 255, (float)color->g / 255, (float)color->b / 255);
    glVertex3f(button->p1.x, button->p0.y, 0);

    glColor3f((float)color->r / 255, (float)color->g / 255, (float)color->b / 255);
    glVertex3f(button->p1.x, button->p1.y, 0);

    glColor3f((float)color->r / 255, (float)color->g / 255, (float)color->b / 255);
    glVertex3f(button->p0.x, button->p1.y, 0);

    glEnd();



    glDisable(GL_BLEND);

    // draw text of button
    glColor3f(1, 1, 1);
    if (button->is_text_field == false)
        drawText(button->p0.x + 0.05f, button->p0.y + 0.05f, 0.02f, button->text);
    else
        drawText(button->p0.x + 0.05f, button->p0.y + 0.05f, 0.02f, (const char*)button->text_field_buffer);

    // clamp button to window
    if (button->window != NULL)
    {
        // needs work - very bad
        button->p0.x = button->window->p0.x;
        button->p1.x = button->window->p0.x + 0.5f;
    };

    if (button->hover == true) // draw white outlines
    {
        // top line
        glBegin(GL_LINES);

        glColor3f(1, 1, 1);
        glVertex3f(button->p0.x, button->p0.y, 0);

        glColor3f(1, 1, 1);
        glVertex3f(button->p1.x, button->p0.y, 0);

        glEnd();

        // bottom line
        glBegin(GL_LINES);

        glColor3f(1, 1, 1);
        glVertex3f(button->p0.x, button->p1.y, 0);

        glColor3f(1, 1, 1);
        glVertex3f(button->p1.x, button->p1.y, 0);

        glEnd();

        // left line
        glBegin(GL_LINES);

        glColor3f(1, 1, 1);
        glVertex3f(button->p0.x, button->p0.y, 0);

        glColor3f(1, 1, 1);
        glVertex3f(button->p0.x, button->p1.y, 0);

        glEnd();

        // right line
        glBegin(GL_LINES);

        glColor3f(1, 1, 1);
        glVertex3f(button->p1.x, button->p0.y, 0);

        glColor3f(1, 1, 1);
        glVertex3f(button->p1.x, button->p1.y, 0);

        glEnd();
    }
    else // draw red outlines
    {
        // top line
        glBegin(GL_LINES);

        glColor3f(1, 0, 0);
        glVertex3f(button->p0.x, button->p0.y, 0);

        glColor3f(1, 0, 0);
        glVertex3f(button->p1.x, button->p0.y, 0);

        glEnd();

        // bottom line
        glBegin(GL_LINES);

        glColor3f(1, 0, 0);
        glVertex3f(button->p0.x, button->p1.y, 0);

        glColor3f(1, 0, 0);
        glVertex3f(button->p1.x, button->p1.y, 0);

        glEnd();

        // left line
        glBegin(GL_LINES);

        glColor3f(1, 0, 0);
        glVertex3f(button->p0.x, button->p0.y, 0);

        glColor3f(1, 0, 0);
        glVertex3f(button->p0.x, button->p1.y, 0);

        glEnd();

        // right line
        glBegin(GL_LINES);

        glColor3f(1, 0, 0);
        glVertex3f(button->p1.x, button->p0.y, 0);

        glColor3f(1, 0, 0);
        glVertex3f(button->p1.x, button->p1.y, 0);

        glEnd();
    };

    if (button->is_focused == true) // draw green outlines
    {
        // top line
        glBegin(GL_LINES);

        glColor3f(0, 1, 0);
        glVertex3f(button->p0.x, button->p0.y, 0);

        glColor3f(0, 1, 0);
        glVertex3f(button->p1.x, button->p0.y, 0);

        glEnd();

        // bottom line
        glBegin(GL_LINES);

        glColor3f(0, 1, 0);
        glVertex3f(button->p0.x, button->p1.y, 0);

        glColor3f(0, 1, 0);
        glVertex3f(button->p1.x, button->p1.y, 0);

        glEnd();

        // left line
        glBegin(GL_LINES);

        glColor3f(0, 1, 0);
        glVertex3f(button->p0.x, button->p0.y, 0);

        glColor3f(0, 1, 0);
        glVertex3f(button->p0.x, button->p1.y, 0);

        glEnd();

        // right line
        glBegin(GL_LINES);

        glColor3f(0, 1, 0);
        glVertex3f(button->p1.x, button->p0.y, 0);

        glColor3f(0, 1, 0);
        glVertex3f(button->p1.x, button->p1.y, 0);

        glEnd();
    };

    glDisable(GL_BLEND);
};

void draw_handle(st_handle* handle, float scale)
{
    glDisable(GL_TEXTURE_2D);

	glPushMatrix();

    if (handle->hover == true)
    {
        // top
        glBegin(GL_LINES);
        glColor3f(1, 0, 0);
        glVertex3f((handle->p.x - scale), (handle->p.y - scale), 0.0f);
        glVertex3f((handle->p.x + scale), (handle->p.y - scale), 0.0f);
        glEnd();

        // bottom
        glBegin(GL_LINES);
        glColor3f(1, 0, 0);
        glVertex3f((handle->p.x - scale), (handle->p.y + scale), 0.0f);
        glVertex3f((handle->p.x + scale), (handle->p.y + scale), 0.0f);
        glEnd();

        // left
        glBegin(GL_LINES);
        glColor3f(1, 0, 0);
        glVertex3f((handle->p.x - scale), (handle->p.y - scale), 0.0f);
        glVertex3f((handle->p.x - scale), (handle->p.y + scale), 0.0f);
        glEnd();

        // right
        glBegin(GL_LINES);
        glColor3f(1, 0, 0);
        glVertex3f((handle->p.x + scale), (handle->p.y - scale), 0.0f);
        glVertex3f((handle->p.x + scale), (handle->p.y + scale), 0.0f);
        glEnd();
    }
    else
    {
        // top
        glBegin(GL_LINES);
        glColor3f(1, 1, 1);
        glVertex3f((handle->p.x - scale), (handle->p.y - scale), 0.0f);
        glVertex3f((handle->p.x + scale), (handle->p.y - scale), 0.0f);
        glEnd();

        // bottom
        glBegin(GL_LINES);
        glColor3f(1, 1, 1);
        glVertex3f((handle->p.x - scale), (handle->p.y + scale), 0.0f);
        glVertex3f((handle->p.x + scale), (handle->p.y + scale), 0.0f);
        glEnd();

        // left
        glBegin(GL_LINES);
        glColor3f(1, 1, 1);
        glVertex3f((handle->p.x - scale), (handle->p.y - scale), 0.0f);
        glVertex3f((handle->p.x - scale), (handle->p.y + scale), 0.0f);
        glEnd();

        // right
        glBegin(GL_LINES);
        glColor3f(1, 1, 1);
        glVertex3f((handle->p.x + scale), (handle->p.y - scale), 0.0f);
        glVertex3f((handle->p.x + scale), (handle->p.y + scale), 0.0f);
        glEnd();
    };

    if (handle->selected == true)
    {
        // top
        glBegin(GL_LINES);
        glColor3f(0, 1, 0);
        glVertex3f((handle->p.x - scale), (handle->p.y - scale), 0.0f);
        glVertex3f((handle->p.x + scale), (handle->p.y - scale), 0.0f);
        glEnd();

        // bottom
        glBegin(GL_LINES);
        glColor3f(0, 1, 0);
        glVertex3f((handle->p.x - scale), (handle->p.y + scale), 0.0f);
        glVertex3f((handle->p.x + scale), (handle->p.y + scale), 0.0f);
        glEnd();

        // left
        glBegin(GL_LINES);
        glColor3f(0, 1, 0);
        glVertex3f((handle->p.x - scale), (handle->p.y - scale), 0.0f);
        glVertex3f((handle->p.x - scale), (handle->p.y + scale), 0.0f);
        glEnd();

        // right
        glBegin(GL_LINES);
        glColor3f(0, 1, 0);
        glVertex3f((handle->p.x + scale), (handle->p.y - scale), 0.0f);
        glVertex3f((handle->p.x + scale), (handle->p.y + scale), 0.0f);
        glEnd();
    };

	glPopMatrix();

    glEnable(GL_TEXTURE_2D);
};

/*int isAllDigits(const char* s)
{
    if (*s == '\0') return 0;  // empty string is not valid

    for (int i = 0; s[i] != '\0'; i++) {
        if (s[i] < '0' || s[i] > '9') {
            return 0; // found a non-digit
        }
    }
    return 1; // all characters are digits
};*/

// same as above but checks for negative values
int isAllDigits(const char* s)
{
    if (s == NULL || *s == '\0')
        return 0;  // empty or NULL string is not valid

    int i = 0;

    // Allow optional leading minus sign
    if (s[i] == '-') {
        i++;
        if (s[i] == '\0')
            return 0;  // "-" alone is not a valid number
    }

    // Check remaining characters are digits
    for (; s[i] != '\0'; i++) {
        if (s[i] < '0' || s[i] > '9') {
            return 0;
        }
    }

    return 1;
};

void text_field_handle_backspace(st_button* button)
{
    if (button->current_char_in_field > 0)
        button->current_char_in_field -= 1;

    button->text_field_buffer[button->current_char_in_field] = '\0';
};

void text_field_handle_enter(st_button* button, int void_types, void* data)
{
    //int* int_ptr;

    switch (void_types)
    {
    case VOID_TYPE_INT:
        if (isAllDigits((const char*)button->text_field_buffer))
        {
            //int* int_ptr = (int*)data;


            // get the string from the text field and convert it to an integer
            //int value = (int)strtol((char*)button->text_field_buffer, NULL, 10);
            //data = (int)strtol((char*)button->text_field_buffer, NULL, 10);
            //int_ptr = (int*)strtol((char*)button->text_field_buffer, NULL, 10);

            //printf("entered data: %d \n", *data);

            //*(int*)data = value;

            //display_text_error = false;
        }
        else
        {
            //display_text_error = true;
        };

        break;
    case VOID_TYPE_STRING:
        const char* ptr = (const char*)data;
        break;
    };
};

void text_field_handle_any_key(st_button* button, int key)
{
    if (button->current_char_in_field < 10)
    {
        button->text_field_buffer[button->current_char_in_field] = key;
        button->current_char_in_field += 1;
    };
};

bool mouse_in_button(st_button* button, float x, float y)
{
    if (button->is_visible == true)
    {
        if (x >= button->p0.x && x <= button->p1.x &&
            y >= button->p0.y && y <= button->p1.y)
        {
            button->hover = true;
            return true;
        };
    };
    
    button->hover = false;
    return false;
};

bool mouse_in_handle(st_handle* handle, float x, float y, float scale)
{
    if (x >= (handle->p.x - scale) && x <= (handle->p.x + scale) &&
        y >= (handle->p.y - scale) && y <= (handle->p.y + scale))
    {
        handle->hover = true;
        return true;
    };

    handle->hover = false;
    return false;
};

bool mouse_in_rect(st_vec2f p0, st_vec2f p1, float x, float y)
{
    if (x >= p0.x && x <= p1.x &&
        y >= p0.y && y <= p1.y)
    {
        return true;
    };

    return false;
};

bool point_in_rect_both_checks(st_vec2f cursor, st_vec2f p0, st_vec2f p1)
{
    float min_x = p0.x < p1.x ? p0.x : p1.x;
    float max_x = p0.x > p1.x ? p0.x : p1.x;
    float min_y = p0.y < p1.y ? p0.y : p1.y;
    float max_y = p0.y > p1.y ? p0.y : p1.y;

    return cursor.x >= min_x && cursor.x <= max_x &&
        cursor.y >= min_y && cursor.y <= max_y;
};

void draw_building(st_building* building)
{
    float building_scale = 0.00195f;

    glPushMatrix();

    glRotatef(90, 1, 0, 0);
    //glTranslatef(0, -64, 0);

    glBegin(GL_LINES);

    glColor3f(1, 1, 1);
    glVertex3f(building->x * building_scale, building->y * building_scale, building->z * building_scale); // Flip Y and Z.

    glColor3f(1, 1, 1);
    glVertex3f(building->x * building_scale, building->y * building_scale + 2, building->z * building_scale); // Flip Y and Z.
    glEnd();



    // Bottom
    glBegin(GL_LINES);

    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(building->colRight * building_scale, building->colBottom * building_scale, building->colUp * building_scale); // Flip Y and Z.

    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(building->colRight * building_scale, building->colBottom * building_scale, building->colDown * building_scale); // Flip Y and Z.

    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(building->colLeft * building_scale, building->colBottom * building_scale, building->colUp * building_scale); // Flip Y and Z.

    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(building->colLeft * building_scale, building->colBottom * building_scale, building->colDown * building_scale); // Flip Y and Z.



    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(building->colLeft * building_scale, building->colBottom * building_scale, building->colUp * building_scale); // Flip Y and Z.

    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(building->colRight * building_scale, building->colBottom * building_scale, building->colUp * building_scale); // Flip Y and Z.

    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(building->colLeft * building_scale, building->colBottom * building_scale, building->colDown * building_scale); // Flip Y and Z.

    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(building->colRight * building_scale, building->colBottom * building_scale, building->colDown * building_scale); // Flip Y and Z.



    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(building->colLeft * building_scale, building->colTop * building_scale, building->colUp * building_scale); // Flip Y and Z.

    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(building->colRight * building_scale, building->colTop * building_scale, building->colUp * building_scale); // Flip Y and Z.

    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(building->colLeft * building_scale, building->colTop * building_scale, building->colDown * building_scale); // Flip Y and Z.

    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(building->colRight * building_scale, building->colTop * building_scale, building->colDown * building_scale); // Flip Y and Z.



    // Bottom to Top
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(building->colLeft * building_scale, building->colBottom * building_scale, building->colDown * building_scale); // Flip Y and Z.

    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(building->colLeft * building_scale, building->colTop * building_scale, building->colDown * building_scale); // Flip Y and Z.

    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(building->colRight * building_scale, building->colBottom * building_scale, building->colDown * building_scale); // Flip Y and Z.

    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(building->colRight * building_scale, building->colTop * building_scale, building->colDown * building_scale); // Flip Y and Z.



    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(building->colRight * building_scale, building->colBottom * building_scale, building->colUp * building_scale); // Flip Y and Z.

    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(building->colRight * building_scale, building->colTop * building_scale, building->colUp * building_scale); // Flip Y and Z.

    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(building->colLeft * building_scale, building->colBottom * building_scale, building->colUp * building_scale); // Flip Y and Z.

    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(building->colLeft * building_scale, building->colTop * building_scale, building->colUp * building_scale); // Flip Y and Z.



    // Top
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(building->colRight * building_scale, building->colTop * building_scale, building->colUp * building_scale); // Flip Y and Z.

    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(building->colRight * building_scale, building->colTop * building_scale, building->colDown * building_scale); // Flip Y and Z.

    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(building->colLeft * building_scale, building->colTop * building_scale, building->colUp * building_scale); // Flip Y and Z.

    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(building->colLeft * building_scale, building->colTop * building_scale, building->colDown * building_scale); // Flip Y and Z.

    glEnd();

    glPopMatrix();
};

void draw_axis(st_vec3f vec, unsigned char type)
{
    glPushMatrix();

    switch (type)
    {
    case AXIS_X:
        glBegin(GL_LINES);
        glColor3f(1, 0, 0);
        glVertex3f(vec.x + 50, vec.y, vec.z);
        glColor3f(1, 0, 0);
        glVertex3f(vec.x - 50, vec.y, vec.z);
        glEnd();
        break;

    case AXIS_Y:
        glBegin(GL_LINES);
        glColor3f(0, 1, 0);
        glVertex3f(vec.x, vec.y + 50, vec.z);
        glColor3f(0, 1, 0);
        glVertex3f(vec.x, vec.y - 50, vec.z);
        glEnd();
        break;

    case AXIS_Z:
        glBegin(GL_LINES);
        glColor3f(0, 0, 1);
        glVertex3f(vec.x, vec.y, vec.z + 50);
        glColor3f(0, 0, 1);
        glVertex3f(vec.x, vec.y, vec.z - 50);
        glEnd();
        break;
    };

    glPopMatrix();
};

void draw_model_poly_group(st_model* model, int index)
{
    SET_TEXTURE(tex_buildings);

    float model_scale = 0.00195f;

    if (model->polyType[index] == 0x00)
        glBegin(GL_TRIANGLES);
    else
        glBegin(GL_QUADS);

    if (model->polyType[index] == 0x00) // triangle
    {
        for (int i = 0; i < model->polyAmount[index]; i++)
        {
            glColor3f
            (
                LBA2Palette.theColors[model->polyColor[index][i]].r * 0.01f,
                LBA2Palette.theColors[model->polyColor[index][i]].g * 0.01f,
                LBA2Palette.theColors[model->polyColor[index][i]].b * 0.01f

                //1, 0, 0
            );
            glVertex3f // 0
            (
                model->x[model->polyV1[index][i]] * model_scale,
                model->y[model->polyV1[index][i]] * model_scale,
                model->z[model->polyV1[index][i]] * model_scale
            );

            glColor3f
            (
                LBA2Palette.theColors[model->polyColor[index][i]].r * 0.01f,
                LBA2Palette.theColors[model->polyColor[index][i]].g * 0.01f,
                LBA2Palette.theColors[model->polyColor[index][i]].b * 0.01f

                //0, 1, 0
            );
            glVertex3f // 1
            (
                model->x[model->polyV2[index][i]] * model_scale,
                model->y[model->polyV2[index][i]] * model_scale,
                model->z[model->polyV2[index][i]] * model_scale
            );

            glColor3f
            (
                LBA2Palette.theColors[model->polyColor[index][i]].r * 0.01f,
                LBA2Palette.theColors[model->polyColor[index][i]].g * 0.01f,
                LBA2Palette.theColors[model->polyColor[index][i]].b * 0.01f

                //0, 0, 1
            );
            glVertex3f // 2
            (
                model->x[model->polyV3[index][i]] * model_scale,
                model->y[model->polyV3[index][i]] * model_scale,
                model->z[model->polyV3[index][i]] * model_scale
            );
        };
    }
    if (model->polyType[index] != 0x00)  // quad
    {
        if (model->polyMat[index] >= 0x0A) // textured
        {
            float textureCalc = 0.000015287f;

            glEnable(GL_TEXTURE_2D);
            SET_TEXTURE(tex_buildings);

            for (int i = 0; i < model->polyAmount[index]; i++)
            {
                glTexCoord2f(model->UV1_x[index][i] * textureCalc, model->UV1_y[index][i] * textureCalc);

                glColor3f
                (
                    LBA2Palette.theColors[model->polyColor[index][i]].r * 0.01f,
                    LBA2Palette.theColors[model->polyColor[index][i]].g * 0.01f,
                    LBA2Palette.theColors[model->polyColor[index][i]].b * 0.01f

                    //1, 0, 0
                );
                glVertex3f
                (
                    model->x[model->polyV1[index][i]] * model_scale,
                    model->y[model->polyV1[index][i]] * model_scale,
                    model->z[model->polyV1[index][i]] * model_scale
                );

                glTexCoord2f(model->UV2_x[index][i] * textureCalc, model->UV2_y[index][i] * textureCalc);

                glColor3f
                (
                    LBA2Palette.theColors[model->polyColor[index][i]].r * 0.01f,
                    LBA2Palette.theColors[model->polyColor[index][i]].g * 0.01f,
                    LBA2Palette.theColors[model->polyColor[index][i]].b * 0.01f

                    //0, 1, 0
                );
                glVertex3f
                (
                    model->x[model->polyV2[index][i]] * model_scale,
                    model->y[model->polyV2[index][i]] * model_scale,
                    model->z[model->polyV2[index][i]] * model_scale
                );

                glTexCoord2f(model->UV3_x[index][i] * textureCalc, model->UV3_y[index][i] * textureCalc);

                glColor3f
                (
                    LBA2Palette.theColors[model->polyColor[index][i]].r * 0.01f,
                    LBA2Palette.theColors[model->polyColor[index][i]].g * 0.01f,
                    LBA2Palette.theColors[model->polyColor[index][i]].b * 0.01f

                    //0, 0, 1
                );
                glVertex3f
                (
                    model->x[model->polyV3[index][i]] * model_scale,
                    model->y[model->polyV3[index][i]] * model_scale,
                    model->z[model->polyV3[index][i]] * model_scale
                );

                glTexCoord2f(model->UV4_x[index][i] * textureCalc, model->UV4_y[index][i] * textureCalc);

                glColor3f
                (
                    LBA2Palette.theColors[model->polyColor[index][i]].r * 0.01f,
                    LBA2Palette.theColors[model->polyColor[index][i]].g * 0.01f,
                    LBA2Palette.theColors[model->polyColor[index][i]].b * 0.01f

                    //1, 0, 1
                );
                glVertex3f
                (
                    model->x[model->polyV4[index][i]] * model_scale,
                    model->y[model->polyV4[index][i]] * model_scale,
                    model->z[model->polyV4[index][i]] * model_scale
                );

                glDisable(GL_TEXTURE_2D);
            };
        }
        else // colored
        {
            glDisable(GL_TEXTURE_2D);

            for (int i = 0; i < model->polyAmount[index]; i++)
            {
                glColor3f
                (
                    LBA2Palette.theColors[model->polyColor[index][i]].r * 0.01f,
                    LBA2Palette.theColors[model->polyColor[index][i]].g * 0.01f,
                    LBA2Palette.theColors[model->polyColor[index][i]].b * 0.01f

                    //1, 0, 0
                );
                glVertex3f
                (
                    model->x[model->polyV1[index][i]] * model_scale,
                    model->y[model->polyV1[index][i]] * model_scale,
                    model->z[model->polyV1[index][i]] * model_scale
                );

                glColor3f
                (
                    LBA2Palette.theColors[model->polyColor[index][i]].r * 0.01f,
                    LBA2Palette.theColors[model->polyColor[index][i]].g * 0.01f,
                    LBA2Palette.theColors[model->polyColor[index][i]].b * 0.01f

                    //0, 1, 0
                );
                glVertex3f
                (
                    model->x[model->polyV2[index][i]] * model_scale,
                    model->y[model->polyV2[index][i]] * model_scale,
                    model->z[model->polyV2[index][i]] * model_scale
                );

                glColor3f
                (
                    LBA2Palette.theColors[model->polyColor[index][i]].r * 0.01f,
                    LBA2Palette.theColors[model->polyColor[index][i]].g * 0.01f,
                    LBA2Palette.theColors[model->polyColor[index][i]].b * 0.01f

                    //0, 0, 1
                );
                glVertex3f
                (
                    model->x[model->polyV3[index][i]] * model_scale,
                    model->y[model->polyV3[index][i]] * model_scale,
                    model->z[model->polyV3[index][i]] * model_scale
                );

                glColor3f
                (
                    LBA2Palette.theColors[model->polyColor[index][i]].r * 0.01f,
                    LBA2Palette.theColors[model->polyColor[index][i]].g * 0.01f,
                    LBA2Palette.theColors[model->polyColor[index][i]].b * 0.01f

                    //1, 0, 1
                );
                glVertex3f
                (
                    model->x[model->polyV4[index][i]] * model_scale,
                    model->y[model->polyV4[index][i]] * model_scale,
                    model->z[model->polyV4[index][i]] * model_scale
                );
            };
        };
    };
    glEnd();
};

void draw_model(st_model* model, st_building* building)
{
    float model_scale = 0.00195f;
    int current_poly_group = 0;

    glPushMatrix();

    glTranslatef(building->x * model_scale, -building->z * model_scale, building->y * model_scale);
    glRotatef(90.0f, 1, 0, 0);
    // this is the ccorrect scaling factor, unlike before where I was doing it in reverse
    glRotatef(building->rotation * (360.0f / 4096.0f), 0, 1, 0);

    // draw point clouds
    glBegin(GL_POINTS);
    for (int i = 0; i < model->vertex; i++)
    {
        glColor3f(1, 1, 1);
        glVertex3f(model->x[i] * model_scale, model->y[i] * model_scale, model->z[i] * model_scale);
    };
    glEnd();

    // draw polygon groups
    for (int i = 0; i < model->poly_group_length; i++)
        draw_model_poly_group(model, i);

    glPopMatrix();
};

void draw_selected_poly_odd()
{
    float height_calc = 0.002f;

    extern unsigned int terrain_poly_indices_0[3][MAX_POLYS]; // from main.cpp

    int testPoly = (selectedPoly / 2);

    if (selectedPoly != -1)
    {
        //if (selectedPoly % 1) // triangle 0
        //{
            glPushMatrix();

            glBegin(GL_LINES);

            // line 0
            glColor3f(1, 0, 0);
            glVertex3f(polyPoints[terrain_poly_indices_0[0][testPoly]].x,
                polyPoints[terrain_poly_indices_0[0][testPoly]].y,
                polyPoints[terrain_poly_indices_0[0][testPoly]].z + (terrainHeights[terrain_poly_indices_0[0][testPoly]] * height_calc) + 0.15f);
            glColor3f(1, 0, 0);
            glVertex3f(polyPoints[terrain_poly_indices_0[1][testPoly]].x,
                polyPoints[terrain_poly_indices_0[1][testPoly]].y,
                polyPoints[terrain_poly_indices_0[1][testPoly]].z + (terrainHeights[terrain_poly_indices_0[1][testPoly]] * height_calc) + 0.15f);

            // line 1
            glColor3f(1, 0, 0);
            glVertex3f(polyPoints[terrain_poly_indices_0[1][testPoly]].x,
                polyPoints[terrain_poly_indices_0[1][testPoly]].y,
                polyPoints[terrain_poly_indices_0[1][testPoly]].z + (terrainHeights[terrain_poly_indices_0[1][testPoly]] * height_calc) + 0.15f);
            glColor3f(1, 0, 0);
            glVertex3f(polyPoints[terrain_poly_indices_0[2][testPoly]].x,
                polyPoints[terrain_poly_indices_0[2][testPoly]].y,
                polyPoints[terrain_poly_indices_0[2][testPoly]].z + (terrainHeights[terrain_poly_indices_0[2][testPoly]] * height_calc) + 0.15f);

            // line 2
            glColor3f(1, 0, 0);
            glVertex3f(polyPoints[terrain_poly_indices_0[2][testPoly]].x,
                polyPoints[terrain_poly_indices_0[2][testPoly]].y,
                polyPoints[terrain_poly_indices_0[2][testPoly]].z + (terrainHeights[terrain_poly_indices_0[2][testPoly]] * height_calc) + 0.15f);
            glColor3f(1, 0, 0);
            glVertex3f(polyPoints[terrain_poly_indices_0[0][testPoly]].x,
                polyPoints[terrain_poly_indices_0[0][testPoly]].y,
                polyPoints[terrain_poly_indices_0[0][testPoly]].z + (terrainHeights[terrain_poly_indices_0[0][testPoly]] * height_calc) + 0.15f);

            glEnd();

            glPopMatrix();
        //};
	};
};

void draw_selected_poly_even()
{
    float height_calc = 0.002f;

    extern unsigned int terrain_poly_indices_1[3][MAX_POLYS]; // from main.cpp

    int testPoly = (selectedPoly / 2);

    if (selectedPoly != -1)
    {
        if (selectedPoly % 2) // triangle 1
        {
            glPushMatrix();

            glBegin(GL_LINES);

            // line 0
            glColor3f(1, 1, 0);
            glVertex3f(polyPoints[terrain_poly_indices_1[0][testPoly]].x,
                polyPoints[terrain_poly_indices_1[0][testPoly]].y,
                polyPoints[terrain_poly_indices_1[0][testPoly]].z + (terrainHeights[terrain_poly_indices_1[0][testPoly]] * height_calc) + 0.15f);
            glColor3f(1, 1, 0);
            glVertex3f(polyPoints[terrain_poly_indices_1[1][testPoly]].x,
                polyPoints[terrain_poly_indices_1[1][testPoly]].y,
                polyPoints[terrain_poly_indices_1[1][testPoly]].z + (terrainHeights[terrain_poly_indices_1[1][testPoly]] * height_calc) + 0.15f);

            // line 1
            glColor3f(1, 1, 0);
            glVertex3f(polyPoints[terrain_poly_indices_1[1][testPoly]].x,
                polyPoints[terrain_poly_indices_1[1][testPoly]].y,
                polyPoints[terrain_poly_indices_1[1][testPoly]].z + (terrainHeights[terrain_poly_indices_1[1][testPoly]] * height_calc) + 0.15f);
            glColor3f(1, 1, 0);
            glVertex3f(polyPoints[terrain_poly_indices_1[2][testPoly]].x,
                polyPoints[terrain_poly_indices_1[2][testPoly]].y,
                polyPoints[terrain_poly_indices_1[2][testPoly]].z + (terrainHeights[terrain_poly_indices_1[2][testPoly]] * height_calc) + 0.15f);

            // line 2
            glColor3f(1, 1, 0);
            glVertex3f(polyPoints[terrain_poly_indices_1[2][testPoly]].x,
                polyPoints[terrain_poly_indices_1[2][testPoly]].y,
                polyPoints[terrain_poly_indices_1[2][testPoly]].z + (terrainHeights[terrain_poly_indices_1[2][testPoly]] * height_calc) + 0.15f);
            glColor3f(1, 1, 0);
            glVertex3f(polyPoints[terrain_poly_indices_1[0][testPoly]].x,
                polyPoints[terrain_poly_indices_1[0][testPoly]].y,
                polyPoints[terrain_poly_indices_1[0][testPoly]].z + (terrainHeights[terrain_poly_indices_1[0][testPoly]] * height_calc) + 0.15f);

            glEnd();

            glPopMatrix();
        };
    };
};

// similar to the above but the neighboring poly indices are determined by the selected poly 
void draw_selected_poly_offset_odd(int index)
{
    float height_calc = 0.002f;

    extern unsigned int terrain_poly_indices_0[3][MAX_POLYS]; // from main.cpp

    int testPoly = index + (selectedPoly / 2);

    if (selectedPoly != -1)
    {
        glPushMatrix();

        glBegin(GL_LINES);

        // line 0
        glColor3f(1, 0, 0);
        glVertex3f(polyPoints[terrain_poly_indices_0[0][testPoly]].x,
            polyPoints[terrain_poly_indices_0[0][testPoly]].y,
            polyPoints[terrain_poly_indices_0[0][testPoly]].z + (terrainHeights[terrain_poly_indices_0[0][testPoly]] * height_calc) + 0.15f);
        glColor3f(1, 0, 0);
        glVertex3f(polyPoints[terrain_poly_indices_0[1][testPoly]].x,
            polyPoints[terrain_poly_indices_0[1][testPoly]].y,
            polyPoints[terrain_poly_indices_0[1][testPoly]].z + (terrainHeights[terrain_poly_indices_0[1][testPoly]] * height_calc) + 0.15f);

        // line 1
        glColor3f(1, 0, 0);
        glVertex3f(polyPoints[terrain_poly_indices_0[1][testPoly]].x,
            polyPoints[terrain_poly_indices_0[1][testPoly]].y,
            polyPoints[terrain_poly_indices_0[1][testPoly]].z + (terrainHeights[terrain_poly_indices_0[1][testPoly]] * height_calc) + 0.15f);
        glColor3f(1, 0, 0);
        glVertex3f(polyPoints[terrain_poly_indices_0[2][testPoly]].x,
            polyPoints[terrain_poly_indices_0[2][testPoly]].y,
            polyPoints[terrain_poly_indices_0[2][testPoly]].z + (terrainHeights[terrain_poly_indices_0[2][testPoly]] * height_calc) + 0.15f);

        // line 2
        glColor3f(1, 0, 0);
        glVertex3f(polyPoints[terrain_poly_indices_0[2][testPoly]].x,
            polyPoints[terrain_poly_indices_0[2][testPoly]].y,
            polyPoints[terrain_poly_indices_0[2][testPoly]].z + (terrainHeights[terrain_poly_indices_0[2][testPoly]] * height_calc) + 0.15f);
        glColor3f(1, 0, 0);
        glVertex3f(polyPoints[terrain_poly_indices_0[0][testPoly]].x,
            polyPoints[terrain_poly_indices_0[0][testPoly]].y,
            polyPoints[terrain_poly_indices_0[0][testPoly]].z + (terrainHeights[terrain_poly_indices_0[0][testPoly]] * height_calc) + 0.15f);

        glEnd();

        glPopMatrix();
    };
};

// similar to the above but the neighboring poly indices are determined by the selected poly 
void draw_selected_poly_offset_even(int index)
{
    float height_calc = 0.002f;

    extern unsigned int terrain_poly_indices_1[3][MAX_POLYS]; // from main.cpp

    int testPoly = index + (selectedPoly / 2);

    if (selectedPoly != -1)
    {
        if (selectedPoly % 2) // triangle 1
        {
            glPushMatrix();

            glBegin(GL_LINES);

            // line 0
            glColor3f(1, 1, 0);
            glVertex3f(polyPoints[terrain_poly_indices_1[0][testPoly]].x,
                polyPoints[terrain_poly_indices_1[0][testPoly]].y,
                polyPoints[terrain_poly_indices_1[0][testPoly]].z + (terrainHeights[terrain_poly_indices_1[0][testPoly]] * height_calc) + 0.15f);
            glColor3f(1, 1, 0);
            glVertex3f(polyPoints[terrain_poly_indices_1[1][testPoly]].x,
                polyPoints[terrain_poly_indices_1[1][testPoly]].y,
                polyPoints[terrain_poly_indices_1[1][testPoly]].z + (terrainHeights[terrain_poly_indices_1[1][testPoly]] * height_calc) + 0.15f);

            // line 1
            glColor3f(1, 1, 0);
            glVertex3f(polyPoints[terrain_poly_indices_1[1][testPoly]].x,
                polyPoints[terrain_poly_indices_1[1][testPoly]].y,
                polyPoints[terrain_poly_indices_1[1][testPoly]].z + (terrainHeights[terrain_poly_indices_1[1][testPoly]] * height_calc) + 0.15f);
            glColor3f(1, 1, 0);
            glVertex3f(polyPoints[terrain_poly_indices_1[2][testPoly]].x,
                polyPoints[terrain_poly_indices_1[2][testPoly]].y,
                polyPoints[terrain_poly_indices_1[2][testPoly]].z + (terrainHeights[terrain_poly_indices_1[2][testPoly]] * height_calc) + 0.15f);

            // line 2
            glColor3f(1, 1, 0);
            glVertex3f(polyPoints[terrain_poly_indices_1[2][testPoly]].x,
                polyPoints[terrain_poly_indices_1[2][testPoly]].y,
                polyPoints[terrain_poly_indices_1[2][testPoly]].z + (terrainHeights[terrain_poly_indices_1[2][testPoly]] * height_calc) + 0.15f);
            glColor3f(1, 1, 0);
            glVertex3f(polyPoints[terrain_poly_indices_1[0][testPoly]].x,
                polyPoints[terrain_poly_indices_1[0][testPoly]].y,
                polyPoints[terrain_poly_indices_1[0][testPoly]].z + (terrainHeights[terrain_poly_indices_1[0][testPoly]] * height_calc) + 0.15f);

            glEnd();

            glPopMatrix();
        };
    };
};

void draw_brush(int brush_size)
{
    // draw brush sizes
    switch (brush_size)
    {
        // skip case 1 since it already draws 2 triangles

    case 2: // 8 triangles
        draw_selected_poly_offset_odd(1);
        draw_selected_poly_offset_even(1);

        // next row
        draw_selected_poly_offset_odd(64);
        draw_selected_poly_offset_even(64);

        draw_selected_poly_offset_odd(65);
        draw_selected_poly_offset_even(65);
        break;

    case 3: // 18 triangles
        draw_selected_poly_offset_odd(1);
        draw_selected_poly_offset_even(1);

        draw_selected_poly_offset_odd(2);
        draw_selected_poly_offset_even(2);

        // next row
        draw_selected_poly_offset_odd(64);
        draw_selected_poly_offset_even(64);

        draw_selected_poly_offset_odd(65);
        draw_selected_poly_offset_even(65);

        draw_selected_poly_offset_odd(66);
        draw_selected_poly_offset_even(66);

        // next row
        draw_selected_poly_offset_odd(64 + 64);
        draw_selected_poly_offset_even(64 + 64);

        draw_selected_poly_offset_odd(65 + 64);
        draw_selected_poly_offset_even(65 + 64);

        draw_selected_poly_offset_odd(66 + 64);
        draw_selected_poly_offset_even(66 + 64);
        break;

    case 4: // 32 triangles

        draw_selected_poly_offset_odd(1);
        draw_selected_poly_offset_even(1);

        draw_selected_poly_offset_odd(2);
        draw_selected_poly_offset_even(2);

        draw_selected_poly_offset_odd(3);
        draw_selected_poly_offset_even(3);

        // next row
        draw_selected_poly_offset_odd(64);
        draw_selected_poly_offset_even(64);

        draw_selected_poly_offset_odd(65);
        draw_selected_poly_offset_even(65);

        draw_selected_poly_offset_odd(66);
        draw_selected_poly_offset_even(66);

        draw_selected_poly_offset_odd(67);
        draw_selected_poly_offset_even(67);

        // next row
        draw_selected_poly_offset_odd(64 + 64);
        draw_selected_poly_offset_even(64 + 64);

        draw_selected_poly_offset_odd(65 + 64);
        draw_selected_poly_offset_even(65 + 64);

        draw_selected_poly_offset_odd(66 + 64);
        draw_selected_poly_offset_even(66 + 64);

        draw_selected_poly_offset_odd(67 + 64);
        draw_selected_poly_offset_even(67 + 64);

        // next row
        draw_selected_poly_offset_odd(64 + 128);
        draw_selected_poly_offset_even(64 + 128);

        draw_selected_poly_offset_odd(65 + 128);
        draw_selected_poly_offset_even(65 + 128);

        draw_selected_poly_offset_odd(66 + 128);
        draw_selected_poly_offset_even(66 + 128);

        draw_selected_poly_offset_odd(67 + 128);
        draw_selected_poly_offset_even(67 + 128);

        break;
    };
};

void copy_height_data_3(short* ptr) // 3x3
{
    extern short terrain_heights_buffer_size_3[9]; // from main.cpp

    // row 0
    for (int i = 0; i < 3; ++i)
    {
        terrain_heights_buffer_size_3[i] = *ptr++;
    };

    // next row
    ptr += 65 - 3;

    for (int i = 3; i < 6; ++i)
    {
        terrain_heights_buffer_size_3[i] = *ptr++;
    };

    // next row
    ptr += 65 - 3;

    for (int i = 6; i < 9; ++i)
    {
        terrain_heights_buffer_size_3[i] = *ptr++;
    };

    printf("height data copied: %d \n", *ptr);
};

// this works a little differently from pasting polygon data
// we only need to reference the points again and not the triangle indices
void paste_height_data_3(short* ptr) // 3x3
{
    extern short terrain_heights_buffer_size_3[9]; // from main.cpp

    // row 0
    for (int i = 0; i < 3; i++)
        *ptr++ = terrain_heights_buffer_size_3[i];

    // next row
    ptr += 65 - 3;

    for (int i = 3; i < 6; i++)
        *ptr++ = terrain_heights_buffer_size_3[i];

    // next row
    ptr += 65 - 3;

    for (int i = 6; i < 9; i++)
        *ptr++ = terrain_heights_buffer_size_3[i];

    printf("height data pasted: %d \n", *ptr);
};

void copy_height_data_4(short* ptr) // 4x4
{
    extern short terrain_heights_buffer_size_4[16]; // from main.cpp

    // row 0
    for (int i = 0; i < 4; ++i)
    {
        terrain_heights_buffer_size_4[i] = *ptr++;
    };

    // next row
    ptr += 65 - 4;

    for (int i = 4; i < 8; ++i)
    {
        terrain_heights_buffer_size_4[i] = *ptr++;
    };

    // next row
    ptr += 65 - 4;

    for (int i = 8; i < 12; ++i)
    {
        terrain_heights_buffer_size_4[i] = *ptr++;
    };

    // next row
    ptr += 65 - 4;

    for (int i = 12; i < 16; ++i)
    {
        terrain_heights_buffer_size_4[i] = *ptr++;
    };

    printf("height data copied: %d \n", *ptr);
};

void paste_height_data_4(short* ptr) // 4x4
{
    extern short terrain_heights_buffer_size_4[16]; // from main.cpp

    // row 0
    for (int i = 0; i < 4; i++)
        *ptr++ = terrain_heights_buffer_size_4[i];

    // next row
    ptr += 65 - 4;

    for (int i = 4; i < 8; i++)
        *ptr++ = terrain_heights_buffer_size_4[i];

    // next row
    ptr += 65 - 4;

    for (int i = 8; i < 12; i++)
        *ptr++ = terrain_heights_buffer_size_4[i];

    // next row
    ptr += 65 - 4;

    for (int i = 12; i < 16; i++)
        *ptr++ = terrain_heights_buffer_size_4[i];

    printf("height data pasted: %d \n", *ptr);
};

void copy_height_data_5(short* ptr) // 5x5
{
    extern short terrain_heights_buffer_size_5[25]; // from main.cpp

    // row 0
    for (int i = 0; i < 5; ++i)
    {
        terrain_heights_buffer_size_5[i] = *ptr++;
    };

    // next row
    ptr += 65 - 5;

    for (int i = 5; i < 10; ++i)
    {
        terrain_heights_buffer_size_5[i] = *ptr++;
    };

    // next row
    ptr += 65 - 5;

    for (int i = 10; i < 15; ++i)
    {
        terrain_heights_buffer_size_5[i] = *ptr++;
    };

    // next row
    ptr += 65 - 5;

    for (int i = 15; i < 20; ++i)
    {
        terrain_heights_buffer_size_5[i] = *ptr++;
    };

    // next row
    ptr += 65 - 5;

    for (int i = 20; i < 25; ++i)
    {
        terrain_heights_buffer_size_5[i] = *ptr++;
    };

    printf("height data copied: %d \n", *ptr);
};

void paste_height_data_5(short* ptr) // 5x5
{
    extern short terrain_heights_buffer_size_5[25]; // from main.cpp

    // row 0
    for (int i = 0; i < 5; i++)
        *ptr++ = terrain_heights_buffer_size_5[i];

    // next row
    ptr += 65 - 5;

    for (int i = 5; i < 10; i++)
        *ptr++ = terrain_heights_buffer_size_5[i];

    // next row
    ptr += 65 - 5;

    for (int i = 10; i < 15; i++)
        *ptr++ = terrain_heights_buffer_size_5[i];

    // next row
    ptr += 65 - 5;

    for (int i = 15; i < 20; i++)
        *ptr++ = terrain_heights_buffer_size_5[i];

    // next row
    ptr += 65 - 5;

    for (int i = 20; i < 25; i++)
        *ptr++ = terrain_heights_buffer_size_5[i];

    printf("height data pasted: %d \n", *ptr);
};

void copy_poly_data_4(T_HALF_POLY* ptr) // 2x2
{
    extern T_HALF_POLY poly_buffer_size_4[8]; // from main.cpp

    // row 0
    for (int i = 0; i < 4; ++i)
    {
        poly_buffer_size_4[i] = *ptr++;
    };

    // next row (64 quads * 2 triangles = 128 half-polys)
    ptr += 128 - 4;

    for (int i = 4; i < 8; ++i)
    {
        poly_buffer_size_4[i] = *ptr++;
    };
};

void paste_poly_data_4() // 2x2
{
    extern T_HALF_POLY poly_buffer_size_4[8]; // from main.cpp

    terrainPolys[selectedPoly] = poly_buffer_size_4[0];
    terrainPolys[selectedPoly + 1] = poly_buffer_size_4[1];
    terrainPolys[selectedPoly + 2] = poly_buffer_size_4[2];
    terrainPolys[selectedPoly + 3] = poly_buffer_size_4[3];

    terrainPolys[selectedPoly + 128] = poly_buffer_size_4[4];
    terrainPolys[selectedPoly + 128 + 1] = poly_buffer_size_4[5];
    terrainPolys[selectedPoly + 128 + 2] = poly_buffer_size_4[6];
    terrainPolys[selectedPoly + 128 + 3] = poly_buffer_size_4[7];
};

void copy_poly_data_9(T_HALF_POLY* ptr) // 3x3
{
    extern T_HALF_POLY poly_buffer_size_9[18]; // from main.cpp

    // row 0
    for (int i = 0; i < 6; ++i)
    {
        poly_buffer_size_9[i] = *ptr++;
    };

    // next row (64 quads * 2 triangles = 128 half-polys)
    ptr += 128 - 6;

    for (int i = 6; i < 12; ++i)
    {
        poly_buffer_size_9[i] = *ptr++;
    };

    // next row
    ptr += 128 - 6;

    for (int i = 12; i < 18; ++i)
    {
        poly_buffer_size_9[i] = *ptr++;
    };
};

void paste_poly_data_9() // 3x3
{
    extern T_HALF_POLY poly_buffer_size_9[18]; // from main.cpp

    terrainPolys[selectedPoly] = poly_buffer_size_9[0];
    terrainPolys[selectedPoly + 1] = poly_buffer_size_9[1];
    terrainPolys[selectedPoly + 2] = poly_buffer_size_9[2];
    terrainPolys[selectedPoly + 3] = poly_buffer_size_9[3];
    terrainPolys[selectedPoly + 4] = poly_buffer_size_9[4];
    terrainPolys[selectedPoly + 5] = poly_buffer_size_9[5];

    terrainPolys[selectedPoly + 128] = poly_buffer_size_9[6];
    terrainPolys[selectedPoly + 128 + 1] = poly_buffer_size_9[7];
    terrainPolys[selectedPoly + 128 + 2] = poly_buffer_size_9[8];
    terrainPolys[selectedPoly + 128 + 3] = poly_buffer_size_9[9];
    terrainPolys[selectedPoly + 128 + 4] = poly_buffer_size_9[10];
    terrainPolys[selectedPoly + 128 + 5] = poly_buffer_size_9[11];

    terrainPolys[selectedPoly + 256] = poly_buffer_size_9[12];
    terrainPolys[selectedPoly + 256 + 1] = poly_buffer_size_9[13];
    terrainPolys[selectedPoly + 256 + 2] = poly_buffer_size_9[14];
    terrainPolys[selectedPoly + 256 + 3] = poly_buffer_size_9[15];
    terrainPolys[selectedPoly + 256 + 4] = poly_buffer_size_9[16];
    terrainPolys[selectedPoly + 256 + 5] = poly_buffer_size_9[17];
};

void copy_poly_data_16(T_HALF_POLY* ptr) // 4x4
{
    extern T_HALF_POLY poly_buffer_size_16[32]; // from main.cpp

    // row 0
    for (int i = 0; i < 8; ++i)
    {
        poly_buffer_size_16[i] = *ptr++;
    };

    // next row (64 quads * 2 triangles = 128 half-polys)
    ptr += 128 - 8;

    for (int i = 8; i < 16; ++i)
    {
        poly_buffer_size_16[i] = *ptr++;
    };

    // next row
    ptr += 128 - 8;

    for (int i = 16; i < 24; ++i)
    {
        poly_buffer_size_16[i] = *ptr++;
    };

    // next row
    ptr += 128 - 8;

    for (int i = 24; i < 32; ++i)
    {
        poly_buffer_size_16[i] = *ptr++;
    };
};

void paste_poly_data_16() // 4x4
{
    extern T_HALF_POLY poly_buffer_size_16[32]; // from main.cpp

    terrainPolys[selectedPoly] = poly_buffer_size_16[0];
    terrainPolys[selectedPoly + 1] = poly_buffer_size_16[1];
    terrainPolys[selectedPoly + 2] = poly_buffer_size_16[2];
    terrainPolys[selectedPoly + 3] = poly_buffer_size_16[3];
    terrainPolys[selectedPoly + 4] = poly_buffer_size_16[4];
    terrainPolys[selectedPoly + 5] = poly_buffer_size_16[5];
    terrainPolys[selectedPoly + 6] = poly_buffer_size_16[6];
    terrainPolys[selectedPoly + 7] = poly_buffer_size_16[7];

    terrainPolys[selectedPoly + 128] = poly_buffer_size_16[8];
    terrainPolys[selectedPoly + 128 + 1] = poly_buffer_size_16[9];
    terrainPolys[selectedPoly + 128 + 2] = poly_buffer_size_16[10];
    terrainPolys[selectedPoly + 128 + 3] = poly_buffer_size_16[11];
    terrainPolys[selectedPoly + 128 + 4] = poly_buffer_size_16[12];
    terrainPolys[selectedPoly + 128 + 5] = poly_buffer_size_16[13];
    terrainPolys[selectedPoly + 128 + 6] = poly_buffer_size_16[14];
    terrainPolys[selectedPoly + 128 + 7] = poly_buffer_size_16[15];

    terrainPolys[selectedPoly + 256] = poly_buffer_size_16[16];
    terrainPolys[selectedPoly + 256 + 1] = poly_buffer_size_16[17];
    terrainPolys[selectedPoly + 256 + 2] = poly_buffer_size_16[18];
    terrainPolys[selectedPoly + 256 + 3] = poly_buffer_size_16[19];
    terrainPolys[selectedPoly + 256 + 4] = poly_buffer_size_16[20];
    terrainPolys[selectedPoly + 256 + 5] = poly_buffer_size_16[21];
    terrainPolys[selectedPoly + 256 + 6] = poly_buffer_size_16[22];
    terrainPolys[selectedPoly + 256 + 7] = poly_buffer_size_16[23];

    terrainPolys[selectedPoly + 384] = poly_buffer_size_16[24];
    terrainPolys[selectedPoly + 384 + 1] = poly_buffer_size_16[25];
    terrainPolys[selectedPoly + 384 + 2] = poly_buffer_size_16[26];
    terrainPolys[selectedPoly + 384 + 3] = poly_buffer_size_16[27];
    terrainPolys[selectedPoly + 384 + 4] = poly_buffer_size_16[28];
    terrainPolys[selectedPoly + 384 + 5] = poly_buffer_size_16[29];
    terrainPolys[selectedPoly + 384 + 6] = poly_buffer_size_16[30];
    terrainPolys[selectedPoly + 384 + 7] = poly_buffer_size_16[31];
};

void draw_terrain_textures()
{
    SET_TEXTURE(tex_ground);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
};

// the row variable exists so it doesn't alter the drawing of polygons while still keeping the original indices intact
// the texture_index variable is similar
void draw_terrain_quad(int current_square, int current_tri, int index, int row, int texture_index)
{
    //float textureCalc = 0.000015287f; // 1 / 256 / 256
    float textureCalc = (1.0f / 65536.0f);
    float height_calc = 0.002f;
    //float shadow_calc = 0.3f;
    float shadow_calc = 0.5f;



    int indices0[] = { 0, 65, 66 }; // triangle 0
    int indices1[] = { 1, 0, 66 }; // triangle 1

    st_color3f terrainColor0;
    st_color3f terrainColor1;

    T_HALF_TEX* tex_indices0;
    T_HALF_TEX* tex_indices1;

    tex_indices0 = &terrainTexs[terrainPolys[texture_index].IndexTex];
    tex_indices1 = &terrainTexs[terrainPolys[texture_index + 1].IndexTex];

    extern T_HALF_TEX current_poly_tex; // from main.cpp
    extern int current_poly_indices[4]; // from main.cpp
    extern unsigned int terrain_poly_indices_0[3][MAX_POLYS]; // from main.cpp
    extern unsigned int terrain_poly_indices_1[3][MAX_POLYS]; // from main.cpp
    //extern unsigned int terrain_poly_indices[MAX_POLYS][3]; // from main.cpp


    if (showHeights == false)
        height_calc = 0.0f;
    else
        height_calc = 0.002f;

    // check for colors
    switch (terrainPolys[texture_index].Bank)
    {
    case POLY_COLOR_PURPLE_0: // purple
        terrainColor0.r = (LBA2Palette.theColors[0].r * 0.01f);
        terrainColor0.g = (LBA2Palette.theColors[0].g * 0.01f);
        terrainColor0.b = (LBA2Palette.theColors[0].b * 0.01f);
        break;
    case POLY_COLOR_BROWN: // brown
        terrainColor0.r = (LBA2Palette.theColors[16].r * 0.01f);
        terrainColor0.g = (LBA2Palette.theColors[16].g * 0.01f);
        terrainColor0.b = (LBA2Palette.theColors[16].b * 0.01f);
        break;
    case POLY_COLOR_TAN: // tan
        terrainColor0.r = (LBA2Palette.theColors[32].r * 0.01f);
        terrainColor0.g = (LBA2Palette.theColors[32].g * 0.01f);
        terrainColor0.b = (LBA2Palette.theColors[32].b * 0.01f);
        break;
    case POLY_COLOR_WHITE: // white
        terrainColor0.r = (LBA2Palette.theColors[48].r * 0.01f);
        terrainColor0.g = (LBA2Palette.theColors[48].g * 0.01f);
        terrainColor0.b = (LBA2Palette.theColors[48].b * 0.01f);
        break;
    case POLY_COLOR_RED: // red
        terrainColor0.r = (LBA2Palette.theColors[64].r * 0.01f);
        terrainColor0.g = (LBA2Palette.theColors[64].g * 0.01f);
        terrainColor0.b = (LBA2Palette.theColors[64].b * 0.01f);
        break;
    case POLY_COLOR_ORANGE: // orange
        terrainColor0.r = (LBA2Palette.theColors[80].r * 0.01f);
        terrainColor0.g = (LBA2Palette.theColors[80].g * 0.01f);
        terrainColor0.b = (LBA2Palette.theColors[80].b * 0.01f);
        break;
    case POLY_COLOR_YELLOW: // yellow
        terrainColor0.r = (LBA2Palette.theColors[96].r * 0.01f);
        terrainColor0.g = (LBA2Palette.theColors[96].g * 0.01f);
        terrainColor0.b = (LBA2Palette.theColors[96].b * 0.01f);
        break;
    case POLY_COLOR_ARMY_GREEN: // army green
        terrainColor0.r = (LBA2Palette.theColors[112].r * 0.01f);
        terrainColor0.g = (LBA2Palette.theColors[112].g * 0.01f);
        terrainColor0.b = (LBA2Palette.theColors[112].b * 0.01f);
        break;
    case POLY_COLOR_GREEN: // green
        terrainColor0.r = (LBA2Palette.theColors[128].r * 0.01f);
        terrainColor0.g = (LBA2Palette.theColors[128].g * 0.01f);
        terrainColor0.b = (LBA2Palette.theColors[128].b * 0.01f);
        break;
    case POLY_COLOR_BLUE_GREEN: // blue-green
        terrainColor0.r = (LBA2Palette.theColors[144].r * 0.01f);
        terrainColor0.g = (LBA2Palette.theColors[144].g * 0.01f);
        terrainColor0.b = (LBA2Palette.theColors[144].b * 0.01f);
        break;
    case POLY_COLOR_CYAN:
        terrainColor0.r = (LBA2Palette.theColors[160].r * 0.01f);
        terrainColor0.g = (LBA2Palette.theColors[160].g * 0.01f);
        terrainColor0.b = (LBA2Palette.theColors[160].b * 0.01f);
        break;
    case POLY_COLOR_GRAY:
        terrainColor0.r = (LBA2Palette.theColors[176].r * 0.01f);
        terrainColor0.g = (LBA2Palette.theColors[176].g * 0.01f);
        terrainColor0.b = (LBA2Palette.theColors[176].b * 0.01f);
        break;
    case POLY_COLOR_BLUE:
        terrainColor0.r = (LBA2Palette.theColors[192].r * 0.01f);
        terrainColor0.g = (LBA2Palette.theColors[192].g * 0.01f);
        terrainColor0.b = (LBA2Palette.theColors[192].b * 0.01f);
        break;
    case POLY_COLOR_PURPLE_1:
        terrainColor0.r = (LBA2Palette.theColors[208].r * 0.01f);
        terrainColor0.g = (LBA2Palette.theColors[208].g * 0.01f);
        terrainColor0.b = (LBA2Palette.theColors[208].b * 0.01f);
        break;

    default:
        terrainColor0.r = 0;
        terrainColor0.g = 0;
        terrainColor0.b = 0;
        break;
    };

    // check for colors
    switch (terrainPolys[texture_index + 1].Bank)
    {
    case POLY_COLOR_PURPLE_0: // purple
        terrainColor1.r = (LBA2Palette.theColors[0].r * 0.01f);
        terrainColor1.g = (LBA2Palette.theColors[0].g * 0.01f);
        terrainColor1.b = (LBA2Palette.theColors[0].b * 0.01f);
        break;
    case POLY_COLOR_BROWN: // brown
        terrainColor1.r = (LBA2Palette.theColors[16].r * 0.01f);
        terrainColor1.g = (LBA2Palette.theColors[16].g * 0.01f);
        terrainColor1.b = (LBA2Palette.theColors[16].b * 0.01f);
        break;
    case POLY_COLOR_TAN: // tan
        terrainColor1.r = (LBA2Palette.theColors[32].r * 0.01f);
        terrainColor1.g = (LBA2Palette.theColors[32].g * 0.01f);
        terrainColor1.b = (LBA2Palette.theColors[32].b * 0.01f);
        break;
    case POLY_COLOR_WHITE: // white
        terrainColor1.r = (LBA2Palette.theColors[48].r * 0.01f);
        terrainColor1.g = (LBA2Palette.theColors[48].g * 0.01f);
        terrainColor1.b = (LBA2Palette.theColors[48].b * 0.01f);
        break;
    case POLY_COLOR_RED: // red
        terrainColor1.r = (LBA2Palette.theColors[64].r * 0.01f);
        terrainColor1.g = (LBA2Palette.theColors[64].g * 0.01f);
        terrainColor1.b = (LBA2Palette.theColors[64].b * 0.01f);
        break;
    case POLY_COLOR_ORANGE: // orange
        terrainColor1.r = (LBA2Palette.theColors[80].r * 0.01f);
        terrainColor1.g = (LBA2Palette.theColors[80].g * 0.01f);
        terrainColor1.b = (LBA2Palette.theColors[80].b * 0.01f);
        break;
    case POLY_COLOR_YELLOW: // yellow
        terrainColor1.r = (LBA2Palette.theColors[96].r * 0.01f);
        terrainColor1.g = (LBA2Palette.theColors[96].g * 0.01f);
        terrainColor1.b = (LBA2Palette.theColors[96].b * 0.01f);
        break;
    case POLY_COLOR_ARMY_GREEN: // army green
        terrainColor1.r = (LBA2Palette.theColors[112].r * 0.01f);
        terrainColor1.g = (LBA2Palette.theColors[112].g * 0.01f);
        terrainColor1.b = (LBA2Palette.theColors[112].b * 0.01f);
        break;
    case POLY_COLOR_GREEN: // green
        terrainColor1.r = (LBA2Palette.theColors[128].r * 0.01f);
        terrainColor1.g = (LBA2Palette.theColors[128].g * 0.01f);
        terrainColor1.b = (LBA2Palette.theColors[128].b * 0.01f);
        break;
    case POLY_COLOR_BLUE_GREEN: // blue-green
        terrainColor1.r = (LBA2Palette.theColors[144].r * 0.01f);
        terrainColor1.g = (LBA2Palette.theColors[144].g * 0.01f);
        terrainColor1.b = (LBA2Palette.theColors[144].b * 0.01f);
        break;
    case POLY_COLOR_CYAN:
        terrainColor1.r = (LBA2Palette.theColors[160].r * 0.01f);
        terrainColor1.g = (LBA2Palette.theColors[160].g * 0.01f);
        terrainColor1.b = (LBA2Palette.theColors[160].b * 0.01f);
        break;
    case POLY_COLOR_GRAY:
        terrainColor1.r = (LBA2Palette.theColors[176].r * 0.01f);
        terrainColor1.g = (LBA2Palette.theColors[176].g * 0.01f);
        terrainColor1.b = (LBA2Palette.theColors[176].b * 0.01f);
        break;
    case POLY_COLOR_BLUE:
        terrainColor1.r = (LBA2Palette.theColors[192].r * 0.01f);
        terrainColor1.g = (LBA2Palette.theColors[192].g * 0.01f);
        terrainColor1.b = (LBA2Palette.theColors[192].b * 0.01f);
        break;
    case POLY_COLOR_PURPLE_1:
        terrainColor1.r = (LBA2Palette.theColors[208].r * 0.01f);
        terrainColor1.g = (LBA2Palette.theColors[208].g * 0.01f);
        terrainColor1.b = (LBA2Palette.theColors[208].b * 0.01f);
        break;

    default:
        terrainColor0.r = 0;
        terrainColor0.g = 0;
        terrainColor0.b = 0;
        break;
    };

    // for now
    //terrainColor0.r = 0.5;
    //terrainColor0.g = 0.5;
    //terrainColor0.b = 0.5;

    //terrainColor1.r = 0.5;
    //terrainColor1.g = 0.5;
    //terrainColor1.b = 0.5;



    // highlight current square
    if (current_square == index)
        glColor3f(1, 0, 0);
    else
        glColor3f(1, 1, 1);

    glPushMatrix();

    // highlight current triangle in current square (triangle 0)
    if (current_square == index && !(current_tri % 2))
        glColor3f(0, 1, 1);



    // sort diagonals
    if (terrainPolys[texture_index].Sens == 0 &&
        terrainPolys[texture_index + 1].Sens == 0) // figure A
    {
        /*indices0[0] = 0; // correct
        indices0[1] = 65;
        indices0[2] = 66;*/

        // even though we already checked both diagonals,
        // check the diagonal on the second triangle again and swap vertex indices
        if (terrainPolys[texture_index + 1].Sens == 1)
        {
            indices0[0] = 0; // correct
            indices0[1] = 65;
            indices0[2] = 66;

            indices1[0] = 1; // correct
            indices1[1] = 0;
            indices1[2] = 66;
        }
        else
        {
            indices0[0] = 0; // correct
            indices0[1] = 65;
            indices0[2] = 66;

            indices1[0] = 66; // correct
            indices1[1] = 1;
            indices1[2] = 0;
        };
    }
    else if (terrainPolys[texture_index].Sens == 0 &&
        terrainPolys[texture_index + 1].Sens == 1) // figure D
    {
        // debug colors
        //glColor3f(0.5, 0.5, 0.5); // gray

        indices0[0] = 0; // correct
        indices0[1] = 65;
        indices0[2] = 66;

        indices1[0] = 1;
        indices1[1] = 0;
        indices1[2] = 66; // correct
    }
    else if (terrainPolys[texture_index].Sens == 1 &&
        terrainPolys[texture_index + 1].Sens == 0) // figure B
    {
        // debug colors
        //glColor3f(1, 0, 1); // purple

        indices0[0] = 1; // correct
        indices0[1] = 0;
        indices0[2] = 65;

        indices1[0] = 65; // correct
        indices1[1] = 66;
        indices1[2] = 1;
    }
    else if (terrainPolys[texture_index].Sens == 1 &&
        terrainPolys[texture_index + 1].Sens == 1) // figure C
    {
        // debug colors
        //glColor3f(0, 1, 1); // cyan

        indices0[0] = 1;
        indices0[1] = 0;
        indices0[2] = 65;

        //indices1[0] = 65;
        //indices1[1] = 66;
        //indices1[2] = 1;

        indices1[0] = 66;
        indices1[1] = 1;
        indices1[2] = 65; // correct
    };



    // the poly is technically still being drawn but we set the indices to 0 for colored polygons
    /*if (terrainPolys[texture_index].PolyFlag > 0)
    {
        indices0[0] = 0;
        indices0[1] = 0;
        indices0[2] = 0;
    };

    if (terrainPolys[texture_index + 1].PolyFlag > 0)
    {
        indices1[0] = 0;
        indices1[1] = 0;
        indices1[2] = 0;
    };*/



    // send current polygon data to the texture window so we can see where it lays in space
    /*if (current_square == index && (current_tri % 2))
    {
        current_poly_indices[0] = (indices0[0] + (index));
        current_poly_indices[1] = (indices0[1] + (index));
        current_poly_indices[2] = (indices0[2] + (index));
        //current_poly_indices[3] = (indices0[0] + (index)+row);
    }
    else if (current_square == index && !(current_tri % 2))
    {
        current_poly_indices[0] = (indices1[0] + (index));
        current_poly_indices[1] = (indices1[1] + (index));
        current_poly_indices[2] = (indices1[2] + (index));
        //current_poly_indices[3] = (indices1[0] + (index)+row);
    };*/

    //
    // triangle 0 (COLOR) (START)
    //
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_TRIANGLES);

    if (showShadows == true)
    {
        glColor3f(terrainColor0.r + terrainClouds[indices0[0] + (index)+row] * 0.02f,
            terrainColor0.g + terrainClouds[indices0[0] + (index)+row] * 0.02f,
            terrainColor0.b + terrainClouds[indices0[0] + (index)+row] * 0.02f);
    }
    else
    {
        glColor3f(terrainColor0.r, terrainColor0.g, terrainColor0.b);
    };

    glVertex3f(polyPoints[indices0[0] + (index)+row].x,
        polyPoints[indices0[0] + (index)+row].y,
        polyPoints[indices0[0] + (index)+row].z - (0.01f) + (terrainHeights[indices0[0] + (index)+row] * height_calc));

    if (showShadows == true)
    {
        glColor3f(terrainColor0.r + terrainClouds[indices0[1] + (index)+row] * 0.02f,
            terrainColor0.g + terrainClouds[indices0[1] + (index)+row] * 0.02f,
            terrainColor0.b + terrainClouds[indices0[1] + (index)+row] * 0.02f);
    }
    else
    {
        glColor3f(terrainColor0.r, terrainColor0.g, terrainColor0.b);
    };

    glVertex3f(polyPoints[indices0[1] + (index)+row].x,
        polyPoints[indices0[1] + (index)+row].y,
        polyPoints[indices0[1] + (index)+row].z - (0.01f) + (terrainHeights[indices0[1] + (index)+row] * height_calc));

    if (showShadows == true)
    {
        glColor3f(terrainColor0.r + terrainClouds[indices0[2] + (index)+row] * 0.02f,
            terrainColor0.g + terrainClouds[indices0[2] + (index)+row] * 0.02f,
            terrainColor0.b + terrainClouds[indices0[2] + (index)+row] * 0.02f);
    }
    else
    {
        glColor3f(terrainColor0.r, terrainColor0.g, terrainColor0.b);
    };

    glVertex3f(polyPoints[indices0[2] + (index)+row].x,
        polyPoints[indices0[2] + (index)+row].y,
        polyPoints[indices0[2] + (index)+row].z - (0.01f) + (terrainHeights[indices0[2] + (index)+row] * height_calc));

    glEnd();
    //glEnable(GL_TEXTURE_2D);
    //
    // triangle 0 (COLOR) (END)
    //


    //
    // triangle 1 (COLOR) (START)
    //
    //glDisable(GL_TEXTURE_2D);
    glBegin(GL_TRIANGLES);

    if (showShadows == true)
    {
        glColor3f(terrainColor1.r + terrainClouds[indices1[0] + (index)+row] * 0.02f,
            terrainColor1.g + terrainClouds[indices1[0] + (index)+row] * 0.02f,
            terrainColor1.b + terrainClouds[indices1[0] + (index)+row] * 0.02f);
    }
    else
    {
        glColor3f(terrainColor1.r, terrainColor1.g, terrainColor1.b);
    };

    glVertex3f(polyPoints[indices1[0] + (index)+row].x,
        polyPoints[indices1[0] + (index)+row].y,
        polyPoints[indices1[0] + (index)+row].z - (0.01f) + (terrainHeights[indices1[0] + (index)+row] * height_calc));

    if (showShadows == true)
    {
        glColor3f(terrainColor1.r + terrainClouds[indices1[1] + (index)+row] * 0.02f,
            terrainColor1.g + terrainClouds[indices1[1] + (index)+row] * 0.02f,
            terrainColor1.b + terrainClouds[indices1[1] + (index)+row] * 0.02f);
    }
    else
    {
        glColor3f(terrainColor1.r, terrainColor1.g, terrainColor1.b);
    };

    glVertex3f(polyPoints[indices1[1] + (index)+row].x,
        polyPoints[indices1[1] + (index)+row].y,
        polyPoints[indices1[1] + (index)+row].z - (0.01f) + (terrainHeights[indices1[1] + (index)+row] * height_calc));

    if (showShadows == true)
    {
        glColor3f(terrainColor1.r + terrainClouds[indices1[2] + (index)+row] * 0.02f,
            terrainColor1.g + terrainClouds[indices1[2] + (index)+row] * 0.02f,
            terrainColor1.b + terrainClouds[indices1[2] + (index)+row] * 0.02f);
    }
    else
    {
        glColor3f(terrainColor1.r, terrainColor1.g, terrainColor1.b);
    };

    glVertex3f(polyPoints[indices1[2] + (index)+row].x,
        polyPoints[indices1[2] + (index)+row].y,
        polyPoints[indices1[2] + (index)+row].z - (0.01f) + (terrainHeights[indices1[2] + (index)+row] * height_calc));

    glEnd();
    glEnable(GL_TEXTURE_2D);
    //
    // triangle 1 (COLOR) (END)
    //



    //
    // triangle 0
    //
    if (showTextures == true)
    {
        glBegin(GL_TRIANGLES); // indices: 1, 65, 66  (tex_indices: 0, 1, 2)

        if (terrainPolys[texture_index].TexFlag > 1)
        {
            glTexCoord2f((float)tex_indices0->Tx0 * textureCalc,
                (float)tex_indices0->Ty0 * textureCalc);
        };

        if (showShadows == true)
        {
            /*glColor3f(terrainClouds[indices0[0] + (index)+row] * 0.2f + shadow_calc,
                terrainClouds[indices0[0] + (index)+row] * 0.2f + shadow_calc,
                terrainClouds[indices0[0] + (index)+row] * 0.2f + shadow_calc);*/

            glColor3f(terrainClouds[indices0[0] + (index)+row] * (0.3f * shadow_calc),
                terrainClouds[indices0[0] + (index)+row] * (0.3f * shadow_calc),
                terrainClouds[indices0[0] + (index)+row] * (0.3f * shadow_calc));
        }
        else
        {
            glColor3f(1, 1, 1);
        };

        if (terrainPolys[texture_index].TexFlag > 1)
        {
            glVertex3f(polyPoints[indices0[0] + (index)+row].x,
                polyPoints[indices0[0] + (index)+row].y,
                polyPoints[indices0[0] + (index)+row].z + (terrainHeights[indices0[0] + (index)+row] * height_calc));
        };

        if (terrainPolys[texture_index].TexFlag > 1)
        {
            glTexCoord2f((float)tex_indices0->Tx1 * textureCalc,
                (float)tex_indices0->Ty1 * textureCalc);
        };

        if (showShadows == true)
        {
            /*glColor3f(terrainClouds[indices0[1] + (index)+row] * 0.2f + shadow_calc,
                terrainClouds[indices0[1] + (index)+row] * 0.2f + shadow_calc,
                terrainClouds[indices0[1] + (index)+row] * 0.2f + shadow_calc);*/

            glColor3f(terrainClouds[indices0[1] + (index)+row] * (0.3f * shadow_calc),
                terrainClouds[indices0[1] + (index)+row] * (0.3f * shadow_calc),
                terrainClouds[indices0[1] + (index)+row] * (0.3f * shadow_calc));
        }
        else
        {
            glColor3f(1, 1, 1);
        };

        if (terrainPolys[texture_index].TexFlag > 1)
        {
            glVertex3f(polyPoints[indices0[1] + (index)+row].x,
                polyPoints[indices0[1] + (index)+row].y,
                polyPoints[indices0[1] + (index)+row].z + (terrainHeights[indices0[1] + (index)+row] * height_calc));
        };

        if (terrainPolys[texture_index].TexFlag > 1)
        {
            glTexCoord2f((float)tex_indices0->Tx2 * textureCalc,
                (float)tex_indices0->Ty2 * textureCalc);
        };

        if (showShadows == true)
        {
            /*glColor3f(terrainClouds[indices0[2] + (index)+row] * 0.2f + shadow_calc,
                terrainClouds[indices0[2] + (index)+row] * 0.2f + shadow_calc,
                terrainClouds[indices0[2] + (index)+row] * 0.2f + shadow_calc);*/

            glColor3f(terrainClouds[indices0[2] + (index)+row] * (0.3f * shadow_calc),
                terrainClouds[indices0[2] + (index)+row] * (0.3f * shadow_calc),
                terrainClouds[indices0[2] + (index)+row] * (0.3f * shadow_calc));
        }
        else
        {
            glColor3f(1, 1, 1);
        };

        if (terrainPolys[texture_index].TexFlag > 1)
        {
            glVertex3f(polyPoints[indices0[2] + (index)+row].x,
                polyPoints[indices0[2] + (index)+row].y,
                polyPoints[indices0[2] + (index)+row].z + (terrainHeights[indices0[2] + (index)+row] * height_calc));
        };

        glEnd();
    };
    //
    // triangle 0
    //



    // highlight current triangle in current square (triangle 1)
    if (current_square == index && (current_tri % 2))
        glColor3f(1, 1, 0);



    //
    // triangle 1
    //
    if (showTextures == true)
    {
        glBegin(GL_TRIANGLES); // indices: 1, 0, 66  (tex_indices: 1, 2, 0)

        if (terrainPolys[texture_index + 1].TexFlag > 1)
        {
            // original = 1, 0
            if (terrainPolys[texture_index + 1].Sens == 1)
            {
                glTexCoord2f((float)tex_indices1->Tx1 * textureCalc,
                    (float)tex_indices1->Ty1 * textureCalc);
            }
            else
            {
                glTexCoord2f(tex_indices1->Tx0 * textureCalc,
                    tex_indices1->Ty0 * textureCalc);
            };
        };

        if (showShadows == true)
        {
            /*glColor3f(terrainClouds[indices1[0] + (index)+row] * 0.2f + shadow_calc,
                terrainClouds[indices1[0] + (index)+row] * 0.2f + shadow_calc,
                terrainClouds[indices1[0] + (index)+row] * 0.2f + shadow_calc);*/

            glColor3f(terrainClouds[indices1[0] + (index)+row] * (0.3f * shadow_calc),
                terrainClouds[indices1[0] + (index)+row] * (0.3f * shadow_calc),
                terrainClouds[indices1[0] + (index)+row] * (0.3f * shadow_calc));
        }
        else
        {
            glColor3f(1, 1, 1);
        };

        if (terrainPolys[texture_index + 1].TexFlag > 1)
        {
            glVertex3f(polyPoints[indices1[0] + (index)+row].x,
                polyPoints[indices1[0] + (index)+row].y,
                polyPoints[indices1[0] + (index)+row].z + (terrainHeights[indices1[0] + (index)+row] * height_calc));
        };

        if (terrainPolys[texture_index + 1].TexFlag > 1)
        {
            // original = 2, 1
            if (terrainPolys[texture_index + 1].Sens == 1)
            {
                glTexCoord2f((float)tex_indices1->Tx2 * textureCalc,
                    (float)tex_indices1->Ty2 * textureCalc);
            }
            else
            {
                glTexCoord2f((float)tex_indices1->Tx1 * textureCalc,
                    (float)tex_indices1->Ty1 * textureCalc);
            };
        };

        if (showShadows == true)
        {
            /*glColor3f(terrainClouds[indices1[1] + (index)+row] * 0.2f + shadow_calc,
                terrainClouds[indices1[1] + (index)+row] * 0.2f + shadow_calc,
                terrainClouds[indices1[1] + (index)+row] * 0.2f + shadow_calc);*/

            glColor3f(terrainClouds[indices1[1] + (index)+row] * (0.3f * shadow_calc),
                terrainClouds[indices1[1] + (index)+row] * (0.3f * shadow_calc),
                terrainClouds[indices1[1] + (index)+row] * (0.3f * shadow_calc));
        }
        else
        {
            glColor3f(1, 1, 1);
        };

        if (terrainPolys[texture_index + 1].TexFlag > 1)
        {
            glVertex3f(polyPoints[indices1[1] + (index)+row].x,
                polyPoints[indices1[1] + (index)+row].y,
                polyPoints[indices1[1] + (index)+row].z + (terrainHeights[indices1[1] + (index)+row] * height_calc));
        };

        if (terrainPolys[texture_index + 1].TexFlag > 1)
        {
            // original = 0, 2
            if (terrainPolys[texture_index + 1].Sens == 1)
            {
                glTexCoord2f((float)tex_indices1->Tx0 * textureCalc,
                    (float)tex_indices1->Ty0 * textureCalc);
            }
            else
            {
                glTexCoord2f((float)tex_indices1->Tx2 * textureCalc,
                    (float)tex_indices1->Ty2 * textureCalc);
            };
        };

        if (showShadows == true)
        {
            /*glColor3f(terrainClouds[indices1[2] + (index)+row] * 0.2f + shadow_calc,
                terrainClouds[indices1[2] + (index)+row] * 0.2f + shadow_calc,
                terrainClouds[indices1[2] + (index)+row] * 0.2f + shadow_calc);*/

            glColor3f(terrainClouds[indices1[2] + (index)+row] * (0.3f * shadow_calc),
                terrainClouds[indices1[2] + (index)+row] * (0.3f * shadow_calc),
                terrainClouds[indices1[2] + (index)+row] * (0.3f * shadow_calc));
        }
        else
        {
            glColor3f(1, 1, 1);
        }

        if (terrainPolys[texture_index + 1].TexFlag > 1)
        {
            glVertex3f(polyPoints[indices1[2] + (index)+row].x,
                polyPoints[indices1[2] + (index)+row].y,
                polyPoints[indices1[2] + (index)+row].z + (terrainHeights[indices1[2] + (index)+row] * height_calc));
        };

        glEnd();
    };
    //
    // triangle 1
    //

    //glPopMatrix();



    // draw line segments for triangles that Twinsen collides with
    if (showCollisions == true)
    {
        if (terrainPolys[texture_index].Col == 1)
        {
            st_vec3f* p0 = &polyPoints[indices0[0] + (index)+row];
            st_vec3f* p1 = &polyPoints[indices0[1] + (index)+row];
            st_vec3f* p2 = &polyPoints[indices0[2] + (index)+row];

            /*st_vec3f mid;
            mid.x = ((p0->x + p1->x + p2->x) / 3);
            mid.y = ((p0->y + p1->y + p2->y) / 3);
            mid.z = ((p0->z + p1->z + p2->z) / 3) + (terrainHeights[indices0[2] + (index)+row] * height_calc);

            glBegin(GL_LINES);

            //glColor3f(0, 0, 1);
            glColor4f(0, 0, 1, 0.5f);

            glVertex3f(mid.x, mid.y, mid.z);
            glVertex3f(mid.x, mid.y, mid.z + 2);

            glEnd();*/

            glBegin(GL_TRIANGLES);

            glColor4f(0, 0, 1, 0.5f);
            glVertex3f(p0->x, p0->y, p0->z + 0.15f + (terrainHeights[indices0[0] + (index)+row] * height_calc));

            glColor4f(0, 0, 1, 0.5f);
            glVertex3f(p1->x, p1->y, p1->z + 0.15f + (terrainHeights[indices0[1] + (index)+row] * height_calc));

            glColor4f(0, 0, 1, 0.5f);
            glVertex3f(p2->x, p2->y, p2->z + 0.15f + (terrainHeights[indices0[2] + (index)+row] * height_calc));

            glEnd();
        };

        if (terrainPolys[texture_index + 1].Col == 1)
        {
            st_vec3f* p0 = &polyPoints[indices1[0] + (index)+row];
            st_vec3f* p1 = &polyPoints[indices1[1] + (index)+row];
            st_vec3f* p2 = &polyPoints[indices1[2] + (index)+row];

            glBegin(GL_TRIANGLES);

            glColor4f(0, 0, 1, 0.5f);
            glVertex3f(p0->x, p0->y, p0->z + 0.15f + (terrainHeights[indices1[0] + (index)+row] * height_calc));

            glColor4f(0, 0, 1, 0.5f);
            glVertex3f(p1->x, p1->y, p1->z + 0.15f + (terrainHeights[indices1[1] + (index)+row] * height_calc));

            glColor4f(0, 0, 1, 0.5f);
            glVertex3f(p2->x, p2->y, p2->z + 0.15f + (terrainHeights[indices1[2] + (index)+row] * height_calc));

            glEnd();
        };
    };

    if (showWireframe == true)
    {
        // triangle 0
        st_vec3f* p0 = &polyPoints[indices0[0] + (index)+row];
        st_vec3f* p1 = &polyPoints[indices0[1] + (index)+row];
        st_vec3f* p2 = &polyPoints[indices0[2] + (index)+row];

        // line 0
        glBegin(GL_LINES);

        glColor3f(0, 0, 1);

        glVertex3f(p0->x, p0->y, p0->z + 0.15f + (terrainHeights[indices0[0] + (index)+row] * height_calc));
        glVertex3f(p1->x, p1->y, p1->z + 0.15f + (terrainHeights[indices0[1] + (index)+row] * height_calc));

        glEnd();

        // line 1
        glBegin(GL_LINES);

        glColor3f(0, 0, 1);

        glVertex3f(p1->x, p1->y, p1->z + 0.15f + (terrainHeights[indices0[1] + (index)+row] * height_calc));
        glVertex3f(p2->x, p2->y, p2->z + 0.15f + (terrainHeights[indices0[2] + (index)+row] * height_calc));

        glEnd();

        // line 2
        glBegin(GL_LINES);

        glColor3f(0, 0, 1);

        glVertex3f(p2->x, p2->y, p2->z + 0.15f + (terrainHeights[indices0[2] + (index)+row] * height_calc));
        glVertex3f(p0->x, p0->y, p0->z + 0.15f + (terrainHeights[indices0[0] + (index)+row] * height_calc));

        glEnd();

        // triangle 1
        p0 = &polyPoints[indices1[0] + (index)+row];
        p1 = &polyPoints[indices1[1] + (index)+row];
        p2 = &polyPoints[indices1[2] + (index)+row];

        // line 0
        glBegin(GL_LINES);

        glColor3f(0, 0, 1);

        glVertex3f(p0->x, p0->y, p0->z + 0.15f + (terrainHeights[indices1[0] + (index)+row] * height_calc));
        glVertex3f(p1->x, p1->y, p1->z + 0.15f + (terrainHeights[indices1[1] + (index)+row] * height_calc));

        glEnd();

        // line 1
        glBegin(GL_LINES);

        glColor3f(0, 0, 1);

        glVertex3f(p1->x, p1->y, p1->z + 0.15f + (terrainHeights[indices1[1] + (index)+row] * height_calc));
        glVertex3f(p2->x, p2->y, p2->z + 0.15f + (terrainHeights[indices1[2] + (index)+row] * height_calc));

        glEnd();

        // line 2
        glBegin(GL_LINES);

        glColor3f(0, 0, 1);

        glVertex3f(p2->x, p2->y, p2->z + 0.15f + (terrainHeights[indices1[2] + (index)+row] * height_calc));
        glVertex3f(p0->x, p0->y, p0->z + 0.15f + (terrainHeights[indices1[0] + (index)+row] * height_calc));

        glEnd();
    };

    glPopMatrix();



	// update terrain poly indices for mouse to hover over and convert 3d coords to 2d coords (triangle 0)
	terrain_poly_indices_0[0][index] = indices0[0] + (index)+row;
	terrain_poly_indices_0[1][index] = indices0[1] + (index)+row;
	terrain_poly_indices_0[2][index] = indices0[2] + (index)+row;

    // update terrain poly indices for mouse to hover over and convert 3d coords to 2d coords (triangle 1)
	terrain_poly_indices_1[0][index] = indices1[0] + (index)+row;
	terrain_poly_indices_1[1][index] = indices1[1] + (index)+row;
	terrain_poly_indices_1[2][index] = indices1[2] + (index)+row;



    // send current polygon data to the texture window so we can see where it lays in space
    if (current_square == index && (current_tri % 2)) // triangle 1
    {
        current_poly_indices[0] = (indices1[0] + (index) + row);
        current_poly_indices[1] = (indices1[1] + (index) + row);
        current_poly_indices[2] = (indices1[2] + (index) + row);

        //terrain_poly_indices[index][0] = (indices0[0] + (index)+row);
        //terrain_poly_indices[index][1] = (indices0[1] + (index)+row);
        //terrain_poly_indices[index][2] = (indices0[2] + (index)+row);
    }
    else if (current_square == index && !(current_tri % 2)) // triangle 0
    {
        current_poly_indices[0] = (indices0[0] + (index) + row);
        current_poly_indices[1] = (indices0[1] + (index) + row);
        current_poly_indices[2] = (indices0[2] + (index) + row);

        //terrain_poly_indices[index][0] = (indices1[0] + (index)+row);
        //terrain_poly_indices[index][1] = (indices1[1] + (index)+row);
        //terrain_poly_indices[index][2] = (indices1[2] + (index)+row);
    };



    //
    // triangle 1 (COLOR) (START)
    //
    /*glDisable(GL_TEXTURE_2D);
    glBegin(GL_TRIANGLES);

    if (showShadows == false)
    {
        glColor3f(terrainColor1.r + terrainClouds[indices1[0] + (index)+row] * 0.02f,
            terrainColor1.g + terrainClouds[indices1[0] + (index)+row] * 0.02f,
            terrainColor1.b + terrainClouds[indices1[0] + (index)+row] * 0.02f);
    }
    else
    {
        glColor3f(terrainColor1.r, terrainColor1.g, terrainColor1.b);
    };

    glVertex3f(polyPoints[indices1[0] + (index)+row].x,
        polyPoints[indices1[0] + (index)+row].y,
        polyPoints[indices1[0] + (index)+row].z - (0.1f) + (terrainHeights[indices1[0] + (index)+row] * height_calc));

    if (showShadows == false)
    {
        glColor3f(terrainColor1.r + terrainClouds[indices1[1] + (index)+row] * 0.02f,
            terrainColor1.g + terrainClouds[indices1[1] + (index)+row] * 0.02f,
            terrainColor1.b + terrainClouds[indices1[1] + (index)+row] * 0.02f);
    }
    else
    {
        glColor3f(terrainColor1.r, terrainColor1.g, terrainColor1.b);
    };

    glVertex3f(polyPoints[indices1[1] + (index)+row].x,
        polyPoints[indices1[1] + (index)+row].y,
        polyPoints[indices1[1] + (index)+row].z - (0.1f) + (terrainHeights[indices1[1] + (index)+row] * height_calc));

    if (showShadows == false)
    {
        glColor3f(terrainColor1.r + terrainClouds[indices1[2] + (index)+row] * 0.02f,
            terrainColor1.g + terrainClouds[indices1[2] + (index)+row] * 0.02f,
            terrainColor1.b + terrainClouds[indices1[2] + (index)+row] * 0.02f);
    }
    else
    {
        glColor3f(terrainColor1.r, terrainColor1.g, terrainColor1.b);
    };

    glVertex3f(polyPoints[indices1[2] + (index)+row].x,
        polyPoints[indices1[2] + (index)+row].y,
        polyPoints[indices1[2] + (index)+row].z - (0.1f) + (terrainHeights[indices1[2] + (index)+row] * height_calc));

    glEnd();
    glEnable(GL_TEXTURE_2D);*/
    //
    // triangle 1 (COLOR) (END)
    //



    glDisable(GL_TEXTURE_2D);

    glColor3f(1, 1, 1);

    // draw line segments at highlighted triangle
    
    // highlight current triangle in current square (triangle 0)
    if (current_square == index && !(current_tri % 2))
    {
        glPushMatrix();

        glBegin(GL_LINES);

        // line 0
        glColor3f(1, 1, 1);
        glVertex3f(polyPoints[indices0[0] + (index)+row].x,
            polyPoints[indices0[0] + (index)+row].y,
            polyPoints[indices0[0] + (index)+row].z + (terrainHeights[indices0[0] + (index)+row] * height_calc) + 0.15f);
        glColor3f(1, 1, 1);
        glVertex3f(polyPoints[indices0[1] + (index)+row].x,
            polyPoints[indices0[1] + (index)+row].y,
            polyPoints[indices0[1] + (index)+row].z + (terrainHeights[indices0[1] + (index)+row] * height_calc) + 0.15f);

        // line 1
        glColor3f(1, 1, 1);
        glVertex3f(polyPoints[indices0[1] + (index)+row].x,
            polyPoints[indices0[1] + (index)+row].y,
            polyPoints[indices0[1] + (index)+row].z + (terrainHeights[indices0[1] + (index)+row] * height_calc) + 0.15f);
        glColor3f(1, 1, 1);
        glVertex3f(polyPoints[indices0[2] + (index)+row].x,
            polyPoints[indices0[2] + (index)+row].y,
            polyPoints[indices0[2] + (index)+row].z + (terrainHeights[indices0[2] + (index)+row] * height_calc) + 0.15f);

        // line 2
        glColor3f(1, 1, 1);
        glVertex3f(polyPoints[indices0[2] + (index)+row].x,
            polyPoints[indices0[2] + (index)+row].y,
            polyPoints[indices0[2] + (index)+row].z + (terrainHeights[indices0[2] + (index)+row] * height_calc) + 0.15f);
        glColor3f(1, 1, 1);
        glVertex3f(polyPoints[indices0[0] + (index)+row].x,
            polyPoints[indices0[0] + (index)+row].y,
            polyPoints[indices0[0] + (index)+row].z + (terrainHeights[indices0[0] + (index)+row] * height_calc) + 0.15f);

        glEnd();

        glPopMatrix();
    };

    // highlight current triangle in current square (triangle 1)
    if (current_square == index && (current_tri % 2))
    {
        glPushMatrix();

        glBegin(GL_LINES);

        // line 0
        glColor3f(1, 1, 1);
        glVertex3f(polyPoints[indices1[0] + (index)+row].x,
            polyPoints[indices1[0] + (index)+row].y,
            polyPoints[indices1[0] + (index)+row].z + (terrainHeights[indices1[0] + (index)+row] * height_calc) + 0.15f);
        glColor3f(1, 1, 1);
        glVertex3f(polyPoints[indices1[1] + (index)+row].x,
            polyPoints[indices1[1] + (index)+row].y,
            polyPoints[indices1[1] + (index)+row].z + (terrainHeights[indices1[1] + (index)+row] * height_calc) + 0.15f);

        // line 1
        glColor3f(1, 1, 1);
        glVertex3f(polyPoints[indices1[1] + (index)+row].x,
            polyPoints[indices1[1] + (index)+row].y,
            polyPoints[indices1[1] + (index)+row].z + (terrainHeights[indices1[1] + (index)+row] * height_calc) + 0.15f);
        glColor3f(1, 1, 1);
        glVertex3f(polyPoints[indices1[2] + (index)+row].x,
            polyPoints[indices1[2] + (index)+row].y,
            polyPoints[indices1[2] + (index)+row].z + (terrainHeights[indices1[2] + (index)+row] * height_calc) + 0.15f);

        // line 2
        glColor3f(1, 1, 1);
        glVertex3f(polyPoints[indices1[2] + (index)+row].x,
            polyPoints[indices1[2] + (index)+row].y,
            polyPoints[indices1[2] + (index)+row].z + (terrainHeights[indices1[2] + (index)+row] * height_calc) + 0.15f);
        glColor3f(1, 1, 1);
        glVertex3f(polyPoints[indices1[0] + (index)+row].x,
            polyPoints[indices1[0] + (index)+row].y,
            polyPoints[indices1[0] + (index)+row].z + (terrainHeights[indices1[0] + (index)+row] * height_calc) + 0.15f);

        glEnd();

        glPopMatrix();
    };

    glEnable(GL_TEXTURE_2D);
};