#include "CurveVisualization.hpp"
#include <glad/glad.h>
#include <engine/Math/Color.hpp>
#include <imgui/imgui.h>
#include <engine/Math/Interpolation.hpp>
#include <engine/Window.hpp>
#include <game/Constants.hpp>
#include <engine/Math/Constants.hpp>
#include <engine/Input/Input.hpp>
#include <game/GuiUtils.hpp>
#include <imgui/implot.h>

CurveVisualization CurveVisualization::make() {

	auto result = CurveVisualization{
		.helixT = CurveTInputs(0.0f, 0.0f, PI<f32> * 4.0f, 0.0f, PI<f32> * 4.0f, nearZero),
		.helixASettings = ParameterInputSettings(nearZero, 1.0f, nearZero),
		.helixBSettings = ParameterInputSettings(0.0f, 1.0f),
		.vivanisCurveT = CurveTInputs(0.0f, -PI<f32>, PI<f32>, -PI<f32>, PI<f32>, -PI<f32>, PI<f32>),
		.vivanisCurveRSettings = ParameterInputSettings(nearZero, 4.0f, nearZero),
		.cycloidT = CurveTInputs(0.0f, 0.0f, TAU<f32> * 2.0f, 0.0f, TAU<f32> * 2.0f),
		.cycloidRSettings = ParameterInputSettings(nearZero, 4.0f, nearZero)
	};

	result.camera.movementSpeed = 2.0f;
	return result;
}

