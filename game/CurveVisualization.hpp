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

struct CurveBoundsInput {
	CurveBoundsInput(
		f32 startT, 
		f32 endT, 
		f32 selectedMin, 
		f32 selectedMax, 
		f32 allowedMin = -std::numeric_limits<f32>::infinity(),
		f32 allowedMax = std::numeric_limits<f32>::infinity());
	f32 startT;
	f32 endT;

	ParameterInputSettings startTSettings;
	ParameterInputSettings endTSettings;
};

struct CurveVisualization {
	CurveVisualization();

	void update(Renderer& renderer);

	f32 selectedT = 0.0f;

	struct StartEnd {
		f32 start;
		f32 end;
	};
	StartEnd startEndT() const;

	CurveBoundsInput helixBounds = CurveBoundsInput(0.0f, PI<f32> * 4.0f, 0.0f, PI<f32> * 4.0f);
	ParameterInputSettings helixTSettings = ParameterInputSettings(0.0f, PI<f32>);
	ParameterInputSettings helixASettings = ParameterInputSettings(0.01f, 1.0f, 0.01f);
	ParameterInputSettings helixBSettings = ParameterInputSettings(0.0f, 1.0f);

	bool showFrenetFrame = false;
	bool showCircleOfCurvature = false;

	void boundsSliders(CurveBoundsInput& bounds);
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