struct BVH
{
	vec4 minBox;
	vec4 maxBox;
	uvec4 mix;
};

vec3 getMinBox(BVH h){return h.minBox.xyz;}
vec3 getMaxBox(BVH h){return h.maxBox.xyz;}
int getLeft(BVH h) {return int(h.minBox.w);}
int getRight(BVH h) {return int(h.maxBox.w);}
int getParent(BVH h) {return int(h.mix.x);}
int getMorton(BVH h) {return int(h.mix.y);}
int getTriangle(BVH h) {return int(h.mix.z);}
