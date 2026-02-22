#include "ImageAlgorithm.h"
#include "Utility.h"
namespace KI
{
bool IsBoundPixel(const PixelData& data, const Vector2i& pixel, const Neighbor8& neighbor)
{
	auto center = data.Get(pixel);
	auto white = !ColorUtility::IsBlack(center);
	for (int i = 0; i < neighbor.Size(); i++) {
		if (!data.IsIn(pixel + neighbor[i])) { continue; }
		if (white && ColorUtility::IsBlack(data.Get(pixel + neighbor[i]))) { return true; }
	}

	return false;
}
Polyline ImageAlgorithm::CreateOutlineByMooreNeighbor(const PixelData& data, int loopNum)
{
	int seedIndex = 0;
	auto neighbor8 = Neighbor8::Create();
	for (const auto& pixel : data) {
		if (IsBoundPixel(data, data.IndexToXY(seedIndex), neighbor8)) {
			break;
		}
		seedIndex++;
	}
	Vector<Vector3> polyline;

	auto seed = data.IndexToXY(seedIndex);

	auto isForeground = [&](const Vector2i& p)->bool
	{
		if (!data.IsIn(p)) return false;
		return ColorUtility::IsBlack(data.Get(p));
	};

	auto dirFromTo = [&](const Vector2i& from, const Vector2i& to)->int
	{
		Vector2i d = to - from;
		for (int i = 0; i < Neighbor8::Size(); ++i) {
			if (neighbor8[i].x == d.x && neighbor8[i].y == d.y) return i;
		}
		return 6; // 合わなければWにしておく（基本ここには来ない）
	};

	// b0 = seed, c0 は seed の左 (W) を外側参照点として置くのが定番
	Vector2i b = seed;
	Vector2i c = Vector2i(seed.x - 1, seed.y);   // 画像外でもOK（外側参照点）

	const Vector2i b0 = b;
	const Vector2i c0 = c;

	polyline.push_back(Vector3((float)b.x, (float)b.y, 0.0f));
	const int maxSteps = data.width * data.height * 8;
	for (int step = 0; step < maxSteps; ++step) {
		// bから見たcの方向
		int dirBC = dirFromTo(b, c);
		// cの次(時計回り)から探索開始
		int startDir = Neighbor8::Next(dirBC);

		bool found = false;
		Vector2i bNext = b;
		Vector2i cNext = c;

		for (int k = 0; k < 8; ++k) {
			int d = Neighbor8::Index(startDir + k);
			Vector2i n = b + neighbor8[d];

			// 前景であること（必要なら境界制限も入れる）
			if (!IsBoundPixel(data,n,neighbor8)) continue;

			// 次の境界点
			bNext = n;

			// 次のcは「見つけた方向の1つ前（反時計回り側）」＝直前に見ていた点
			int prev = Neighbor8::Prev(d);
			cNext = b + neighbor8[prev];

			found = true;
			printf("(%d, %d)\n", dirBC, Neighbor8::Index(startDir + k));
			break;
		}

		if (!found) break;

		b = bNext;
		c = cNext;

		polyline.push_back(Vector3((float)b.x, (float)b.y, 0.0f));

		// 終了条件：bとcが開始状態に戻ったら完了（重要）
		if (b == b0) break;

		// 念のための安全弁
		if ((int)polyline.size() >= data.width * data.height) break;
		if (loopNum == (int)polyline.size()) break;
	}
	printf("----------------------------\n");
	return Polyline(std::move(polyline), Polyline::Hint::LineLoop);
}

}