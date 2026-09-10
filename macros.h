#pragma once

#define SET_TEXTURE(data) \
glBindTexture(GL_TEXTURE_2D, *data);\
\
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);\
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);\
\
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);\
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);\
\
glTexImage2D(GL_TEXTURE_2D, 0, 4, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); // opaque

//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);\
//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);\

// bit macros
//#define SET_BIT(val, bitIndex) val |= (1 << bitIndex)
//#define CLEAR_BIT(val, bitIndex) val &= ~(1 << bitIndex)
//#define TOGGLE_BIT(val, bitIndex) val ^= (1 << bitIndex)
//#define BIT_IS_SET(val, bitIndex) (val & (1 << bitIndex))

//#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
//#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
//#define BIT_FLIP(a,b) ((a) ^= (1ULL<<(b)))
//#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b))))

// only set of bit macros that compile for some reason
#define BitSet(arg,posn) ((arg) | (1L << (posn)))
#define BitClr(arg,posn) ((arg) & ~(1L << (posn)))
#define BitTst(arg,posn) BOOL((arg) & (1L << (posn)))
#define BitFlp(arg,posn) ((arg) ^ (1L << (posn)))

// macros
// poly bit positions macros
#define BIT_POLY_BANK        0
#define BIT_POLY_TEX_FLAG    4
#define BIT_POLY_POLY_FLAG   6
#define BIT_POLY_SAMPLE_STEP 8
#define BIT_POLY_CODE_JEU    12
#define BIT_POLY_SENS        16
#define BIT_POLY_COL         17
#define BIT_POLY_DUMMY       18
#define BIT_POLY_INDEX_TEX   19

#define BOUNDING_BOX_TRIS 12

#define MAX_SQUARES 4096
#define MAX_VERTS 4225 // 65 * 65
#define MAX_POLYS 8192 // 64 * 64 * 2

#define MAX_COLORS 256

#define TEXTURE_X 256
#define TEXTURE_Y 256

#define MAX_TEXTURE 65536

#define MAX_ISLAND_PARTS 256

#define MAX_ISLAND_OBJECTS 0x28

// model macros
#define BONE_NUMBER    0x20
#define BONE_OFFSET    0x24
#define VERTEX_NUMBER  0x28
#define VERTEX_OFFSET  0x2C
#define NORMAL_NUMBER  0x30
#define NORMAL_OFFSET  0x34
#define UNKNOWN_NUMBER 0x38
#define UNKNOWN_OFFSET 0x3C
#define POLYGON_NUMBER 0x40
#define POLYGON_OFFSET 0x44
#define LINE_NUMBER    0x48
#define LINE_OFFSET    0x4C
#define SPHERE_NUMBER  0x50
#define SPHERE_OFFSET  0x54
#define TEXTURE_NUMBER 0x58
#define TEXTURE_OFFSET 0x5C
#define HEADER_END     0x60

#define POLYGON_TRIANGLE 0x00
#define POLYGON_QUAD     0x80

// file system macros
#define MAX_FILES 1000
#define MAX_BYTES 65536

typedef enum
{
	GROUND_TEXTURES,
	BUILDING_TEXTURES,
	SKY_TEXTURES,
	OBJECT_TEXTURES,
} en_textures;

typedef enum
{
	VOID_TYPE_INT,
	VOID_TYPE_SHORT,
	VOID_TYPE_STRING,
} en_void_types;

/*#define POLY_COLOR_PURPLE_0 0
#define POLY_COLOR_BROWN 1
#define POLY_COLOR_TAN 2
#define POLY_COLOR_WHITE 3
#define POLY_COLOR_RED 4
#define POLY_COLOR_ORANGE 5
#define POLY_COLOR_YELLOW 6
#define POLY_COLOR_ARMY_GREEN 7
#define POLY_COLOR_GREEN 8
#define POLY_COLOR_BLUE_GREEN 9
#define POLY_COLOR_CYAN 10
#define POLY_COLOR_GRAY 11
#define POLY_COLOR_BLUE 12
#define POLY_COLOR_PURPLE_1 13*/

