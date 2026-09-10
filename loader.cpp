#define _CRT_SECURE_NO_WARNINGS



#include <stdio.h>

#include "macros.h"
#include "structs.h"

#include <sys/stat.h>



int  headerOffset;

//char externalFileNames[MAX_BYTES][MAX_FILES];
const char* external_file_names[256];
char externalFileBytes[MAX_BYTES][MAX_FILES];

int  externalFileSizes[MAX_FILES];
int  externalFileOffsets[MAX_FILES];



extern unsigned char numBuildings; // from main.cpp
extern st_building buildings[255]; // from main.cpp

extern T_HALF_POLY terrainPolys[MAX_POLYS];
extern T_HALF_TEX  terrainTexs[MAX_POLYS];

extern short terrainHeights[MAX_VERTS];
extern unsigned char terrainClouds[MAX_VERTS];

extern int numTerrainTexs;

extern st_pal LBA2Palette;

extern unsigned char tex_ground[65536 * 4];
extern unsigned char tex_sky[65536 * 4];
extern unsigned char tex_buildings[65536 * 4];
extern unsigned char tex_objects[65536 * 4];



// https://github.com/LBALab/lba2remake/blob/master/src/resources/parsers/scene2.ts#L194
#define SCENE_OUTSIDE  0x05
#define SCENE_AMBIENCE 0x07

unsigned char text_bank_id = 0; // 0 = Citadel Island

unsigned char game_over_scene = 0;
short unknown_1 = 0;
short unknown_2 = 0;

unsigned char is_outside_scene = 0; // 0 = interior, 1 = exterior

// the hero is Twinsen
short hero_x = 0;
short hero_y = 0;
short hero_z = 0;
short hero_move_script_size = 0;
short hero_life_script_size = 0;

short num_actors = 0;

typedef struct
{
    short lighting_alpha;
    short lighting_beta;
    // samples: [],
    short sample_min_delay;
    short sample_min_delay_rand;
    // sampleElapsedTime: 0,
    unsigned char music_index;
} st_scene_ambience;

typedef struct
{
    short ambience;
    short repeat;
    short round;
    short frequency;
    short volume;
} st_sample;

typedef struct
{
    int static_flags;

    short entity_index;
    unsigned char body_index;
    short anim_index;
    short sprite_index;

    short pos_x;
    short pos_y;
    short pos_z;

    unsigned char hit_strength;

    short extra_type;
    short angle;
    short speed;

    unsigned char dir_mode;

    short info_0;
    short info_1;
    short info_2;
    short info_3;

    short extra_amount;
    unsigned char text_color;

    bool sprite_anim_3d;

} st_actor;

st_actor the_actors[255];

size_t getFileSize(const char* filename)
{
    struct stat st;

    if (stat(filename, &st) != 0)
    {
        return 0;
    }

    return st.st_size;
};

int calculateFileOffset(int index)
{
    int amount = headerOffset;

    //amount = externalFileOffsets[index];

    for (int j = 0; j < index; j++)
    {
        amount += (externalFileSizes[j] + 10);
    };

    return amount;
};

/*st_lim convert_bmp_to_lim(const char* path)
{
    st_lim result;

    result.color[0] = 0;

    return result;
};*/


