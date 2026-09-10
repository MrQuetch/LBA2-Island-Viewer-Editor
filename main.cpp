// Useful links:
// https://github.com/2point21/lba2-classic/blob/main/SOURCES/3DEXT/LBA_EXT.H

// https://stackoverflow.com/questions/3809044/how-many-values-can-be-represented-with-n-bits
// https://stackoverflow.com/questions/47981/how-to-set-clear-and-toggle-a-single-bit
// https://stackoverflow.com/questions/1044654/bitfield-manipulation-in-c
// https://www.reddit.com/r/C_Programming/comments/5hczva/integer_in_c_and_how_to_print_its_binary/



#define _CRT_SECURE_NO_WARNINGS

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
#include <windows.h>

#include "macros.h"
#include "structs.h"
#include "loader.h"
#include "drawer.h"
#include "text.h"

//#include "textures.h"

//#include "texture_concrete_1.h"

const char* bankStrings[] =
{
    "PURPLE_0",
    "BROWN",
    "TAN",
    "WHITE",
    "RED",
    "ORANGE",
    "YELLOW",
    "ARMY_GREEN",
    "GREEN",
    "BLUE_0",
    "BLUE_1",
    "BLUE_2",
    "BLUE_3",
    "PURPLE_1",
    "PURPLE_2",
    "FIRE",
};

const char* poly_colors_texts[] =
{
    "POLY_COLOR_PURPLE_0",
    "POLY_COLOR_BROWN",
    "POLY_COLOR_TAN",
    "POLY_COLOR_WHITE",
    "POLY_COLOR_RED",
    "POLY_COLOR_ORANGE",
    "POLY_COLOR_YELLOW",
    "POLY_COLOR_ARMY_GREEN",
    "POLY_COLOR_GREEN",
    "POLY_COLOR_BLUE_GREEN",
    "POLY_COLOR_CYAN",
    "POLY_COLOR_GRAY",
    "POLY_COLOR_BLUE",
    "POLY_COLOR_PURPLE_1",
};

const char* sample_steps_texts[] =
{
    "SAMPLE_STEP_NONE",
    "SAMPLE_STEP_1",
    "SAMPLE_STEP_GRASS",
    "SAMPLE_STEP_3",
    "SAMPLE_STEP_DIRT",
    "SAMPLE_STEP_STONE",
    "SAMPLE_STEP_6",
    "SAMPLE_STEP_7",
    "SAMPLE_STEP_8",
    "SAMPLE_STEP_FLOWERS",
    "SAMPLE_STEP_10",
    "SAMPLE_STEP_WATER",
    "SAMPLE_STEP_SAND",
    "SAMPLE_STEP_13",
    "SAMPLE_STEP_14",
    "SAMPLE_STEP_15",
    "SAMPLE_STEP_16",
};

const char* code_jeu_texts[] =
{
    "CJ_NONE",
    "CJ_WATER",
    "CJ_ELECTRIC",
    "CJ_ESCALATOR_OUEST",
    "CJ_ESCALATOR_EST",
    "CJ_ESCALATOR_NORD",
    "CJ_ESCALATOR_SUD",
    "CJ_LABYRINTHE",
    "CJ_POS_INVALID",
    "CJ_LAVE",
    "CJ_GAZ",
    "CJ_FOOT_WATER",
    "CJ_ANIMATED_LAVE",
    "CJ_ANIMATED_GAZ",
};

const char* oneBitStrings[] =
{
    "NO",
    "YES",
};

typedef enum
{
    MENU_MAIN,
    MENU_EDIT_ISLAND,
    MENU_EDIT_HEIGHT,
    MENU_EDIT_POLYS,
    MENU_EDIT_BUILDINGS,
    MENU_EDIT_TEXTURES,
    MENU_EDIT_SCENE,
    MENU_SHOW,
} en_menu;

unsigned char current_menu = MENU_MAIN;

st_camera theCamera = {
    -15, -32, 15, // pos
    0, 0, 0, // look
    0, 0, 1, // up vec
    25, 25, 25, // lerp vec
    0, // rot lerp
    0, 15, // rot, dist
    0, // speed
    false, // lerp
    true, // center
};

unsigned char island_parts;

st_object_info object_info;
unsigned char numBuildings;
st_building buildings[255];

int current_building;

st_model models[255];

T_HALF_POLY terrainPolys[MAX_POLYS];
T_HALF_TEX  terrainTexs[MAX_POLYS];
unsigned int terrain_poly_indices_0[3][MAX_POLYS];
unsigned int terrain_poly_indices_1[3][MAX_POLYS];
//unsigned int terrain_poly_indices[MAX_POLYS][3];

// this code is here to color the terrain quickly
st_vec3f terrainNormals[MAX_VERTS];
float terrainBrightness[MAX_VERTS];

st_handle poly_handles[MAX_VERTS];

unsigned char poly_tex_flags[] = { 0, 3 };

T_HALF_POLY poly_buffer; // to be used for copying and pasting polygon data

T_HALF_POLY poly_buffer_size_4[8]; // 4 quads -> 8 triangles
T_HALF_POLY poly_buffer_size_9[18]; //  9 quads -> 18 triangles
T_HALF_POLY poly_buffer_size_16[32]; // 16 quads -> 32 triangles

short terrain_heights_buffer_size_3[9]; // 4 quads -> 9 vertices
short terrain_heights_buffer_size_4[16]; // 9 quads -> 16 vertices
short terrain_heights_buffer_size_5[25]; // 16 quads -> 25 vertices

short terrainHeights[MAX_VERTS];
unsigned char terrainClouds[MAX_VERTS];

//T_HALF_TEX *current_poly_tex;
T_HALF_TEX current_poly_tex;
int current_poly_indices[4] = {0, 0, 0, 0};

//int terrainPolysAsInt[MAX_POLYS];
int numTerrainTexs;
int current_terrain_tex = 0;
int currentPoly = 128*1;
//int currentPoly = 0;
int currentRow = 0;
int currentColumn = 0;
int currentSquare = 0;

int selectedPoly = -1;

st_vec3f polyPoints[MAX_VERTS];

st_pal LBA2Palette;

st_lim groundTextures;
st_lim skyTextures;
st_lim buildingTextures;
st_lim objectTextures;

int brush_size = 1;



// mouse vars
float x_gl, y_gl;
float prev_mouse_x, prev_mouse_y;
bool mouse_held;

// mouse vars for textures
int drag_start_tx;
int drag_start_ty;

float drag_start_x_gl;
float drag_start_y_gl;



bool selection_box_visible = false;
st_vec2f selection_box_p0;
st_vec2f selection_box_p1;
bool all_verts_unselected = false;

typedef enum
{
    POLY_COPY,
    POLY_PASTE,
    POLY_FLIP,
    POLY_COLLISION,
    POLY_USE_COLOR,
    POLY_USE_TEX,
} en_polygon_operations;

const char* polygon_operations_texts[] =
{
    "Copy",
    "Paste",
    "Flip",
    "Collide",
    "Use Color",
    "Use Tex",
};

typedef enum
{
    BUILDING_COPY,
    BUILDING_PASTE,
    BUILDING_BOUND, // recalculate bounding box
    BUILDING_ALIGN, // align to ground
    BUILDING_SNAP, // snap to vertex
    BUILDING_ROTATE,
    BUILDING_INDEX,
} en_building_operations;

const char* building_operations_texts[] =
{
    "Copy",
    "Paste",
    "Bound", // recalculate bounding box
    "Align", // align to ground
    "Snap", // snap to vertex
    "Rotate",
    "Index",
};

typedef enum
{
    TERRAIN_CALC_SHADE,
    TERRAIN_RESET_SHADE,
    TERRAIN_RESET_HEIGHT,
} en_terrain_operations;

const char* terrain_operations_texts[] =
{
    "Calc Shade",
    "Reset Shade",
    "Reset Height",
};

const char* button_texture_rectangles_texts[] =
{
    "set texture rect",
    "add texture",
    "delete texture"
};

st_button button_main[11];
st_button button_edit_island[256];
st_button button_tabs[8];
st_button button_text_field;
st_button button_text_field_textures[8];
st_button button_texture_rectangles[3];
st_button button_polygon_brush_text_field;
st_button button_polygon_reset_all;
st_button button_polygon_colors[POLY_COLOR_PURPLE_1];
st_button button_polygon_sample[SAMPLE_STEP_16];
st_button button_polygon_code_jeu[CJ_ANIMATED_GAZ];
st_button button_polygon_operations[6];
st_button button_building_operations[7];
st_button button_terrain_operations[3];
st_button button_palette[256];
bool display_text_error;
bool display_loading_message;

st_handle handles[3]; // texture points
st_handle handles_textures[2] = { {-0.5f, -0.5f}, {0.5f, 0.5f } }; // rectangle points
bool create_texture_rectangle = false;
short texture_rect_x[2] = { -4096, 4964 };
short texture_rect_y[2] = { -4096, 4096 };
int num_texture_points_rect = 3;
st_vec2f texture_points_rect[3];
st_short2 texture_points_rect_short[3];
float edit_tex_scale = 1.0f;
float edit_tex_slider = 0.0f;
float tex_margin = 0.188f;
bool use_snap = false;
short x_grid = 1;
short y_grid = 1;

st_window window_palette;

bool loadOnce;

bool centerPoly = true;

bool editPolys = false;
bool editBuildings = false;

unsigned char current_axis = AXIS_X_Y;
unsigned char last_axis = AXIS_NONE;

//bool showInfo = false;

int tx = 0;
int ty = 0;

bool showHeights = true;
bool showTextures = true;
bool showShadows = true;
bool showCollisions = false;
bool showBuildings = false;
bool showWireframe = false;
bool showPalette = false;

typedef enum
{
    BUTTON_MAIN_EDIT_ISLAND,
    BUTTON_MAIN_EDIT_HEIGHT,
    BUTTON_MAIN_EDIT_POLYS,
    BUTTON_MAIN_EDIT_BUILDINGS,
    BUTTON_MAIN_EDIT_TEXTURES,
    BUTTON_MAIN_EDIT_SCENE,
    BUTTON_MAIN_SHOW_INFO,
    BUTTON_MAIN_LOAD_HQR,
    BUTTON_MAIN_SAVE_HQR,
    BUTTON_MAIN_LOAD_OBL,
    BUTTON_MAIN_SAVE_OBL,
} en_buttons_main;

const char* button_main_texts[] =
{
    "Edit Island",
    "Edit Height",
    "Edit Polys",
    "Edit Buildings",
    "Edit Textures",
    "Edit Scene",
    "Show Info",
    "Load HQR",
    "Save HQR",
    "Load OBL",
    "Save OBL",
};

typedef enum
{
    BUTTON_SHOW_HEIGHTS,
    BUTTON_SHOW_TEXTURES,
    BUTTON_SHOW_SHADOWS,
    BUTTON_SHOW_COLLISIONS,
    BUTTON_SHOW_BUILDINGS,
    BUTTON_SHOW_WIREFRAMES,
    BUTTON_SHOW_PALETTE,
    BUTTON_SAVE_SEA_INFO,
} en_buttons;

const char* button_texts[] =
{
    "Show Heights",
    "Show Textures",
    "Show Shadows",
    "Show Collisions",
    "Show Buildings",
    "Show Wireframes",
    "Show Palette",
    "Save Sea Info",
};

bool showPolysOfSampleType; // sound effect
bool showPolysOfCodeType;
bool showPolysOfTextureIndex;

st_button_presses button_presses;

//unsigned char escape_pressed_times = 0;


float lerp(float a, float b, float f)
{
    return a + f * (b - a);
};

float degToRad(float value)
{
    return ((M_PI / 180.0f) * value);
};

float radToDeg(float value)
{
    return (value * (180.0f / M_PI));
};

void binprintf(int v)
{
    unsigned int mask = 1 << ((sizeof(int) << 3) - 1);

    while (mask)
    {
        printf("%d", (v & mask ? 1 : 0));
        mask >>= 1;
    };
};

int bitwiseORTerrain(T_HALF_POLY *thePoly)
{
    return
        (
        (thePoly->Bank << BIT_POLY_BANK) |
        (thePoly->TexFlag << BIT_POLY_TEX_FLAG) |
        (thePoly->PolyFlag << BIT_POLY_POLY_FLAG) |
        (thePoly->SampleStep << BIT_POLY_SAMPLE_STEP) |
        (thePoly->CodeJeu << BIT_POLY_CODE_JEU) |
        (thePoly->Sens << BIT_POLY_SENS) |
        (thePoly->Col << BIT_POLY_COL) |
        (thePoly->Dummy << BIT_POLY_DUMMY) |
        (thePoly->IndexTex << BIT_POLY_INDEX_TEX)
        );
};

float convertColorToFloat(unsigned char value)
{
    return (float)(value / 255);
};

void handle_texture_drag(int tx, int ty)
{
    if (current_axis != last_axis)
    {
        drag_start_tx = tx;
        drag_start_ty = ty;

        drag_start_x_gl = x_gl;
        drag_start_y_gl = y_gl;

        last_axis = current_axis;
    };
};

void MyKeyboardSpecial(unsigned char Key, int x, int y)
{
    // use this for getting special keys such as alt, ctrl and shift
    int modifiers = glutGetModifiers();

    //printf("%d \n", Key);

    if (modifiers & GLUT_ACTIVE_CTRL)
    {
        if (Key == 'c')
            printf("%d \n", Key);
    };
};

