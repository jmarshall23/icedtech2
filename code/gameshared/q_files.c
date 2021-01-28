// q_files.c
//

#include "q_shared.h"

#ifdef QUAKE_ENGINE
#include "../qcommon/qcommon.h"

#define trap_FS_Read FS_Read
#define trap_FS_Write FS_Write
#else
void	trap_FS_Read(void* buffer, int len, fileHandle_t f);
void	trap_FS_Write(const void* buffer, int len, fileHandle_t f);
#endif

int FS_ReadInt(fileHandle_t f) {
	int val;
	trap_FS_Read(&val, sizeof(int), f);
	return val;
}

unsigned int FS_ReadUnsignedInt(fileHandle_t f) {
	unsigned int val;
	trap_FS_Read(&val, sizeof(unsigned int), f);
	return val;
}

short FS_ReadShort(fileHandle_t f) {
	short val;
	trap_FS_Read(&val, sizeof(short), f);
	return val;
}

unsigned short FS_ReadUnsignedShort(fileHandle_t f) {
	unsigned short val;
	trap_FS_Read(&val, sizeof(unsigned short), f);
	return val;
}

char FS_ReadChar(fileHandle_t f) {
	char val;
	trap_FS_Read(&val, sizeof(char), f);
	return val;
}

unsigned char FS_ReadUnsignedChar(fileHandle_t f) {
	unsigned char val;
	trap_FS_Read(&val, sizeof(unsigned char), f);
	return val;
}

float FS_ReadFloat(fileHandle_t f) {
	float val;
	trap_FS_Read(&val, sizeof(float), f);
	return val;
}

qboolean FS_ReadBool(fileHandle_t f) {
	qboolean val;
	trap_FS_Read(&val, sizeof(qboolean), f);
	return val;
}

void FS_ReadString(fileHandle_t f, char* str) {
	int len = FS_ReadInt(f);
	str[len] = 0;
	str[len + 1] = 0;
	trap_FS_Read(str, len, f);
}

void FS_ReadVec(fileHandle_t f, float* v, int numComponents) {
	//FS_Write2(f, sizeof(float) * numComponents, v);
	for (int i = 0; i < numComponents; i++)
		v[i] = FS_ReadFloat(f);
}

void FS_Write2(fileHandle_t h, int len, const void* buffer) {
	trap_FS_Write(buffer, len, h);
}

void FS_WriteInt(fileHandle_t f, int val) {
	FS_Write2(f, sizeof(int), &val);
}

void FS_WriteUnsignedInt(fileHandle_t f, unsigned int val) {
	FS_Write2(f, sizeof(unsigned int), &val);
}

void FS_WriteShort(fileHandle_t f, short val) {
	FS_Write2(f, sizeof(short), &val);
}

void FS_WriteUnsignedShort(fileHandle_t f, unsigned short val) {
	FS_Write2(f, sizeof(unsigned short), &val);
}

void FS_WriteChar(fileHandle_t f, char val) {
	FS_Write2(f, sizeof(char), &val);
}

void FS_WriteUnsignedChar(fileHandle_t f, unsigned char val) {
	FS_Write2(f, sizeof(unsigned char), &val);
}

void FS_WriteFloat(fileHandle_t f, float val) {
	FS_Write2(f, sizeof(float), &val);
}

void FS_WriteBool(fileHandle_t f, qboolean val) {
	FS_Write2(f, sizeof(qboolean), &val);
}

void FS_WriteString(fileHandle_t f, char* str) {
	int len = strlen(str);
	FS_WriteInt(f, len);
	FS_Write2(f, len, str);
}

void FS_WriteVec2(fileHandle_t f, vec2_t v) {
	FS_Write2(f, sizeof(vec2_t), v);
}

void FS_WriteVec3(fileHandle_t f, vec3_t v) {
	FS_Write2(f, sizeof(vec3_t), v);
}

void FS_WriteVec4(fileHandle_t f, vec4_t v) {
	FS_Write2(f, sizeof(vec4_t), v);
}
