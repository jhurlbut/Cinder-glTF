/*
*
* Copyright (c) 2014, James Hurlbut
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or
* without modification, are permitted provided that the following
* conditions are met:
*
* Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in
* the documentation and/or other materials provided with the
* distribution.
*
* Neither the name of James Hurlbut nor the names of its
* contributors may be used to endorse or promote products
* derived from this software without specific prior written
* permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#pragma once

#include "cinder/app/app.h"

static glm::quat fromAngleAxis(const float& rfAngle_radian,
	const glm::vec3& rkAxis)
{
	// assert:  axis[] is unit length
	//
	// The quaternion representing the rotation is
	//   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

	float fHalfAngle_radian(0.5 * rfAngle_radian);
	float fSin = sin(fHalfAngle_radian);
	float w = glm::cos(fHalfAngle_radian);
	float x = fSin * rkAxis.x;
	float y = fSin * rkAxis.y;
	float z = fSin * rkAxis.z;
	return glm::quat(w, x, y, z);
}
/// Make a linear combination of two vectors and return the result.
// result = (a * ascl) + (b * bscl)
static void v3Combine(const glm::dvec3& a, const glm::dvec3& b, glm::dvec3& result, double ascl, double bscl)
{
	result = (a * ascl) + (b * bscl);
}


static void v3Scale(glm::dvec3& v, double desiredLength)
{
	double len = glm::length(v);
	if (len != 0) {
		double l = desiredLength / len;
		v[0] *= l;
		v[1] *= l;
		v[2] *= l;
	}
}

// decompose the matrix into its component parts
typedef struct {
	double scaleX, scaleY, scaleZ;
	double skewXY, skewXZ, skewYZ;
	double quaternionX, quaternionY, quaternionZ, quaternionW;
	double translateX, translateY, translateZ;
	double perspectiveX, perspectiveY, perspectiveZ, perspectiveW;
} DecomposedType;
/**
* Matrix decompose
* http://www.opensource.apple.com/source/WebCore/WebCore-514/platform/graphics/transforms/TransformationMatrix.cpp
* Decomposes the mode matrix to translations,rotation scale components
*
*/