void loadScene(const char* path)
{
    // file structure:
    // 0x00: text id bank (unsigned char)
    // 0x01: game over scene (unsigned char)
    // 0x07: alpha light (short)
    // 0x09: beta light (short)
    
    // 0x0B: sample 1 index (short)
    // 0x0D: sample 1 repeat count (short)
    // 0x0F: sample 1 pitch range (short)
    // 0x15: sample 2 index (short)
    // 0x17: sample 2 repeat count (short)
    // 0x19: sample 2 pitch range (short)
    // 0x1F: sample 3 index (short)

    // 0x33: minimal delay (short)
    // 0x35: minimal delay range (short)
    // 0x37: music index (unsigned char)

    // 0x38: twinsen X (short)
    // 0x3A: twinsen Y (short)
    // 0x3C: twinsen Z (short)

    // an empty track script contains 3 bytes (01, 00, 00)
    // the first value is a short
    // 0x3E: number of bytes in track script

    // an empty life script contains 3 bytes (01, 00, 00)
    // the first value is a short



    FILE* file = fopen(path, "rb");

    if (file)
    {
        printf("loading scene: %s \n", path);

        //fseek(file, 0x00, SEEK_SET);
        fread(&text_bank_id, sizeof(unsigned char), 0x1, file);
        printf("text_bank_id: %d \n", text_bank_id);

        fread(&game_over_scene, sizeof(unsigned char), 0x1, file);
        printf("game_over_scene: %d \n", game_over_scene);
        fread(&unknown_1, sizeof(short), 0x1, file);
        fread(&unknown_2, sizeof(short), 0x1, file);
        fread(&is_outside_scene, sizeof(unsigned char), 0x1, file);
        printf("is_outside_scene: %d \n", is_outside_scene);

        //fseek(file, SCENE_OUTSIDE, SEEK_SET);
        //fread(&is_outside_scene, sizeof(unsigned char), 0x1, file);
        //printf("is_outside_scene: %d \n", is_outside_scene);

        // skip ambience for now
        //fseek(file, SCENE_AMBIENCE, SEEK_SET);
        fseek(file, SCENE_AMBIENCE + 0x49, SEEK_SET);
        //fseek(file, SCENE_AMBIENCE + 19, SEEK_SET);

        printf("pos in file: 0x%04x \n", ftell(file));

        // load hero data
        fread(&hero_x, sizeof(short), 0x1, file);
        fread(&hero_y, sizeof(short), 0x1, file);
        fread(&hero_z, sizeof(short), 0x1, file);

        printf("hero_x: 0x%04x \n", hero_x);
        printf("hero_y: 0x%04x \n", hero_y);
        printf("hero_z: 0x%04x \n", hero_z);

        printf("pos in file: 0x%04x \n", ftell(file));

        fread(&hero_move_script_size, sizeof(short), 0x1, file);
        printf("hero_move_script_size: 0x%04x, %d \n", hero_move_script_size, hero_move_script_size);
        fseek(file, hero_move_script_size, SEEK_CUR);

        fread(&hero_life_script_size, sizeof(short), 0x1, file);
        printf("hero_life_script_size: 0x%04x, %d \n", hero_life_script_size, hero_life_script_size);
        fseek(file, hero_life_script_size, SEEK_CUR);

        printf("pos in file: 0x%04x \n", ftell(file));

        fread(&num_actors, sizeof(short), 0x1, file);
        printf("num_actors: %d \n", num_actors);

        /*
        for (int i = 0; i < num_actors; i++)
        {
            fread(&the_actors[i].static_flags, sizeof(int), 0x1, file);
            printf("static_flags: %d \n", the_actors[i].static_flags);

            fread(&the_actors[i].entity_index, sizeof(short), 0x1, file);
            printf("entity_index: %d \n", the_actors[i].entity_index);

            fread(&the_actors[i].body_index, sizeof(unsigned char), 0x1, file);
            fread(&the_actors[i].anim_index, sizeof(short), 0x1, file);
            fread(&the_actors[i].sprite_index, sizeof(short), 0x1, file);

            fread(&the_actors[i].pos_x, sizeof(short), 0x1, file);
            fread(&the_actors[i].pos_y, sizeof(short), 0x1, file);
            fread(&the_actors[i].pos_z, sizeof(short), 0x1, file);

            fread(&the_actors[i].hit_strength, sizeof(unsigned char), 0x1, file);

            fread(&the_actors[i].extra_type, sizeof(short), 0x1, file);
            fread(&the_actors[i].angle, sizeof(short), 0x1, file);
            fread(&the_actors[i].speed, sizeof(short), 0x1, file);

            fread(&the_actors[i].dir_mode, sizeof(unsigned char), 0x1, file);

            fread(&the_actors[i].info_0, sizeof(short), 0x1, file);
            fread(&the_actors[i].info_1, sizeof(short), 0x1, file);
            fread(&the_actors[i].info_2, sizeof(short), 0x1, file);
            fread(&the_actors[i].info_3, sizeof(short), 0x1, file);

            fread(&the_actors[i].extra_amount, sizeof(short), 0x1, file);
            fread(&the_actors[i].text_color, sizeof(short), 0x1, file);

            if (the_actors[i].sprite_anim_3d)
            {
                //fread(&the_actors[i].index, sizeof(int), 0x1, file);
                //fread(&the_actors[i].fps, sizeof(short), 0x1, file);
            };
            
        };*/

        fclose(file);
    };
};

void saveObjects(const char* path)
{
    FILE* file = fopen(path, "wb");

    unsigned char zero = 0x00;

    // Get the number of buildings.
    if (file)
    {
        // stub file
        for (int i = 0; i < 0x28; i++)
            fwrite(&zero, sizeof(unsigned char), 0x1, file);

        fseek(file, 0x08, SEEK_SET);
        fread(&numBuildings, sizeof(short), 0x1, file);

        printf("Buildings: %d \n", numBuildings);

        fclose(file);
    };
};

void loadObjects(const char* path)
{
    FILE* file = fopen(path, "rb");

    //char _header = 0x60;

    // Get the number of buildings.
    if (file)
    {
        printf("reading %s \n", path);

        fseek(file, 0x08, SEEK_SET);
        fread(&numBuildings, sizeof(short), 0x1, file);

        printf("Buildings: %d \n", numBuildings);

        // bytes 0x0A through 0x20 appear to use info for animated surfaces: water, gas, lava, etc
        // since I cheat though and don't want to use animated surfaces, I will zero these out when saving to the final file

        fclose(file);
    };
};

