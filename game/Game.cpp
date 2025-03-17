#include <game/Game.hpp>
#include <engine/Input/Input.hpp>
#include <gfx2d/Quad2dPt.hpp>
#include <engine/Math/Interpolation.hpp>
#include <engine/Math/Color.hpp>
#include <game/UniformPartition.hpp>
#include <imgui/imgui.h>
#include <engine/Math/Mat4.hpp>
#include <glad/glad.h>
#include <game/Constants.hpp>
#include <engine/Window.hpp>
#include <engine/Math/Constants.hpp>

Game::Game()
	: renderer(GameRenderer::make()) {

	surface.initialize(Surface::Type::TORUS);
	//Window::disableCursor();

	/*for (i32 i = 0; i < 100; i++) {
 		const auto pos = surface.randomPointOnSurface();
		const auto velocity = surface.randomUnitTangentVectorAt(pos);
		bullets.push_back(Bullet{
			.position = pos,
			.velocity = surface.scaleTangent(velocity, 0.5f),
		});
	}*/
	/*basicEmitters.push_back(BasicEmitter{ 
		.elapsed = 0.0f,
		.position = surface.randomPointOnSurface()
	});*/
}

#include <GLFW/include/GLFW/glfw3.h>

template<typename T, typename Function>
void iterateAndRemove(std::vector<T>& v, Function function) {
	v.erase(
		std::remove_if(
			v.begin(),
			v.end(),
			function
		),
		v.end()
	);
}

