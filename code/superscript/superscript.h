// g_quakec.h
//

#pragma once

#define ATTN_IDLE 0
#define ATTN_NORM 1
#define ATTN_STATIC 2

#define null nullptr
#define CenterPrint(y, x) G_CenterPrint(y - g_entities, x)

#define FreeEntity G_FreeEntity
#define Print G_Printf

#define FindEntity(x) G_Find(NULL, FOFS(targetname), x)

static idVec3_t	VEC_HULL2_MIN = idVec3_t(-32, -32, -24);
static idVec3_t	VEC_HULL2_MAX = idVec3_t(32, 32, 64);

__forceinline qboolean walkmove(gentity_t *self, int heading, int dist) {
	return SV_StepDirection(self, heading, dist);
}

void sound(gentity_t* self, int channel, char* filename, int notused, int attenuation);

__forceinline vec3_t NormalizeQuakeC(vec3_t r)
{
	r.Normalize();
	return r;
}

__forceinline void FireBullets(gentity_t *self, int count, vec3_t dir, vec3_t spread) {
	vec3_t forward, right, up;
	//AngleVectors(dir, forward, right, up);
	SetupMuzzlePoint(self, dir, vec3_origin, vec3_origin);
	//for (int i = 0; i < count; i++)
	//{
	//	//vec3_t direction = dir + crandom() * spread[0] * right + crandom() * spread[1] * up;
	//	
	//}
	Bullet_Fire(self, spread[0], 4 * count);
}

__forceinline void sound(gentity_t* self, int channel, char* filename, int notused, int attenuation) {
	int s = G_SoundIndex(va("sound/%s", filename));
	
	engine->SV_GameSendServerCommand(-1, va("sound %d %f %f %f", s, self->r.currentOrigin[0], self->r.currentOrigin[1], self->r.currentOrigin[2]));
}

__forceinline void setsize(gentity_t* self, vec3_t mins, vec3_t maxs) {
	self->r.mins = mins;
	self->r.maxs = maxs;
}

__forceinline void precache_sound(char* filename) {
	G_SoundIndex(va("sound/%s", filename));
}

__forceinline void precache_model(char* filename) {
	G_ModelIndex(filename);
}

__forceinline void setmodel(gentity_t* ent, char* filename) {
	ent->s.modelindex = G_ModelIndex(filename);
	VectorSet(ent->r.mins, -16, -16, -16);
	VectorSet(ent->r.maxs, 16, 16, 16);
	engine->SV_LinkEntity(ent);
}

__forceinline void DropBackpack(gentity_t* self) {

}

__forceinline qboolean CanDamage(gentity_t* enemy, gentity_t* self) {
	return CanDamage(enemy, self->r.currentOrigin);
}

__forceinline void T_Damage(gentity_t * targ, gentity_t* inflictor, gentity_t* attacker, float damage) {
	G_Damage(targ, inflictor, attacker, vec3_origin, targ->r.currentOrigin, damage, 0, MOD_UNKNOWN);
}

static vec3_t v_forward, v_right, v_up;

__forceinline void makevectors(vec3_t angles) {
	AngleVectors(angles, v_forward, v_right, v_up);
}

__forceinline void SpawnGib(gentity_t *self) {
	G_TempEntity(self->r.currentOrigin, EV_NEW_GIBS);
}


extern float g_damage;
void G_SendAnimUpdate(gentity_t* self, animEventType_t eventType, int startFrame, int endFrame);


#define TRUE 1.0f
#define FALSE 0.0f
