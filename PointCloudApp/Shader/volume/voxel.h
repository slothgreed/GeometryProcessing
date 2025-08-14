struct Voxel
{
	vec4 pitch;
	vec4 bdbMin;
	vec4 bdbMax;
	ivec4 resolute;
};
int getIndex(Voxel voxel, int x, int y, int z)
{
	return
		x +
		y * voxel.resolute.x +
		z * voxel.resolute.x * voxel.resolute.y;
}

ivec3 getCoordinate(Voxel voxel, int index)
{
	int x = index % voxel.resolute.x;
	int y = (index / voxel.resolute.x) % voxel.resolute.y;
	int z = index / (voxel.resolute.x * voxel.resolute.y);
	return ivec3(x, y, z);
}
vec3 getPosition(Voxel voxel, vec3 index)
{
	return vec3(index.x, index.y, index.z) * voxel.pitch.xyz;
}