void Game::update() {

	/*const auto doesJoysticExist = glfwJoystickPresent(GLFW_JOYSTICK_1);
	if (doesJoysticExist) {
		glfwGetJoystickAxes(GLFW_JOYSTICK_1);
	}*/


	/*if (Input::isKeyDown(KeyCode::F3)) {
		showGui = !showGui;
	}*/
	//const auto bulletSpeed = 0.35f;
	const auto bulletSpeed = 0.30f;
	/*if (showGui) {
		gui();
	}*/
	ImGui::Begin("settings");

	Vec2 directionInput(0.0f);
	{
		if (Input::isKeyHeld(KeyCode::W)) {
			directionInput.y += 1.0f;
		}
		if (Input::isKeyHeld(KeyCode::S)) {
			directionInput.y -= 1.0f;
		}
		if (Input::isKeyHeld(KeyCode::D)) {
			directionInput.x += 1.0f;
		}
		if (Input::isKeyHeld(KeyCode::A)) {
			directionInput.x -= 1.0f;
		}
		directionInput = directionInput.normalized();

		int count;
		const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);
		if (const auto isJoystickPresent = axes != nullptr) {
			if (count >= 2) {
				if (directionInput == Vec2(0.0f)) {

					const auto axesInput = Vec2(axes[0], -axes[1]);

					if (axesInput.length() > 0.01f) {
						const auto angle = axesInput.angle();
						// The move in the square [-1, 1]^2.
						const auto length = std::min(1.0f, axesInput.length());
						directionInput = Vec2::fromPolar(angle, length);
					}
					// Override keyboard input
				}
			}
		}
		ImGui::InputFloat2("dir", directionInput.data());
	}

	ImGui::SliderFloat("speed", &playerSpeed, 0.0f, 1.0f);
	ImGui::SliderFloat("shift speed", &shiftPlayerSpeed, 0.0f, 1.0f);
	ImGui::SliderFloat("player size", &playerSize, 0.0f, 1.0f);
	ImGui::SliderFloat("bulletsSize", &bulletSize, 0.0f, 1.0f);
	ImGui::SliderFloat("camera height", &cameraHeight, 0.0f, 1.0f);

	ImGui::End();

	if (Input::isKeyDown(KeyCode::TAB)) {
		if (cameraMode == CameraMode::ON_SURFACE) {
			Window::disableCursor();
			cameraMode = CameraMode::IN_SPACE;
		} else if (cameraMode == CameraMode::IN_SPACE) {
			Window::enableCursor();
			cameraMode = CameraMode::ON_SURFACE;
		}
		ImGui::SetWindowFocus(nullptr);
	}

	if (Input::isKeyDown(KeyCode::X)) {
		Window::close();
	}

	auto view = Mat4::identity;
	Vec3 cameraPosition(0.0f);
	switch (cameraMode) {
		using enum CameraMode;
	case ON_SURFACE: {

		surfaceCamera.angleToTangentPlane = PI<f32> / 2.0f - 0.01f;
		surfaceCamera.height = cameraHeight;
		const auto speed = Input::isKeyHeld(KeyCode::LEFT_SHIFT) ? shiftPlayerSpeed : playerSpeed;
		const auto r = surfaceCamera.update(surface, directionInput, Constants::dt, speed);
		view = r;
		cameraPosition = surfaceCamera.cameraPosition(surface);
		break;
	}

	case IN_SPACE: {
		fpsCamera.update(Constants::dt);
		view = fpsCamera.viewMatrix();
		cameraPosition = fpsCamera.position;
		break;
	}
		
	}
	
	iterateAndRemove(
		bullets,
		[&](Bullet& bullet) {
			surface.integrateParticle(bullet.position, bullet.velocity);
			bullet.lifetime -= Constants::dt;
			const auto remove = bullet.lifetime < 0.0f;
			return remove;
		}
	);

	auto updateLifetime = [&](f32& lifetime) -> bool {
		lifetime -= Constants::dt;
		const auto isDead = lifetime < 0.0f;
		return isDead;
	};

	iterateAndRemove(
		spiralEmitters,
		[&](SpiralEmitter& emitter) {
			emitter.angle += Constants::dt * TAU<f32> * 2.5f;
			emitter.frame++;
			const auto dead = updateLifetime(emitter.lifetime);

			if (emitter.frame % 5 == 0) {
				bullets.push_back(Bullet{
					.position = emitter.position,
					.velocity = surface.tangentVectorFromPolar(emitter.position, emitter.angle, bulletSpeed),
					.lifetime = emitter.lifetime,
				});
			}
			return dead;
		}
	);

	iterateAndRemove(
		directionalEmitters,
		[&](DirectionalEmitter& e) {
			const auto isDead = updateLifetime(e.lifetime);
			if (e.frame % 30 == 0) {
				bullets.push_back(Bullet{
					.position = e.position,
					.velocity = e.direction,
					.lifetime = e.lifetime,
				});
			}
			e.frame++;
			return isDead;
		}
	);

	auto spawnCircle = [&](SurfacePosition position, i32 angleCount) {
		for (auto& angle : UniformPartition(angleCount, 0.0f, TAU<f32>)) {
			bullets.push_back(Bullet{
				.position = position,
				.velocity = surface.tangentVectorFromPolar(position, angle.x, bulletSpeed),
				.lifetime = 5.0f
			});
		}
	};

	if (Input::isKeyDown(KeyCode::L)) {
		for (i32 i = 0; i < 20; i++) {
			auto emitter = [&](SurfacePosition position, f32 angle) {
				directionalEmitters.push_back(DirectionalEmitter{
					.lifetime = 10.0f,
					.direction = surface.tangentVectorFromPolar(position, angle, bulletSpeed),
					.position = position,
				});
			};
			const auto directions = 5;
			const auto position = surface.randomPointOnSurface();
			for (i32 j = 0; j < directions; j++) {
				emitter(position, f32(j) / f32(directions) * TAU<f32>);
			}
		}
	}

	if (Input::isKeyDown(KeyCode::K)) {
		for (i32 i = 0; i < 100; i++) {
			spawnCircle(surface.randomPointOnSurface(), 20);
		}
	}

	if (Input::isKeyDown(KeyCode::J)) {
		for (i32 i = 0; i < 5; i++) {
			auto spiral = [&](SurfacePosition position, f32 angle) {
				spiralEmitters.push_back(SpiralEmitter{
					.lifetime = 5.0f,
					.angle = angle,
					.position = position,
				});
			};
			const auto directions = 2;
			const auto position = surface.randomPointOnSurface();
			for (i32 j = 0; j < directions; j++) {
				spiral(position, f32(j) / f32(directions) * TAU<f32>);
			}
		}
	}

	if (Input::isKeyDown(KeyCode::H)) {
		const auto count = 10;
		for (i32 i = 0; i < count; i++) {
			auto spiral = [&](SurfacePosition position, f32 angle) {
				spiralEmitters.push_back(SpiralEmitter{
					.lifetime = 10.0f,
					.angle = angle,
					.position = position,
				});
			};
			const auto directions = 3;
			const auto position = SurfacePosition(Vec2(
				lerp(surface.uMin(), surface.uMax(), f32(i) / f32(count)), i % 2 == 0 ? 0.0f : PI<f32>
			));
			for (i32 j = 0; j < directions; j++) {
				spiral(position, f32(j) / f32(directions) * TAU<f32>);
			}
		}
	}
	if (Input::isKeyDown(KeyCode::N)) {
		const auto uCount = 8;
		const auto vCount = 3;
		for (i32 ui = 0; ui < uCount; ui++) {
			for (i32 vi = 0; vi < vCount; vi++) {
				auto u = lerp(surface.uMin(), surface.uMax(), f32(ui) / f32(uCount));
				auto v = lerp(surface.vMin(), surface.vMax(), f32(vi) / f32(vCount));

				/*if (ui % 2 == 0) {
					u += TAU<f32> / uCount / 2.0f;
				}*/

				if (ui % 2 == 0) {
					v += TAU<f32> / vCount / 2.0f;
				}
				const auto pos = SurfacePosition::makeUv(Vec2(u, v));
				spawnCircle(pos, 30);
			}
		}
	}

	for (auto& emitter : basicEmitters) {
		emitter.elapsed += Constants::dt;
		const auto shootTime = bulletSpeed;
		if (emitter.elapsed > shootTime) {
			emitter.elapsed -= shootTime;
			spawnCircle(emitter.position, 30);
		}
	}

	render(view, cameraPosition);
}