void loadBuildings(const char* path)
{
    FILE* file = fopen(path, "rb");

    if (file)
    {
        printf("reading %s \n", path);

        for (int i = 0; i < numBuildings; i++)
        {
            fread(&buildings[i].modelId, sizeof(int), 0x1, file);
            fread(&buildings[i].x, sizeof(int), 0x1, file);
            fread(&buildings[i].y, sizeof(int), 0x1, file);
            fread(&buildings[i].z, sizeof(int), 0x1, file);

            fread(&buildings[i].floorId, sizeof(int), 0x1, file);
            fread(&buildings[i].rotation, sizeof(int), 0x1, file);
            fread(&buildings[i].colLeft, sizeof(int), 0x1, file);
            fread(&buildings[i].colBottom, sizeof(int), 0x1, file);

            fread(&buildings[i].colUp, sizeof(int), 0x1, file);
            fread(&buildings[i].colRight, sizeof(int), 0x1, file);
            fread(&buildings[i].colTop, sizeof(int), 0x1, file);
            fread(&buildings[i].colDown, sizeof(int), 0x1, file);

            //printf("modelId: %.2x \n", _theBuildings[i].modelId);

            /*for (int j = 0; j < 100; j++)
            {
                if (_theBuildings[i].modelId == j)
                    loadModel(i, lba2Buildings[j]);
            };*/
        };

        fclose(file);
    };
};

void saveBuildings(const char* path)
{
    FILE* file = fopen(path, "wb");

    if (file)
    {
        for (int i = 0; i < numBuildings; i++)
        {
            fwrite(&buildings[i].modelId, sizeof(int), 0x1, file);
            fwrite(&buildings[i].x, sizeof(int), 0x1, file);
            fwrite(&buildings[i].y, sizeof(int), 0x1, file);
            fwrite(&buildings[i].z, sizeof(int), 0x1, file);

            fwrite(&buildings[i].floorId, sizeof(int), 0x1, file);
            fwrite(&buildings[i].rotation, sizeof(int), 0x1, file);
            fwrite(&buildings[i].colLeft, sizeof(int), 0x1, file);
            fwrite(&buildings[i].colBottom, sizeof(int), 0x1, file);

            fwrite(&buildings[i].colUp, sizeof(int), 0x1, file);
            fwrite(&buildings[i].colRight, sizeof(int), 0x1, file);
            fwrite(&buildings[i].colTop, sizeof(int), 0x1, file);
            fwrite(&buildings[i].colDown, sizeof(int), 0x1, file);
        };

        printf("Building information saved! \n");

        fclose(file);
    };
};

void loadSea(const char* path)
{
    FILE* file = fopen(path, "rb");

    if (file)
    {
        printf("reading %s \n", path);

        for (int i = 0; i < MAX_POLYS; i++)
            fread(&terrainPolys[i], sizeof(int), 0x1, file);

        fclose(file);
    };
};

void saveSea(const char* path)
{
    FILE* file = fopen(path, "wb");

    if (file)
    {
        /*for (int i = 0; i < 48; i++)
        {
            terrainPolys[i].Bank = i;
            //terrainPolys[i].Bank = POLY_COLOR_RED; // color
            terrainPolys[i].TexFlag = 0;
            terrainPolys[i].PolyFlag = 3; // dithered
            terrainPolys[i].SampleStep = SAMPLE_STEP_DIRT;
            terrainPolys[i].CodeJeu = 0;
            if (i < 2)
                terrainPolys[i].Sens = 1;
            else
                terrainPolys[i].Sens = 0;
            terrainPolys[i].Col = 0;
            terrainPolys[i].Dummy = 0;
            terrainPolys[i].IndexTex = 0;
        };*/

        // Gets rid of all textures on terrain
        /*for (int i = 0; i < MAX_POLYS; i++)
        {
            terrainPolys[i].TexFlag = 0;
            terrainPolys[i].PolyFlag = 3;
        };*/

        // loop through terrain polys
        /*for (int i = 0; i < MAX_POLYS; i++)
        {
            // Gets rid of all textures on terrain
            terrainPolys[i].TexFlag = 0;
            terrainPolys[i].PolyFlag = 3;

            // Colors polygons by diagonals
            if (terrainPolys[i].Sens == 0)
            {
				if (i == 0)
					terrainPolys[i].Bank = POLY_COLOR_YELLOW;
                else
                    terrainPolys[i].Bank = POLY_COLOR_BLUE;
            }
            else
            {
                terrainPolys[i].Bank = POLY_COLOR_RED;
            };

			if (i == 1)
				terrainPolys[i].Bank = POLY_COLOR_GREEN;

            if (i == 0)
                terrainPolys[i].Sens = 1;
            if (i == 1)
                terrainPolys[i].Sens = 1;

            // textures
            if (i == 0)
            {
                terrainPolys[i].TexFlag = 3;
                terrainPolys[i].IndexTex = 8;
            };
            if (i == 1)
            {
                terrainPolys[i].TexFlag = 3;
                terrainPolys[i].IndexTex = 23;
            };
        };*/

        /*for (int i = 0; i < MAX_POLYS; i++)
        {
            terrainPolys[i].TexFlag = 0;
            terrainPolys[i].TexFlag = 3;
        };*/

        for (int i = 0; i < MAX_POLYS; i++)
            fwrite(&terrainPolys[i], sizeof(int), 0x1, file);

        printf("Sea information saved! \n");

        fclose(file);
    };
};