static bool Decompose(glm::mat4& mat, DecomposedType& result){
	glm::dmat4 localMatrix = glm::dmat4(mat);


	// Normalize the matrix.
	if (localMatrix[3][3] == 0){
		return false;
	}

	int i, j;
	for (i = 0; i < 4; i++){
		for (j = 0; j < 4; j++){
			localMatrix[i][j] /= localMatrix[3][3];
		}
	}

	// perspectiveMatrix is used to solve for perspective, but it also provides
	// an easy way to test for singularity of the upper 3x3 component.
	glm::dmat4 perspectiveMatrix = localMatrix;

	for (i = 0; i < 3; i++){
		perspectiveMatrix[i][3] = 0;
	}
	perspectiveMatrix[3][3] = 1;

	if (glm::determinant(perspectiveMatrix) == 0){
		return false;
	}

	// First, isolate perspective.  This is the messiest.
	if (localMatrix[0][3] != 0 || localMatrix[1][3] != 0 || localMatrix[2][3] != 0) {
		// rightHandSide is the right hand side of the equation.
		glm::dvec4 rightHandSide;
		rightHandSide[0] = localMatrix[0][3];
		rightHandSide[1] = localMatrix[1][3];
		rightHandSide[2] = localMatrix[2][3];
		rightHandSide[3] = localMatrix[3][3];

		// Solve the equation by inverting perspectiveMatrix and multiplying
		// rightHandSide by the inverse.  (This is the easiest way, not
		// necessarily the best.)
		glm::dmat4 inversePerspectiveMatrix, transposedInversePerspectiveMatrix;
		inversePerspectiveMatrix = glm::inverse(perspectiveMatrix);//   inverse(perspectiveMatrix, inversePerspectiveMatrix);
		transposedInversePerspectiveMatrix = glm::transpose(inversePerspectiveMatrix);//   transposeMatrix4(inversePerspectiveMatrix, transposedInversePerspectiveMatrix);

		glm::dvec4 perspectivePoint;

		perspectivePoint = transposedInversePerspectiveMatrix * rightHandSide;
		//  v4MulPointByMatrix(rightHandSide, transposedInversePerspectiveMatrix, perspectivePoint);

		result.perspectiveX = perspectivePoint[0];
		result.perspectiveY = perspectivePoint[1];
		result.perspectiveZ = perspectivePoint[2];
		result.perspectiveW = perspectivePoint[3];

		// Clear the perspective partition
		localMatrix[0][3] = localMatrix[1][3] = localMatrix[2][3] = 0;
		localMatrix[3][3] = 1;
	}
	else {
		// No perspective.
		result.perspectiveX = result.perspectiveY = result.perspectiveZ = 0;
		result.perspectiveW = 1;
	}

	// Next take care of translation (easy).
	result.translateX = localMatrix[3][0];
	localMatrix[3][0] = 0;
	result.translateY = localMatrix[3][1];
	localMatrix[3][1] = 0;
	result.translateZ = localMatrix[3][2];
	localMatrix[3][2] = 0;

	// Vector4 type and functions need to be added to the common set.
	glm::dvec3 row[3], pdum3;

	// Now get scale and shear.
	for (i = 0; i < 3; i++) {
		row[i][0] = localMatrix[i][0];
		row[i][1] = localMatrix[i][1];
		row[i][2] = localMatrix[i][2];
	}

	// Compute X scale factor and normalize first row.
	result.scaleX = glm::length(row[0]);// v3Length(row[0]);

	v3Scale(row[0], 1.0);

	// Compute XY shear factor and make 2nd row orthogonal to 1st.
	result.skewXY = glm::dot(row[0], row[1]);
	v3Combine(row[1], row[0], row[1], 1.0, -result.skewXY);

	// Now, compute Y scale and normalize 2nd row.
	result.scaleY = glm::length(row[1]);
	v3Scale(row[1], 1.0);
	result.skewXY /= result.scaleY;

	// Compute XZ and YZ shears, orthogonalize 3rd row.
	result.skewXZ = glm::dot(row[0], row[2]);
	v3Combine(row[2], row[0], row[2], 1.0, -result.skewXZ);
	result.skewYZ = glm::dot(row[1], row[2]);
	v3Combine(row[2], row[1], row[2], 1.0, -result.skewYZ);

	// Next, get Z scale and normalize 3rd row.
	result.scaleZ = glm::length(row[2]);
	v3Scale(row[2], 1.0);
	result.skewXZ /= result.scaleZ;
	result.skewYZ /= result.scaleZ;

	// At this point, the matrix (in rows[]) is orthonormal.
	// Check for a coordinate system flip.  If the determinant
	// is -1, then negate the matrix and the scaling factors.
	pdum3 = glm::cross(row[1], row[2]); // v3Cross(row[1], row[2], pdum3);
	if (glm::dot(row[0], pdum3) < 0) {
		for (i = 0; i < 3; i++) {
			result.scaleX *= -1;
			row[i][0] *= -1;
			row[i][1] *= -1;
			row[i][2] *= -1;
		}
	}

	// Now, get the rotations out, as described in the gem.

	// FIXME - Add the ability to return either quaternions (which are
	// easier to recompose with) or Euler angles (rx, ry, rz), which
	// are easier for authors to deal with. The latter will only be useful
	// when we fix https://bugs.webkit.org/show_bug.cgi?id=23799, so I
	// will leave the Euler angle code here for now.

	// ret.rotateY = asin(-row[0][2]);
	// if (cos(ret.rotateY) != 0) {
	//     ret.rotateX = atan2(row[1][2], row[2][2]);
	//     ret.rotateZ = atan2(row[0][1], row[0][0]);
	// } else {
	//     ret.rotateX = atan2(-row[2][0], row[1][1]);
	//     ret.rotateZ = 0;
	// }

	double s, t, x, y, z, w;

	t = row[0][0] + row[1][1] + row[2][2] + 1.0;

	if (t > 1e-4) {
		s = 0.5 / sqrt(t);
		w = 0.25 / s;
		x = (row[2][1] - row[1][2]) * s;
		y = (row[0][2] - row[2][0]) * s;
		z = (row[1][0] - row[0][1]) * s;
	}
	else if (row[0][0] > row[1][1] && row[0][0] > row[2][2]) {
		s = sqrt(1.0 + row[0][0] - row[1][1] - row[2][2]) * 2.0; // S=4*qx 
		x = 0.25 * s;
		y = (row[0][1] + row[1][0]) / s;
		z = (row[0][2] + row[2][0]) / s;
		w = (row[2][1] - row[1][2]) / s;
	}
	else if (row[1][1] > row[2][2]) {
		s = sqrt(1.0 + row[1][1] - row[0][0] - row[2][2]) * 2.0; // S=4*qy
		x = (row[0][1] + row[1][0]) / s;
		y = 0.25 * s;
		z = (row[1][2] + row[2][1]) / s;
		w = (row[0][2] - row[2][0]) / s;
	}
	else {
		s = sqrt(1.0 + row[2][2] - row[0][0] - row[1][1]) * 2.0; // S=4*qz
		x = (row[0][2] + row[2][0]) / s;
		y = (row[1][2] + row[2][1]) / s;
		z = 0.25 * s;
		w = (row[1][0] - row[0][1]) / s;
	}

	result.quaternionX = x;
	result.quaternionY = y;
	result.quaternionZ = z;
	result.quaternionW = w;

	return true;

}
