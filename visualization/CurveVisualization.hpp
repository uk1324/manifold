#pragma once

#include <game/Renderer.hpp>
#include <game/Curves.hpp>
#include <game/FpsCamera3d.hpp>
#include <engine/Math/Constants.hpp>

struct ParameterInputSettings {
	ParameterInputSettings(
		f32 selectedMin, 
		f32 selectedMax, 
		f32 allowedMin = -std::numeric_limits<f32>::infinity(),
		f32 allowedMax = std::numeric_limits<f32>::infinity());
	f32 selectedMin;
	f32 selectedMax;
	f32 allowedMin;
	f32 allowedMax;
};

//struct CurveBoundsInput {
//	CurveBoundsInput(
//		f32 startT, 
//		f32 endT, 
//		f32 selectedMin, 
//		f32 selectedMax, 
//		f32 allowedMin = -std::numeric_limits<f32>::infinity(),
//		f32 allowedMax = std::numeric_limits<f32>::infinity());
//	f32 startT;
//	f32 endT;
//
//	ParameterInputSettings startTSettings;
//	ParameterInputSettings endTSettings;
//};

struct CurveTInputs {
	CurveTInputs(
		f32 selectedT, 
		f32 startT, 
		f32 endT, 
		f32 selectedMin, 
		f32 selectedMax,
		f32 allowedMin = -std::numeric_limits<f32>::infinity(),
		f32 allowedMax = std::numeric_limits<f32>::infinity()
	);

	f32 selectedT;
	f32 startT;
	f32 endT;
	ParameterInputSettings selectedTSettings;
	ParameterInputSettings startTSettings;
	ParameterInputSettings endTSettings;
};

struct CurveVisualization {
	static CurveVisualization make();

	void update(Renderer& renderer);

	const CurveTInputs& selectedCurveTInputs() const;

	static constexpr auto nearZero = 0.01f;

	CurveTInputs helixT;
	ParameterInputSettings helixASettings;
	ParameterInputSettings helixBSettings;

	CurveTInputs vivanisCurveT;
	ParameterInputSettings vivanisCurveRSettings;

	CurveTInputs cycloidT;
	ParameterInputSettings cycloidRSettings;

	bool showFrenetFrame = false;
	bool showCircleOfCurvature = false;

	bool parametrizeByArclength = true;

	//void boundsSliders(CurveBoundsInput& bounds);
	void tInputs(CurveTInputs& inputs);
	void parameterSlider(const char* label, f32& parameter, ParameterInputSettings& settings);
	void parameterSettingsGui();
	struct OpenParameterSettings {
		f32* parameter;
		ParameterInputSettings* settings;
	};
	std::optional<OpenParameterSettings> currentlyOpenParameterSettings;

	Curves curves;
	FpsCamera3d camera;
};