void loadTextures(const char* path)
{
    FILE* file = fopen(path, "rb");

    if (file)
    {
        printf("reading %s \n", path);

        // seek to end of file and get the size
        fseek(file, 0L, SEEK_END);
        int size = ftell(file);

        // seek back to the beginning of the file
        fseek(file, 0, SEEK_SET);

        numTerrainTexs = (size / 12);
        printf("numTerrainTexs: %d, %x \n", numTerrainTexs, numTerrainTexs);

        // feed the texture array
        for (int i = 0; i < numTerrainTexs; i++)
        {
            fread(&terrainTexs[i].Tx0, sizeof(short), 0x1, file);
            fread(&terrainTexs[i].Ty0, sizeof(short), 0x1, file);
            fread(&terrainTexs[i].Tx1, sizeof(short), 0x1, file);
            fread(&terrainTexs[i].Ty1, sizeof(short), 0x1, file);
            fread(&terrainTexs[i].Tx2, sizeof(short), 0x1, file);
            fread(&terrainTexs[i].Ty2, sizeof(short), 0x1, file);

            //if (i == 0)
            //{
                //printf("%d, %x \n", terrainTexs[i].Tx0, terrainTexs[i].Tx0);
                //printf("%d, %x \n", terrainTexs[i].Ty0, terrainTexs[i].Ty0);
                //printf("%d, %x \n", terrainTexs[i].Tx1, terrainTexs[i].Tx1);
                //printf("%d, %x \n", terrainTexs[i].Ty1, terrainTexs[i].Ty1);
                //printf("%d, %x \n", terrainTexs[i].Tx2, terrainTexs[i].Tx2);
                //printf("%d, %x \n", terrainTexs[i].Ty2, terrainTexs[i].Ty2);
            //};
        };
        //fread(&terrainTexs[i], sizeof(T_HALF_TEX), 0x1, file);

        fclose(file);
    };
};

short toBigEndian(short v)
{
    return (v >> 8) | (v << 8);
};

void saveTextures(const char* path)
{
    FILE* file = fopen(path, "wb");

    if (file)
    {
        for (int i = 0; i < numTerrainTexs; i++)
        {
            fwrite(&terrainTexs[i].Tx0, sizeof(short), 0x1, file);
            fwrite(&terrainTexs[i].Ty0, sizeof(short), 0x1, file);
            fwrite(&terrainTexs[i].Tx1, sizeof(short), 0x1, file);
            fwrite(&terrainTexs[i].Ty1, sizeof(short), 0x1, file);
            fwrite(&terrainTexs[i].Tx2, sizeof(short), 0x1, file);
            fwrite(&terrainTexs[i].Ty2, sizeof(short), 0x1, file);

            /*short value_Tx0 = toBigEndian(terrainTexs[i].Tx0);
            short value_Ty0 = toBigEndian(terrainTexs[i].Ty0);

            short value_Tx1 = toBigEndian(terrainTexs[i].Tx1);
            short value_Ty1 = toBigEndian(terrainTexs[i].Ty1);

            short value_Tx2 = toBigEndian(terrainTexs[i].Tx2);
            short value_Ty2 = toBigEndian(terrainTexs[i].Ty2);*/

            /*fwrite(&value_Tx0, sizeof(short), 0x1, file);
            fwrite(&value_Ty0, sizeof(short), 0x1, file);

            fwrite(&value_Tx1, sizeof(short), 0x1, file);
            fwrite(&value_Ty1, sizeof(short), 0x1, file);

            fwrite(&value_Tx2, sizeof(short), 0x1, file);
            fwrite(&value_Ty2, sizeof(short), 0x1, file);*/
        };

        printf("Textures information saved! \n");

        fclose(file);
    };
};

void loadTerrain(const char* path)
{
    FILE* file = fopen(path, "rb");

    if (file)
    {
        printf("reading %s \n", path);

        for (int i = 0; i < MAX_VERTS; i++)
            fread(&terrainHeights[i], sizeof(short), 0x1, file);

        fclose(file);
    };
};

void saveTerrain(const char* path)
{
    FILE* file = fopen(path, "wb");

    if (file)
    {
        for (int i = 0; i < MAX_VERTS; i++)
            fwrite(&terrainHeights[i], sizeof(short), 0x1, file);

        printf("Terrain information saved! \n");

        fclose(file);
    };
};

void loadClouds(const char* path)
{
    FILE* file = fopen(path, "rb");

    if (file)
    {
        printf("reading %s \n", path);

        for (int i = 0; i < MAX_VERTS; i++)
            fread(&terrainClouds[i], sizeof(unsigned char), 0x1, file);

        fclose(file);
    };
};

void saveClouds(const char* path)
{
    FILE* file = fopen(path, "wb");

    if (file)
    {
        for (int i = 0; i < MAX_VERTS; i++)
            fwrite(&terrainClouds[i], sizeof(unsigned char), 0x1, file);

        printf("Cloud information saved! \n");

        fclose(file);
    };
};

void loadIsland(const char* startPath)
{

};

void loadPal(st_pal* thePalette, const char* path)
{
    FILE* file = fopen(path, "rb");

    if (file)
    {
        printf("reading %s \n", path);

        for (int i = 0; i < MAX_COLORS; i++)
        {
            fread(&thePalette->theColors[i].r, sizeof(unsigned char), 0x1, file);
            fread(&thePalette->theColors[i].g, sizeof(unsigned char), 0x1, file);
            fread(&thePalette->theColors[i].b, sizeof(unsigned char), 0x1, file);
            //printf("[0x%02X, 0x%02X, 0x%02X], # %d \n", thePalette->theColors[i].r, thePalette->theColors[i].g, thePalette->theColors[i].b, i);

            // there is actually not any transparency in the palette files
            // black can be used as transparency so we have appended our own alpha value to the structure

            // if the pixel is black, mark it as transparent
            if (thePalette->theColors[i].r == 0 &&
                thePalette->theColors[i].g == 0 &&
                thePalette->theColors[i].b == 0)
            {
                thePalette->theColors[i].a = 0; // transparent
            }
            else
            {
                thePalette->theColors[i].a = 255; // opaque
            };
        };

        fclose(file);
    };
};

