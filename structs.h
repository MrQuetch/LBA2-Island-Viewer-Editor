#pragma once

typedef struct
{
    unsigned char color[TEXTURE_X * TEXTURE_Y];
} st_lim;

typedef struct
{
    float r;
    float g;
    float b;
} st_color3f;

typedef struct
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} st_color3;

typedef struct
{
    st_color3 theColors[MAX_COLORS];
} st_pal;

typedef struct
{
    float x;
    float y;
    float z;
} st_vec3f;

typedef struct
{
    float x;
    float y;
} st_vec2f;

typedef struct
{
    short x;
    short y;
} st_short2;

typedef struct
{
    st_vec2f p0;
    st_vec2f p1;

    const char* text;
    bool hover;

    // this button is a void pointer since the button struct is defined later
    //void* button_close;
} st_window;

typedef struct
{
    st_vec2f p0;
    st_vec2f p1;

    const char* text;
    bool hover;

    bool is_text_field;
    bool is_focused;
    bool is_visible;

    unsigned char text_field_buffer[10];
    unsigned char current_char_in_field;

    bool is_drop_down;

    st_window* window;
} st_button;

typedef struct
{
    bool pressed_w;
    bool pressed_a;
    bool pressed_s;
    bool pressed_d;

    bool pressed_q;
    bool pressed_e;

    bool pressed_g;

    bool pressed_x;
    bool pressed_y;
    bool pressed_z;

    bool pressed_1;
    bool pressed_3;
} st_button_presses;

typedef struct
{
    st_vec2f p;
    bool hover;
    bool selected;
} st_handle;

/*typedef struct
{
    st_vec2f p0;
    st_vec2f p1;

    const char* text;
    bool hover;
} st_text_field;*/

typedef struct
{
    st_vec3f pos;
    st_vec3f look;
    st_vec3f upVec;
    st_vec3f lerpVec;

    float rot;
    float rot_lerp;
    float dist;
    float speed;

    bool lerp;
    bool center;
} st_camera;

typedef struct
{
    float value;
    unsigned char index;
} st_float_data;

typedef struct
{
    short data1;
    short data2;
    short data3;
    short data4;
    int   numOfBuildings; // 0x08
    int   data6;
    int   data7;
    int   data8;
    int   data9;
    int   data10;
    int   ff1;
    int   ff2;
} st_object_info;

typedef struct
{
    int modelId;   // 0x00
    int x;         // 0x04
    int y;	       // 0x08
    int z;		   // 0x0C

    int floorId;   // 0x10
    int rotation;  // 0x14
    int colLeft;   // 0x18
    int colBottom; // 0x1C

    int colUp;     // 0x20
    int colRight;  // 0x24
    int colTop;    // 0x28
    int colDown;   // 0x2C
} st_building;

typedef struct
{
    char  polyMat;
    char  polyType;
    short polyAmount;
    short polyBytes;
    short polyPadding;
} st_poly_block;

typedef struct
{
    int vertex;
    int vertexOffset;
    int polygon;
    int polygonOffset;
    int line;
    int lineOffset;

    short x[200];
    short y[200];
    short z[200];
    short bone[200];

    int poly_group_length;

    char  polyMat[10];
    char  polyType[10];
    short polyAmount[10];
    short polyBytes[10];
    short polyPadding1[530];

    char  polyColor[10][530];

    char  polyPadding2[530];
    short polyPadding3[530];

    // current element, current polygon vertex
    short polyV1[10][500];
    short polyV2[10][500];
    short polyV3[10][500];
    short polyV4[10][500];

    short UV1_x[10][500];
    short UV2_x[10][500];
    short UV3_x[10][500];
    short UV4_x[10][500];

    short UV1_y[10][500];
    short UV2_y[10][500];
    short UV3_y[10][500];
    short UV4_y[10][500];
} st_model;

// from LBA2 source code (in COMMON.H)
typedef struct
{
    int    X0;
    int    Y0;
    int    Z0;
    int    X1;
    int    Y1;
    int    Z1;
    int    Info0;
    int    Info1;
    int    Info2;
    int    Info3;
    int    Info4;	       	// Rajout de 4 champs infos le 08/12/95
    int    Info5;
    int    Info6;
    int    Info7;

    short    Type;
    short    Num;

/*#ifdef  LBA_EDITOR
    S16    Snap;         	// ce champ ne sert que
    // pour l'edition dans Visu3d !!!!
#endif*/
} T_ZONE;

// from LBA2 source code
typedef	struct
{
    unsigned char	Func;
    unsigned char	TypeAnswer;
    short	Value;
} T_EXE_SWITCH;