void MyKeyboardFunc(unsigned char Key, int x, int y)
{
    //printf("%d \n", Key);

    if (current_menu == MENU_EDIT_ISLAND)
    {
        for (int i = 0; i < MAX_ISLAND_PARTS; i++)
        {
            if (button_edit_island[i].is_text_field == true && button_edit_island[i].is_focused == true)
            {
                if (Key == 8) // backspace
                {
                    text_field_handle_backspace(&button_edit_island[i]);
                }
                else if (Key == 13) // enter key
                {
                    if (isAllDigits((const char*)button_edit_island[i].text_field_buffer))
                    {
                        // get the string from the text field and convert it to an integer
                        int value = (int)strtol((char*)button_edit_island[i].text_field_buffer, NULL, 10);

                        //brush_size = value;
                    };
                }
                else // any other character
                {
                    text_field_handle_any_key(&button_edit_island[i], Key);
                };
            };
        };
    };

    if (current_menu == MENU_EDIT_POLYS)
    {
        // text field handling
        if (button_text_field.is_text_field == true && button_text_field.is_focused == true)
        {
            // GLFW_KEY_BACKSPACE returns a different code than the one expected
            // for some reason it is index 8 and not 259
            //printf("%d \n", Key);

            if (Key == 8) // backspace
            {
                text_field_handle_backspace(&button_text_field);
            }
            else if (Key == 13) // enter key
            {
                if (isAllDigits((const char*)button_text_field.text_field_buffer))
                {
                    // get the string from the text field and convert it to an integer
                    int value = (int)strtol((char*)button_text_field.text_field_buffer, NULL, 10);

                    terrainPolys[selectedPoly].IndexTex = value;

                    display_text_error = false;
                }
                else
                {
                    display_text_error = true;
                };

                //text_field_handle_enter(&button_text_field, VOID_TYPE_INT, (void*)terrainHeights[current_poly_indices[0]]);
                //text_field_handle_enter(&button_text_field, VOID_TYPE_INT, (void*)terrainHeights[current_poly_indices[1]]);
                //text_field_handle_enter(&button_text_field, VOID_TYPE_INT, (void*)terrainHeights[current_poly_indices[2]]);

                //text_field_handle_enter(&button_text_field, VOID_TYPE_INT, (void*)terrainHeights[0]);
                //text_field_handle_enter(&button_text_field, VOID_TYPE_INT, (void*)terrainHeights[1]);
                //text_field_handle_enter(&button_text_field, VOID_TYPE_INT, (void*)terrainHeights[2]);

                /*if (isAllDigits((const char*)button_text_field.text_field_buffer))
                {
                    // get the string from the text field and convert it to an integer
                    int value = (int)strtol((char*)button_text_field.text_field_buffer, NULL, 10);

                    terrainHeights[current_poly_indices[0]] = value;
                    terrainHeights[current_poly_indices[1]] = value;
                    terrainHeights[current_poly_indices[2]] = value;

                    display_text_error = false;
                }
                else
                {
                    display_text_error = true;
                };*/
            }
            else // any other character
            {
                text_field_handle_any_key(&button_text_field, Key);
            };
        };

        if (button_polygon_brush_text_field.is_text_field == true && button_polygon_brush_text_field.is_focused == true)
        {
            if (Key == 8) // backspace
            {
                text_field_handle_backspace(&button_polygon_brush_text_field);
            }
            else if (Key == 13) // enter key
            {
                if (isAllDigits((const char*)button_polygon_brush_text_field.text_field_buffer))
                {
                    // get the string from the text field and convert it to an integer
                    int value = (int)strtol((char*)button_polygon_brush_text_field.text_field_buffer, NULL, 10);

                    brush_size = value;
                };
            }
            else // any other character
            {
                text_field_handle_any_key(&button_polygon_brush_text_field, Key);
            };
        };
    };

    if (current_menu == MENU_EDIT_TEXTURES)
    {
        if (Key == 13) // enter key
        {
            if (create_texture_rectangle == true)
            {
                if (num_texture_points_rect == 3)
                {
                    //
                    // first row
                    //
                    for (int i = 0; i < 4; i++)
                    {
                        //
                        // first triangle
                        //

						if (i == 0)
							terrainTexs[numTerrainTexs].Tx0 = texture_rect_x[0];
						else
							terrainTexs[numTerrainTexs].Tx0 = texture_points_rect_short[i - 1].x;

                        // Y remains unchanged
						terrainTexs[numTerrainTexs].Ty0 = texture_rect_y[0];

                        if (i != 3)
                            terrainTexs[numTerrainTexs].Tx1 = texture_points_rect_short[i].x;
                        else
                            terrainTexs[numTerrainTexs].Tx1 = texture_rect_x[1];

                        // Y remains unchanged
						terrainTexs[numTerrainTexs].Ty1 = texture_rect_y[0];

                        if (i != 3)
                            terrainTexs[numTerrainTexs].Tx2 = texture_points_rect_short[i].x;
                        else
                            terrainTexs[numTerrainTexs].Tx2 = texture_rect_x[1];

                        // Y remains unchanged
						terrainTexs[numTerrainTexs].Ty2 = texture_points_rect_short[0].y;

                        // scale the polygon so we don't have to manually do it
                        scale_poly_texture(&terrainTexs[numTerrainTexs]);

                        numTerrainTexs += 1;

                        //
                        // second triangle
                        //
                        if (i == 0)
                            terrainTexs[numTerrainTexs].Tx0 = texture_rect_x[0];
                        else
                            terrainTexs[numTerrainTexs].Tx0 = texture_points_rect_short[i - 1].x;

                        // Y remains unchanged
                        terrainTexs[numTerrainTexs].Ty0 = texture_rect_y[0];

                        if (i == 0)
                            terrainTexs[numTerrainTexs].Tx1 = texture_rect_x[0]; 
                        else
                            terrainTexs[numTerrainTexs].Tx1 = texture_points_rect_short[i - 1].x;

                        // Y remains unchanged
                        terrainTexs[numTerrainTexs].Ty1 = texture_points_rect_short[0].y;

                        if (i != 3)
                            terrainTexs[numTerrainTexs].Tx2 = texture_points_rect_short[i].x;
                        else
                            terrainTexs[numTerrainTexs].Tx2 = texture_rect_x[1];

                        // Y remains unchanged
                        terrainTexs[numTerrainTexs].Ty2 = texture_points_rect_short[0].y;

                        // scale the polygon so we don't have to manually do it
                        scale_poly_texture(&terrainTexs[numTerrainTexs]);

                        numTerrainTexs += 1;
                    };

                    //
                    // second row
                    //
                    for (int i = 0; i < 4; i++)
                    {
                        //
                        // first triangle
                        //

                        if (i == 0)
                            terrainTexs[numTerrainTexs].Tx0 = texture_rect_x[0];
                        else
                            terrainTexs[numTerrainTexs].Tx0 = texture_points_rect_short[i - 1].x;

                        // Y remains unchanged
                        terrainTexs[numTerrainTexs].Ty0 = texture_points_rect_short[0].y;

                        if (i != 3)
                            terrainTexs[numTerrainTexs].Tx1 = texture_points_rect_short[i].x;
                        else
                            terrainTexs[numTerrainTexs].Tx1 = texture_rect_x[1];

                        // Y remains unchanged
                        terrainTexs[numTerrainTexs].Ty1 = texture_points_rect_short[0].y;

                        if (i != 3)
                            terrainTexs[numTerrainTexs].Tx2 = texture_points_rect_short[i].x;
                        else
                            terrainTexs[numTerrainTexs].Tx2 = texture_rect_x[1];

                        // Y remains unchanged
                        terrainTexs[numTerrainTexs].Ty2 = texture_points_rect_short[1].y;

                        // scale the polygon so we don't have to manually do it
                        scale_poly_texture(&terrainTexs[numTerrainTexs]);

                        numTerrainTexs += 1;

                        //
                        // second triangle
                        //
                        if (i == 0)
                            terrainTexs[numTerrainTexs].Tx0 = texture_rect_x[0];
                        else
                            terrainTexs[numTerrainTexs].Tx0 = texture_points_rect_short[i - 1].x;

                        // Y remains unchanged
                        terrainTexs[numTerrainTexs].Ty0 = texture_points_rect_short[0].y;

                        if (i == 0)
                            terrainTexs[numTerrainTexs].Tx1 = texture_rect_x[0];
                        else
                            terrainTexs[numTerrainTexs].Tx1 = texture_points_rect_short[i - 1].x;

                        // Y remains unchanged
                        terrainTexs[numTerrainTexs].Ty1 = texture_points_rect_short[1].y;

                        if (i != 3)
                            terrainTexs[numTerrainTexs].Tx2 = texture_points_rect_short[i].x;
                        else
                            terrainTexs[numTerrainTexs].Tx2 = texture_rect_x[1];

                        // Y remains unchanged
                        terrainTexs[numTerrainTexs].Ty2 = texture_points_rect_short[1].y;

                        // scale the polygon so we don't have to manually do it
                        scale_poly_texture(&terrainTexs[numTerrainTexs]);

                        numTerrainTexs += 1;
                    };

                    //
                    // third row
                    //
                    for (int i = 0; i < 4; i++)
                    {
                        //
                        // first triangle
                        //

                        if (i == 0)
                            terrainTexs[numTerrainTexs].Tx0 = texture_rect_x[0];
                        else
                            terrainTexs[numTerrainTexs].Tx0 = texture_points_rect_short[i - 1].x;

                        // Y remains unchanged
                        terrainTexs[numTerrainTexs].Ty0 = texture_points_rect_short[1].y;

                        if (i != 3)
                            terrainTexs[numTerrainTexs].Tx1 = texture_points_rect_short[i].x;
                        else
                            terrainTexs[numTerrainTexs].Tx1 = texture_rect_x[1];

                        // Y remains unchanged
                        terrainTexs[numTerrainTexs].Ty1 = texture_points_rect_short[1].y;

                        if (i != 3)
                            terrainTexs[numTerrainTexs].Tx2 = texture_points_rect_short[i].x;
                        else
                            terrainTexs[numTerrainTexs].Tx2 = texture_rect_x[1];

                        // Y remains unchanged
                        terrainTexs[numTerrainTexs].Ty2 = texture_points_rect_short[2].y;

                        // scale the polygon so we don't have to manually do it
                        scale_poly_texture(&terrainTexs[numTerrainTexs]);

                        numTerrainTexs += 1;

                        //
                        // second triangle
                        //
                        if (i == 0)
                            terrainTexs[numTerrainTexs].Tx0 = texture_rect_x[0];
                        else
                            terrainTexs[numTerrainTexs].Tx0 = texture_points_rect_short[i - 1].x;

                        // Y remains unchanged
                        terrainTexs[numTerrainTexs].Ty0 = texture_points_rect_short[1].y;

                        if (i == 0)
                            terrainTexs[numTerrainTexs].Tx1 = texture_rect_x[0];
                        else
                            terrainTexs[numTerrainTexs].Tx1 = texture_points_rect_short[i - 1].x;

                        // Y remains unchanged
                        terrainTexs[numTerrainTexs].Ty1 = texture_points_rect_short[2].y;

                        if (i != 3)
                            terrainTexs[numTerrainTexs].Tx2 = texture_points_rect_short[i].x;
                        else
                            terrainTexs[numTerrainTexs].Tx2 = texture_rect_x[1];

                        // Y remains unchanged
                        terrainTexs[numTerrainTexs].Ty2 = texture_points_rect_short[2].y;

                        // scale the polygon so we don't have to manually do it
                        scale_poly_texture(&terrainTexs[numTerrainTexs]);

                        numTerrainTexs += 1;
                    };

                    //
                    // fourth row (last row)
                    //
                    for (int i = 0; i < 4; i++)
                    {
                        //
                        // first triangle
                        //

                        if (i == 0)
                            terrainTexs[numTerrainTexs].Tx0 = texture_rect_x[0];
                        else
                            terrainTexs[numTerrainTexs].Tx0 = texture_points_rect_short[i - 1].x;

                        // Y remains unchanged
                        terrainTexs[numTerrainTexs].Ty0 = texture_points_rect_short[2].y;

                        if (i != 3)
                            terrainTexs[numTerrainTexs].Tx1 = texture_points_rect_short[i].x;
                        else
                            terrainTexs[numTerrainTexs].Tx1 = texture_rect_x[1];

                        // Y remains unchanged
                        terrainTexs[numTerrainTexs].Ty1 = texture_points_rect_short[2].y;

                        if (i != 3)
                            terrainTexs[numTerrainTexs].Tx2 = texture_points_rect_short[i].x;
                        else
                            terrainTexs[numTerrainTexs].Tx2 = texture_rect_x[1];

                        // Y remains unchanged
                        terrainTexs[numTerrainTexs].Ty2 = texture_rect_y[1];

                        // scale the polygon so we don't have to manually do it
                        scale_poly_texture(&terrainTexs[numTerrainTexs]);

                        numTerrainTexs += 1;

                        //
                        // second triangle
                        //
                        if (i == 0)
                            terrainTexs[numTerrainTexs].Tx0 = texture_rect_x[0];
                        else
                            terrainTexs[numTerrainTexs].Tx0 = texture_points_rect_short[i - 1].x;

                        // Y remains unchanged
                        terrainTexs[numTerrainTexs].Ty0 = texture_points_rect_short[2].y;

                        if (i == 0)
                            terrainTexs[numTerrainTexs].Tx1 = texture_rect_x[0];
                        else
                            terrainTexs[numTerrainTexs].Tx1 = texture_points_rect_short[i - 1].x;

                        // Y remains unchanged
                        terrainTexs[numTerrainTexs].Ty1 = texture_rect_y[1];

                        if (i != 3)
                            terrainTexs[numTerrainTexs].Tx2 = texture_points_rect_short[i].x;
                        else
                            terrainTexs[numTerrainTexs].Tx2 = texture_rect_x[1];

                        // Y remains unchanged
                        terrainTexs[numTerrainTexs].Ty2 = texture_rect_y[1];

                        // scale the polygon so we don't have to manually do it
                        scale_poly_texture(&terrainTexs[numTerrainTexs]);

                        numTerrainTexs += 1;
                    };

                    /*// get last terrain texture in array and increment

                    terrainTexs[numTerrainTexs].Tx0 = 0;
                    terrainTexs[numTerrainTexs].Ty0 = 0;

                    terrainTexs[numTerrainTexs].Tx1 = 4096;
                    terrainTexs[numTerrainTexs].Ty1 = 4096;

                    terrainTexs[numTerrainTexs].Tx2 = 0;
                    terrainTexs[numTerrainTexs].Ty2 = 4096;

                    numTerrainTexs += 1;*/
                };

                create_texture_rectangle = false;
            };
        };

        // text field handling for texture coordinates
        for (int i = 0; i < 8; i++)
        {
            // text field handling
            if (button_text_field_textures[i].is_text_field == true && button_text_field_textures[i].is_focused == true)
            {
                if (Key == 8) // backspace
                {
                    text_field_handle_backspace(&button_text_field_textures[i]);
                }
                else if (Key == 13) // enter key
                {
                    if (isAllDigits((const char*)button_text_field_textures[i].text_field_buffer))
                    {
                        // get the string from the text field and convert it to an integer
                        int value = (int)strtol((char*)button_text_field_textures[i].text_field_buffer, NULL, 10);

                        switch (i)
                        {
                        case 0:
                            terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx0 = value;
                            break;
                        case 1:
                            terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty0 = value;
                            break;

                        case 2:
                            terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx1 = value;
                            break;
                        case 3:
                            terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty1 = value;
                            break;

                        case 4:
                            terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx2 = value;
                            break;
                        case 5:
                            terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty2 = value;
                            break;

                        case 6: // x grid
                            x_grid = value;
                            break;
                        case 7: // y grid
                            y_grid = value;
                            break;
                        };

                        //display_text_error = false;
                    }

                    //switch (i)
                    //{
                    //case 0:
                        //text_field_handle_enter(&button_text_field_textures[i], VOID_TYPE_INT, (void*)terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx0);
                        //break;
                    //case 1:
                        //text_field_handle_enter(&button_text_field_textures[i], VOID_TYPE_INT, (void*)terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty0);
                        //break;
                    //};

                    /*text_field_handle_enter(&button_text_field_textures[i], VOID_TYPE_INT, (void*)terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx0);
                    text_field_handle_enter(&button_text_field_textures[i], VOID_TYPE_INT, (void*)terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty0);

                    text_field_handle_enter(&button_text_field_textures[i], VOID_TYPE_INT, (void*)terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx1);
                    text_field_handle_enter(&button_text_field_textures[i], VOID_TYPE_INT, (void*)terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty1);

                    text_field_handle_enter(&button_text_field_textures[i], VOID_TYPE_INT, (void*)terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx2);
                    text_field_handle_enter(&button_text_field_textures[i], VOID_TYPE_INT, (void*)terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty2);*/
                }
                else // any other character
                {
                    text_field_handle_any_key(&button_text_field_textures[i], Key);
                };
            };
        };
    };

    if (Key == 27) // escape
    {
        if (current_menu == MENU_EDIT_HEIGHT)
        {
            if (all_verts_unselected == true)
                current_menu = MENU_MAIN;

            if (selection_box_visible == false)
            {
                for (int i = 0; i < MAX_VERTS; i++)
                {
                    if (poly_handles[i].selected == true)
                    {
                        poly_handles[i].selected = false;

                        all_verts_unselected = true;
                    };
                };
            };

            selection_box_visible = false;
        }
        else if (current_menu == MENU_EDIT_TEXTURES)
        {
            edit_tex_scale = 3.0f;

            // check if any handles are selected first
            // this will keep us in the texture editor until all of them have been deselected

            if (handles[0].selected == false &&
                handles[1].selected == false &&
                handles[2].selected == false &&
                create_texture_rectangle == false)
                current_menu = MENU_MAIN;

            for (int i = 0; i < 3; i++)
            {
                handles[i].selected = false;
            };

            if (create_texture_rectangle == true)
                create_texture_rectangle = false;
        }
        else
        {
            current_menu = MENU_MAIN;
        };

        //current_menu = MENU_MAIN;
    };

    switch (Key)
    {
    case 'w':
        button_presses.pressed_w = true;
        break;

    case 'a':
        button_presses.pressed_a = true;
        break;

    case 's':
        button_presses.pressed_s = true;
        break;

    case 'd':
        button_presses.pressed_d = true;
        break;

    case 'q':
        button_presses.pressed_q = true;
        break;

    case 'e':
        button_presses.pressed_e = true;
        break;

    case 'g':
        button_presses.pressed_g = true;
        break;

    case 'x':
        button_presses.pressed_x = true;

        current_axis = AXIS_X;

        handle_texture_drag(tx, ty);

        //if (terrainPolys[currentPoly].IndexTex < numTerrainTexs)
            //terrainPolys[currentPoly].IndexTex += 1;
        //printf("%d IndexTex: %d \n", currentPoly, terrainPolys[currentPoly].IndexTex);

        break;
    case 'y':
        button_presses.pressed_y = true;

        current_axis = AXIS_Y;

        handle_texture_drag(tx, ty);
        break;
    case 'z':
        button_presses.pressed_z = true;

        current_axis = AXIS_Z;

        handle_texture_drag(tx, ty);

        //if (terrainPolys[currentPoly].IndexTex > 0)
            //terrainPolys[currentPoly].IndexTex -= 1;
        //printf("%d IndexTex: %d \n", currentPoly, terrainPolys[currentPoly].IndexTex);

        break;

    case 'c':
        current_axis = AXIS_X_Y;
        break;

    case 'v':
        break;

    case 'b':
        break;

    case 'k':
        if (current_menu == MENU_EDIT_TEXTURES)
        {
            if (terrainPolys[selectedPoly].IndexTex > 0)
                terrainPolys[selectedPoly].IndexTex -= 1;
        };
        break;

    case 'l':
        if (current_menu == MENU_EDIT_TEXTURES)
        {
            if (terrainPolys[selectedPoly].IndexTex < (numTerrainTexs - 1))
                terrainPolys[selectedPoly].IndexTex += 1;
        };
        break;

    case 'n':
        if (current_menu == MENU_EDIT_TEXTURES)
        {
            use_snap = !use_snap;
        };
        break;

    case 'o':
        if (current_menu == MENU_EDIT_TEXTURES)
        {
            if (edit_tex_slider > 0.0f)
            {
                edit_tex_slider -= 0.1f;

                current_terrain_tex -= 1;
            };
        };
        break;

    case 'p':
        if (current_menu == MENU_EDIT_TEXTURES)
        {
            if (edit_tex_slider < (numTerrainTexs * 0.1f))
            {
                edit_tex_slider += 0.1f;

                current_terrain_tex += 1;
            };
        };
        break;

    case 'm':
        if (current_menu == MENU_EDIT_TEXTURES)
        {
            // scale triangle from center
            // this seems to solve the texture wrapping problem
            // (at least partly)
            st_short2 center =
            {
                (terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx0 +
                 terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx1 +
                 terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx2) / 3,
                (terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty0 +
                 terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty1 +
                 terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty2) / 3,
            };

            st_short2 p0 =
            {
                terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx0,
                terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty0,
            };

            st_short2 p1 =
            {
                terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx1,
                terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty1,
            };

            st_short2 p2 =
            {
                terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx2,
                terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty2,
            };

            // apply new scaling to triangle
            terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx0 = (center.x + (p0.x - center.x) * 0.95);
            terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty0 = (center.y + (p0.y - center.y) * 0.95);

            terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx1 = (center.x + (p1.x - center.x) * 0.95);
            terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty1 = (center.y + (p1.y - center.y) * 0.95);

            terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx2 = (center.x + (p2.x - center.x) * 0.95);
            terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty2 = (center.y + (p2.y - center.y) * 0.95);

            // scale the polygon so we don't have to manually do it
            //scale_poly_texture(&terrainTexs[terrainPolys[selectedPoly].IndexTex]);
        };
        break;

    case '1':
        button_presses.pressed_1 = true;

        if (current_menu == MENU_EDIT_HEIGHT)
        {
            for (int i = 0; i < MAX_VERTS; i++)
            {
                if (poly_handles[i].selected == true)
                {
                    terrainHeights[i] += 256;
                };
            };
        };

        if (current_menu == MENU_EDIT_TEXTURES)
        {
            /*
            // get data from the current texture
            T_HALF_TEX temp_tex =
            {
                terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx0,
                terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty0,

                terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx1,
                terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty1,

                terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx2,
                terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty2,
            };

            // flip the tex coords 0 and 1
            terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx0 = temp_tex.Tx1;
            terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty0 = temp_tex.Ty1;

            terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx1 = temp_tex.Tx0;
            terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty1 = temp_tex.Ty0;
            */
        };

        /*printf("currentPoly: %d, currentSquare: %d \n", currentPoly, currentSquare);

        if (currentPoly > 0)
        {
            //currentPoly -= 1;
            printf("[%d]Bank: %d, %s", currentPoly, terrainPolys[currentPoly].Bank, bankStrings[terrainPolys[currentPoly].Bank]); printf("\n");
            printf("[%d]TexFlag: %d", currentPoly, terrainPolys[currentPoly].TexFlag); printf("\n");
            printf("[%d]PolyFlag: %d", currentPoly, terrainPolys[currentPoly].PolyFlag); printf("\n");
            printf("[%d]SampleStep: %d, %s", currentPoly, terrainPolys[currentPoly].SampleStep, sample_steps_texts[terrainPolys[currentPoly].SampleStep]); printf("\n");
            printf("[%d]CodeJeu: %d, %s", currentPoly, terrainPolys[currentPoly].CodeJeu, code_jeu_texts[terrainPolys[currentPoly].CodeJeu]); printf("\n");
            printf("[%d]Sens: %d, %s", currentPoly, terrainPolys[currentPoly].Sens, oneBitStrings[terrainPolys[currentPoly].Sens]); printf("\n");
            printf("[%d]Col: %d, %s", currentPoly, terrainPolys[currentPoly].Col, oneBitStrings[terrainPolys[currentPoly].Col]); printf("\n");
            printf("[%d]Dummy: %d", currentPoly, terrainPolys[currentPoly].Dummy); printf("\n");
            printf("[%d]IndexTex: %d", currentPoly, terrainPolys[currentPoly].IndexTex); printf("\n");
        };*/
        break;

    case '2':
        if (current_menu == MENU_EDIT_HEIGHT)
        {
            for (int i = 0; i < MAX_VERTS; i++)
            {
                if (poly_handles[i].selected == true)
                {
                    if (terrainHeights[i] > 0)
                        terrainHeights[i] -= 256;
                    else
                        terrainHeights[i] = 0;
                };
            };
        };

        if (current_menu == MENU_EDIT_TEXTURES)
        {
            /*
            // get data from the current texture
            T_HALF_TEX temp_tex =
            {
                terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx0,
                terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty0,

                terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx1,
                terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty1,

                terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx2,
                terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty2,
            };

            // flip the tex coords 0 and 1
            terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx0 = temp_tex.Tx1;
            terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty0 = temp_tex.Ty1;

            terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx1 = temp_tex.Tx0;
            terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty1 = temp_tex.Ty0;
            */
        };

        /*printf("currentPoly: %d, currentSquare: %d \n", currentPoly, currentSquare);

        if (currentPoly < (128 * 64) - 1)
        {
            //currentPoly += 1;
            printf("[%d]Bank: %d, %s", currentPoly, terrainPolys[currentPoly].Bank, bankStrings[terrainPolys[currentPoly].Bank]); printf("\n");
            printf("[%d]TexFlag: %d", currentPoly, terrainPolys[currentPoly].TexFlag); printf("\n");
            printf("[%d]PolyFlag: %d", currentPoly, terrainPolys[currentPoly].PolyFlag); printf("\n");
            printf("[%d]SampleStep: %d, %s", currentPoly, terrainPolys[currentPoly].SampleStep, sample_steps_texts[terrainPolys[currentPoly].SampleStep]); printf("\n");
            printf("[%d]CodeJeu: %d, %s", currentPoly, terrainPolys[currentPoly].CodeJeu, code_jeu_texts[terrainPolys[currentPoly].CodeJeu]); printf("\n");
            printf("[%d]Sens: %d, %s", currentPoly, terrainPolys[currentPoly].Sens, oneBitStrings[terrainPolys[currentPoly].Sens]); printf("\n");
            printf("[%d]Col: %d, %s", currentPoly, terrainPolys[currentPoly].Col, oneBitStrings[terrainPolys[currentPoly].Col]); printf("\n");
            printf("[%d]Dummy: %d", currentPoly, terrainPolys[currentPoly].Dummy); printf("\n");
            printf("[%d]IndexTex: %d", currentPoly, terrainPolys[currentPoly].IndexTex); printf("\n");
        };*/
        break;

    case '3':
        button_presses.pressed_3 = true;
        break;

    case '4':
        break;

    case '5':
        if (current_menu == MENU_EDIT_HEIGHT)
        {
            for (int i = 0; i < MAX_VERTS; i++)
            {
                if (poly_handles[i].selected == true)
                {
                    if (terrainClouds[i] < 0xFF)
                        terrainClouds[i] += 0x01;
                    else
                        terrainClouds[i] = 0xFF;
                };
            };
        };

        // the center of the texture is actually 0,0 - not the center like in a normalized texture (0.5, 0.5)
        // -32767 is the left and 32767 is the right (same for down and up)

        /*terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx0 = -32767;
        terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty0 = -32767;

        terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx1 = 32767;
        terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty1 = -32767;

        terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx2 = 32767;
        terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty2 = 32767;*/

        //printf("%d \n", terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx0);
        break;

    case '6':
        if (current_menu == MENU_EDIT_HEIGHT)
        {
            for (int i = 0; i < MAX_VERTS; i++)
            {
                if (poly_handles[i].selected == true)
                {
                    if (terrainClouds[i] > 0)
                        terrainClouds[i] -= 0x01;
                    else
                        terrainClouds[i] = 0;
                };
            };
        };

        //showTextures = !showTextures;
        break;

    case '0':
        break;
    };
};

