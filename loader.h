#pragma once

size_t getFileSize(const char* filename);
int calculateFileOffset(int index);

//st_lim convert_bmp_to_lim(const char* path);

void loadScene(const char* path);

void saveObjects(const char* path);
void loadObjects(const char* path);
void loadBuildings(const char* path);
void saveBuildings(const char* path);
void loadSea(const char* path);
void saveSea(const char* path);
void loadTextures(const char* path);
void saveTextures(const char* path);
void loadTerrain(const char* path);
void saveTerrain(const char* path);
void loadClouds(const char* path);
void saveClouds(const char* path);

void loadIsland(const char* startPath);

void loadPal(st_pal* thePalette, const char* path);
void loadLIM(st_lim* theLim, const char* path, int texture_index);

void loadLM2_polyGroup(FILE* file, st_model* model, int index);
void loadLM2(const char* path, st_model* model);
void loadLM2_Blender(const char* path, st_model* model);

void loadHQR(const char* path);

void write_header(FILE* ptr, int size, int* total_bytes);
long get_file_length(FILE* fp);

void loadHQR_Island(const char* path);
void saveHQR_Island(const char* path);
void loadHQR_Building(const char* path);
void loadHQR_Scene(const char* path);