void loadLIM(st_lim* theLim, const char* path, int texture_index)
{
    int index = 0;
    FILE* file = fopen(path, "rb");

    if (file)
    {
        printf("reading %s \n", path);

        for (int i = 0; i < (TEXTURE_X * TEXTURE_Y); i++)
        {
            fread(&theLim->color[i], sizeof(unsigned char), 0x1, file);

            // assign correct colors from the palette
            switch (texture_index)
            {
            case GROUND_TEXTURES:
                tex_ground[index + 0] = LBA2Palette.theColors[theLim->color[i]].r;
                tex_ground[index + 1] = LBA2Palette.theColors[theLim->color[i]].g;
                tex_ground[index + 2] = LBA2Palette.theColors[theLim->color[i]].b;
                tex_ground[index + 3] = LBA2Palette.theColors[theLim->color[i]].a;
                index += 4;
                break;
            case BUILDING_TEXTURES:
                tex_buildings[index + 0] = LBA2Palette.theColors[theLim->color[i]].r;
                tex_buildings[index + 1] = LBA2Palette.theColors[theLim->color[i]].g;
                tex_buildings[index + 2] = LBA2Palette.theColors[theLim->color[i]].b;
                tex_buildings[index + 3] = LBA2Palette.theColors[theLim->color[i]].a;
                index += 4;
                break;
            case SKY_TEXTURES:
                tex_sky[index + 0] = LBA2Palette.theColors[theLim->color[i]].r;
                tex_sky[index + 1] = LBA2Palette.theColors[theLim->color[i]].g;
                tex_sky[index + 2] = LBA2Palette.theColors[theLim->color[i]].b;
                tex_sky[index + 3] = LBA2Palette.theColors[theLim->color[i]].a;
                index += 4;
                break;
            case OBJECT_TEXTURES:
                tex_objects[index + 0] = LBA2Palette.theColors[theLim->color[i]].r;
                tex_objects[index + 1] = LBA2Palette.theColors[theLim->color[i]].g;
                tex_objects[index + 2] = LBA2Palette.theColors[theLim->color[i]].b;
                tex_objects[index + 3] = LBA2Palette.theColors[theLim->color[i]].a;
                index += 4;
                break;
            };
        };

        // debug
        /*index = 0;
        for (int i = (256 * 64); i < ((256 * 64) + (256 * 112)); i++)
        {
            tex_ground[index + 2] = 255;
            index += 4;
        };

        index = 0;
        for (int i = (256 * 96); i < ((256 * 96) + (256 * 112)); i++)
        {
            tex_ground[index + 2] = 0;
            index += 2;
        };*/

        fclose(file);
    };
};