void MyKeyboardUpFunc(unsigned char Key, int x, int y)
{
    switch (Key)
    {
    case 'w':
        button_presses.pressed_w = false;
        break;

    case 'a':
        button_presses.pressed_a = false;
        break;

    case 's':
        button_presses.pressed_s = false;
        break;

    case 'd':
        button_presses.pressed_d = false;
        break;

    case 'q':
        button_presses.pressed_q = false;
        break;

    case 'e':
        button_presses.pressed_e = false;
        break;

    case 'g':
        button_presses.pressed_g = false;
        break;

    case 'x':
        button_presses.pressed_x = false;
        break;

    case 'y':
        button_presses.pressed_y = false;
        break;

    case 'z':
        button_presses.pressed_z = false;
        break;

    case '1':
        button_presses.pressed_1 = false;
        break;

    case '3':
        button_presses.pressed_3 = false;
        break;
    };
};

int mouse_x, mouse_y;

void mouseMotion(int x, int y)
{
    // x, y are mouse coordinates
    mouse_x = x;
    mouse_y = y;

    if (mouse_held == true)
    {
        //prev_mouse_x = window_palette.p0.x;
        //prev_mouse_y = window_palette.p0.y;
    }
    else
    {
        prev_mouse_x = x_gl;
        prev_mouse_y = y_gl;
    };
};

void mouseClick(int button, int state, int x, int y)
{
	if (current_menu == MENU_EDIT_TEXTURES)
	{
		//printf("button=%d state=%d\n", button, state);

		if (button == GLUT_LEFT_BUTTON)
		{
			for (int i = 0; i < 3; i++)
			{
				if (mouse_in_handle(&handles[i], x_gl, y_gl, 0.01f) == true)
				{
					handles[i].selected = !handles[i].selected;
				};
			};

            for (int i = 0; i < 2; i++)
            {
                if (mouse_in_handle(&handles_textures[i], x_gl, y_gl, 0.01f) == true)
                {
                    handles_textures[i].selected = !handles_textures[i].selected;
                };
            };
		};

		if (button == GLUT_MIDDLE_BUTTON)
		{
			edit_tex_scale += 0.1f;
		};

		if (button == 0x04)
		{
			edit_tex_scale -= 0.1f;
		};
	};

    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouse_held = true;
        }
        else
        {
            mouse_held = false;
        };
    };

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        //printf("left pressed \n");
        //showHeights = !showHeights;

        //st_vec2f mouse_pos = { x_gl, y_gl };
        /*if (selectedPoly != -1)
        {
            selectedPoly = currentPoly;
            printf("selected poly %d", currentPoly);
        };*/
    };
        

    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        if (current_menu == MENU_MAIN)
        {
            for (int i = 0; i < 11; i++)
            {
                if (mouse_in_button(&button_main[i], x_gl, y_gl) == true)
                {
                    switch (i)
                    {
                    case BUTTON_MAIN_EDIT_ISLAND:
                        current_menu = MENU_EDIT_ISLAND;
                        break;
                    case BUTTON_MAIN_EDIT_HEIGHT:
                        current_menu = MENU_EDIT_HEIGHT;
                        break;
                    case BUTTON_MAIN_EDIT_POLYS:
                        current_menu = MENU_EDIT_POLYS;
                        break;
                    case BUTTON_MAIN_EDIT_BUILDINGS:
                        current_menu = MENU_EDIT_BUILDINGS;
                        break;
                    case BUTTON_MAIN_EDIT_TEXTURES:
                        current_menu = MENU_EDIT_TEXTURES;
                        break;
                    case BUTTON_MAIN_EDIT_SCENE:
                        current_menu = MENU_EDIT_SCENE;
                        break;
                    case BUTTON_MAIN_SHOW_INFO:
                        current_menu = MENU_SHOW;
                        break;

                    case BUTTON_MAIN_SAVE_HQR:
                        saveHQR_Island("CITABAU.ILE");
                        //saveHQR_Island("MEDIEVAL.HQR");
                        break;

                    case BUTTON_MAIN_LOAD_OBL:
                        //glFlush();
                        //glutSwapBuffers();

                        //Sleep(300);
                        //display_loading_message = true;

                        // force flush and swap buffers
                        // this allows the message to actually be seen before loading the files
                        //glFlush();
                        //glutSwapBuffers();

                        // sleep for one milisecond before loading in the data
                        // this gives time for the loading message to appear
                        //Sleep(300);
                        display_loading_message = true;

                        //if (display_loading_message == true)
                        //{
                            /*for (int i = 0; i < 115; i++)
                            {
                                char buffer[100];

                                sprintf(buffer, "lm2/CITABAU%03d.lfn", i + 1);
                                loadLM2(buffer, &models[i]);
                                printf("loading %s \n", buffer);
                            };*/
                        //};

                        //display_loading_message = false;
                        break;
                    };
                };
            };
        };

        if (current_menu == MENU_EDIT_ISLAND)
        {
            for (int i = 0; i < MAX_ISLAND_PARTS; i++)
            {
                if (mouse_in_button(&button_edit_island[i], x_gl, y_gl) == true)
                {
                    if (button_edit_island[i].is_text_field == true)
                    {
                        button_edit_island[i].is_focused = !button_edit_island[i].is_focused;
                    };
                };
            };
        };

        if (current_menu == MENU_EDIT_HEIGHT)
        {
            st_vec3f light_dir = { 0.5f, 0.5f, 1.0f };

            for (int i = 0; i < 3; i++)
            {
                if (mouse_in_button(&button_terrain_operations[i], x_gl, y_gl) == true)
                {
                    switch (i)
                    {
                    case 0:
                        //
                        // perform shade calculation on vertices (triangle 0)
                        //
                        for (int j = 0; j < MAX_POLYS; j++)
                        {
                            // get the indices of the polygon
                            st_vec3f p0 =
                            {
                                polyPoints[terrain_poly_indices_0[0][j]].x,
                                polyPoints[terrain_poly_indices_0[0][j]].y,
                                polyPoints[terrain_poly_indices_0[0][j]].z + (terrainHeights[terrain_poly_indices_0[0][j]] * 0.002f),
                            };
                            st_vec3f p1 =
                            {
                                polyPoints[terrain_poly_indices_0[1][j]].x,
                                polyPoints[terrain_poly_indices_0[1][j]].y,
                                polyPoints[terrain_poly_indices_0[1][j]].z + (terrainHeights[terrain_poly_indices_0[1][j]] * 0.002f),
                            };
                            st_vec3f p2 =
                            {
                                polyPoints[terrain_poly_indices_0[2][j]].x,
                                polyPoints[terrain_poly_indices_0[2][j]].y,
                                polyPoints[terrain_poly_indices_0[2][j]].z + (terrainHeights[terrain_poly_indices_0[2][j]] * 0.002f),
                            };

                            // compute the normal with the triangle indices
                            st_vec3f norm = triangleNormal(p0, p1, p2);

                            terrainNormals[terrain_poly_indices_0[0][j]].x += norm.x;
                            terrainNormals[terrain_poly_indices_0[0][j]].y += norm.y;
                            terrainNormals[terrain_poly_indices_0[0][j]].z += norm.z;

                            terrainNormals[terrain_poly_indices_0[1][j]].x += norm.x;
                            terrainNormals[terrain_poly_indices_0[1][j]].y += norm.y;
                            terrainNormals[terrain_poly_indices_0[1][j]].z += norm.z;

                            terrainNormals[terrain_poly_indices_0[2][j]].x += norm.x;
                            terrainNormals[terrain_poly_indices_0[2][j]].y += norm.y;
                            terrainNormals[terrain_poly_indices_0[2][j]].z += norm.z;
                        };

                        for (int i = 0; i < MAX_VERTS; i++)
                        {
                            float brightness = dot(terrainNormals[i], light_dir);

                            if (brightness < 0.0f)
                                brightness = 0.0f;

                            //float ambient = 0.25f;
                            //terrainBrightness[i] = ambient + brightness * 0.75f;
                            terrainBrightness[i] = (brightness * 0.75f);
                            terrainClouds[i] = (short)(terrainBrightness[i] * 2);
                        };

                        //
                        // perform shade calculation on vertices (triangle 1)
                        //
                        for (int j = 0; j < MAX_POLYS; j++)
                        {
                            // get the indices of the polygon
                            st_vec3f p0 =
                            {
                                polyPoints[terrain_poly_indices_1[0][j]].x,
                                polyPoints[terrain_poly_indices_1[0][j]].y,
                                polyPoints[terrain_poly_indices_1[0][j]].z + (terrainHeights[terrain_poly_indices_1[0][j]] * 0.002f),
                            };
                            st_vec3f p1 =
                            {
                                polyPoints[terrain_poly_indices_1[1][j]].x,
                                polyPoints[terrain_poly_indices_1[1][j]].y,
                                polyPoints[terrain_poly_indices_1[1][j]].z + (terrainHeights[terrain_poly_indices_1[1][j]] * 0.002f),
                            };
                            st_vec3f p2 =
                            {
                                polyPoints[terrain_poly_indices_1[2][j]].x,
                                polyPoints[terrain_poly_indices_1[2][j]].y,
                                polyPoints[terrain_poly_indices_1[2][j]].z + (terrainHeights[terrain_poly_indices_1[2][j]] * 0.002f),
                            };

                            // compute the normal with the triangle indices
                            st_vec3f norm = triangleNormal(p0, p1, p2);

                            terrainNormals[terrain_poly_indices_1[0][j]].x += norm.x;
                            terrainNormals[terrain_poly_indices_1[0][j]].y += norm.y;
                            terrainNormals[terrain_poly_indices_1[0][j]].z += norm.z;

                            terrainNormals[terrain_poly_indices_1[1][j]].x += norm.x;
                            terrainNormals[terrain_poly_indices_1[1][j]].y += norm.y;
                            terrainNormals[terrain_poly_indices_1[1][j]].z += norm.z;

                            terrainNormals[terrain_poly_indices_1[2][j]].x += norm.x;
                            terrainNormals[terrain_poly_indices_1[2][j]].y += norm.y;
                            terrainNormals[terrain_poly_indices_1[2][j]].z += norm.z;
                        };

                        for (int i = 0; i < MAX_VERTS; i++)
                        {
                            float brightness = dot(terrainNormals[i], light_dir);

                            if (brightness < 0.0f)
                                brightness = 0.0f;

                            //float ambient = 0.25f;
                            //terrainBrightness[i] = ambient + brightness * 0.75f;
                            terrainBrightness[i] = (brightness * 0.75f);
                            terrainClouds[i] = (short)(terrainBrightness[i] * 2);
                        };

                        // perform shade calculation on vertices (triangle 1)
                        /*for (int j = 0; j < MAX_POLYS; j++)
                        {
                            // get the indices of the polygon
                            st_vec3f p0 =
                            {
                                polyPoints[terrain_poly_indices_1[0][j]].x,
                                polyPoints[terrain_poly_indices_1[0][j]].y,
                                polyPoints[terrain_poly_indices_1[0][j]].z + (terrainHeights[terrain_poly_indices_1[0][j]] * 0.002f),
                            };
                            st_vec3f p1 =
                            {
                                polyPoints[terrain_poly_indices_1[1][j]].x,
                                polyPoints[terrain_poly_indices_1[1][j]].y,
                                polyPoints[terrain_poly_indices_1[1][j]].z + (terrainHeights[terrain_poly_indices_1[1][j]] * 0.002f),
                            };
                            st_vec3f p2 =
                            {
                                polyPoints[terrain_poly_indices_1[2][j]].x,
                                polyPoints[terrain_poly_indices_1[2][j]].y,
                                polyPoints[terrain_poly_indices_1[2][j]].z + (terrainHeights[terrain_poly_indices_1[2][j]] * 0.002f),
                            };

                            // compute the normal with the triangle indices
                            st_vec3f norm = triangleNormal(p0, p1, p2);

                            float brightness = dot(norm, light_dir);

                            // loop through the points in the polygon
                            for (int i = 0; i < 3; i++)
                            {
                                terrainClouds[terrain_poly_indices_1[0][j]] = (brightness * 2);
                                terrainClouds[terrain_poly_indices_1[1][j]] = (brightness * 2);
                                terrainClouds[terrain_poly_indices_1[2][j]] = (brightness * 2);
                            };
                        };*/

                        /*for (int i = 0; i < MAX_VERTS; i++)
                        {
                            terrainClouds[i] = 0x00;
                        };*/
                        break;
                    case 1:
                        // reset shade
                        for (int i = 0; i < MAX_VERTS; i++)
                        {
                            terrainClouds[i] = 0x00;

                            terrainNormals[i].x = 0.0f;
                            terrainNormals[i].y = 0.0f;
                            terrainNormals[i].z = 0.0f;
                        };
                        break;

                    case 2:
                        // reset height
                        for (int i = 0; i < MAX_VERTS; i++)
                        {
                            terrainHeights[i] = 0x00;
                        };
                        break;
                    };
                };
            };
        };

        if (current_menu == MENU_EDIT_POLYS)
        {
            if (mouse_in_button(&button_text_field, x_gl, y_gl) == true)
            {
                if (button_text_field.is_text_field == true)
                {
                    button_text_field.is_focused = !button_text_field.is_focused;
                };
            };
            
            if (mouse_in_button(&button_polygon_brush_text_field, x_gl, y_gl) == true)
            {
                if (button_polygon_brush_text_field.is_text_field == true)
                {
                    button_polygon_brush_text_field.is_focused = !button_polygon_brush_text_field.is_focused;
                };
            };

            if (mouse_in_button(&button_polygon_reset_all, x_gl, y_gl) == true)
            {
                for (int i = 0; i < MAX_POLYS; i++)
                {
                    terrainPolys[i].Bank = POLY_COLOR_BLUE_GREEN;
                    terrainPolys[i].TexFlag = poly_tex_flags[0];
                    terrainPolys[i].PolyFlag = poly_tex_flags[1];
                    terrainPolys[i].SampleStep = SAMPLE_STEP_NONE;
                    terrainPolys[i].CodeJeu = CJ_NONE;
                    terrainPolys[i].Sens = 0;
                    terrainPolys[i].Col = 0;
                    terrainPolys[i].Dummy = 0;
                    terrainPolys[i].IndexTex = 0;
                };
            };

            for (int i = 0; i < CJ_ANIMATED_GAZ; i++)
            {
                if (mouse_in_button(&button_polygon_code_jeu[i], x_gl, y_gl) == true)
                {
                    terrainPolys[selectedPoly].CodeJeu = i;
                };
            };

            for (int i = 0; i < SAMPLE_STEP_16; i++)
            {
                if (mouse_in_button(&button_polygon_sample[i], x_gl, y_gl) == true)
                {
                    terrainPolys[selectedPoly].SampleStep = i;
                };
            };

            for (int i = 0; i < POLY_COLOR_PURPLE_1; i++)
            {
                if (mouse_in_button(&button_polygon_colors[i], x_gl, y_gl) == true)
                {
                    terrainPolys[selectedPoly].Bank = i;
                };
            };

            for (int i = 0; i < 6; i++)
            {
                if (mouse_in_button(&button_polygon_operations[i], x_gl, y_gl) == true)
                {
                    switch (i)
                    {
                    case POLY_COPY:
                        switch (brush_size)
                        {
                        case 1:
                            poly_buffer = terrainPolys[selectedPoly];
                            break;
                        case 2:
                            copy_poly_data_4(&terrainPolys[selectedPoly]);
                            copy_height_data_3(&terrainHeights[terrain_poly_indices_0[0][selectedPoly / 2]]);
                            break;
                        case 3:
                            copy_poly_data_9(&terrainPolys[selectedPoly]);
                            copy_height_data_4(&terrainHeights[terrain_poly_indices_0[0][selectedPoly / 2]]);
                            break;
                        case 4:
                            copy_poly_data_16(&terrainPolys[selectedPoly]);
                            copy_height_data_5(&terrainHeights[terrain_poly_indices_0[0][selectedPoly / 2]]);
                            break;
                        };
                        break;

                    case POLY_PASTE:
                        switch (brush_size)
                        {
                        case 1:
                            terrainPolys[selectedPoly] = poly_buffer;
                            break;
                        case 2:
                            paste_poly_data_4();
                            paste_height_data_3(&terrainHeights[terrain_poly_indices_0[0][selectedPoly / 2]]);
                            break;
                        case 3:
                            paste_poly_data_9();
                            paste_height_data_4(&terrainHeights[terrain_poly_indices_0[0][selectedPoly / 2]]);
                            break;
                        case 4:
                            paste_poly_data_16();
                            paste_height_data_5(&terrainHeights[terrain_poly_indices_0[0][selectedPoly / 2]]);
                            break;
                        };
                        break;

                    case POLY_FLIP:
                        terrainPolys[selectedPoly].Sens = !terrainPolys[selectedPoly].Sens;
                        break;
                    case POLY_COLLISION:
                        terrainPolys[selectedPoly].Col = !terrainPolys[selectedPoly].Col;
                        break;
                    case POLY_USE_COLOR:
                        terrainPolys[selectedPoly].PolyFlag = poly_tex_flags[!terrainPolys[selectedPoly].PolyFlag];
                        break;
                    case POLY_USE_TEX:
                        terrainPolys[selectedPoly].TexFlag = poly_tex_flags[!terrainPolys[selectedPoly].TexFlag];
                        break;
                    };
                };
            };
        };

        if (current_menu == MENU_EDIT_BUILDINGS)
        {
            for (int i = 0; i < 7; i++)
            {
                if (mouse_in_button(&button_building_operations[i], x_gl, y_gl) == true)
                {
                    switch (i)
                    {
                    case BUILDING_COPY:
                        break;
                    case BUILDING_PASTE:
                        break;
                    case BUILDING_BOUND: // recalculate bounding box
                        break;
                    case BUILDING_ALIGN: // align to ground
                        break;
                    case BUILDING_SNAP: // snap to vertex
                        break;
                    case BUILDING_ROTATE:
                        buildings[current_building].rotation += 256;
                        break;
                    case BUILDING_INDEX:
                        if (buildings[current_building].modelId < 255)
                            buildings[current_building].modelId += 1;
                        break;
                    };
                };
            };
        };

        if (current_menu == MENU_EDIT_TEXTURES)
        {
            for (int i = 0; i < 3; i++)
            {
                if (mouse_in_button(&button_texture_rectangles[i], x_gl, y_gl) == true)
                {
                    switch (i)
                    {
                    case 0:
                        create_texture_rectangle = true;
                        break;
                    case 1: // add texture
                        terrainTexs[numTerrainTexs].Tx0 = 0;
                        terrainTexs[numTerrainTexs].Ty0 = 0;

                        terrainTexs[numTerrainTexs].Tx1 = 4096;
                        terrainTexs[numTerrainTexs].Ty1 = 4096;

                        terrainTexs[numTerrainTexs].Tx2 = 0;
                        terrainTexs[numTerrainTexs].Ty2 = 4096;

                        numTerrainTexs += 1;
                        break;
                    case 2: // delete texture
                        if (numTerrainTexs > 1)
                            numTerrainTexs -= 1;

                        terrainTexs[numTerrainTexs].Tx0 = 0;
                        terrainTexs[numTerrainTexs].Ty0 = 0;

                        terrainTexs[numTerrainTexs].Tx1 = 0;
                        terrainTexs[numTerrainTexs].Ty1 = 0;

                        terrainTexs[numTerrainTexs].Tx2 = 0;
                        terrainTexs[numTerrainTexs].Ty2 = 0;

                        // loop through every polygon
                        for (int i = 0; i < MAX_POLYS; i++)
                        {
                            // check if its texture index is in range of textures
                            if (terrainPolys[i].IndexTex >= numTerrainTexs)
                            {
                                //terrainPolys[i].TexFlag = poly_tex_flags[0];
                                //terrainPolys[i].IndexTex = 0;

                                terrainPolys[i].IndexTex -= 1;
                            };
                        };

                        break;
                    };

                };
            };

            for (int i = 0; i < 8; i++)
            {
                if (mouse_in_button(&button_text_field_textures[i], x_gl, y_gl) == true)
                {
                    if (button_text_field_textures[i].is_text_field == true)
                    {
                        button_text_field_textures[i].is_focused = !button_text_field_textures[i].is_focused;
                    };
                };
            };
        };

        if (current_menu == MENU_SHOW)
        {
            for (int i = 0; i < 8; i++)
            {
                if (mouse_in_button(&button_tabs[i], x_gl, y_gl) == true)
                {
                    printf("in button \n");
                    switch (i)
                    {
                    case BUTTON_SHOW_HEIGHTS:
                        showHeights = !showHeights;
                        break;
                    case BUTTON_SHOW_TEXTURES:
                        showTextures = !showTextures;
                        break;
                    case BUTTON_SHOW_SHADOWS:
                        showShadows = !showShadows;
                        break;
                    case BUTTON_SHOW_COLLISIONS:
                        showCollisions = !showCollisions;
                        break;
                    case BUTTON_SHOW_BUILDINGS:
                        showBuildings = !showBuildings;
                        break;
                    case BUTTON_SHOW_WIREFRAMES:
                        showWireframe = !showWireframe;
                        break;
                    case BUTTON_SHOW_PALETTE:
                        showPalette = !showPalette;
                        break;
                    case BUTTON_SAVE_SEA_INFO:

                        /*saveBuildings("testBuildings.lun");
                        saveSea("testSea.lun");
                        saveTextures("testTextures.lun");
                        saveTerrain("testTerrain.lun");
                        saveClouds("testClouds.lun");*/

                        saveBuildings("medieval_buildings.lun");
                        saveSea("medieval_sea.lun");
                        saveTextures("medieval_textures.lun");
                        saveTerrain("medieval_terrain.lun");
                        saveClouds("medieval_clouds.lun");

                        printf("texture coordinates sometimes will wrap back to the opposite side of the texture window on exporting \n");
                        printf("the only fix to this is to slightly offset some coordinates and re-export and test in-game \n");

                        break;
                    };
                };
            };
        };

        printf("Right click!\n");
    };
};

