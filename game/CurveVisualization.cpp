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

CurveVisualization::CurveVisualization() {
	camera.movementSpeed = 2.0f;
}

void CurveVisualization::update(Renderer& renderer) {
	ImGui::Begin("settings");

	ImGui::Checkbox("frenet frame", &showFrenetFrame);
	ImGui::Checkbox("circle of curvature", &showCircleOfCurvature);

	switch (curves.type) {
		using enum Curves::Type;
	case HELIX:
		boundsSliders(helixBounds);
		parameterSlider("t", selectedT, helixTSettings);
		parameterSlider("a", curves.helix.a, helixASettings);
		parameterSlider("b", curves.helix.b, helixBSettings);
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

	const auto color = Color3::WHITE;
	const auto tBounds = startEndT();
	const auto samplePoints = 100;
	const auto circlePoints = 50;
	f32 radius = 0.02f;
	for (i32 ti = 0; ti < samplePoints; ti++) {
		const auto t = lerp(tBounds.start, tBounds.end, f32(ti) / f32(samplePoints - 1));
		const auto curvePosition = curves.position(t);
		const auto curveNormal = curves.normal(t);
		const auto binormal = curves.binormal(t);
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
	//std::vector<f32> arclengths;
	std::vector<f32> curvatures;
	std::vector<f32> torsions;
	{
		f32 currentArclength = 0.0f;
		const auto n = 200;
		for (i32 i = 0; i < n; i++) {
			const auto t = lerp(tBounds.start, tBounds.end, f32(i) / f32(n - 1));
			const auto curvature = curves.curvature(t);
			const auto torsion = curves.torsion(t);
			ts.push_back(t);
			curvatures.push_back(curvature);
			torsions.push_back(torsion);
		}
	}

	//ImPlot::ShowDemoWindow();
	ImGui::Begin("plot");
	if (ImPlot::BeginPlot("plot", ImVec2(-1.0f, -1.0f), ImPlotFlags_Equal)) {
		ImPlot::SetupAxes("t", "value");
		ImPlot::SetupAxisLimits(ImAxis_X1, tBounds.start, tBounds.end);
		{
			ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
			ImPlot::PlotLine("curvature", ts.data(), curvatures.data(), ts.size());
			ImPlot::PopStyleColor();
		}
		{
			ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
			ImPlot::PlotLine("torsion", ts.data(), torsions.data(), ts.size());
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
		f32 previousAi = circlePoints - 1;
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

	const auto curvePosition = curves.position(selectedT);
	const auto curveTangent = curves.tangent(selectedT);
	const auto curveNormal = curves.normal(selectedT);
	if (showFrenetFrame) {
		const auto binormal = curves.binormal(selectedT);
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
		const auto radiusOfCurvature = 1.0f / curves.curvature(selectedT);
		renderer.circleArc(curvePosition + curveNormal * radiusOfCurvature, curveNormal, curveTangent, radiusOfCurvature, Color3::YELLOW);
	}

	renderer.renderColoredTriangles(1.0f);
	renderer.renderCones();
	renderer.renderCyllinders();
}

CurveVisualization::StartEnd CurveVisualization::startEndT() const {
	switch (curves.type) {
		using enum Curves::Type;
	case HELIX: return { helixBounds.startT, helixBounds.endT };
	}
	return { 0.0f, 0.0f };
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

void CurveVisualization::boundsSliders(CurveBoundsInput& bounds) {
	parameterSlider("start t", bounds.startT, bounds.startTSettings);
	parameterSlider("end t", bounds.endT, bounds.endTSettings);
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

ParameterInputSettings::ParameterInputSettings(f32 selectedMin, f32 selectedMax, f32 allowedMin, f32 allowedMax)
	: selectedMin(selectedMin)
	, selectedMax(selectedMax)
	, allowedMin(allowedMin)
	, allowedMax(allowedMax) {}

CurveBoundsInput::CurveBoundsInput(f32 startT, f32 endT, f32 selectedMin, f32 selectedMax, f32 allowedMin, f32 allowedMax)
	: startT(startT)
	, endT(endT)
	, startTSettings(selectedMin, selectedMax, allowedMin, allowedMax)
	, endTSettings(selectedMin, selectedMax, allowedMin, allowedMax) {
}
