// g_newmath.cpp
//

#include "../splines/math_vector.h"
#include "../splines/math_matrix.h"
#include "../splines/math_angles.h"

/*
==============
RotateBounds
==============
*/
void RotateBounds(float *mins, float *maxs, float *angle) {
	idVec3_t _mins(mins[0], mins[1], mins[2]);
	idVec3_t _maxs(maxs[0], maxs[1], maxs[2]);


	idVec3_t angles(angle[0], angle[1], angle[2]);
	mat3_t axis;

	toMatrix(angles, axis);

	idVec3_t center, extents, rotatedExtents;
	
	center = (_mins + _maxs) * 0.5f;
	extents = _maxs - center;

	for (int i = 0; i < 3; i++) {
		rotatedExtents[i] = fabs(extents[0] * axis[0][i]) +
							fabs(extents[1] * axis[1][i]) +
							fabs(extents[2] * axis[2][i]);
	}

	idVec3_t b[2];
	center = center * axis;
	b[0] = center - rotatedExtents;
	b[1] = center + rotatedExtents;

	memcpy(mins, b[0], sizeof(float) * 3);
	memcpy(maxs, b[1], sizeof(float) * 3);
}