void loadLM2_polyGroup(FILE* file, st_model* model, int index)
{
    fread(&model->polyMat[index], sizeof(char), 0x1, file);
    fread(&model->polyType[index], sizeof(char), 0x1, file);
    fread(&model->polyAmount[index], sizeof(short), 0x1, file);
    fread(&model->polyBytes[index], sizeof(short), 0x1, file);
    fread(&model->polyPadding1[index], sizeof(short), 0x1, file);

    printf("mat: %d \n", model->polyMat[index]);

    //printf("model->polyMat[%d] = %d \n", model->polyMat[index], index);

    if (model->polyType[index] == 0x00) // tri
    {
        printf("rading tri \n");

        if (model->polyMat[index] < 0x06) // colored
        {
            for (int i = 0; i < model->polyAmount[index]; i++)
            {
                // 12 bytes
                fread(&model->polyV1[index][i], sizeof(short), 0x1, file);
                fread(&model->polyV2[index][i], sizeof(short), 0x1, file);
                fread(&model->polyV3[index][i], sizeof(short), 0x1, file);
                fread(&model->polyV4[index][i], sizeof(short), 0x1, file);
                fread(&model->polyColor[index][i], sizeof(char), 0x1, file);
                fread(&model->polyPadding2[i], sizeof(char), 0x1, file);
                fread(&model->polyPadding3[i], sizeof(short), 0x1, file);
            };
        }
        else // textured
        {
            for (int i = 0; i < model->polyAmount[index]; i++)
            {
                // 24 bytes
                fread(&model->polyV1[index][i], sizeof(short), 0x1, file);
                fread(&model->polyV2[index][i], sizeof(short), 0x1, file);
                fread(&model->polyV3[index][i], sizeof(short), 0x1, file);
                fread(&model->polyV4[index][i], sizeof(short), 0x1, file);
                fread(&model->polyColor[index][i], sizeof(char), 0x1, file);
                fread(&model->polyPadding2[i], sizeof(char), 0x1, file);
                fread(&model->polyPadding3[i], sizeof(short), 0x1, file);

                // texture coordinates
                fread(&model->UV1_x[index][i], sizeof(short), 0x1, file);
                fread(&model->UV2_x[index][i], sizeof(short), 0x1, file);
                fread(&model->UV3_x[index][i], sizeof(short), 0x1, file);

                fread(&model->UV1_y[index][i], sizeof(short), 0x1, file);
                fread(&model->UV2_y[index][i], sizeof(short), 0x1, file);
                fread(&model->UV3_y[index][i], sizeof(short), 0x1, file);
            };
        };
    }
    //if (model->polyType[index] == 0x80) // quad
    if (model->polyType[index] != 0x00) // quad
    {
        printf("reading quad \n");

        if (model->polyMat[index] < 0x06) // colored
        {
            for (int i = 0; i < model->polyAmount[index]; i++)
            {
                // 12 bytes
                fread(&model->polyV1[index][i], sizeof(short), 0x1, file);
                fread(&model->polyV2[index][i], sizeof(short), 0x1, file);
                fread(&model->polyV3[index][i], sizeof(short), 0x1, file);
                fread(&model->polyV4[index][i], sizeof(short), 0x1, file);
                fread(&model->polyColor[index][i], sizeof(char), 0x1, file);
                fread(&model->polyPadding2[i], sizeof(char), 0x1, file);
                fread(&model->polyPadding3[i], sizeof(short), 0x1, file);
            };
        }
        //else // textured
        if (model->polyMat[index] > 0x06) // textured
        {
            for (int i = 0; i < model->polyAmount[index]; i++)
            {
                // 32 bytes
                fread(&model->polyV1[index][i], sizeof(short), 0x1, file);
                fread(&model->polyV2[index][i], sizeof(short), 0x1, file);
                fread(&model->polyV3[index][i], sizeof(short), 0x1, file);
                fread(&model->polyV4[index][i], sizeof(short), 0x1, file);
                fread(&model->polyColor[index][i], sizeof(char), 0x1, file);
                fread(&model->polyPadding2[i], sizeof(char), 0x1, file);
                fread(&model->polyPadding3[i], sizeof(short), 0x1, file);

                // texture coordinates
                fread(&model->UV1_x[index][i], sizeof(short), 0x1, file);
                fread(&model->UV2_x[index][i], sizeof(short), 0x1, file);
                fread(&model->UV3_x[index][i], sizeof(short), 0x1, file);
                fread(&model->UV4_x[index][i], sizeof(short), 0x1, file);
                fread(&model->UV1_y[index][i], sizeof(short), 0x1, file);

                fread(&model->UV2_y[index][i], sizeof(short), 0x1, file);
                fread(&model->UV3_y[index][i], sizeof(short), 0x1, file);
                fread(&model->UV4_y[index][i], sizeof(short), 0x1, file);
                fread(&model->UV4_y[index][i], sizeof(short), 0x1, file);
                fread(&model->UV4_y[index][i], sizeof(short), 0x1, file);
            };
        };
    };
};

void loadLM2(const char* path, st_model* model)
{
    int current_poly_group = 0;
    int total_poly_group = 0;

    int polygon_current_byte = 0;
    int polygon_total_bytes = 0;

    FILE* file = fopen(path, "rb");

    if (file)
    {
        //printf("begin reading lm2 \n");

        fseek(file, VERTEX_NUMBER, SEEK_SET);
        fread(&model->vertex, sizeof(short), 0x1, file);

        fseek(file, VERTEX_OFFSET, SEEK_SET);
        fread(&model->vertexOffset, sizeof(short), 0x1, file);

        fseek(file, POLYGON_NUMBER, SEEK_SET);
        fread(&model->polygon, sizeof(short), 0x1, file);

        fseek(file, POLYGON_OFFSET, SEEK_SET);
        fread(&model->polygonOffset, sizeof(short), 0x1, file);

        fseek(file, LINE_NUMBER, SEEK_SET);
        fread(&model->line, sizeof(short), 0x1, file);

        fseek(file, LINE_OFFSET, SEEK_SET);
        fread(&model->lineOffset, sizeof(short), 0x1, file);

        //printf("got header data \n");

        // Go to vertex data.
        fseek(file, model->vertexOffset, SEEK_SET);

        //printf("reading vertices \n");
        for (int i = 0; i < model->vertex; i++)
        {
            fread(&model->x[i], sizeof(short), 0x1, file);
            fread(&model->y[i], sizeof(short), 0x1, file);
            fread(&model->z[i], sizeof(short), 0x1, file);
            fread(&model->bone[i], sizeof(short), 0x1, file);
        };
        //printf("got vertex data \n");



        polygon_total_bytes = ((model->lineOffset - model->polygonOffset));
        printf("polygon_total_bytes: %d \n", polygon_total_bytes);

        // Seek to polygon data.
        fseek(file, model->polygonOffset, SEEK_SET);



        long polygon_block_start = ftell(file);
        long polygon_block_end = polygon_block_start + polygon_total_bytes;

        while (ftell(file) < polygon_block_end)
        {
            st_poly_block header;

            if (fread(&header, sizeof(st_poly_block), 1, file) != 1)
                break;

            current_poly_group++;

            /* Skip polygon payload */
            fseek(file, header.polyBytes, SEEK_CUR);
        };

        // add one for now
        current_poly_group += 1;

        total_poly_group = current_poly_group;
        printf("there are %d poly groups in model \n\n", total_poly_group);

        model->poly_group_length = total_poly_group;

        fseek(file, model->polygonOffset, SEEK_SET);

        for (int i = 0; i < model->poly_group_length; i++)
            loadLM2_polyGroup(file, model, i);

        fclose(file);
    };
};