float map_short_to_float(short value,
    short inMin, short inMax,
    float outMin, float outMax)
{
    return outMin +
        (float)(value - inMin) * (outMax - outMin) /
        (float)(inMax - inMin);
};

/*double snap(double value, short grid)
{
    return round(value / grid) * grid;
};*/

short snap_int(short value, short grid)
{
    return ((value + grid / 2) / grid) * grid;
};

constexpr int FB_WIDTH = 640;
constexpr int FB_HEIGHT = 480;

unsigned char framebuffer[FB_WIDTH * FB_HEIGHT * 3];

void display()
{
    char textBuf[200];
    sprintf(textBuf, "LBA2 Terrain Editor | Show Heights: %d | Show Textures: %d | Show Shadows: %d | Show Collisions: %d | Show Wireframes: %d | Show Palette: %d | Use Snap: %d | Num Textures: %d", showHeights, showTextures, showShadows, showCollisions, showWireframe, showPalette, use_snap, numTerrainTexs);
    glutSetWindowTitle(textBuf);



    glViewport(0, 0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();
    gluPerspective(60.0, GLfloat(glutGet(GLUT_WINDOW_WIDTH)) / GLfloat(glutGet(GLUT_WINDOW_HEIGHT)), 0.5, 400);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //glRasterPos2i(0, 0);
    //glPixelZoom(1.0f, 1.0f);
    //glDrawPixels(640, 480, GL_RGB, GL_UNSIGNED_BYTE, framebuffer);

    gluLookAt(theCamera.pos.x, theCamera.pos.y, theCamera.pos.z,
        theCamera.look.x, theCamera.look.y, theCamera.look.z,
        theCamera.upVec.x, theCamera.upVec.y, theCamera.upVec.z);



    //printf("pressed_w: %d, pressed_a: %d \n", pressed_w, pressed_a);

    // Update camera
    /*theCamera.pos.x = theCamera.pos.x + cosf(theCamera.rot_lerp) * (0 + theCamera.speed);
    theCamera.pos.y = theCamera.pos.y + sinf(theCamera.rot_lerp) * (0 + theCamera.speed);

    theCamera.look.x = theCamera.pos.x + cosf(theCamera.rot_lerp) * (2 + theCamera.speed);
    theCamera.look.y = theCamera.pos.y + sinf(theCamera.rot_lerp) * (2 + theCamera.speed);
    theCamera.look.z = theCamera.pos.z - 1.0f;*/

    theCamera.rot_lerp = lerp(theCamera.rot_lerp, theCamera.rot, 0.1f);

    if (button_presses.pressed_w == true)
    {
        theCamera.speed += 0.025f;

        if (theCamera.speed > 0.1f)
            theCamera.speed = 0.1f;
    }
    else if (button_presses.pressed_s == true)
    {
        theCamera.speed -= 0.025f;

        if (theCamera.speed < -0.1f)
            theCamera.speed = -0.1f;
    }
    else
    {
        theCamera.speed = lerp(theCamera.speed, 0, 0.1f);
    };

    if (button_presses.pressed_a == true) // strafe left
    {
        theCamera.pos.x = theCamera.pos.x + cosf(theCamera.rot_lerp) * (0 + theCamera.speed);
        theCamera.pos.y = theCamera.pos.y + sinf(theCamera.rot_lerp) * (0 + theCamera.speed);

        theCamera.look.x = theCamera.pos.x + cosf(theCamera.rot_lerp + degToRad(90.0f)) * (2 + theCamera.speed);
        theCamera.look.y = theCamera.pos.y + sinf(theCamera.rot_lerp + degToRad(90.0f)) * (2 + theCamera.speed);
        //theCamera.look.z = theCamera.pos.z - 1.0f;
    }
    else if (button_presses.pressed_d == true) // strafe right
    {
        theCamera.pos.x = theCamera.pos.x + cosf(theCamera.rot_lerp) * (0 + theCamera.speed);
        theCamera.pos.y = theCamera.pos.y + sinf(theCamera.rot_lerp) * (0 + theCamera.speed);

        theCamera.look.x = theCamera.pos.x + cosf(theCamera.rot_lerp) * (2 + theCamera.speed) + (cosf(degToRad(-90.0f)) * 0.1f);
        theCamera.look.y = theCamera.pos.y + sinf(theCamera.rot_lerp) * (2 + theCamera.speed) + (cosf(degToRad(-90.0f)) * 0.1f);
        //theCamera.look.z = theCamera.pos.z - 1.0f;
    }
    else
    {
        theCamera.pos.x = theCamera.pos.x + cosf(theCamera.rot_lerp) * (0 + theCamera.speed);
        theCamera.pos.y = theCamera.pos.y + sinf(theCamera.rot_lerp) * (0 + theCamera.speed);

        theCamera.look.x = theCamera.pos.x + cosf(theCamera.rot_lerp) * (2 + theCamera.speed);
        theCamera.look.y = theCamera.pos.y + sinf(theCamera.rot_lerp) * (2 + theCamera.speed);
        //theCamera.look.z = theCamera.pos.z - 1.0f;
    };

    //theCamera.lerpVec.z = lerp(theCamera.lerpVec.z, theCamera.pos.z - 1.0f, 0.1f);
    //theCamera.look.z = lerp(theCamera.look.z, theCamera.pos.z - 1.0f, 0.1f);
    theCamera.look.z = theCamera.pos.z - 1.0f;

    if (button_presses.pressed_q == true)
    {
        theCamera.rot += 0.05f;
    };

    if (button_presses.pressed_e == true)
    {
        theCamera.rot -= 0.05f;
    };

    if (button_presses.pressed_1 == true)
    {
        theCamera.pos.z = lerp(theCamera.pos.z, theCamera.pos.z + 1, 0.1f);
        theCamera.look.z = lerp(theCamera.look.z, theCamera.look.z + 1, 0.1f);
    };

    if (button_presses.pressed_3 == true)
    {
        theCamera.pos.z = lerp(theCamera.pos.z, theCamera.pos.z - 1, 0.1f);
        theCamera.look.z = lerp(theCamera.look.z, theCamera.look.z - 1, 0.1f);
    };


    // convert mouse coordinates to gluOrtho2D coords (-1, 1, -1, 1)
    //x_gl = (mouse_x / (float)glutGet(GLUT_WINDOW_WIDTH)) * 2.0f - 1.0f;
    //y_gl = 1.0f - (mouse_y / (float)glutGet(GLUT_WINDOW_HEIGHT)) * 2.0f;

    // this is the same as above but now works for a screen being stretched
    float winW = (float)glutGet(GLUT_WINDOW_WIDTH);
    float winH = (float)glutGet(GLUT_WINDOW_HEIGHT);

    float mouse_aspect = winW / winH;

    // Convert to normalized device coordinates (-1 .. +1)
    float x_ndc = (mouse_x / winW) * 2.0f - 1.0f;
    float y_ndc = 1.0f - (mouse_y / winH) * 2.0f;

    // Convert NDC -> your adjusted ortho coordinates
    if (mouse_aspect >= 1.0f)
    {
        x_gl = x_ndc * mouse_aspect;  // X was stretched
        y_gl = y_ndc;
    }
    else {
        x_gl = x_ndc;
        y_gl = y_ndc * (1.0f / mouse_aspect);  // Y was stretched
    };



    // debug mouse
    //printf("mouse: %.2f, %.2f \n", x_gl, y_gl);
    //printf("mouse_held: %d \n", mouse_held);
    //printf("x_ndc: %.2f, y_ndc: %.2f \n", x_ndc, y_ndc);



    GLdouble model[16], proj[16];
    GLint viewport[4];

    // Get matrices and viewport
    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetIntegerv(GL_VIEWPORT, viewport);



    //printf("selectedPoly: %d \n", selectedPoly);

	// loop through all vertices in heightmap
	if (current_menu == MENU_EDIT_HEIGHT)
	{
        // selection box handling
        if (mouse_held == true)
        {
            selection_box_visible = true;

            selection_box_p0.x = prev_mouse_x;
            selection_box_p0.y = prev_mouse_y;

            selection_box_p1.x = x_gl;
            selection_box_p1.y = y_gl;
        };

		for (int i = 0; i < MAX_VERTS; i++)
		{
            st_vec2f coord_point = { 0, 0 };
            st_vec3f point = polyPoints[i];
            point.z = (point.z + (terrainHeights[i] * 0.002f));

            convert_3d_coord_to_2d(model, proj, viewport, winW, winH, mouse_aspect, &point, &coord_point);

            poly_handles[i].p.x = coord_point.x;
            poly_handles[i].p.y = coord_point.y;

            if (coord_2d_is_visible(poly_handles[i].p, mouse_aspect) == true)
            {
                // check if point is in selection box
                if (point_in_rect_both_checks(poly_handles[i].p, selection_box_p0, selection_box_p1) == true &&
                    selection_box_visible == true)
                {
                    poly_handles[i].selected = true;

                    all_verts_unselected = false;
                };

                // check if poly handles are in screen bounds
                if ((poly_handles[i].p.x > -1 && poly_handles[i].p.x < 1) &&
                    (poly_handles[i].p.y > -1 && poly_handles[i].p.y < 1))
                {
                    if (mouse_in_handle(&poly_handles[i], x_gl, y_gl, 0.01f) == true)
                    {
                        //terrainHeights[i] += 64;
                    };
                };
            };
		};
	};

    // loop through all polygons in heightmap
    for (int j = 0; j < MAX_POLYS; j++)
    {
        // data to convert
        st_vec2f tri_coord_2d[3] =
        {
            {0, 0},
            {0, 0},
            {0, 0},
        };

        st_vec3f tri_coord[3] =
        {
            {
            polyPoints[terrain_poly_indices_0[0][j]].x,
            polyPoints[terrain_poly_indices_0[0][j]].y,
            polyPoints[terrain_poly_indices_0[0][j]].z + (terrainHeights[terrain_poly_indices_0[0][j]] * 0.002f)
            },

            {
            polyPoints[terrain_poly_indices_0[1][j]].x,
            polyPoints[terrain_poly_indices_0[1][j]].y,
            polyPoints[terrain_poly_indices_0[1][j]].z + (terrainHeights[terrain_poly_indices_0[1][j]] * 0.002f)
            },

            {
            polyPoints[terrain_poly_indices_0[2][j]].x,
            polyPoints[terrain_poly_indices_0[2][j]].y,
            polyPoints[terrain_poly_indices_0[2][j]].z + (terrainHeights[terrain_poly_indices_0[2][j]] * 0.002f)
            },
        };

		// perform conversion
		for (int i = 0; i < 3; i++)
			convert_3d_coord_to_2d(model, proj, viewport, winW, winH, mouse_aspect, &tri_coord[i], &tri_coord_2d[i]);

        // check if the 2d triangle coordinates are within screen space
        if (coord_2d_is_visible(tri_coord_2d[0], mouse_aspect) ||
            coord_2d_is_visible(tri_coord_2d[1], mouse_aspect) || 
            coord_2d_is_visible(tri_coord_2d[2], mouse_aspect))
		{
			// check if mouse is in polygon
			st_vec2f mouse_pos = { x_gl, y_gl };
			if (pointInTriangle_2(mouse_pos, tri_coord_2d[0], tri_coord_2d[1], tri_coord_2d[2]))
			{
                if (current_menu == MENU_EDIT_TEXTURES)
                {
                    if ((x_gl < (edit_tex_scale * -tex_margin) ||
                         x_gl >(edit_tex_scale * tex_margin)) ||
                        (y_gl < (edit_tex_scale * -tex_margin) ||
                         y_gl >(edit_tex_scale * tex_margin)))
                    {
                        //printf("mouse in poly %d! \n", (j * 2));

                        // only select polygons if not editing textures
                        if (handles[0].selected == false &&
                            handles[1].selected == false &&
                            handles[2].selected == false)
                        {
                            currentPoly = (j * 2);

                            if (mouse_held == true)
                                selectedPoly = (j * 2);

                            break;
                        };
                    };
                }
                else
                {
                    //printf("mouse in poly %d! \n", (j * 2));
                    currentPoly = (j * 2);

                    if (mouse_held == true)
                        selectedPoly = (j * 2);

                    break;
                };
			};
		};
	};

    for (int j = 0; j < MAX_POLYS; j++)
    {
        // data to convert
        st_vec2f tri_coord_2d[3] =
        {
            {0, 0},
            {0, 0},
            {0, 0},
        };

        st_vec3f tri_coord[3] =
        {
            {
            polyPoints[terrain_poly_indices_1[0][j]].x,
            polyPoints[terrain_poly_indices_1[0][j]].y,
            polyPoints[terrain_poly_indices_1[0][j]].z + (terrainHeights[terrain_poly_indices_1[0][j]] * 0.002f)
            },

            {
            polyPoints[terrain_poly_indices_1[1][j]].x,
            polyPoints[terrain_poly_indices_1[1][j]].y,
            polyPoints[terrain_poly_indices_1[1][j]].z + (terrainHeights[terrain_poly_indices_1[1][j]] * 0.002f)
            },

            {
            polyPoints[terrain_poly_indices_1[2][j]].x,
            polyPoints[terrain_poly_indices_1[2][j]].y,
            polyPoints[terrain_poly_indices_1[2][j]].z + (terrainHeights[terrain_poly_indices_1[2][j]] * 0.002f)
            },
        };

		// perform conversion
		for (int i = 0; i < 3; i++)
			convert_3d_coord_to_2d(model, proj, viewport, winW, winH, mouse_aspect, &tri_coord[i], &tri_coord_2d[i]);

        // check if the 2d triangle coordinates are within screen space
        if (coord_2d_is_visible(tri_coord_2d[0], mouse_aspect) ||
            coord_2d_is_visible(tri_coord_2d[1], mouse_aspect) ||
            coord_2d_is_visible(tri_coord_2d[2], mouse_aspect))
        {
			// check if mouse is in polygon
			st_vec2f mouse_pos = { x_gl, y_gl };
			if (pointInTriangle_2(mouse_pos, tri_coord_2d[0], tri_coord_2d[1], tri_coord_2d[2]))
			{
                if (current_menu == MENU_EDIT_TEXTURES)
                {
                    if ((x_gl < (edit_tex_scale * -tex_margin) ||
                         x_gl >(edit_tex_scale * tex_margin)) ||
                        (y_gl < (edit_tex_scale * -tex_margin) ||
                         y_gl >(edit_tex_scale * tex_margin)))
                    {
                        //printf("mouse in poly %d! \n", (j * 2) + 1);

                        // only select polygons if not editing textures
                        if (handles[0].selected == false &&
                            handles[1].selected == false &&
                            handles[2].selected == false)
                        {
                            currentPoly = ((j * 2) + 1);

                            if (mouse_held == true)
                                selectedPoly = ((j * 2) + 1);
                            //selectedPoly = ((j) + 1);

                            break;
                        };
                    };
                }
                else
                {
                    //printf("mouse in poly %d! \n", (j * 2) + 1);
                    currentPoly = ((j * 2) + 1);

                    if (mouse_held == true)
                        selectedPoly = ((j * 2) + 1);
                    //selectedPoly = ((j) + 1);

                    break;
                };
			};
		};
    };



    //printf("%d, %d, %d \n", terrain_poly_indices_0[0][0], terrain_poly_indices_0[0][1], terrain_poly_indices_0[0][2]);



    if (mouse_held == true)
    {
        if (mouse_in_window(&window_palette, x_gl, y_gl) == true)
        {
            float dx = (x_gl - prev_mouse_x);
            float dy = (y_gl - prev_mouse_y);

            window_palette.p0.x += dx;
            window_palette.p0.y += dy;

            window_palette.p1.x += dx;
            window_palette.p1.y += dy;

            // prevent from dragging off screen
            //if (window_palette.p1.x > x_ndc)
                //window_palette.p1.x = x_ndc;

            prev_mouse_x = x_gl;
            prev_mouse_y = y_gl;
        };
    };



    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);


    glDisable(GL_TEXTURE_2D);

    glColor3f(1, 1, 1);

    if (showBuildings == true)
    {
        for (int i = 0; i < numBuildings; i++)
            draw_building(&buildings[i]);
    };

    st_vec3f current_building_pos;
    current_building_pos.x = (buildings[current_building].x * 0.00195f);
    current_building_pos.y = (buildings[current_building].y * 0.00195f);
    current_building_pos.z = (buildings[current_building].z * 0.00195f);
    current_building_pos = vertex_rotate_x(current_building_pos, degToRad(90.0f));
    if (current_menu == MENU_EDIT_BUILDINGS)
        draw_axis(current_building_pos, current_axis);

    for (int i = 0; i < numBuildings; i++)
    {
        st_vec3f building_pos;
        building_pos.x = (buildings[i].x * 0.00195f);
        building_pos.y = (buildings[i].y * 0.00195f);
        building_pos.z = (buildings[i].z * 0.00195f);
        building_pos = vertex_rotate_x(building_pos, degToRad(90.0f));

        // draw buildings only if close to the camera
        // since we are using deprecated OpenGL, this makes things run faster
        if (distance3D(theCamera.pos.x, theCamera.pos.y, theCamera.pos.z,
            building_pos.x, building_pos.y, building_pos.z) <= 32)
        {
            draw_model(&models[buildings[i].modelId], &buildings[i]);
        };
    };



    glEnable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glDisable(GL_LIGHTING);

    // draw terrain triangle
    currentSquare = (currentPoly / 2);

    draw_terrain_textures();
    for (int i = 0; i < 64; i++) draw_terrain_quad(currentSquare, currentPoly, i, 0, (i * 2)); // TODO: change to (texture_index * 2) so we don't need -> i * 2
    for (int i = 64; i < (64 * 2); i++) draw_terrain_quad(currentSquare, currentPoly, i, 1, (i * 2));
    for (int i = (64 * 2); i < (64 * 3); i++) draw_terrain_quad(currentSquare, currentPoly, i, 2, (i * 2));
    for (int i = (64 * 3); i < (64 * 4); i++) draw_terrain_quad(currentSquare, currentPoly, i, 3, (i * 2));
    for (int i = (64 * 4); i < (64 * 5); i++) draw_terrain_quad(currentSquare, currentPoly, i, 4, (i * 2));
    for (int i = (64 * 5); i < (64 * 6); i++) draw_terrain_quad(currentSquare, currentPoly, i, 5, (i * 2));
    for (int i = (64 * 6); i < (64 * 7); i++) draw_terrain_quad(currentSquare, currentPoly, i, 6, (i * 2));
    for (int i = (64 * 7); i < (64 * 8); i++) draw_terrain_quad(currentSquare, currentPoly, i, 7, (i * 2));
    for (int i = (64 * 8); i < (64 * 9); i++) draw_terrain_quad(currentSquare, currentPoly, i, 8, (i * 2));
    for (int i = (64 * 9); i < (64 * 10); i++) draw_terrain_quad(currentSquare, currentPoly, i, 9, (i * 2));
    for (int i = (64 * 10); i < (64 * 11); i++) draw_terrain_quad(currentSquare, currentPoly, i, 10, (i * 2));
    for (int i = (64 * 11); i < (64 * 12); i++) draw_terrain_quad(currentSquare, currentPoly, i, 11, (i * 2));
    for (int i = (64 * 12); i < (64 * 13); i++) draw_terrain_quad(currentSquare, currentPoly, i, 12, (i * 2));
    for (int i = (64 * 13); i < (64 * 14); i++) draw_terrain_quad(currentSquare, currentPoly, i, 13, (i * 2));
    for (int i = (64 * 14); i < (64 * 15); i++) draw_terrain_quad(currentSquare, currentPoly, i, 14, (i * 2));
    for (int i = (64 * 15); i < (64 * 16); i++) draw_terrain_quad(currentSquare, currentPoly, i, 15, (i * 2));
    for (int i = (64 * 16); i < (64 * 17); i++) draw_terrain_quad(currentSquare, currentPoly, i, 16, (i * 2));
    for (int i = (64 * 17); i < (64 * 18); i++) draw_terrain_quad(currentSquare, currentPoly, i, 17, (i * 2));
    for (int i = (64 * 18); i < (64 * 19); i++) draw_terrain_quad(currentSquare, currentPoly, i, 18, (i * 2));
    for (int i = (64 * 19); i < (64 * 20); i++) draw_terrain_quad(currentSquare, currentPoly, i, 19, (i * 2));
    for (int i = (64 * 20); i < (64 * 21); i++) draw_terrain_quad(currentSquare, currentPoly, i, 20, (i * 2));
    for (int i = (64 * 21); i < (64 * 22); i++) draw_terrain_quad(currentSquare, currentPoly, i, 21, (i * 2));
    for (int i = (64 * 22); i < (64 * 23); i++) draw_terrain_quad(currentSquare, currentPoly, i, 22, (i * 2));
    for (int i = (64 * 23); i < (64 * 24); i++) draw_terrain_quad(currentSquare, currentPoly, i, 23, (i * 2));
    for (int i = (64 * 24); i < (64 * 25); i++) draw_terrain_quad(currentSquare, currentPoly, i, 24, (i * 2));
    for (int i = (64 * 25); i < (64 * 26); i++) draw_terrain_quad(currentSquare, currentPoly, i, 25, (i * 2));
    for (int i = (64 * 26); i < (64 * 27); i++) draw_terrain_quad(currentSquare, currentPoly, i, 26, (i * 2));
    for (int i = (64 * 27); i < (64 * 28); i++) draw_terrain_quad(currentSquare, currentPoly, i, 27, (i * 2));
    for (int i = (64 * 28); i < (64 * 29); i++) draw_terrain_quad(currentSquare, currentPoly, i, 28, (i * 2));
    for (int i = (64 * 29); i < (64 * 30); i++) draw_terrain_quad(currentSquare, currentPoly, i, 29, (i * 2));
    for (int i = (64 * 30); i < (64 * 31); i++) draw_terrain_quad(currentSquare, currentPoly, i, 30, (i * 2));
    for (int i = (64 * 31); i < (64 * 32); i++) draw_terrain_quad(currentSquare, currentPoly, i, 31, (i * 2));
    for (int i = (64 * 32); i < (64 * 33); i++) draw_terrain_quad(currentSquare, currentPoly, i, 32, (i * 2));
    for (int i = (64 * 33); i < (64 * 34); i++) draw_terrain_quad(currentSquare, currentPoly, i, 33, (i * 2));
    for (int i = (64 * 34); i < (64 * 35); i++) draw_terrain_quad(currentSquare, currentPoly, i, 34, (i * 2));
    for (int i = (64 * 35); i < (64 * 36); i++) draw_terrain_quad(currentSquare, currentPoly, i, 35, (i * 2));
    for (int i = (64 * 36); i < (64 * 37); i++) draw_terrain_quad(currentSquare, currentPoly, i, 36, (i * 2));
    for (int i = (64 * 37); i < (64 * 38); i++) draw_terrain_quad(currentSquare, currentPoly, i, 37, (i * 2));
    for (int i = (64 * 38); i < (64 * 39); i++) draw_terrain_quad(currentSquare, currentPoly, i, 38, (i * 2));
    for (int i = (64 * 39); i < (64 * 40); i++) draw_terrain_quad(currentSquare, currentPoly, i, 39, (i * 2));
    for (int i = (64 * 40); i < (64 * 41); i++) draw_terrain_quad(currentSquare, currentPoly, i, 40, (i * 2));
    for (int i = (64 * 41); i < (64 * 42); i++) draw_terrain_quad(currentSquare, currentPoly, i, 41, (i * 2));
    for (int i = (64 * 42); i < (64 * 43); i++) draw_terrain_quad(currentSquare, currentPoly, i, 42, (i * 2));
    for (int i = (64 * 43); i < (64 * 44); i++) draw_terrain_quad(currentSquare, currentPoly, i, 43, (i * 2));
    for (int i = (64 * 44); i < (64 * 45); i++) draw_terrain_quad(currentSquare, currentPoly, i, 44, (i * 2));
    for (int i = (64 * 45); i < (64 * 46); i++) draw_terrain_quad(currentSquare, currentPoly, i, 45, (i * 2));
    for (int i = (64 * 46); i < (64 * 47); i++) draw_terrain_quad(currentSquare, currentPoly, i, 46, (i * 2));
    for (int i = (64 * 47); i < (64 * 48); i++) draw_terrain_quad(currentSquare, currentPoly, i, 47, (i * 2));
    for (int i = (64 * 48); i < (64 * 49); i++) draw_terrain_quad(currentSquare, currentPoly, i, 48, (i * 2));
    for (int i = (64 * 49); i < (64 * 50); i++) draw_terrain_quad(currentSquare, currentPoly, i, 49, (i * 2));
    for (int i = (64 * 50); i < (64 * 51); i++) draw_terrain_quad(currentSquare, currentPoly, i, 50, (i * 2));
    for (int i = (64 * 51); i < (64 * 52); i++) draw_terrain_quad(currentSquare, currentPoly, i, 51, (i * 2));
    for (int i = (64 * 52); i < (64 * 53); i++) draw_terrain_quad(currentSquare, currentPoly, i, 52, (i * 2));
    for (int i = (64 * 53); i < (64 * 54); i++) draw_terrain_quad(currentSquare, currentPoly, i, 53, (i * 2));
    for (int i = (64 * 54); i < (64 * 55); i++) draw_terrain_quad(currentSquare, currentPoly, i, 54, (i * 2));
    for (int i = (64 * 55); i < (64 * 56); i++) draw_terrain_quad(currentSquare, currentPoly, i, 55, (i * 2));
    for (int i = (64 * 56); i < (64 * 57); i++) draw_terrain_quad(currentSquare, currentPoly, i, 56, (i * 2));
    for (int i = (64 * 57); i < (64 * 58); i++) draw_terrain_quad(currentSquare, currentPoly, i, 57, (i * 2));
    for (int i = (64 * 58); i < (64 * 59); i++) draw_terrain_quad(currentSquare, currentPoly, i, 58, (i * 2));
    for (int i = (64 * 59); i < (64 * 60); i++) draw_terrain_quad(currentSquare, currentPoly, i, 59, (i * 2));
    for (int i = (64 * 60); i < (64 * 61); i++) draw_terrain_quad(currentSquare, currentPoly, i, 60, (i * 2));
    for (int i = (64 * 61); i < (64 * 62); i++) draw_terrain_quad(currentSquare, currentPoly, i, 61, (i * 2));
    for (int i = (64 * 62); i < (64 * 63); i++) draw_terrain_quad(currentSquare, currentPoly, i, 62, (i * 2));
    for (int i = (64 * 63); i < (64 * 64); i++) draw_terrain_quad(currentSquare, currentPoly, i, 63, (i * 2));


    
    draw_selected_poly_odd();
    draw_selected_poly_even();

    draw_brush(brush_size);

    //draw_selected_poly_offset_odd(1);
    //draw_selected_poly_offset_even(1);
    //draw_selected_poly_offset_odd(-1);
    //draw_selected_poly_offset_even(-1);

    glDisable(GL_TEXTURE_2D);

    if (current_menu == MENU_EDIT_BUILDINGS)
    {
        if (mouse_held == true)
        {
            float dx = (x_gl - prev_mouse_x);
            float dy = (y_gl - prev_mouse_y);

            if (current_axis == AXIS_X)
            {
                // prevent building from leaving bounds
                if (buildings[current_building].colLeft > 0 ||
                    buildings[current_building].colRight < 64)
                {
                    buildings[current_building].x += dx * 2048;
                    buildings[current_building].colLeft += dx * 2048;
                    buildings[current_building].colRight += dx * 2048;
                };
            };
            if (current_axis == AXIS_Y)
            {
                buildings[current_building].z += dx * 2048;
                buildings[current_building].colUp += dx * 2048;
                buildings[current_building].colDown += dx * 2048;
            };
            if (current_axis == AXIS_Z)
            {
                buildings[current_building].y += dy * 2048;
                buildings[current_building].colTop += dy * 2048;
                buildings[current_building].colBottom += dy * 2048;
            };

            prev_mouse_x = x_gl;
            prev_mouse_y = y_gl;
        };

        // check for all buildings
        for (int j = 0; j < numBuildings; j++)
        {
            st_building* building_pointer;
            building_pointer = &buildings[j];

            float building_scale = 0.00195f;

            // data to convert
            st_vec2f tri_coord_2d[BOUNDING_BOX_TRIS][3] =
            {
                { // tri 0 (bottom)
                {0, 0},
                {0, 0},
                {0, 0},
                },
                { // tri 1 (bottom)
                {0, 0},
                {0, 0},
                {0, 0},
                },
                { // tri 2 (top)
                {0, 0},
                {0, 0},
                {0, 0},
                },
                { // tri 3 (top)
                {0, 0},
                {0, 0},
                {0, 0},
                },

                { // tri 4 (left)
                {0, 0},
                {0, 0},
                {0, 0},
                },
                { // tri 5 (left)
                {0, 0},
                {0, 0},
                {0, 0},
                },
            };

            st_vec3f tri_coord[BOUNDING_BOX_TRIS][3] =
            {
                { // tri 0 (bottom)
                    {
                    building_pointer->colRight * building_scale,
                    building_pointer->colBottom * building_scale,
                    building_pointer->colDown * building_scale
                    },

                    {
                    building_pointer->colRight * building_scale,
                    building_pointer->colBottom * building_scale,
                    building_pointer->colUp * building_scale
                    },

                    {
                    building_pointer->colLeft * building_scale,
                    building_pointer->colBottom * building_scale,
                    building_pointer->colUp * building_scale
                    },
                },
                { // tri 1 (bottom)
                    {
                    building_pointer->colRight * building_scale,
                    building_pointer->colBottom * building_scale,
                    building_pointer->colDown * building_scale
                    },

                    {
                    building_pointer->colLeft * building_scale,
                    building_pointer->colBottom * building_scale,
                    building_pointer->colUp * building_scale
                    },

                    {
                    building_pointer->colLeft * building_scale,
                    building_pointer->colBottom * building_scale,
                    building_pointer->colDown * building_scale
                    },
                },

                { // tri 2 (top)
                    {
                    building_pointer->colRight * building_scale,
                    building_pointer->colTop * building_scale,
                    building_pointer->colDown * building_scale
                    },

                    {
                    building_pointer->colRight * building_scale,
                    building_pointer->colTop * building_scale,
                    building_pointer->colUp * building_scale
                    },

                    {
                    building_pointer->colLeft * building_scale,
                    building_pointer->colTop * building_scale,
                    building_pointer->colUp * building_scale
                    },
                },
                { // tri 3 (top)
                    {
                    building_pointer->colRight * building_scale,
                    building_pointer->colTop * building_scale,
                    building_pointer->colDown * building_scale
                    },

                    {
                    building_pointer->colLeft * building_scale,
                    building_pointer->colTop * building_scale,
                    building_pointer->colUp * building_scale
                    },

                    {
                    building_pointer->colLeft * building_scale,
                    building_pointer->colTop * building_scale,
                    building_pointer->colDown * building_scale
                    },
                },

                { // tri 4 (left)
                    {
                    building_pointer->colLeft * building_scale,
                    building_pointer->colTop * building_scale, // top or bottom
                    building_pointer->colDown * building_scale
                    },

                    {
                    building_pointer->colLeft * building_scale,
                    building_pointer->colTop * building_scale, // top or bottom
                    building_pointer->colUp * building_scale
                    },

                    {
                    building_pointer->colLeft * building_scale,
                    building_pointer->colBottom * building_scale, // top or bottom
                    building_pointer->colDown * building_scale
                    },
                },
                { // tri 5 (left)
                    {
                    building_pointer->colLeft * building_scale,
                    building_pointer->colBottom * building_scale, // top or bottom
                    building_pointer->colDown * building_scale
                    },

                    {
                    building_pointer->colLeft * building_scale,
                    building_pointer->colTop * building_scale, // top or bottom
                    building_pointer->colUp * building_scale
                    },

                    {
                    building_pointer->colLeft * building_scale,
                    building_pointer->colBottom * building_scale, // top or bottom
                    building_pointer->colUp * building_scale
                    },
                },

                { // tri 6 (right)
                    {
                    building_pointer->colRight * building_scale,
                    building_pointer->colTop * building_scale, // top or bottom
                    building_pointer->colUp * building_scale
                    },

                    {
                    building_pointer->colRight * building_scale,
                    building_pointer->colTop * building_scale, // top or bottom
                    building_pointer->colDown * building_scale
                    },

                    {
                    building_pointer->colRight * building_scale,
                    building_pointer->colBottom * building_scale, // top or bottom
                    building_pointer->colDown * building_scale
                    },
                },
                { // tri 7 (right)
                    {
                    building_pointer->colRight * building_scale,
                    building_pointer->colTop * building_scale, // top or bottom
                    building_pointer->colUp * building_scale
                    },

                    {
                    building_pointer->colRight * building_scale,
                    building_pointer->colBottom * building_scale, // top or bottom
                    building_pointer->colDown * building_scale
                    },

                    {
                    building_pointer->colRight * building_scale,
                    building_pointer->colBottom * building_scale, // top or bottom
                    building_pointer->colUp * building_scale
                    },
                },

                { // tri 8 (up)
                    {
                    building_pointer->colLeft * building_scale,
                    building_pointer->colTop * building_scale, // top or bottom
                    building_pointer->colUp * building_scale
                    },

                    {
                    building_pointer->colRight * building_scale,
                    building_pointer->colTop * building_scale, // top or bottom
                    building_pointer->colUp * building_scale
                    },

                    {
                    building_pointer->colLeft * building_scale,
                    building_pointer->colBottom * building_scale, // top or bottom
                    building_pointer->colUp * building_scale
                    },
                },

                { // tri 9 (up)
                    {
                    building_pointer->colLeft * building_scale,
                    building_pointer->colBottom * building_scale, // top or bottom
                    building_pointer->colUp * building_scale
                    },

                    {
                    building_pointer->colRight * building_scale,
                    building_pointer->colTop * building_scale, // top or bottom
                    building_pointer->colUp * building_scale
                    },

                    {
                    building_pointer->colRight * building_scale,
                    building_pointer->colBottom * building_scale, // top or bottom
                    building_pointer->colUp * building_scale
                    },
                },

                { // tri 10 (down)
                    {
                    building_pointer->colRight * building_scale,
                    building_pointer->colTop * building_scale, // top or bottom
                    building_pointer->colDown * building_scale
                    },

                    {
                    building_pointer->colLeft * building_scale,
                    building_pointer->colTop * building_scale, // top or bottom
                    building_pointer->colDown * building_scale
                    },

                    {
                    building_pointer->colLeft * building_scale,
                    building_pointer->colBottom * building_scale, // top or bottom
                    building_pointer->colDown * building_scale
                    },
                },

                { // tri 11 (down)
                    {
                    building_pointer->colRight * building_scale,
                    building_pointer->colTop * building_scale, // top or bottom
                    building_pointer->colDown * building_scale
                    },

                    {
                    building_pointer->colLeft * building_scale,
                    building_pointer->colBottom * building_scale, // top or bottom
                    building_pointer->colDown * building_scale
                    },

                    {
                    building_pointer->colRight * building_scale,
                    building_pointer->colBottom * building_scale, // top or bottom
                    building_pointer->colDown * building_scale
                    },
                },
            };

            // rotate the vertex points so they match what we see
            for (int i = 0; i < BOUNDING_BOX_TRIS; i++)
            {
                tri_coord[i][0] = vertex_rotate_x(tri_coord[i][0], degToRad(90.0f));
                tri_coord[i][1] = vertex_rotate_x(tri_coord[i][1], degToRad(90.0f));
                tri_coord[i][2] = vertex_rotate_x(tri_coord[i][2], degToRad(90.0f));
            };

            // debug
            glEnable(GL_BLEND);
            for (int i = 0; i < BOUNDING_BOX_TRIS; i++)
            {
                if (current_building == j)
                {
                    glBegin(GL_TRIANGLES);
                    glColor4f(1, 0, 0, 0.5f);
                    glVertex3f(tri_coord[i][0].x, tri_coord[i][0].y, tri_coord[i][0].z);
                    glColor4f(1, 0, 0, 0.5f);
                    glVertex3f(tri_coord[i][1].x, tri_coord[i][1].y, tri_coord[i][1].z);
                    glColor4f(1, 0, 0, 0.5f);
                    glVertex3f(tri_coord[i][2].x, tri_coord[i][2].y, tri_coord[i][2].z);
                    glEnd();
                }
                else
                {
                    glBegin(GL_TRIANGLES);
                    glColor4f(1, 1, 1, 0.5f);
                    glVertex3f(tri_coord[i][0].x, tri_coord[i][0].y, tri_coord[i][0].z);
                    glColor4f(1, 1, 1, 0.5f);
                    glVertex3f(tri_coord[i][1].x, tri_coord[i][1].y, tri_coord[i][1].z);
                    glColor4f(1, 1, 1, 0.5f);
                    glVertex3f(tri_coord[i][2].x, tri_coord[i][2].y, tri_coord[i][2].z);
                    glEnd();
                };
            };
            glDisable(GL_BLEND);

            // perform conversion
            for (int i = 0; i < BOUNDING_BOX_TRIS; i++) // num triangles
            {
                for (int k = 0; k < 3; k++) // three vertex points
                {
                    convert_3d_coord_to_2d(model, proj, viewport, winW, winH, mouse_aspect, &tri_coord[i][k], &tri_coord_2d[i][k]);
                };
            };

            // check if the 2d triangle coordinates are within screen space
            for (int i = 0; i < BOUNDING_BOX_TRIS; i++)
            {
                if (coord_2d_is_visible(tri_coord_2d[i][0], mouse_aspect) ||
                    coord_2d_is_visible(tri_coord_2d[i][1], mouse_aspect) ||
                    coord_2d_is_visible(tri_coord_2d[i][2], mouse_aspect))
                {
                    // check if mouse is in polygon
                    st_vec2f mouse_pos = { x_gl, y_gl };
                    if (pointInTriangle_2(mouse_pos, tri_coord_2d[i][0], tri_coord_2d[i][1], tri_coord_2d[i][2]))
                    {
                        //printf("mouse in building %d! \n", j);
                        //currentPoly = ((j * 2) + 1);

                        //current_building = j;
                        printf("building %d \n", current_building);

                        if (mouse_held == true)
                            current_building = j;

                        //selectedPoly = ((j * 2) + 1);
                    //selectedPoly = ((j) + 1);

                        break;
                    };
                };
            };
        };
    };



    //
    // 2D DRAWING
    //
    glDisable(GL_DEPTH_TEST);
    //glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // original
    //gluOrtho2D(-1, 1, -1, 1);

    // same as above but actually calculates the aspect ratio
    // so all 2D drawn elements always remain the same scale
    float gluOrthoAspect = (float)glutGet(GLUT_WINDOW_WIDTH) / (float)glutGet(GLUT_WINDOW_HEIGHT);

    if (gluOrthoAspect >= 1.0f)
    {
        // Wide window -> extend X
        gluOrtho2D(-gluOrthoAspect, gluOrthoAspect, -1, 1);
    }
    else {
        // Tall window -> extend Y
        gluOrtho2D(-1, 1, -1 / gluOrthoAspect, 1 / gluOrthoAspect);
    };

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(1, 1, 1);



    switch (current_menu)
    {
    case MENU_MAIN:
        glPushMatrix();

        for (int i = 0; i < 11; i++)
        {
            mouse_in_button(&button_main[i], x_gl, y_gl);
            //clamp_button_old(&button_main[i], (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), CLAMP_RIGHT);
            clamp_button(&button_main[i], (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), CLAMP_TOP_RIGHT, 0.1f, 0.1f + (i * 0.1f));
            draw_button(&button_main[i]);
        };

        if (display_loading_message == true)
            drawText(0.0f, 0.9f, 0.02f, "Loading data... This may take a minute.");

        glPopMatrix();
        break;

    case MENU_EDIT_ISLAND:
        for (int i = 0; i < MAX_ISLAND_PARTS; i++)
        {
            mouse_in_button(&button_edit_island[i], x_gl, y_gl);
            //clamp_button_old(&button_main[i], (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), CLAMP_RIGHT);
            //clamp_button(&button_main[i], (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), CLAMP_TOP_RIGHT, 0.1f, 0.1f + (i * 0.1f));
            draw_button(&button_edit_island[i]);
        };
        break;

    case MENU_EDIT_HEIGHT:
        glPushMatrix();

        for (int i = 0; i < 3; i++)
        {
            mouse_in_button(&button_terrain_operations[i], x_gl, y_gl);
            //clamp_button_old(&button_main[i], (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), CLAMP_RIGHT);
            clamp_button(&button_terrain_operations[i], (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), CLAMP_TOP_RIGHT, 0.1f, 0.1f + (i * 0.1f));
            draw_button(&button_terrain_operations[i]);
        };

        // draw selection box
        if (selection_box_visible == true)
        {
            glDisable(GL_TEXTURE_2D);

            // top line
            glBegin(GL_LINES);
            glColor3f(1, 1, 1);
            glVertex3f(selection_box_p0.x, selection_box_p0.y, 0.0f);
            glVertex3f(selection_box_p1.x, selection_box_p0.y, 0.0f);
            glEnd();

            // bottom line
            glBegin(GL_LINES);
            glColor3f(1, 1, 1);
            glVertex3f(selection_box_p0.x, selection_box_p1.y, 0.0f);
            glVertex3f(selection_box_p1.x, selection_box_p1.y, 0.0f);
            glEnd();

            // left line
            glBegin(GL_LINES);
            glColor3f(1, 1, 1);
            glVertex3f(selection_box_p0.x, selection_box_p0.y, 0.0f);
            glVertex3f(selection_box_p0.x, selection_box_p1.y, 0.0f);
            glEnd();

            // right line
            glBegin(GL_LINES);
            glColor3f(1, 1, 1);
            glVertex3f(selection_box_p1.x, selection_box_p0.y, 0.0f);
            glVertex3f(selection_box_p1.x, selection_box_p1.y, 0.0f);
            glEnd();

            glEnable(GL_TEXTURE_2D);
        };

        for (int i = 0; i < MAX_VERTS; i++)
        {
            st_vec2f test_p = poly_handles[i].p;
            test_p.x = (test_p.x - 0.01f);
            test_p.y = (test_p.y - 0.01f);

            if (coord_2d_is_visible(poly_handles[i].p, mouse_aspect) ||
                coord_2d_is_visible(test_p, mouse_aspect))
            {
                //st_vec2f corners[] =
                //{
                    //{-1, 1},
                    //{1, -1}
                //};
                
                //if (point_in_rect_both_checks(poly_handles[i].p, corners[0], corners[1]) == true)

                //if ((poly_handles[i].p.x > -1 && poly_handles[i].p.x < 1) &&
                    //(poly_handles[i].p.y > -1 && poly_handles[i].p.y < 1))
                //{
                    draw_handle(&poly_handles[i], 0.01f);
                    //break;
                //};
            };
        };

        glPopMatrix();
        break;

    case MENU_EDIT_POLYS:

        // POLYGON COLORS
		// first button causes "drop down"
        if (mouse_in_button(&button_polygon_colors[0], x_gl, y_gl) == true)
        {
            for (int i = 1; i < CJ_ANIMATED_GAZ; i++)
                button_polygon_colors[i].is_visible = true;
        };

        // check if mouse is outside button
		if (x_gl > button_polygon_colors[0].p1.x ||
			x_gl < button_polygon_colors[0].p0.x)
		{
			for (int i = 1; i < CJ_ANIMATED_GAZ; i++)
                button_polygon_colors[i].is_visible = false;
		};

        // POLYGON SAMPLE
        // first button causes "drop down"
        if (mouse_in_button(&button_polygon_sample[0], x_gl, y_gl) == true)
        {
            for (int i = 1; i < CJ_ANIMATED_GAZ; i++)
                button_polygon_sample[i].is_visible = true;
        };

        // check if mouse is outside button
        if (x_gl > button_polygon_sample[0].p1.x ||
            x_gl < button_polygon_sample[0].p0.x)
        {
            for (int i = 1; i < CJ_ANIMATED_GAZ; i++)
                button_polygon_sample[i].is_visible = false;
        };

        // POLYGON CODE JEU
        // first button causes "drop down"
        if (mouse_in_button(&button_polygon_code_jeu[0], x_gl, y_gl) == true)
        {
            for (int i = 1; i < CJ_ANIMATED_GAZ; i++)
                button_polygon_code_jeu[i].is_visible = true;
        };

        // check if mouse is outside button
        if (x_gl > button_polygon_code_jeu[0].p1.x ||
            x_gl < button_polygon_code_jeu[0].p0.x)
        {
            for (int i = 1; i < CJ_ANIMATED_GAZ; i++)
                button_polygon_code_jeu[i].is_visible = false;
        };

        glPushMatrix();



        mouse_in_button(&button_text_field, x_gl, y_gl);

        clamp_button(&button_text_field, (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), CLAMP_TOP_LEFT, 0.1f, 0.1f);

        draw_button(&button_text_field);
        if (display_text_error == true)
            drawText(button_text_field.p0.x, button_text_field.p0.y + 0.15f, 0.02f, "digit please");

        mouse_in_button(&button_polygon_brush_text_field, x_gl, y_gl);
        clamp_button(&button_polygon_brush_text_field, (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), CLAMP_TOP_LEFT, 0.1f, 0.2f);
        draw_button(&button_polygon_brush_text_field);

        drawText(button_polygon_brush_text_field.p0.x + 0.5f, button_polygon_brush_text_field.p0.y + 0.07f, 0.02f, "brush size - max 4");

        mouse_in_button(&button_polygon_reset_all, x_gl, y_gl);
        clamp_button(&button_polygon_reset_all, (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), CLAMP_TOP_LEFT, 0.1f, 0.3f);
        draw_button(&button_polygon_reset_all);


        // polygon operations
        for (int i = 0; i < 6; i++)
        {
            mouse_in_button(&button_polygon_operations[i], x_gl, y_gl);
            clamp_button(&button_polygon_operations[i], (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), CLAMP_BOTTOM_RIGHT, 0.1f + (i * 0.3f), 0.1f);

            draw_button(&button_polygon_operations[i]);
        };

        for (int i = 0; i < CJ_ANIMATED_GAZ; i++)
        {
            mouse_in_button(&button_polygon_code_jeu[i], x_gl, y_gl);
            clamp_button(&button_polygon_code_jeu[i], (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), CLAMP_TOP_RIGHT, 0.5f, 0.1f + (i * 0.1f));

            if (button_polygon_code_jeu[i].is_visible == true)
                draw_button(&button_polygon_code_jeu[i]);
        };

        for (int i = 0; i < SAMPLE_STEP_16; i++)
        {
            mouse_in_button(&button_polygon_sample[i], x_gl, y_gl);
            clamp_button(&button_polygon_sample[i], (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), CLAMP_TOP_RIGHT, 0.9f, 0.1f + (i * 0.1f));

            if (button_polygon_sample[i].is_visible == true)
                draw_button(&button_polygon_sample[i]);
        };

        for (int i = 0; i < POLY_COLOR_PURPLE_1; i++)
        {
            mouse_in_button(&button_polygon_colors[i], x_gl, y_gl);
            clamp_button(&button_polygon_colors[i], (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), CLAMP_TOP_RIGHT, 0.1f, 0.1f + (i * 0.1f));

            if (button_polygon_colors[i].is_visible == true)
                draw_button(&button_polygon_colors[i]);
        };

        glPopMatrix();
        break;

    case MENU_EDIT_BUILDINGS:
        for (int i = 0; i < 7; i++)
        {
            mouse_in_button(&button_building_operations[i], x_gl, y_gl);
            clamp_button(&button_building_operations[i], (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), CLAMP_BOTTOM_RIGHT, 0.1f + (i * 0.3f), 0.1f);

            draw_button(&button_building_operations[i]);
        };
        break;

    case MENU_EDIT_TEXTURES:
        char buffer[100];
        sprintf(buffer, "indexTex: %d", terrainPolys[selectedPoly].IndexTex);


        drawText(button_text_field_textures[0].p0.x, 0.95f, 0.02f, buffer);

        for (int i = 1; i < 3; i++)
        {
            mouse_in_button(&button_texture_rectangles[i], x_gl, y_gl);
            clamp_button(&button_texture_rectangles[i], (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), CLAMP_TOP_LEFT, 0.1f, 1.3f + (i * 0.1f));
            draw_button(&button_texture_rectangles[i]);
        };

        mouse_in_button(&button_texture_rectangles[0], x_gl, y_gl);
        clamp_button(&button_texture_rectangles[0], (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), CLAMP_TOP_LEFT, 0.1f, 1.3f);

        if (create_texture_rectangle == false)
        {
            draw_button(&button_texture_rectangles[0]);
        }
        else
        {
            float tex_scale = 0.0058f;

            drawText(button_texture_rectangles[0].p0.x, button_texture_rectangles[0].p0.y + 0.05f, 0.02f, "set texture uvs");

            handles_textures[0].p.x = (texture_rect_x[0] * 0.001f * tex_scale) * -edit_tex_scale;
            handles_textures[0].p.y = (texture_rect_y[0] * 0.001f * tex_scale) * -edit_tex_scale;

            handles_textures[1].p.x = (texture_rect_x[1] * 0.001f * tex_scale) * -edit_tex_scale;
            handles_textures[1].p.y = (texture_rect_y[1] * 0.001f * tex_scale) * -edit_tex_scale;

            interpolatePoints(handles_textures[0].p, handles_textures[1].p, texture_points_rect, num_texture_points_rect);

            st_short2 tex_rect_0 = { texture_rect_x[0], texture_rect_y[0] };
            st_short2 tex_rect_1 = { texture_rect_x[1], texture_rect_y[1] };

            interpolatePoints_short(tex_rect_0, tex_rect_1, texture_points_rect_short, num_texture_points_rect);

            for (int i = 0; i < 2; i++)
            {
                draw_handle(&handles_textures[i], 0.01f);

                mouse_in_handle(&handles_textures[i], x_gl, y_gl, 0.01f);

				//if (mouse_in_handle(&handles_textures[i], x_gl, y_gl, 0.01f) == true)
				if (handles_textures[i].selected == true)
				{
					texture_rect_x[i] = (short)((-x_gl * (65535 * 2.65f)) / edit_tex_scale); //+ pan_x;
					texture_rect_y[i] = (short)((-y_gl * (65535 * 2.65f)) / edit_tex_scale); //+ pan_y;
				};
            };

            // rotate points
            //vertex_rotate_2d(&handles_textures[1].p, handles_textures[0].p, degToRad(45.0f));
            //vertex_rotate_2d_short(&tex_rect_1, tex_rect_0, 45.0f);
            //for (int i = 0; i < num_texture_points_rect; i++)
            //{
                //vertex_rotate_2d(&texture_points_rect[i], handles_textures[0].p, degToRad(45.0f));
                //vertex_rotate_2d_short(&texture_points_rect_short[i], tex_rect_0, 45.0f);
            //};

            glDisable(GL_TEXTURE_2D);

            // draw line segments
            glBegin(GL_LINES);
            glColor3f(1, 1, 1);
            // top
            glVertex3f(handles_textures[0].p.x, handles_textures[0].p.y, 0);
            glVertex3f(handles_textures[1].p.x, handles_textures[0].p.y, 0);

            // bottom
            glVertex3f(handles_textures[0].p.x, handles_textures[1].p.y, 0);
            glVertex3f(handles_textures[1].p.x, handles_textures[1].p.y, 0);

            // left
            glVertex3f(handles_textures[0].p.x, handles_textures[0].p.y, 0);
            glVertex3f(handles_textures[0].p.x, handles_textures[1].p.y, 0);

            // right
            glVertex3f(handles_textures[1].p.x, handles_textures[0].p.y, 0);
            glVertex3f(handles_textures[1].p.x, handles_textures[1].p.y, 0);
            glEnd();

            if (num_texture_points_rect == 3)
            {
                // draw line segments
                glBegin(GL_LINES);
                glColor3f(0, 1, 1);
                // vertical
                glVertex3f(texture_points_rect[0].x, handles_textures[0].p.y, 0);
                glVertex3f(texture_points_rect[0].x, handles_textures[1].p.y, 0);

                glVertex3f(texture_points_rect[1].x, handles_textures[0].p.y, 0);
                glVertex3f(texture_points_rect[1].x, handles_textures[1].p.y, 0);

                glVertex3f(texture_points_rect[2].x, handles_textures[0].p.y, 0);
                glVertex3f(texture_points_rect[2].x, handles_textures[1].p.y, 0);
                glEnd();

                // draw line segments
                glBegin(GL_LINES);
                glColor3f(0, 1, 1);
                // horizontal
                glVertex3f(handles_textures[0].p.x, texture_points_rect[0].y, 0);
                glVertex3f(handles_textures[1].p.x, texture_points_rect[0].y, 0);

                glVertex3f(handles_textures[0].p.x, texture_points_rect[1].y, 0);
                glVertex3f(handles_textures[1].p.x, texture_points_rect[1].y, 0);

                glVertex3f(handles_textures[0].p.x, texture_points_rect[2].y, 0);
                glVertex3f(handles_textures[1].p.x, texture_points_rect[2].y, 0);
                glEnd();
            };

            glEnable(GL_TEXTURE_2D);
        };


        for (int i = 0; i < 8; i++)
        {
            mouse_in_button(&button_text_field_textures[i], x_gl, y_gl);

            clamp_button(&button_text_field_textures[i], (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), CLAMP_TOP_LEFT, 0.1f, 0.1f + (i * 0.15f));

            draw_button(&button_text_field_textures[i]);
            //if (display_text_error == true)
                //drawText(button_text_field.p0.x, button_text_field.p0.y + 0.15f, 0.02f, "digit please");

            const char* coords[] = { "Tx0", "Ty0", "Tx1", "Ty1", "Tx2", "Ty2" };
            short tex_coords[] =
            {
                terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx0,
                terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty0,

                terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx1,
                terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty1,

                terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx2,
                terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty2,
            };

            if (i < 6)
            {
                sprintf(buffer, "%s: %d", coords[i], tex_coords[i]);
                drawText(button_text_field_textures[i].p1.x + 0.1f, button_text_field_textures[i].p0.y, 0.02f, buffer);
            }
            else
            {
                if (i == 6) drawText(button_text_field_textures[6].p1.x + 0.1f, button_text_field_textures[6].p0.y, 0.02f, "x grid");
                if (i == 7) drawText(button_text_field_textures[7].p1.x + 0.1f, button_text_field_textures[7].p0.y, 0.02f, "y grid");
            };
        };
        break;

    case MENU_SHOW:

        // window handling
        glPushMatrix();
        mouse_in_window(&window_palette, x_gl, y_gl);
        draw_window(&window_palette);
        glPopMatrix();

        // button handling
        glPushMatrix();

        if (showPalette == true)
        {
            unsigned char color_index = 16;

            // color palette
            for (int i = 0; i < 16; i++)
            {
                mouse_in_button(&button_palette[i], x_gl, y_gl);

                // clamp to the left and offset each color from the same side
                clamp_button(&button_palette[i], (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), CLAMP_LEFT, 0, 0);
                button_palette[i].p0.x += (0.05f * i);
                button_palette[i].p1.x += (0.05f * i);

                // clamp to palette window
                //button_palette[i].p0.x = window_palette.p0.x + (0.05f * i);
                //button_palette[i].p1.x = window_palette.p0.x + (0.05f * i) + (0.05f * i);

                //button_palette[i].p0.y = window_palette.p0.y + (0.05f * i);
                //button_palette[i].p1.y = window_palette.p1.y + (0.05f * i);

                draw_button_color(&button_palette[i], &LBA2Palette.theColors[color_index]);
                color_index += 1;
            };
            for (int i = 16; i < 32; i++)
            {
                mouse_in_button(&button_palette[i], x_gl, y_gl);

                // clamp to the left and offset each color from the same side
                clamp_button(&button_palette[i], (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), CLAMP_LEFT, 0, 0);
                button_palette[i].p0.x += (0.05f * i) - 0.8f;
                button_palette[i].p1.x += (0.05f * i) - 0.8f;

                draw_button_color(&button_palette[i], &LBA2Palette.theColors[color_index]);
                color_index += 1;
            };
            for (int i = 32; i < 48; i++)
            {
                mouse_in_button(&button_palette[i], x_gl, y_gl);

                // clamp to the left and offset each color from the same side
                clamp_button(&button_palette[i], (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), CLAMP_LEFT, 0, 0);
                button_palette[i].p0.x += (0.05f * i) - 1.6f;
                button_palette[i].p1.x += (0.05f * i) - 1.6f;

                draw_button_color(&button_palette[i], &LBA2Palette.theColors[color_index]);
                color_index += 1;
            };
            for (int i = 48; i < 64; i++)
            {
                mouse_in_button(&button_palette[i], x_gl, y_gl);

                // clamp to the left and offset each color from the same side
                clamp_button(&button_palette[i], (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), CLAMP_LEFT, 0, 0);
                button_palette[i].p0.x += (0.05f * i) - 2.4f;
                button_palette[i].p1.x += (0.05f * i) - 2.4f;

                draw_button_color(&button_palette[i], &LBA2Palette.theColors[color_index]);
                color_index += 1;
            };
            for (int i = 64; i < 80; i++)
            {
                mouse_in_button(&button_palette[i], x_gl, y_gl);
                draw_button_color(&button_palette[i], &LBA2Palette.theColors[color_index]);
                color_index += 1;
            };
            for (int i = 80; i < 96; i++)
            {
                mouse_in_button(&button_palette[i], x_gl, y_gl);
                draw_button_color(&button_palette[i], &LBA2Palette.theColors[color_index]);
                color_index += 1;
            };
            for (int i = 96; i < 112; i++)
            {
                mouse_in_button(&button_palette[i], x_gl, y_gl);
                draw_button_color(&button_palette[i], &LBA2Palette.theColors[color_index]);
                color_index += 1;
            };
        };

        for (int i = 0; i < 8; i++)
        {
            mouse_in_button(&button_tabs[i], x_gl, y_gl);
            //clamp_button(&button_tabs[i], (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), CLAMP_RIGHT, 0, 0.0f);
            clamp_button(&button_tabs[i], (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), CLAMP_TOP_RIGHT, 0.1f, 0.1f + (i * 0.1f));
            draw_button(&button_tabs[i]);
        };

        glPopMatrix();

        break;
    };

    if (current_menu == MENU_EDIT_POLYS)
    {
        // draw poly properties
        glDisable(GL_TEXTURE_2D);
        glColor3f(1, 1, 1);

        char buffer[100];
        float top_poly_data = -0.55f;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // draw back of poly info
        glBegin(GL_QUADS);

        glColor4f(0, 0, 0, 0.5f);
        glVertex3f(-0.07f, top_poly_data, 0);

        glColor4f(0, 0, 0, 0.5f);
        glVertex3f(0.5f, top_poly_data, 0);

        glColor4f(0, 0, 0, 0.5f);
        glVertex3f(0.5f, top_poly_data + 0.5f, 0);

        glColor4f(0, 0, 0, 0.5f);
        glVertex3f(-0.07f, top_poly_data + 0.5f, 0);

        glEnd();
        //

        if (selectedPoly == -1) // show current poly info
        {
            sprintf(buffer, "currentPoly %d", currentPoly);
            drawText(-0.9f, top_poly_data, 0.02f, buffer);

            sprintf(buffer, "Bank %s", poly_colors_texts[terrainPolys[currentPoly].Bank]);
            drawText(-0.9f, top_poly_data + (-0.05f), 0.02f, buffer);

            sprintf(buffer, "TexFlag %d", terrainPolys[currentPoly].TexFlag);
            drawText(-0.9f, top_poly_data + (-0.05f * 2), 0.02f, buffer);

            sprintf(buffer, "PolyFlag %d", terrainPolys[currentPoly].PolyFlag);
            drawText(-0.9f, top_poly_data + (-0.05f * 3), 0.02f, buffer);

            sprintf(buffer, "SampleStep %s, %d", sample_steps_texts[terrainPolys[currentPoly].SampleStep], terrainPolys[currentPoly].SampleStep);
            drawText(-0.9f, top_poly_data + (-0.05f * 4), 0.02f, buffer);

            sprintf(buffer, "CodeJeu %s", code_jeu_texts[terrainPolys[currentPoly].CodeJeu]);
            drawText(-0.9f, top_poly_data + (-0.05f * 5), 0.02f, buffer);

            sprintf(buffer, "Sens %d", terrainPolys[currentPoly].Sens);
            drawText(-0.9f, top_poly_data + (-0.05f * 6), 0.02f, buffer);

            sprintf(buffer, "Col %d", terrainPolys[currentPoly].Col);
            drawText(-0.9f, top_poly_data + (-0.05f * 7), 0.02f, buffer);

            // we would display terrainPolys[currentPoly].Dummy data here but it doesn't actually mean anything

            sprintf(buffer, "IndexTex %d", terrainPolys[currentPoly].IndexTex);
            drawText(-0.9f, top_poly_data + (-0.05f * 8), 0.02f, buffer);
        }
        else // show selected poly info
        {
            sprintf(buffer, "currentPoly %d", selectedPoly);
            drawText(-0.9f, top_poly_data, 0.02f, buffer);

            sprintf(buffer, "Bank %s", poly_colors_texts[terrainPolys[selectedPoly].Bank]);
            drawText(-0.9f, top_poly_data + (-0.05f), 0.02f, buffer);

            sprintf(buffer, "TexFlag %d", terrainPolys[selectedPoly].TexFlag);
            drawText(-0.9f, top_poly_data + (-0.05f * 2), 0.02f, buffer);

            sprintf(buffer, "PolyFlag %d", terrainPolys[selectedPoly].PolyFlag);
            drawText(-0.9f, top_poly_data + (-0.05f * 3), 0.02f, buffer);

            sprintf(buffer, "SampleStep %s, %d", sample_steps_texts[terrainPolys[selectedPoly].SampleStep], terrainPolys[selectedPoly].SampleStep);
            drawText(-0.9f, top_poly_data + (-0.05f * 4), 0.02f, buffer);

            sprintf(buffer, "CodeJeu %s", code_jeu_texts[terrainPolys[selectedPoly].CodeJeu]);
            drawText(-0.9f, top_poly_data + (-0.05f * 5), 0.02f, buffer);

            sprintf(buffer, "Sens %d", terrainPolys[selectedPoly].Sens);
            drawText(-0.9f, top_poly_data + (-0.05f * 6), 0.02f, buffer);

            sprintf(buffer, "Col %d", terrainPolys[selectedPoly].Col);
            drawText(-0.9f, top_poly_data + (-0.05f * 7), 0.02f, buffer);

            // we would display terrainPolys[selectedPoly].Dummy data here but it doesn't actually mean anything

            sprintf(buffer, "IndexTex %d", terrainPolys[selectedPoly].IndexTex);
            drawText(-0.9f, top_poly_data + (-0.05f * 8), 0.02f, buffer);
        };
    };

    if (current_menu == MENU_EDIT_BUILDINGS)
    {
        char buffer[100];
        float top_poly_data = -0.55f;

        sprintf(buffer, "current_building %d", current_building);
        drawText(-0.9f, top_poly_data, 0.02f, buffer);

        sprintf(buffer, "modelId %d", buildings[current_building].modelId);
        drawText(-0.9f, top_poly_data + (-0.05f), 0.02f, buffer);

        sprintf(buffer, "rotation %d, %.2f", buildings[current_building].rotation, (float)(buildings[current_building].rotation / 360));
        drawText(-0.9f, top_poly_data + (-0.05f * 2), 0.02f, buffer);
    };

    glColor3f(1, 1, 1);
    glEnable(GL_TEXTURE_2D);

    // Draw highlighted polygon
    //glPushMatrix();

    //glTranslatef(-0.9f, 0.9f, 0.0f);
    //glTranslatef(-0.8f, 0.7f, 0.0f);
    //glTranslatef(0.0f, -0.45f, 0.0f);
    //glRotatef(90.0f, 0, 0, 1);
    //glScalef(-1, -1, 1);

    float mov_x = -0.65f;
    //float mov_y = -0.525f;
    float mov_y = 0.65f;
    float tex_scale = 0.0058f;
    //float tex_scale = (1.0f / 65536.0f);
    float texture_calc = (1.0f / 65536.0f);

    float mov_x_dropdown = 1.5f;

    // draw texture window
    
    if (current_menu == MENU_EDIT_TEXTURES)
    {
        //mov_x = 1.5f;
        mov_x = 1.0f;

        if (showTextures == true)
        {
            glPushMatrix();

            glScalef(-1, -1, 1);

            glDisable(GL_CULL_FACE);
            glBegin(GL_TRIANGLES);

            // show texture window
            // DO NOT EDIT, orientation is correct
            // triangle 0
            glTexCoord2f(1, 0);
            glVertex3f(mov_x + (32767 * 0.001 * tex_scale) * -1, mov_y + (-32767 * 0.001 * tex_scale) * -1, -1);

            glTexCoord2f(1, 1);
            glVertex3f(mov_x + (32767 * 0.001 * tex_scale) * -1, mov_y + (32767 * 0.001 * tex_scale) * -1, -1);

            glTexCoord2f(0, 1);
            glVertex3f(mov_x + (-32767 * 0.001 * tex_scale) * -1, mov_y + (32767 * 0.001 * tex_scale) * -1, -1);

            // triangle 1

            glTexCoord2f(1, 0);
            glVertex3f(mov_x + (32767 * 0.001 * tex_scale) * -1, mov_y + (-32767 * 0.001 * tex_scale) * -1, -1);

            glTexCoord2f(0, 1);
            glVertex3f(mov_x + (-32767 * 0.001 * tex_scale) * -1, mov_y + (32767 * 0.001 * tex_scale) * -1, -1);

            glTexCoord2f(0, 0);
            glVertex3f(mov_x + (-32767 * 0.001 * tex_scale) * -1, mov_y + (-32767 * 0.001 * tex_scale) * -1, -1);

            glEnd();
            glEnable(GL_CULL_FACE);

            glPopMatrix();
        };
    };

    if (current_menu == MENU_EDIT_TEXTURES)
    {
        mov_x = mov_y = 0.0f;

        glDisable(GL_TEXTURE_2D);

        char buffer[25];
        sprintf(buffer, "indexTex: %d", current_terrain_tex);
        drawText(mov_x_dropdown - 0.5f, 0.0f, 0.02f, buffer);

        drawText(-0.5f, 0.95f, 0.02f, "Press P and O to traverse texture hierarchy");
        drawText(-0.5f, 0.95f - 0.05f, 0.02f, "Press K and L to change selected poly texture index");
        drawText(-0.5f, 0.95f - 0.10f, 0.02f, "Press M to scale selected poly texture");
        drawText(-0.5f, 0.95f - 0.15f, 0.02f, "Press N to snap texture coordinates");
        drawText(-0.5f, 0.95f - 0.20f, 0.02f, "Press X, Y, or C to change axes");
        drawText(-0.5f, 0.95f - 0.25f, 0.02f, "Press escape to deselect verts and / or exit");

        glEnable(GL_TEXTURE_2D);

        glPushMatrix();

        // draw all textures in window at once
        for (int i = 0; i < numTerrainTexs; i++)
        {
            // draw wireframes
            /*
            glBegin(GL_LINES);
            glColor3f(1, 0, 1);
            glVertex3f(mov_x + (terrainTexs[i].Tx0 * 0.001 * tex_scale) * -edit_tex_scale, mov_y + (terrainTexs[i].Ty0 * 0.001 * tex_scale) * -edit_tex_scale, -1);
            glVertex3f(mov_x + (terrainTexs[i].Tx1 * 0.001 * tex_scale) * -edit_tex_scale, mov_y + (terrainTexs[i].Ty1 * 0.001 * tex_scale) * -edit_tex_scale, -1);
            glEnd();

            glBegin(GL_LINES);
            glColor3f(1, 0, 1);
            glVertex3f(mov_x + (terrainTexs[i].Tx1 * 0.001 * tex_scale) * -edit_tex_scale, mov_y + (terrainTexs[i].Ty1 * 0.001 * tex_scale) * -edit_tex_scale, -1);
            glVertex3f(mov_x + (terrainTexs[i].Tx2 * 0.001 * tex_scale) * -edit_tex_scale, mov_y + (terrainTexs[i].Ty2 * 0.001 * tex_scale) * -edit_tex_scale, -1);
            glEnd();

            glBegin(GL_LINES);
            glColor3f(1, 0, 1);
            glVertex3f(mov_x + (terrainTexs[i].Tx2 * 0.001 * tex_scale) * -edit_tex_scale, mov_y + (terrainTexs[i].Ty2 * 0.001 * tex_scale) * -edit_tex_scale, -1);
            glVertex3f(mov_x + (terrainTexs[i].Tx0 * 0.001 * tex_scale) * -edit_tex_scale, mov_y + (terrainTexs[i].Ty0 * 0.001 * tex_scale) * -edit_tex_scale, -1);
            glEnd();
            */

            // draw selected poly
            glDisable(GL_TEXTURE_2D);

			glBegin(GL_LINES);
			glColor3f(1, 1, 1);
			glVertex3f(mov_x + (terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx0 * 0.001 * tex_scale) * -edit_tex_scale,
				mov_y + (terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty0 * 0.001 * tex_scale) * -edit_tex_scale,
				-1);
			glVertex3f(mov_x + (terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx1 * 0.001 * tex_scale) * -edit_tex_scale,
				mov_y + (terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty1 * 0.001 * tex_scale) * -edit_tex_scale,
				-1);
			glEnd();

			glBegin(GL_LINES);
			glColor3f(1, 1, 1);
			glVertex3f(mov_x + (terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx1 * 0.001 * tex_scale) * -edit_tex_scale,
				mov_y + (terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty1 * 0.001 * tex_scale) * -edit_tex_scale,
				-1);
			glVertex3f(mov_x + (terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx2 * 0.001 * tex_scale) * -edit_tex_scale,
				mov_y + (terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty2 * 0.001 * tex_scale) * -edit_tex_scale,
				-1);
			glEnd();

			glBegin(GL_LINES);
			glColor3f(1, 1, 1);
			glVertex3f(mov_x + (terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx2 * 0.001 * tex_scale) * -edit_tex_scale,
				mov_y + (terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty2 * 0.001 * tex_scale) * -edit_tex_scale,
				-1);
			glVertex3f(mov_x + (terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx0 * 0.001 * tex_scale) * -edit_tex_scale,
				mov_y + (terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty0 * 0.001 * tex_scale) * -edit_tex_scale,
				-1);
			glEnd();

            handles[0].p.x = mov_x + (terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx0 * 0.001 * tex_scale) * -edit_tex_scale;
            handles[0].p.y = mov_y + (terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty0 * 0.001 * tex_scale) * -edit_tex_scale;

            handles[1].p.x = mov_x + (terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx1 * 0.001 * tex_scale) * -edit_tex_scale;
            handles[1].p.y = mov_y + (terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty1 * 0.001 * tex_scale) * -edit_tex_scale;

            handles[2].p.x = mov_x + (terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx2 * 0.001 * tex_scale) * -edit_tex_scale;
            handles[2].p.y = mov_y + (terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty2 * 0.001 * tex_scale) * -edit_tex_scale;

            float pan_x = 0.0f;
            float pan_y = 0.0f;

            //float tex_margin = 0.188f;

            glBegin(GL_LINES);
            glVertex3f(edit_tex_scale * tex_margin, -0.25f, 0);
            glVertex3f(edit_tex_scale * tex_margin, 0.25f, 0);
            glEnd();

            glBegin(GL_LINES);
            glVertex3f(edit_tex_scale * -tex_margin, -0.25f, 0);
            glVertex3f(edit_tex_scale * -tex_margin, 0.25f, 0);
            glEnd();

            glBegin(GL_LINES);
            glVertex3f(-0.25f, edit_tex_scale * tex_margin, 0);
            glVertex3f(0.25f, edit_tex_scale * tex_margin, 0);
            glEnd();

            glBegin(GL_LINES);
            glVertex3f(-0.25f, edit_tex_scale* -tex_margin, 0);
            glVertex3f(0.25f, edit_tex_scale* -tex_margin, 0);
            glEnd();

            //if (x_grid != 1 && y_grid != 1)
                //use_snap = true;

            const int TEX_MIN = -32768;
            const int TEX_MAX = 32767;

            //const int TEX_MIN_SNAP = (TEX_MIN / x_grid) * x_grid;
            //const int TEX_MAX_SNAP = (TEX_MAX / x_grid) * x_grid;

            const int TEX_MIN_SNAP = snap_up(TEX_MIN, x_grid);
            const int TEX_MAX_SNAP = snap_down(TEX_MAX, x_grid);

            //int tx = 0;
            //int ty = 0;

            int tx_prev = 0;
            int ty_prev = 0;

            int dx = 0;
            int dy = 0;

			for (int j = 0; j < 3; j++)
			{
				mouse_in_handle(&handles[j], x_gl, y_gl, 0.01f);

				if (handles[j].selected == true)
				{
                    switch (j)
                    {
                    case 0:
                        /*if (x_gl < (edit_tex_scale * tex_margin) &&
                            x_gl > (edit_tex_scale * -tex_margin) &&
                            y_gl < (edit_tex_scale * tex_margin) &&
                            y_gl > (edit_tex_scale * -tex_margin))
                        {*/
                            // original
                            /*terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx0 = (short)((-x_gl * (65535 * 2.65f)) / edit_tex_scale) + pan_x;
                            terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty0 = (short)((-y_gl * (65535 * 2.65f)) / edit_tex_scale) + pan_y;

                            // snap if grid is set
                            if (use_snap == true)
                            {
                                terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx0 = snap_int(terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx0, x_grid);
                                terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty0 = snap_int(terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty0, y_grid);
                            };*/

                        // original
                        //tx = (int)((-x_gl * (65535 * 2.65f)) / edit_tex_scale) + pan_x;
                        //ty = (int)((-y_gl * (65535 * 2.65f)) / edit_tex_scale) + pan_y;

						dx = (int)((-(x_gl - drag_start_x_gl) * (65535 * 2.65f)) / edit_tex_scale) + pan_x;
						dy = (int)((-(y_gl - drag_start_y_gl) * (65535 * 2.65f)) / edit_tex_scale) + pan_y;

						if (current_axis == AXIS_X)
						{
							tx = drag_start_tx + dx;
							ty = drag_start_ty;
						}
						else if (current_axis == AXIS_Y)
						{
							tx = drag_start_tx;
							ty = drag_start_ty + dy;
						}
						else if (current_axis == AXIS_X_Y)
						{
							tx = (int)((-x_gl * (65535 * 2.65f)) / edit_tex_scale) + pan_x;
							ty = (int)((-y_gl * (65535 * 2.65f)) / edit_tex_scale) + pan_y;

							//drag_start_tx = (int)((-x_gl * (65535 * 2.65f)) / edit_tex_scale) + pan_x;
							//drag_start_ty = (int)((-y_gl * (65535 * 2.65f)) / edit_tex_scale) + pan_y;

							//tx = drag_start_tx + dx;
							//ty = drag_start_ty + dy;
						};

						// snap if grid is set
						if (use_snap == true)
						{
							tx = snap_int(tx, x_grid);
							ty = snap_int(ty, y_grid);

							//tx = int_clamp(tx, TEX_MIN_SNAP, TEX_MAX_SNAP);
							//ty = int_clamp(ty, TEX_MIN_SNAP, TEX_MAX_SNAP);
						};

						tx = int_max(TEX_MIN, int_min(TEX_MAX, tx));
						ty = int_max(TEX_MIN, int_min(TEX_MAX, ty));

						//tx = int_clamp(tx, TEX_MIN, TEX_MAX);
						//ty = int_clamp(ty, TEX_MIN, TEX_MAX);

						terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx0 = (short)tx;
						terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty0 = (short)ty;

                        break;

                    case 1:
                        dx = (int)((-(x_gl - drag_start_x_gl) * (65535 * 2.65f)) / edit_tex_scale) + pan_x;
                        dy = (int)((-(y_gl - drag_start_y_gl) * (65535 * 2.65f)) / edit_tex_scale) + pan_y;

                        if (current_axis == AXIS_X)
                        {
                            tx = drag_start_tx + dx;
                            ty = drag_start_ty;
                        }
                        else if (current_axis == AXIS_Y)
                        {
                            tx = drag_start_tx;
                            ty = drag_start_ty + dy;
                        }
                        else if (current_axis == AXIS_X_Y)
                        {
                            tx = (int)((-x_gl * (65535 * 2.65f)) / edit_tex_scale) + pan_x;
                            ty = (int)((-y_gl * (65535 * 2.65f)) / edit_tex_scale) + pan_y;

                            //drag_start_tx = tx;
                            //drag_start_ty = ty;

                            //tx = drag_start_tx + dx;
                            //ty = drag_start_ty + dy;
                        };

                        // snap if grid is set
                        if (use_snap == true)
                        {
                            tx = snap_int(tx, x_grid);
                            ty = snap_int(ty, y_grid);

                            //tx = int_clamp(tx, TEX_MIN_SNAP, TEX_MAX_SNAP);
                            //ty = int_clamp(ty, TEX_MIN_SNAP, TEX_MAX_SNAP);
                        };

                        tx = int_max(TEX_MIN, int_min(TEX_MAX, tx));
                        ty = int_max(TEX_MIN, int_min(TEX_MAX, ty));

                        //tx = int_clamp(tx, TEX_MIN, TEX_MAX);
                        //ty = int_clamp(ty, TEX_MIN, TEX_MAX);

                        terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx1 = (short)tx;
                        terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty1 = (short)ty;
                        break;

                    case 2:
                        dx = (int)((-(x_gl - drag_start_x_gl) * (65535 * 2.65f)) / edit_tex_scale) + pan_x;
                        dy = (int)((-(y_gl - drag_start_y_gl) * (65535 * 2.65f)) / edit_tex_scale) + pan_y;

                        if (current_axis == AXIS_X)
                        {
                            tx = drag_start_tx + dx;
                            ty = drag_start_ty;
                        }
                        else if (current_axis == AXIS_Y)
                        {
                            tx = drag_start_tx;
                            ty = drag_start_ty + dy;
                        }
                        else if (current_axis == AXIS_X_Y)
                        {
                            tx = (int)((-x_gl * (65535 * 2.65f)) / edit_tex_scale) + pan_x;
                            ty = (int)((-y_gl * (65535 * 2.65f)) / edit_tex_scale) + pan_y;

                            //drag_start_tx = tx;
                            //drag_start_ty = ty;

                            //tx = drag_start_tx + dx;
                            //ty = drag_start_ty + dy;
                        };

                        // snap if grid is set
                        if (use_snap == true)
                        {
                            tx = snap_int(tx, x_grid);
                            ty = snap_int(ty, y_grid);

                            //tx = int_clamp(tx, TEX_MIN_SNAP, TEX_MAX_SNAP);
                            //ty = int_clamp(ty, TEX_MIN_SNAP, TEX_MAX_SNAP);
                        };

                        tx = int_max(TEX_MIN, int_min(TEX_MAX, tx));
                        ty = int_max(TEX_MIN, int_min(TEX_MAX, ty));

                        //tx = int_clamp(tx, TEX_MIN, TEX_MAX);
                        //ty = int_clamp(ty, TEX_MIN, TEX_MAX);

                        terrainTexs[terrainPolys[selectedPoly].IndexTex].Tx2 = (short)tx;
                        terrainTexs[terrainPolys[selectedPoly].IndexTex].Ty2 = (short)ty;
                        break;
                    };
				};

				draw_handle(&handles[j], 0.01f);
			};

            glEnable(GL_TEXTURE_2D);

            // same as above but textured to allow for better previews
            glDisable(GL_CULL_FACE);
            glBegin(GL_TRIANGLES);

            glTexCoord2f(terrainTexs[i].Tx0 * texture_calc, terrainTexs[i].Ty0 * texture_calc);
            glColor3f(1, 1, 1);
            glVertex3f(mov_x + (terrainTexs[i].Tx0 * 0.001 * tex_scale) * -edit_tex_scale, mov_y + (terrainTexs[i].Ty0 * 0.001 * tex_scale) * -edit_tex_scale, -1);

            glTexCoord2f(terrainTexs[i].Tx1 * texture_calc, terrainTexs[i].Ty1 * texture_calc);
            glColor3f(1, 1, 1);
            glVertex3f(mov_x + (terrainTexs[i].Tx1 * 0.001 * tex_scale) * -edit_tex_scale, mov_y + (terrainTexs[i].Ty1 * 0.001 * tex_scale) * -edit_tex_scale, -1);

            glTexCoord2f(terrainTexs[i].Tx2 * texture_calc, terrainTexs[i].Ty2 * texture_calc);
            glColor3f(1, 1, 1);
            glVertex3f(mov_x + (terrainTexs[i].Tx2 * 0.001 * tex_scale) * -edit_tex_scale, mov_y + (terrainTexs[i].Ty2 * 0.001 * tex_scale) * -edit_tex_scale, -1);
            
            glEnd();

            // draw texture drop down on the right side
            if (i == terrainPolys[selectedPoly].IndexTex) // draw red
            {
                glBegin(GL_TRIANGLES);

                glTexCoord2f(terrainTexs[i].Tx0 * texture_calc, terrainTexs[i].Ty0 * texture_calc);
                glColor3f(1, 0, 0);
                glVertex3f(mov_x_dropdown + (0.0f) * -1,
                    mov_y + edit_tex_slider + (i * -0.1f) + (0.0f) * -1,
                    -1);

                glTexCoord2f(terrainTexs[i].Tx1 * texture_calc, terrainTexs[i].Ty1 * texture_calc);
                glColor3f(1, 0, 0);
                glVertex3f(mov_x_dropdown + (0.0f) * -1,
                    mov_y + edit_tex_slider + (i * -0.1f) + (0.1f) * -1,
                    -1);

                glTexCoord2f(terrainTexs[i].Tx2 * texture_calc, terrainTexs[i].Ty2 * texture_calc);
                glColor3f(1, 0, 0);
                glVertex3f(mov_x_dropdown + (0.1f) * -1,
                    mov_y + edit_tex_slider + (i * -0.1f) + (0.0f) * -1,
                    -1);

                glEnd();
            }
            else // draw white
            {
                glBegin(GL_TRIANGLES);

                glTexCoord2f(terrainTexs[i].Tx0* texture_calc, terrainTexs[i].Ty0* texture_calc);
                glColor3f(1, 1, 1);
                glVertex3f(mov_x_dropdown + (0.0f) * -1,
                    mov_y + edit_tex_slider + (i * -0.1f) + (0.0f) * -1,
                    -1);

                glTexCoord2f(terrainTexs[i].Tx1* texture_calc, terrainTexs[i].Ty1* texture_calc);
                glColor3f(1, 1, 1);
                glVertex3f(mov_x_dropdown + (0.0f) * -1,
                    mov_y + edit_tex_slider + (i * -0.1f) + (0.1f) * -1,
                    -1);

                glTexCoord2f(terrainTexs[i].Tx2* texture_calc, terrainTexs[i].Ty2* texture_calc);
                glColor3f(1, 1, 1);
                glVertex3f(mov_x_dropdown + (0.1f) * -1,
                    mov_y + edit_tex_slider + (i * -0.1f) + (0.0f) * -1,
                    -1);

                glEnd();
            };

            glEnable(GL_CULL_FACE);
        };

        glPopMatrix();
    };

    glDisable(GL_TEXTURE_2D);

    if (current_menu != MENU_EDIT_TEXTURES)
    {
        if (showTextures == true)
        {
            glPushMatrix();

            // draw all textures in window at once
            for (int i = 0; i < numTerrainTexs; i++)
            {
                glBegin(GL_LINES);
                glColor3f(1, 0, 1);
                glVertex3f(mov_x + (terrainTexs[i].Tx0 * 0.001 * tex_scale) * -1, mov_y + (terrainTexs[i].Ty0 * 0.001 * tex_scale) * -1, -1);
                glVertex3f(mov_x + (terrainTexs[i].Tx1 * 0.001 * tex_scale) * -1, mov_y + (terrainTexs[i].Ty1 * 0.001 * tex_scale) * -1, -1);
                glEnd();

                glBegin(GL_LINES);
                glColor3f(1, 0, 1);
                glVertex3f(mov_x + (terrainTexs[i].Tx1 * 0.001 * tex_scale) * -1, mov_y + (terrainTexs[i].Ty1 * 0.001 * tex_scale) * -1, -1);
                glVertex3f(mov_x + (terrainTexs[i].Tx2 * 0.001 * tex_scale) * -1, mov_y + (terrainTexs[i].Ty2 * 0.001 * tex_scale) * -1, -1);
                glEnd();

                glBegin(GL_LINES);
                glColor3f(1, 0, 1);
                glVertex3f(mov_x + (terrainTexs[i].Tx2 * 0.001 * tex_scale) * -1, mov_y + (terrainTexs[i].Ty2 * 0.001 * tex_scale) * -1, -1);
                glVertex3f(mov_x + (terrainTexs[i].Tx0 * 0.001 * tex_scale) * -1, mov_y + (terrainTexs[i].Ty0 * 0.001 * tex_scale) * -1, -1);
                glEnd();

                // draw selected poly
                glBegin(GL_LINES);
                glColor3f(1, 1, 1);
                glVertex3f(mov_x + (terrainTexs[terrainPolys[currentPoly].IndexTex].Tx0 * 0.001 * tex_scale) * -1,
                    mov_y + (terrainTexs[terrainPolys[currentPoly].IndexTex].Ty0 * 0.001 * tex_scale) * -1,
                    -1);
                glVertex3f(mov_x + (terrainTexs[terrainPolys[currentPoly].IndexTex].Tx1 * 0.001 * tex_scale) * -1,
                    mov_y + (terrainTexs[terrainPolys[currentPoly].IndexTex].Ty1 * 0.001 * tex_scale) * -1,
                    -1);
                glEnd();

                glBegin(GL_LINES);
                glColor3f(1, 1, 1);
                glVertex3f(mov_x + (terrainTexs[terrainPolys[currentPoly].IndexTex].Tx1 * 0.001 * tex_scale) * -1,
                    mov_y + (terrainTexs[terrainPolys[currentPoly].IndexTex].Ty1 * 0.001 * tex_scale) * -1,
                    -1);
                glVertex3f(mov_x + (terrainTexs[terrainPolys[currentPoly].IndexTex].Tx2 * 0.001 * tex_scale) * -1,
                    mov_y + (terrainTexs[terrainPolys[currentPoly].IndexTex].Ty2 * 0.001 * tex_scale) * -1,
                    -1);
                glEnd();

                glBegin(GL_LINES);
                glColor3f(1, 1, 1);
                glVertex3f(mov_x + (terrainTexs[terrainPolys[currentPoly].IndexTex].Tx2 * 0.001 * tex_scale) * -1,
                    mov_y + (terrainTexs[terrainPolys[currentPoly].IndexTex].Ty2 * 0.001 * tex_scale) * -1,
                    -1);
                glVertex3f(mov_x + (terrainTexs[terrainPolys[currentPoly].IndexTex].Tx0 * 0.001 * tex_scale) * -1,
                    mov_y + (terrainTexs[terrainPolys[currentPoly].IndexTex].Ty0 * 0.001 * tex_scale) * -1,
                    -1);
                glEnd();
            };

            glPopMatrix();
        };

        if (showTextures == true)
        {
            glPushMatrix();

            // draw selected poly as texture
            glEnable(GL_TEXTURE_2D);
            glBegin(GL_TRIANGLES);

            glTexCoord2f((terrainTexs[terrainPolys[currentPoly].IndexTex].Tx0 * texture_calc),
                (terrainTexs[terrainPolys[currentPoly].IndexTex].Ty0 * texture_calc));
            glColor3f(1, 1, 1);
            glVertex3f(-0.9f, -0.4f, 0);

            glTexCoord2f((terrainTexs[terrainPolys[currentPoly].IndexTex].Tx1 * texture_calc),
                (terrainTexs[terrainPolys[currentPoly].IndexTex].Ty1 * texture_calc));
            glColor3f(1, 1, 1);
            glVertex3f(-0.5f, -0.4f, 0);

            glTexCoord2f((terrainTexs[terrainPolys[currentPoly].IndexTex].Tx2 * texture_calc),
                (terrainTexs[terrainPolys[currentPoly].IndexTex].Ty2 * texture_calc));
            glColor3f(1, 1, 1);
            glVertex3f(-0.5f, 0.0f, 0);

            glEnd();
            glDisable(GL_TEXTURE_2D);

            glPopMatrix();
        };
    };



    glFlush();
    glutSwapBuffers();
};

