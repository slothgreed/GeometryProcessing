struct GLTFNode
{
	mat4 matrix;
	vec4 scale;
	vec4 translate;
	mat4 rotate;
	mat4 localMatrix;
	int skinId;
	float padding[23];
} node;



struct GLTFMaterial
{
	vec4 baseColor;
	vec4 emissiveColor;
	int baseTexture;
	float metallic;
	float roughness;
	int metalRoughnessTexture;
	int normalTexture;
	int occlusionTexture;
	int emissiveTexture;
	float normalScale;
	float alphaCuttoff;
	int alphaMode;
	int doubleSided;
	float padding[45];
};

#define CHANNEL_PATH_TRANSLATE 0
#define CHANNEL_PATH_SCALE 1
#define CHANNEL_PATH_ROTATE 2
struct GLTFChannel
{
	int sampler;
	int node;
	int path;
};




#define GetSamplerTimer(sampler, index){ return sampler.data[5 * index + 1];};

#define GetSamplerTransform(sampler, index) \
	vec4(							 \
		sampler.data[5 * index + 2], \
		sampler.data[5 * index + 3], \
		sampler.data[5 * index + 4], \
		sampler.data[5 * index + 5]);\


#define Matrix4x4ToFloat(data,offset,mat) \
	data[offset] = mat[0][0]; 	  \
	data[offset + 1] = mat[0][1]; \
	data[offset + 2] = mat[0][2]; \
	data[offset + 3] = mat[0][3]; \
	data[offset + 4] = mat[1][0]; \
	data[offset + 5] = mat[1][1]; \
	data[offset + 6] = mat[1][2]; \
	data[offset + 7] = mat[1][3]; \
	data[offset + 8] = mat[2][0]; \
	data[offset + 9] = mat[2][1]; \
	data[offset + 10] = mat[2][2]; \
	data[offset + 11] = mat[2][3]; \
	data[offset + 12] = mat[3][0]; \
	data[offset + 13] = mat[3][1]; \
	data[offset + 14] = mat[3][2]; \
	data[offset + 15] = mat[3][3]; \
	
	
#define FloatToMatrix4x4(data,offset) \
	mat4(data[offset],data[offset + 1],data[offset + 2],data[offset + 3], \
		 data[offset + 4],data[offset + 5],data[offset + 6],data[offset + 7], \
		 data[offset + 8],data[offset + 9],data[offset + 10],data[offset + 11], \
		 data[offset + 12],data[offset + 13],data[offset + 14],data[offset + 15]) \
		 

#define GetRootSkinIndex(skins,skinIndex)int(skins[0]);
#define GetNodeSize(skins,skinIndex)int(skins[1]);
#define GetNodeIndex(skins,skinIndex,index)int(skins[index*33+2]);
#define GetBindMatrix(skins,skinIndex,childNodeIndex)FloatToMatrix4x4(skins,childNodeIndex*33+3);
#define GetJointMatrix(skins,skinIndex,childNodeIndex)FloatToMatrix4x4(skins,childNodeIndex*33+3+16);
#define SetJointMatrix(skins,skinIndex,childNodeIndex,matrix) Matrix4x4ToFloat(skins,childNodeIndex*33+3+16,matrix);