// similar to the above but this skips the inner polygon groups entirely and only determines if the currently read triangle uses a color or texture
void loadLM2_Blender(const char* path, st_model* model)
{

};

void loadHQR(const char* path)
{

};

void loadHQR_Island(const char* path)
{
    // entries:
    // 1 - island info
    // 2 - ground texture
    // 3 - building texture

    // 4 - object info (part 1)
    // 5 - building info (part 1)
    // 6 - sea info (part 1)
    // 7 - texture info (part 1)
    // 8 - surface info (part 1)
    // 9 - cloud info (part 1)
    
    // repeat for all other island parts
};

typedef struct
{
    int   original_size;
    int   compressed_size;
    short compression_type;
} st_address_header;

// helper for writing header data
// we really only care about the size as compressed size is also the same and compression type is always zero
void write_header(FILE* ptr, int size, int* total_bytes)
{
    // Setup file header.
    st_address_header the_header;

    the_header.original_size = size;
    the_header.compressed_size = size;
    the_header.compression_type = 0x00;

    // Fill file header data.
    fwrite(&the_header.original_size, sizeof(int), 0x1, ptr);
    fwrite(&the_header.compressed_size, sizeof(int), 0x1, ptr);
    fwrite(&the_header.compression_type, sizeof(short), 0x1, ptr);

    *total_bytes += sizeof(st_address_header);
};

long get_file_length(FILE* fp)
{
    long size = -1; // Initialize size to -1 for error indication

    if (fp == NULL) {
        return size; // Handle NULL file pointer error
    }

    // 1. Seek to the end of the file
    // fseek(file_pointer, offset, origin)
    if (fseek(fp, 0, SEEK_END) == 0) {
        // 2. Get the current position (which is the length of the file)
        size = ftell(fp);

        // 3. Reset the file pointer to the beginning for future operations
        rewind(fp); // or fseek(fp, 0, SEEK_SET);
    }

    return size;
};