// from LBA2 source code
typedef	struct
{
    unsigned char	GenBody;		// 0 à 254 mais je veux -1
    unsigned char	Col;			/* brick en collision (inutile?) */
    short	SizeSHit;		// Toujours carres

    unsigned short	GenAnim;
    unsigned short	NextGenAnim;

    int	OldPosX;		/* old pos world */
    int	OldPosY;
    int	OldPosZ;

    int	Info;		/* infos pour DoDir */
    int	Info1;
    int	Info2;
    int	Info3;

    union {
        struct {
            int	SHitX;
            int	SHitY;			// Coups Super Hit
            int	SHitZ;			// Servent aussi pour les Anim3DS
        } SHit;

        // ATTENTION: un PUSHABLE ne peut pas lancer de SUPER_HIT
        struct {
            int	PushX;
            int	PushY;
            int	PushZ;
        } Push;

        struct {
            int	Num;
            int	Deb;
            int	Fin;
        } A3DS;
    } Coord;

    /* B*/	unsigned char	HitBy;			/* frappe par */
    /* B*/	unsigned char	HitForce;		/* si !=0 force de frappe anim */
    /* B*/	short	LifePoint;		/* point de vie en cours */
    short	OptionFlags;		/* flag d'init 2 */

    unsigned short* PtrAnimAction;

    short	Sprite;
    short	OffsetLabelTrack;

    // divers
    //T_OBJ_3D Obj;

    unsigned char* PtrFile3D;
    int	IndexFile3D;

    // constantes
    /* B*/	short	NbBonus;		/* nb bonus to give */
    /* B*/	unsigned char	Armure;		/* resistance */
    unsigned char	CoulObj;		/* couleur dominante de l'objet */

    /* game infos */
    short	XMin;			/* ZV */
    short	XMax;
    short	YMin;
    short	YMax;
    short	ZMin;
    short	ZMax;

    short	OldBeta;		/* angle de la boucle precedente */


    //BOUND_MOVE	BoundAngle;	/* valeur real time de rotation */

    unsigned char* PtrTrack;	/* ptr track prog */
    short	OffsetTrack;	/* offset dans la track */

    short	SRot;			/* vitesse de rotation */

    unsigned char* PtrLife;

    short	OffsetLife;	/* offset dans la vie */
    unsigned short	AnimDial;	// se trouve ici pour l'alignement

    /* B*/	short	CarryBy;
    /* B*/	unsigned char	Move;		/* type de deplacement */
    /* B*/	unsigned char	ObjCol;		/* num obj en collision */

    short	ZoneSce;		/* zone declenchement scenarique */
    /* B*/	short	LabelTrack;		/* dernier label de track */
    /* B*/	short	MemoLabelTrack;	/* memo dernier label de track */
    /* B*/  short     MemoComportement;	/* memo comportement life */

    unsigned int	Flags;			/* flags divers permanent */
    //	U16	Flags2 ;		/* flags divers permanent */
    unsigned int	WorkFlags;		/* flags de gestion */

    short	DoorWidth;		// pour les DOOR

    /* B*/	unsigned char	FlagAnim;		/* type d'anim en cours (dans flags?) */

    unsigned char	CodeJeu;		// brick spéciale sample ou action

    T_EXE_SWITCH	ExeSwitch;

    //	S16	MessageChapter[MAX_CHAPTER] ;

    T_ZONE* PtrZoneRail;

    int	SampleAlways;	// Handle du sample always en train de se jouer
    unsigned char	SampleVolume;
}	T_OBJET;

// from LBA2 source code
typedef struct
{
    short	Tx0; // 2
    short	Ty0; // 4
    short	Tx1; // 6
    short	Ty1; // 8
    short	Tx2; // 10
    short	Ty2; // 12
} T_HALF_TEX;

// from LBA2 source code
typedef	struct
{
    unsigned int	Bank : 4;	// coul bank poly
    unsigned int	TexFlag : 2;	// flag texture 00 rien 01 triste 10 flat 11 gouraud
    unsigned int	PolyFlag : 2;	// flag poly 00 rien 01 flat 10 gouraud 11 dither
    unsigned int	SampleStep : 4;	// sample pas twinsen
    unsigned int	CodeJeu : 4;	// code jeu
    unsigned int	Sens : 1;	// sens diagonale
    unsigned int	Col : 1;
    unsigned int	Dummy : 1;
    unsigned int	IndexTex : 13;	// index texture 8192
} T_HALF_POLY;

// 4 + 2 + 2 + 4 + 4 + 1 + 1 + 1 + 13 = 32

// 1 bit = 2 values
// 2 bits = 4 values
// 3 bits = 8 values
// 4 bits = 16 values
// 5 bits = 32 values
// 6 bits = 64 values
// 7 bits = 128 values
// 8 bits = 256 values
// 9 bits = 512 values
// 10 bits = 1024 values
// 11 bits = 2048 values
// 12 bits = 4096 values
// 13 bits = 8192 values

//  0123    01         01          0123          0123       0      0     0       0123456789012
//  16 vals 4 values   4 values    16 values     16 values  1 val  1 val 1 val   8192 values
// | BANK | TEX_FLAG | POLY_FLAG | SAMPLE_STEP | CODE_JEU | SENS | COL | DUMMY | INDEX_TEX