void Game::render(const Mat4& view, Vec3 cameraPosition) {
	// Could convert to Mat3 and just do a transpose.
	const auto cameraForward = (Vec4(Vec3::FORWARD, 0.0f) * view.inversed()).xyz().normalized();
	const auto aspectRatio = Window::aspectRatio();
	const auto projection = Mat4::perspective(PI<f32> / 2.0f, aspectRatio, 0.1f, 1000.0f);
	const auto swaxpYZ = Mat4(Mat3(Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, 1.0f, 0.0f)));
	renderer.transform = projection * view;
	renderer.view = view;
	renderer.projection = projection;
	renderer.resizeBuffers(Vec2T<i32>(Window::size()));
	glViewport(0, 0, i32(Window::size().x), i32(Window::size().y));

	auto drawQuatPt = [this]() {
		renderer.quadPtVao.bind();
		quad2dPtDraw();
	};


	// opaque
	{
		renderer.opaqueFbo.bind();
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_BLEND);

		renderer.sphere(surface.position(surfaceCamera.position), playerSize, Color3::WHITE);
		renderer.renderHemispheres();
		Fbo::unbind();
	}

	// transparent
	{
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunci(renderer.accumulateTextureColorBufferIndex, GL_ONE, GL_ONE);
		glBlendFunci(renderer.revealTextureColorBufferIndex, GL_ZERO, GL_ONE_MINUS_SRC_COLOR); 
		glBlendEquation(GL_FUNC_ADD);

		renderer.transparentFbo.bind();
		f32 zeros[]{ 0.0f, 0.0f, 0.0f, 0.0f };
		glClearBufferfv(GL_COLOR, 0, zeros); 
		f32 ones[]{ 1.0f, 1.0f, 1.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 1, ones);

		const auto meshOpacity = 0.7f;
		const auto indicesOffset = i32(renderer.surfaceTriangles.currentIndex());
		for (i32 i = 0; i < surface.vertexCount(); i++) {
			const auto position = surface.positions[i];

			const auto uvt = surface.uvts[i];

			renderer.surfaceTriangles.addVertex(Vertex3Pnt{
				.position = position,
				.normal = surface.normals[i],
				.uv = surface.uvts[i]
			});
		}
		for (i32 i = 0; i < surface.triangleCount(); i++) {
			const auto index = indicesOffset + surface.sortedTriangles[i] * 3;
			const auto i0 = surface.indices[index];
			const auto i1 = surface.indices[index + 1];
			const auto i2 = surface.indices[index + 2];
			renderer.surfaceTriangles.addTri(i0, i1, i2);
		}
		renderer.renderSurfaceTriangles(meshOpacity);
		Fbo::unbind();
	}
	// composite
	{
		renderer.opaqueFbo.bind();
		glDepthFunc(GL_ALWAYS);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		renderer.transparencyCompositingShader.use();

		glActiveTexture(GL_TEXTURE0);
		renderer.accumulateTexture.bind(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE1);
		renderer.revealTexture.bind(GL_TEXTURE_2D);
		drawQuatPt();
	}

	// draw quad
	{
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE); // enable depth writes so glClear won't ignore clearing the depth buffer
		glDisable(GL_BLEND);

		Fbo::unbind();
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		renderer.fullscreenTexturedQuadShader.use();
		renderer.fullscreenTexturedQuadShader.setTexture("textureSampler", 0, renderer.opaqueColorTexture);
		drawQuatPt();
	}

	//for (const auto& bullet : bullets) {
	//	const auto position = surface.position(bullet.position) + surface.normal(bullet.position) * 0.03f;
	//	//renderer.sphere(surface.position(bullet.position), bulletSize / 2.0f, Color3::RED);
	//	renderer.bullet(bulletSize, position, Vec4(Color3::RED));
	//}

	//renderer.renderHemispheres();
	////renderer.renderCubemap();

	//const auto meshOpacity = 1.0f;
	////const auto meshOpacity = 0.7f;
	//const auto isVisible = meshOpacity > 0.0f;
	//const auto isTransparent = meshOpacity < 1.0f;
	//if (isTransparent) {
	//	surface.sortTriangles(cameraPosition);
	//}

	//const auto indicesOffset = i32(renderer.surfaceTriangles.currentIndex());
	//for (i32 i = 0; i < surface.vertexCount(); i++) {
	//	const auto position = surface.positions[i];

	//	const auto uvt = surface.uvts[i];

	//	renderer.surfaceTriangles.addVertex(Vertex3Pnt{
	//		.position = position,
	//		.normal = surface.normals[i],
	//		.uv = surface.uvts[i]
	//		});
	//}
	//for (i32 i = 0; i < surface.triangleCount(); i++) {
	//	const auto index = indicesOffset + surface.sortedTriangles[i] * 3;
	//	const auto i0 = surface.indices[index];
	//	const auto i1 = surface.indices[index + 1];
	//	const auto i2 = surface.indices[index + 2];
	//	renderer.surfaceTriangles.addTri(i0, i1, i2);
	//}

	//if (isTransparent) {
	//	glEnable(GL_BLEND);
	//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//	glDepthMask(GL_FALSE);
	//}

	//renderer.renderSurfaceTriangles(meshOpacity);

	//if (isTransparent) {
	//	glDisable(GL_BLEND);
	//	glDepthMask(GL_TRUE);
	//}

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	////glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	//glDepthMask(GL_FALSE);
	//renderer.renderBullets(view.inversed());
	//glDisable(GL_BLEND);
	//glDepthMask(GL_TRUE);
}