void saveHQR_Island(const char* path)
{
    // entries:
    // 1 - island info
    // 2 - ground texture
    // 3 - building texture

    // 4 - object info (part 1)
    // 5 - building info (part 1)
    // 6 - sea info (part 1)
    // 7 - texture info (part 1)
    // 8 - surface info (part 1)
    // 9 - cloud info (part 1)

    // repeat for all other island parts



    // save island data
    /*saveObjects("medieval_objects.lun");
    saveBuildings("medieval_buildings.lun");
    saveSea("medieval_sea.lun");
    saveTextures("medieval_textures.lun");
    saveTerrain("medieval_terrain.lun");
    saveClouds("medieval_clouds.lun");*/



    // get island data ready for HQR package
    int entries = 3;

    int island_parts = 1;
    int island_index = 1;

    int total_bytes = 0;
    int file_length = 0;

    int file_offsets[10];

    for (int i = 0; i < 3; i++)
        file_offsets[i] = 0x0000;

    for (int i = 0; i < island_parts; i++)
        entries += 6;

    FILE* file = fopen(path, "wb");

    int zero_int = 0x02;
    unsigned char zero_char = 0x00;

    unsigned char island_part_bytes[MAX_ISLAND_PARTS];
    for (int i = 0; i < MAX_ISLAND_PARTS; i++)
        island_part_bytes[i] = 0xFF;

    extern st_lim groundTextures; // from main.cpp
    extern st_lim buildingTextures; // from main.cpp

    if (file)
    {
        // stub offsets
        for (int i = 0; i < entries; i++)
        {
            fwrite(&zero_int, sizeof(int), 0x1, file);
            total_bytes += sizeof(int);
        };
        // stub file size
        fwrite(&total_bytes, sizeof(int), 0x1, file);

        // island parts
        file_offsets[0] = ftell(file);
        write_header(file, MAX_ISLAND_PARTS, &total_bytes);
        FILE* island_parts_file = fopen("medieval_island_parts.lun", "rb");

        for (int i = 0; i < MAX_ISLAND_PARTS; i++)
        {
            fread(&island_part_bytes[i], sizeof(unsigned char), 0x1, island_parts_file);
            fwrite(&island_part_bytes[i], sizeof(unsigned char), 0x1, file);
            total_bytes += sizeof(unsigned char);
        };

        fclose(island_parts_file);

        // ground texture
        file_offsets[1] = ftell(file);
        write_header(file, MAX_TEXTURE, &total_bytes);
        for (int i = 0; i < MAX_TEXTURE; i++)
        {
            fwrite(&groundTextures.color[i], sizeof(unsigned char), 0x1, file);
            total_bytes += sizeof(unsigned char);
        };

        // building texture
        file_offsets[2] = ftell(file);
        write_header(file, MAX_TEXTURE, &total_bytes);
        for (int i = 0; i < MAX_TEXTURE; i++)
        {
            fwrite(&buildingTextures.color[i], sizeof(unsigned char), 0x1, file);
            total_bytes += sizeof(unsigned char);
        };



		for (int i = 0; i < island_parts; i++)
		{
            // the current island part that is loaded in memory gets written to new memory for the packed file
            // before writing the contents of the next island part, we need to load the next island part first
            /*loadObjects("medieval_9_objects.lun");
            loadBuildings("medieval_9_buildings.lun");
            loadSea("medieval_sea.lun");
            loadTextures("medieval_textures.lun");
            loadTerrain("medieval_terrain.lun");
            loadClouds("medieval_clouds.lun");*/



            file_offsets[2 + (island_index + 0)] = ftell(file);
            write_header(file, MAX_ISLAND_OBJECTS, &total_bytes);

            // object info
			for (int j = 0; j < MAX_ISLAND_OBJECTS; j++)
			{
                fwrite(&zero_char, sizeof(unsigned char), 0x1, file);

				// stub file
                //if (j != 0x08)
		            //fwrite(&zero_char, sizeof(unsigned char), 0x1, file);
                //else
		            //fwrite(&numBuildings, sizeof(unsigned char), 0x1, file);

				//fseek(file, 0x08, SEEK_SET);
				//fread(&numBuildings, sizeof(short), 0x1, file);

				//printf("Buildings: %d \n", numBuildings);

                total_bytes += sizeof(unsigned char);
			};

            file_offsets[2 + (island_index + 1)] = ftell(file);
            write_header(file, (numBuildings * sizeof(st_building)), &total_bytes);

            // building info
            for (int j = 0; j < numBuildings; j++)
            {
                fwrite(&buildings[j].modelId, sizeof(int), 0x1, file);
                fwrite(&buildings[j].x, sizeof(int), 0x1, file);
                fwrite(&buildings[j].y, sizeof(int), 0x1, file);
                fwrite(&buildings[j].z, sizeof(int), 0x1, file);

                fwrite(&buildings[j].floorId, sizeof(int), 0x1, file);
                fwrite(&buildings[j].rotation, sizeof(int), 0x1, file);
                fwrite(&buildings[j].colLeft, sizeof(int), 0x1, file);
                fwrite(&buildings[j].colBottom, sizeof(int), 0x1, file);

                fwrite(&buildings[j].colUp, sizeof(int), 0x1, file);
                fwrite(&buildings[j].colRight, sizeof(int), 0x1, file);
                fwrite(&buildings[j].colTop, sizeof(int), 0x1, file);
                fwrite(&buildings[j].colDown, sizeof(int), 0x1, file);

                total_bytes += sizeof(st_building);
            };

            file_offsets[2 + (island_index + 2)] = ftell(file);
            write_header(file, (MAX_POLYS * sizeof(int)), &total_bytes);

            // sea info
            for (int j = 0; j < MAX_POLYS; j++)
            {
                fwrite(&terrainPolys[j], sizeof(int), 0x1, file);

                total_bytes += sizeof(int);
            };

            file_offsets[2 + (island_index + 3)] = ftell(file);
            write_header(file, (numTerrainTexs * (sizeof(T_HALF_TEX))), &total_bytes);

			// texture info
			for (int j = 0; j < numTerrainTexs; j++)
			{
				fwrite(&terrainTexs[j].Tx0, sizeof(short), 0x1, file);
				fwrite(&terrainTexs[j].Ty0, sizeof(short), 0x1, file);
				fwrite(&terrainTexs[j].Tx1, sizeof(short), 0x1, file);
				fwrite(&terrainTexs[j].Ty1, sizeof(short), 0x1, file);
				fwrite(&terrainTexs[j].Tx2, sizeof(short), 0x1, file);
				fwrite(&terrainTexs[j].Ty2, sizeof(short), 0x1, file);

                total_bytes += (sizeof(T_HALF_TEX));
			};

            file_offsets[2 + (island_index + 4)] = ftell(file);
            write_header(file, (MAX_VERTS * sizeof(short)), &total_bytes);

            // surface info
            for (int j = 0; j < MAX_VERTS; j++)
            {
                fwrite(&terrainHeights[j], sizeof(short), 0x1, file);

                total_bytes += sizeof(short);
            };

            file_offsets[2 + (island_index + 5)] = ftell(file);
            write_header(file, MAX_VERTS, &total_bytes);

            // cloud info
            for (int j = 0; j < MAX_VERTS; j++)
            {
                fwrite(&terrainClouds[j], sizeof(unsigned char), 0x1, file);

                total_bytes += sizeof(unsigned char);
            };

            island_index += 6;
		};



        file_length = get_file_length(file);
        //printf("file_length: %d \n", file_length);

        // go back to the beinning of the file
        fseek(file, 0x00, SEEK_SET);

        // write offsets
        for (int i = 0; i < entries; i++)
        {
            fwrite(&file_offsets[i], sizeof(int), 0x1, file);
        };
        // write file size
        fwrite(&file_length, sizeof(int), 0x1, file);



        printf("HQR file saved.");

        fclose(file);
    };
};

void loadHQR_Building(const char* path)
{

};

void loadHQR_Scene(const char* path)
{

};