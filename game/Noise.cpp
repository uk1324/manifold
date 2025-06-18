#include "Noise.hpp"
//#include <FastNoise/FastNoise.h>

void generateNoise(f32* out, f32* x, f32* y, f32* z, f32* w, i32 count, i32 seed, f32 gain, f32 lacunarity) {
	/*auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
	auto fnFractal = FastNoise::New<FastNoise::FractalFBm>();
	fnFractal->SetSource(fnSimplex);
	fnFractal->SetOctaveCount(5);
	fnFractal->SetLacunarity(lacunarity);
	fnFractal->SetGain(gain);
	const auto r = fnFractal->GenPositionArray4D(out, count, x, y, z, w, 0.0f, 0.0f, 0.0f, 0.0f, seed);*/

}