typedef enum
{
	CLAMP_TOP_LEFT,
	CLAMP_TOP_RIGHT,
	CLAMP_BOTTOM_LEFT,
	CLAMP_BOTTOM_RIGHT,

	CLAMP_LEFT,
	CLAMP_RIGHT,
	CLAMP_TOP,
	CLAMP_BOTTOM,
} en_clamp_type;

typedef enum
{
	AXIS_NONE,
	AXIS_X,
	AXIS_Y,
	AXIS_Z,
	AXIS_X_Y,
} en_axis;

typedef enum
{
	POLY_COLOR_PURPLE_0,
	POLY_COLOR_BROWN,
	POLY_COLOR_TAN,
	POLY_COLOR_WHITE,
	POLY_COLOR_RED,
	POLY_COLOR_ORANGE,
	POLY_COLOR_YELLOW,
	POLY_COLOR_ARMY_GREEN,
	POLY_COLOR_GREEN,
	POLY_COLOR_BLUE_GREEN,
	POLY_COLOR_CYAN,
	POLY_COLOR_GRAY,
	POLY_COLOR_BLUE,
	POLY_COLOR_PURPLE_1,
} en_poly_colors;

typedef enum
{
	SAMPLE_STEP_NONE,
	SAMPLE_STEP_1,
	SAMPLE_STEP_GRASS,
	SAMPLE_STEP_3,
	SAMPLE_STEP_DIRT,
	SAMPLE_STEP_STONE,
	SAMPLE_STEP_6,
	SAMPLE_STEP_7,
	SAMPLE_STEP_8,
	SAMPLE_STEP_FLOWERS,
	SAMPLE_STEP_10,
	SAMPLE_STEP_WATER,
	SAMPLE_STEP_SAND,
	SAMPLE_STEP_13,
	SAMPLE_STEP_14,
	SAMPLE_STEP_15,
	SAMPLE_STEP_16,
} en_sample_steps;

typedef enum
{
	CJ_NONE,       // mine
	CJ_WATER,       // eau
	CJ_ELECTRIC,       // Pseudo électrifié (mort instantanée) -> "nickname electrified (instant death)"
	CJ_ESCALATOR_OUEST,       // Tapis roulant OUEST
	CJ_ESCALATOR_EST,       // Tapis roulant EST
	CJ_ESCALATOR_NORD,       // Tapis roulant NORD
	CJ_ESCALATOR_SUD,       // Tapis roulant SUD
	CJ_LABYRINTHE,	// Pseudo visible seulement sous
	// les pieds de l'objet
	CJ_POS_INVALID,	// Position non valide
	CJ_LAVE,	// psshhht !
	CJ_GAZ,	// prout !
	CJ_FOOT_WATER,	// Eau où l'on a pied
	CJ_ANIMATED_LAVE,
	CJ_ANIMATED_GAZ,
} en_code_jeu;

/*#define CJ_NONE                 0       // mine
#define CJ_WATER                1       // eau
#define CJ_ELECTRIC             2       // Pseudo électrifié (mort instantanée) -> "nickname electrified (instant death)"
#define CJ_ESCALATOR_OUEST      3       // Tapis roulant OUEST
#define CJ_ESCALATOR_EST        4       // Tapis roulant EST
#define CJ_ESCALATOR_NORD       5       // Tapis roulant NORD
#define CJ_ESCALATOR_SUD        6       // Tapis roulant SUD
#define	CJ_LABYRINTHE		7	// Pseudo visible seulement sous
                    // les pieds de l'objet
#define	CJ_POS_INVALID		8	// Position non valide
#define	CJ_LAVE			9	// psshhht !
#define	CJ_GAZ			11	// prout !
#define	CJ_FOOT_WATER		12	// Eau où l'on a pied
#define	CJ_ANIMATED_LAVE	13
#define	CJ_ANIMATED_GAZ		14*/