void CurveVisualization::update(Renderer& renderer) {
	ImGui::Begin("settings");

	{
		auto curveTypeStr = [](Curves::Type type) {
			switch (type) {
				using enum Curves::Type;
			case HELIX: return "helix";
			case VIVANIS_CURVE: return "vivani's curve";
			case CYCLOID: return "cycloid";
			}
			return "";
		};

		using enum Curves::Type;
		Curves::Type surfaceTypes[]{
			HELIX,
			VIVANIS_CURVE,
			CYCLOID,
		};

		if (ImGui::BeginCombo("curve", curveTypeStr(curves.type))) {
			for (const auto& type : surfaceTypes) {
				const auto isSelected = curves.type == type;
				if (ImGui::Selectable(curveTypeStr(type), isSelected)) {
					curves.type = type;
				}
				if (isSelected) { ImGui::SetItemDefaultFocus(); }
			}
			ImGui::EndCombo();
		}
	}

	ImGui::Checkbox("frenet frame", &showFrenetFrame);
	ImGui::Checkbox("circle of curvature", &showCircleOfCurvature);

	ImGui::Checkbox("parametrize plot by arclength", &parametrizeByArclength);

	switch (curves.type) {
		using enum Curves::Type;
	case HELIX:
		tInputs(helixT);
		parameterSlider("a", curves.helix.a, helixASettings);
		parameterSlider("b", curves.helix.b, helixBSettings);
		break;

	case VIVANIS_CURVE: 
		tInputs(vivanisCurveT);
		parameterSlider("r", curves.vivanisCurve.r, vivanisCurveRSettings);
		break;

	case CYCLOID:
		tInputs(cycloidT);
		parameterSlider("r", curves.cycloid.r, cycloidRSettings);
		break;
	}

	const auto hasValue = currentlyOpenParameterSettings.has_value();
	if (hasValue) ImGui::PushID(currentlyOpenParameterSettings->parameter);
	parameterSettingsGui();
	if (hasValue) ImGui::PopID();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, i32(Window::size().x), i32(Window::size().y));

	ImGui::End();


	camera.update(Constants::dt);

	const auto view = camera.viewMatrix();
	const auto cameraForward = (Vec4(Vec3::FORWARD, 0.0f) * view.inversed()).xyz().normalized();
	const auto aspectRatio = Window::aspectRatio();
	const auto projection = Mat4::perspective(PI<f32> / 2.0f, aspectRatio, 0.1f, 1000.0f);
	const auto swaxpYZ = Mat4(Mat3(Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, 1.0f, 0.0f)));
	renderer.transform = projection * view;
	renderer.view = view;

	std::vector<Vec3> centersOfCurvature;
	const auto tInputs = selectedCurveTInputs();
	const auto color = Color3::WHITE;
	const auto samplePoints = 100;
	const auto circlePoints = 50;
	f32 radius = 0.02f;
	for (i32 ti = 0; ti < samplePoints; ti++) {
		const auto t = lerp(tInputs.startT, tInputs.endT, f32(ti) / f32(samplePoints - 1));
		const auto curvePosition = curves.position(t);
		const auto curveNormal = curves.normal(t);
		const auto binormal = curves.binormal(t);
		const auto radiusOfCurvature = 1.0f / curves.curvature(t);
		centersOfCurvature.push_back(curvePosition + radiusOfCurvature * curveNormal);
		for (i32 ai = 0; ai < circlePoints; ai++) {
			const auto a = f32(ai) / f32(circlePoints) * TAU<f32>;
			const auto vertexNormal = curveNormal * cos(a) + binormal * sin(a);
			const auto vertexPosition = curvePosition + vertexNormal * radius;
			renderer.coloredTriangles.addVertex(Vertex3Pnc{
				.position = vertexPosition,
				.normal = vertexNormal,
				.color = color,
			});
		}
	}
	auto index = [&](i32 ai, i32 ti) -> i32 {
		return ti * circlePoints + ai;
	};

	std::vector<f32> ts;
	std::vector<f32> arclengths;
	std::vector<f32> curvatures;
	std::vector<f32> torsions;
	{
		f32 currentArclength = 0.0f;
		const auto n = 200;
		arclengths.push_back(0.0f);
		for (i32 i = 0; i < n; i++) {
			auto tOfI = [&](i32 i){
				return lerp(tInputs.startT, tInputs.endT, f32(i) / f32(n - 1));
			};

			const auto t = tOfI(i);
			if (i < n - 1) {
				const auto t1 = tOfI(i + 1);
				currentArclength += curves.position(t).distanceTo(curves.position(t1));
				arclengths.push_back(currentArclength);
			}
			const auto curvature = curves.curvature(t);
			const auto torsion = curves.torsion(t);
			ts.push_back(t);
			curvatures.push_back(curvature);
			torsions.push_back(torsion);
		}
	}

	for (i32 i = 0; i < centersOfCurvature.size() - 1; i++) {
		renderer.line(centersOfCurvature[i], centersOfCurvature[i + 1], radius, Vec3(1.0f, 0.75f, 0.0f));
	}

	std::vector<f32>* argument;
	if (parametrizeByArclength) {
		argument = &arclengths;
	} else {
		argument = &ts;
	}

	//ImPlot::ShowDemoWindow();
	ImGui::Begin("plot");
	if (ImPlot::BeginPlot("plot", ImVec2(-1.0f, -1.0f), ImPlotFlags_Equal)) {
		if (parametrizeByArclength) {
			ImPlot::SetupAxes("arclength", "value");
		} else {
			ImPlot::SetupAxes("t", "value");
		}
		ImPlot::SetupAxisLimits(ImAxis_X1, tInputs.startT, tInputs.endT);
		{
			ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
			ImPlot::PlotLine("curvature", argument->data(), curvatures.data(), int(ts.size()));
			ImPlot::PopStyleColor();
		}
		{
			ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
			ImPlot::PlotLine("torsion", argument->data(), torsions.data(), int(ts.size()));
			ImPlot::PopStyleColor();
		}
		////ImPlot::SetupAxesLimits(tBounds.start, tBounds.end, , surface.vMax, ImPlotCond_Always);
		//Vec2 forward = surfaceCamera.uvPosition + Vec2::oriented(surfaceCamera.uvForwardAngle) * 0.3f;
		//f32 xs[] = { surfaceCamera.uvPosition.x, forward.x };
		//f32 ys[] = { surfaceCamera.uvPosition.y, forward.y };
		//ImPlot::PlotLine("arrow", xs, ys, 2);
		//plotVec2Scatter("points", uvPositions);
		ImPlot::EndPlot();
	}
	ImGui::End();

	for (i32 ti = 0; ti < samplePoints - 1; ti++) {
		i32 previousAi = circlePoints - 1;
		for (i32 ai = 0; ai < circlePoints; ai++) {
			renderer.coloredTriangles.addQuad(
				index(previousAi, ti),
				index(previousAi, ti + 1),
				index(ai, ti + 1),
				index(ai, ti)
			);
			previousAi = ai;
		}
	}

	const auto curvePosition = curves.position(tInputs.selectedT);
	const auto curveTangent = curves.tangent(tInputs.selectedT);
	const auto curveNormal = curves.normal(tInputs.selectedT);
	if (showFrenetFrame) {
		const auto binormal = curves.binormal(tInputs.selectedT);
		auto vector = [&](Vec3 vector, Vec3 color) {
			const auto radius = 0.02f;
			const auto coneRadius = 2.0f * radius;
			const auto coneLength = 2.0f * coneRadius;
			renderer.arrowStartDirection(curvePosition, vector, radius, coneRadius, coneLength, color, color);
		};
		vector(curveTangent, Color3::RED);
		vector(curveNormal, Color3::GREEN);
		vector(binormal, Color3::BLUE);
	}
	if (showCircleOfCurvature) {
		const auto radiusOfCurvature = 1.0f / curves.curvature(tInputs.selectedT);
		const auto centerOfCurvature = curvePosition + curveNormal * radiusOfCurvature;
		renderer.sphere(centerOfCurvature, 0.03f, Color3::YELLOW);
		renderer.circleArc(centerOfCurvature, curveNormal, curveTangent, radiusOfCurvature, Color3::YELLOW);
	}

	renderer.renderColoredTriangles(1.0f);
	renderer.renderCones();
	renderer.renderCyllinders();
	renderer.renderHemispheres();
}

