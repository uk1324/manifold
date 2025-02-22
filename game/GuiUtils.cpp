#include "GuiUtils.hpp"
#include <imgui/imgui.h>

void sliderF32(const char* label, f32& value, f32 min, f32 max) {
	 if (ImGui::SliderFloat(label, &value, min, max)) {
		 value = std::clamp(value, min, max);
	 }
}