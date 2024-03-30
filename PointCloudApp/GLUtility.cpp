#include "GLUtility.h"

namespace KI
{

void Printf(const Matrix4x4& mat)
{
	printf(
		"%.3lf,%.3lf,%.3lf,%.3lf\n%.3lf,%.3lf,%.3lf,%.3lf\n%.3lf,%.3lf,%.3lf,%.3lf\n%.3lf,%.3lf,%.3lf,%.3lf\n",
		mat[0][0], mat[0][1], mat[0][2], mat[0][3],
		mat[1][0], mat[1][1], mat[1][2], mat[1][3],
		mat[2][0], mat[2][1], mat[2][2], mat[2][3],
		mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
}


}
