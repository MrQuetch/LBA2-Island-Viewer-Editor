#pragma once

//#ifndef _DRAWER_H_
//#define _DRAWER_H_

/*void terrainQuad2(float x, float y, int index, int type);
void terrainQuad(float x, float y, st_color3f* col, unsigned char* clouds, short* heights);
void terrainStripQuads(int row, float x);
void terrainStripNew(int row, int start);
void terrainStrip(int start, int end);
void terrainStrip1();*/

void convert_3d_coord_to_2d(GLdouble model[16], GLdouble proj[16], GLint viewport[4], float winW, float winH, float aspect, st_vec3f* vec_3d, st_vec2f* vec_2d);
bool coord_2d_is_visible(st_vec2f p, float aspect);

float cross(st_vec2f a, st_vec2f b, st_vec2f c);
bool pointInTriangle(st_vec2f p, st_vec2f a, st_vec2f b, st_vec2f c);
bool pointInTriangle_2(st_vec2f p, st_vec2f a, st_vec2f b, st_vec2f c);

st_vec3f cross_2(st_vec3f a, st_vec3f b);
st_vec3f normalize(st_vec3f v);
st_vec3f triangleNormal(st_vec3f A, st_vec3f B, st_vec3f C);

void scale_poly_texture(T_HALF_TEX* poly);

float dot(st_vec3f a, st_vec3f b);

float distance3D(float x1, float y1, float z1, float x2, float y2, float z2);

void interpolatePoints(st_vec2f a, st_vec2f b, st_vec2f* out, int count);
void interpolatePoints_short(st_short2 a, st_short2 b, st_short2* out, int count);

unsigned char getSmallestInArray(st_float_data values[], int length);

int int_min(int a, int b);
int int_max(int a, int b);
int int_clamp(int value, int min_val, int max_val);
int snap_down(int value, int grid);
int snap_up(int value, int grid);

st_vec3f vertex_rotate_x(st_vec3f v, float theta);
st_vec3f vertex_rotate_y(st_vec3f v, float theta);
st_vec3f vertex_rotate_z(st_vec3f v, float theta);

void vertex_rotate_2d(st_vec2f* p, st_vec2f center, float angle);
void vertex_rotate_2d_short(st_short2* p, st_short2 center, float angle);

void init_window(st_window* window, float x, float y, float size_x, float size_y, const char* text);
void draw_window(st_window* window);

bool mouse_in_window(st_window* window, float x, float y);

void init_button(st_button* button, float x, float y, float size_x, float size_y, const char* text);
void clamp_button_old(st_button* button, float screen_w, float screen_h, int clamp_type);
void clamp_button(st_button* button, float screen_w, float screen_h, int clamp_type, float offset_x, float offset_y);
void draw_button(st_button* button);

void draw_button_color(st_button* button, st_color3* color);

void draw_handle(st_handle* handle, float scale);

int isAllDigits(const char* s);

void text_field_handle_backspace(st_button* button);
void text_field_handle_enter(st_button* button, int void_types, void* data);
void text_field_handle_any_key(st_button* button, int key);

bool mouse_in_button(st_button* button, float x, float y);
bool mouse_in_handle(st_handle* handle, float x, float y, float scale);
bool mouse_in_rect(st_vec2f p0, st_vec2f p1, float x, float y);
bool point_in_rect_both_checks(st_vec2f cursor, st_vec2f p0, st_vec2f p1);

void draw_building(st_building* building);

void draw_axis(st_vec3f vec, unsigned char type);

void draw_model_poly_group(st_model* model, int index);
void draw_model(st_model* model, st_building* building);

void draw_selected_poly_odd();
void draw_selected_poly_even();
void draw_selected_poly_offset_odd(int index);
void draw_selected_poly_offset_even(int index);

void draw_brush(int brush_size);

void copy_height_data_3(short* ptr);
void paste_height_data_3(short* ptr);

void copy_height_data_4(short* ptr);
void paste_height_data_4(short* ptr);

void copy_height_data_5(short* ptr);
void paste_height_data_5(short* ptr);

void copy_poly_data_4(T_HALF_POLY* ptr);
void paste_poly_data_4();

void copy_poly_data_9(T_HALF_POLY* ptr);
void paste_poly_data_9();

void copy_poly_data_16(T_HALF_POLY* ptr);
void paste_poly_data_16();

void draw_terrain_textures();

// the row variable exists so it doesn't alter the drawing of polygons while still keeping the original indices intact
void draw_terrain_quad(int current_square, int current_tri, int index, int row, int texture_index);

//#endif // _DRAWER_H_