void timer(int v)
{
    static GLfloat u = 0.0;
    u += 0.01;
    glLoadIdentity();
    //gluLookAt(theCamera.pos.x, theCamera.pos.y, theCamera.pos.z,
        //theCamera.look.x, theCamera.look.y, theCamera.look.z,
        //theCamera.upVec.x, theCamera.upVec.y, theCamera.upVec.z);
    glutPostRedisplay();
    glutTimerFunc(1000 / 60.0, timer, v);
};

void reshape(int w, int h)
{
    // stub
};

void init()
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
};

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1200, 1000);

    glutCreateWindow("LBA2 Terrain Editor");
    glutReshapeFunc(reshape);
    glutTimerFunc(100, timer, 0);
    glutDisplayFunc(display);

    init_button(&button_text_field, 0.5f, 0.8f, 0.4f, 0.1f, "field");
    button_text_field.is_text_field = true;
    button_text_field.is_focused = false;

    init_button(&button_polygon_brush_text_field, 0.5f, 0.8f, 0.4f, 0.1f, "field");
    button_polygon_brush_text_field.is_text_field = true;
    button_polygon_brush_text_field.is_focused = false;

    init_button(&button_polygon_reset_all, 0.5f, 1.0f, 0.4f, 0.1f, "reset all");

    for (int i = 0; i < 3; i++)
    {
        init_button(&button_texture_rectangles[i], 0.5f, 0.8f + (i * -0.1f), 0.4f, 0.1f, button_texture_rectangles_texts[i]);
    };

    for (int i = 0; i < 8; i++)
    {
        init_button(&button_text_field_textures[i], 0.5f, 0.8f + (i * -0.1f), 0.4f, 0.1f, "field");
        button_text_field_textures[i].is_text_field = true;
        button_text_field_textures[i].is_focused = false;
    };

    float current_island_x = 0.0f;
    float current_island_y = 0.0f;
    int current_island_index = 0;

    for (int i = 0; i < MAX_ISLAND_PARTS; i++)
    {
        init_button(&button_edit_island[i], 0.3f + (current_island_x), 0.8f + (current_island_y), 0.1f, 0.1f, "0");
        button_edit_island[i].is_text_field = true;
        button_edit_island[i].is_focused = false;

        button_edit_island[i].text_field_buffer[0] = '0';

        current_island_x -= 0.1f;
        current_island_index += 1;

        if (current_island_index >= 16)
        {
            current_island_x = 0.0f;
            current_island_y -= 0.1f;
            current_island_index = 0;
        };
	};

    for (int i = 0; i < 11; i++)
    {
        init_button(&button_main[i], 0.3f, 0.5f + (i * -0.1f), 0.4f, 0.1f, button_main_texts[i]);
    };

    for (int i = 0; i < 8; i++)
    {
        // right side
        init_button(&button_tabs[i], 0.5f, 0.5f + (i * -0.2f), 0.4f, 0.1f, button_texts[i]);

        // top
        //init_button(&button_tabs[i], -0.95f + (i * 0.4f), 0.85f, 0.3f, 0.1f, button_texts[i]);

        //init_button(&button_tabs[i], -3.0f + (i * 0.4f), 0.85f, 0.3f, 0.1f, button_texts[i]);
    };

    for (int i = 0; i < POLY_COLOR_PURPLE_1; i++)
    {
        init_button(&button_polygon_colors[i], 0.3f, 0.5f + (i * -0.1f), 0.4f, 0.1f, poly_colors_texts[i]);

        if (i > 0)
            button_polygon_colors[i].is_visible = false;
    };
    for (int i = 0; i < SAMPLE_STEP_16; i++)
    {
        init_button(&button_polygon_sample[i], 0.3f, 0.5f + (i * -0.1f), 0.4f, 0.1f, sample_steps_texts[i]);

        if (i > 0)
            button_polygon_sample[i].is_visible = false;
    };
    for (int i = 0; i < CJ_ANIMATED_GAZ; i++)
    {
        init_button(&button_polygon_code_jeu[i], 0.3f, 0.5f + (i * -0.1f), 0.4f, 0.1f, code_jeu_texts[i]);

        if (i > 0)
            button_polygon_code_jeu[i].is_visible = false;
    };
    for (int i = 0; i < 6; i++)
    {
        init_button(&button_polygon_operations[i], 0.3f + (i * 0.1f), 0.5f, 0.2f, 0.1f, polygon_operations_texts[i]);
    };

    for (int i = 0; i < 7; i++)
    {
        init_button(&button_building_operations[i], 0.3f + (i * 0.1f), 0.5f, 0.2f, 0.1f, building_operations_texts[i]);
    };

    for (int i = 0; i < 3; i++)
    {
        init_button(&button_terrain_operations[i], 0.3f + (i * 0.1f), 0.5f, 0.2f, 0.1f, terrain_operations_texts[i]);
    };

    for (int i = 0; i < 16; i++)
    {
        init_button(&button_palette[i], -0.9f + (i * 0.05f), 0.0f, 0.05f, 0.05f, "");
        button_palette[i].window = &window_palette;
    };
    for (int i = 16; i < 32; i++) init_button(&button_palette[i], -1.7f + (i * 0.05f), -0.05f, 0.05f, 0.05f, "");
    for (int i = 32; i < 48; i++) init_button(&button_palette[i], -2.5f + (i * 0.05f), -0.10f, 0.05f, 0.05f, "");
    for (int i = 48; i < 64; i++) init_button(&button_palette[i], -3.3f + (i * 0.05f), -0.15f, 0.05f, 0.05f, "");
    for (int i = 64; i < 80; i++) init_button(&button_palette[i], -4.1f + (i * 0.05f), -0.20f, 0.05f, 0.05f, "");
    for (int i = 80; i < 96; i++) init_button(&button_palette[i], -4.9f + (i * 0.05f), -0.25f, 0.05f, 0.05f, "");
    for (int i = 96; i < 112; i++) init_button(&button_palette[i], -5.7f + (i * 0.05f), -0.30f, 0.05f, 0.05f, "");

    init_window(&window_palette, 0, 0, 0.5f, 0.5f, "palette");

    for (int i = 0; i < MAX_VERTS; i++)
    {
        polyPoints[i].x = (currentColumn * 1.0f);
        polyPoints[i].y = -(currentRow * 1.0f);

        currentColumn += 1;

        if (currentColumn >= 65)
        {
            currentColumn = 0;
            currentRow += 1;
        };
    };

    //glGenTextures(0, (GLuint*)tex_concrete_1);

    //loadIsland("CITABAU01.lun");

    loadPal(&LBA2Palette, "RESS01.pal");

    //loadLIM(&groundTextures, "CITABAU02.lim", GROUND_TEXTURES);
    loadLIM(&groundTextures, "medieval_ground.lim", GROUND_TEXTURES);
    loadLIM(&buildingTextures, "CITABAU03.lim", BUILDING_TEXTURES);

    loadScene("SCENE051.ls2");

    /*loadObjects("CITABAU52.lun");
    loadBuildings("CITABAU53.lun");
    loadSea("CITABAU54.lun");
    loadTextures("CITABAU55.lun");
    loadTerrain("CITABAU56.lun");
    loadClouds("CITABAU57.lun");*/

    loadObjects("medieval_9_objects.lun");
    loadBuildings("medieval_9_buildings.lun");
    loadSea("medieval_sea.lun");
    loadTextures("medieval_textures.lun");
    loadTerrain("medieval_terrain.lun");
    loadClouds("medieval_clouds.lun");

    /*for (int i = 0; i < 115; i++)
    {
        char buffer[100];

        sprintf(buffer, "lm2/CITABAU%03d.lfn", i + 1);
        loadLM2(buffer, &models[i]);
        printf("loading %s \n", buffer);

        //loadLM2("lm2/CITABAU001.lfn", &models[0]);
    };*/

    init();
    glfwInit();

    glutKeyboardFunc(MyKeyboardFunc); // keyboard presses
    glutKeyboardUpFunc(MyKeyboardUpFunc); // keyboard releases
    //glutSpecialFunc(MyKeyboardSpecial);

    glutPassiveMotionFunc(mouseMotion);
    glutMotionFunc(mouseMotion);
    glutMouseFunc(mouseClick);

    glutMainLoop();
};