static constexpr auto parameterSettingsWindowName = "parameter settings";


void CurveVisualization::parameterSlider(const char* label, f32& parameter, ParameterInputSettings& settings) {
	ImGui::PushID(&parameter);
	ImGui::Text("%s", label);
	ImGui::SameLine();
	const auto disableSlider = settings.selectedMin >= settings.selectedMax;
	if (disableSlider) ImGui::BeginDisabled();
	ImGui::SliderFloat("##parameterSlider", &parameter, settings.selectedMin, settings.selectedMax);
	auto clampBound = [&](f32 value) -> f32 {
		return std::clamp(value, settings.allowedMin, settings.selectedMax);
	};
	settings.selectedMin = clampBound(settings.selectedMin);
	settings.selectedMax = clampBound(settings.selectedMax);

	if (disableSlider) ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	if (ImGui::Button("o")) {
		currentlyOpenParameterSettings = OpenParameterSettings{
			.parameter = &parameter,
			.settings = &settings,
		};
		ImGui::OpenPopup(parameterSettingsWindowName);
	}

	ImGui::PopStyleColor();
	ImGui::PopID();
}

void CurveVisualization::parameterSettingsGui() {
	const auto center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	if (!ImGui::BeginPopupModal(parameterSettingsWindowName, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		return;
	}

	if (!currentlyOpenParameterSettings.has_value()) {
		ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
		return;
	}

	ImGui::InputFloat("min", &currentlyOpenParameterSettings->settings->selectedMin);
	ImGui::InputFloat("max", &currentlyOpenParameterSettings->settings->selectedMax);

	if (ImGui::Button("close")) {
		currentlyOpenParameterSettings = std::nullopt;
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

const CurveTInputs& CurveVisualization::selectedCurveTInputs() const {
	switch (curves.type) {
		using enum Curves::Type;
	case HELIX: return helixT;
	case VIVANIS_CURVE: return vivanisCurveT;
	case CYCLOID: return cycloidT;
	}
	ASSERT_NOT_REACHED();
}

void CurveVisualization::tInputs(CurveTInputs& inputs) {
	parameterSlider("selcted t", inputs.selectedT, inputs.selectedTSettings);
	parameterSlider("start t", inputs.startT, inputs.startTSettings);
	parameterSlider("end t", inputs.endT, inputs.endTSettings);
}

ParameterInputSettings::ParameterInputSettings(f32 selectedMin, f32 selectedMax, f32 allowedMin, f32 allowedMax)
	: selectedMin(selectedMin)
	, selectedMax(selectedMax)
	, allowedMin(allowedMin)
	, allowedMax(allowedMax) {}


CurveTInputs::CurveTInputs(f32 selectedT, f32 startT, f32 endT, f32 selectedMin, f32 selectedMax, f32 allowedMin, f32 allowedMax)
	: startTSettings(selectedMin, selectedMax, allowedMin, allowedMax)
	, endTSettings(selectedMin, selectedMax, allowedMin, allowedMax)
	, selectedTSettings(selectedMin, selectedMax, allowedMin, allowedMax)
	, selectedT(selectedT)
	, startT(startT)
	, endT(endT) {}

