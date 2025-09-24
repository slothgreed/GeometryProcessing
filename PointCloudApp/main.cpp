
#include "PointCloudApp.h"
#include "ComputePointCloudApp.h"
#include "ComputeShaderTest.h"
#include "MeshShaderTest.h"

#include <random>




int main()
{
	std::cout << std::fixed << std::setprecision(2); // 浮動小数点2桁まで
    //KI::ComputeShaderTest app;
	//KI::ComputePointCloudApp app;
	KI::PointCloudApp app;
	//KI::MeshShaderTest app;
	app.Initialize();
	app.Execute();
	app.Finalize();

	return 0;
}

//
//// エンコード関数
//uint32_t Part1By2(uint32_t x)
//{
//    x &= 0x3FF; // 10 ビットのみ使用
//    x = (x | (x << 16)) & 0x030000FF;
//    x = (x | (x << 8)) & 0x0300F00F;
//    x = (x | (x << 4)) & 0x030C30C3;
//    x = (x | (x << 2)) & 0x09249249;
//    return x;
//}
//
//uint32_t EncodeMorton3D(uint32_t x, uint32_t y, uint32_t z)
//{
//    return (Part1By2(x) | (Part1By2(y) << 1) | (Part1By2(z) << 2));
//}
//
//// デコード関数
//uint32_t Compact1By2(uint32_t x)
//{
//    x &= 0x09249249;
//    x = (x ^ (x >> 2)) & 0x030C30C3;
//    x = (x ^ (x >> 4)) & 0x0300F00F;
//    x = (x ^ (x >> 8)) & 0x030000FF;
//    x = (x ^ (x >> 16)) & 0x000003FF;
//    return x;
//}
//
//uint32_t DecodeMorton3D_X(uint32_t morton)
//{
//    return Compact1By2(morton);
//}
//
//uint32_t DecodeMorton3D_Y(uint32_t morton)
//{
//    return Compact1By2(morton >> 1);
//}
//
//uint32_t DecodeMorton3D_Z(uint32_t morton)
//{
//    return Compact1By2(morton >> 2);
//}
//
//int main()
//{
//    // 入力座標
//    uint32_t x = 234, y = 423, z = 345;
//
//    // エンコード (Morton Codeの生成)
//    uint32_t mortonCode = EncodeMorton3D(x, y, z);
//    std::cout << "Morton Code: " << mortonCode << std::endl;
//
//    // デコード (Morton Codeから座標の復元)
//    uint32_t decodedX = DecodeMorton3D_X(mortonCode);
//    uint32_t decodedY = DecodeMorton3D_Y(mortonCode);
//    uint32_t decodedZ = DecodeMorton3D_Z(mortonCode);
//
//    // 結果表示
//    std::cout << "Decoded X: " << decodedX << std::endl;
//    std::cout << "Decoded Y: " << decodedY << std::endl;
//    std::cout << "Decoded Z: " << decodedZ << std::endl;
//
//    return 0;
//}
