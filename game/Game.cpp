#include <game/Game.hpp>
#include <engine/Input/Input.hpp>
#include <gfx2d/Quad2dPt.hpp>
#include <engine/Math/Interpolation.hpp>
#include <game/UniformPartition.hpp>
#include <game/PeriodicUniformPartition.hpp>
#include <imgui/imgui.h>
#include <engine/Math/Mat4.hpp>
#include <glad/glad.h>
#include <game/Constants.hpp>
#include <engine/Window.hpp>
#include <engine/Math/Constants.hpp>

Game::Game()
	: renderer(GameRenderer::make())
	, surfacePositions(Array2d<Vec3>::uninitialized(25, 25)) {

	surface.initialize(Surface::Type::TORUS);

	// I don't think it makes sense to use dijktra's algorithm to aim bullets, because it will only have 8 possible directions for the initial velocity so it probably will just miss especially in areas with a lot of variation in the geodesic paths.
	//for (const auto& u : PeriodicUniformPartition(0.0f, TAU<f32>, surfacePositions.sizeX())) {
	//	for (const auto& v : PeriodicUniformPartition(0.0f, TAU<f32>, surfacePositions.sizeY())) {
	//		surface.position(SurfacePosition::makeUv(u, v));
	//	}
	//}

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

const auto enemySize = 0.06f;

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

	ImGui::SliderFloat("mesh opacity", &meshOpacity, 0.0f, 1.0f);

	ImGui::SliderFloat("speed", &playerSpeed, 0.0f, 1.0f);
	ImGui::SliderFloat("shift speed", &shiftPlayerSpeed, 0.0f, 1.0f);
	ImGui::SliderFloat("player size", &playerSize, 0.0f, 1.0f);
	ImGui::SliderFloat("bulletsSize", &bulletSize, 0.0f, 1.0f);
	ImGui::SliderFloat("camera height", &cameraHeight, 0.0f, 1.0f);
	ImGui::Checkbox("wireframe", &wireframe);

	if (ImGui::Button("randomly spanwed shooting enemies")) {
		randomEnemiesShootingWave.initialize();
		currentWave = &randomEnemiesShootingWave;
	}
	if (ImGui::Button("spiral")) {
		spiralWave.initialize(*this);
		currentWave = &spiralWave;
	}
	if (ImGui::Button("circles")) {
		circleSpawningWave.initialize(*this);
		currentWave = &circleSpawningWave;
	}
	if (ImGui::Button("spawn around player")) {
		spawnAroundPlayerWave.initialize(*this);
		currentWave = &spawnAroundPlayerWave;
	}

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

	{
		const auto cursorEnabled = Window::isCursorEnabled();

		const auto flags =
			ImGuiConfigFlags_NavNoCaptureKeyboard |
			ImGuiConfigFlags_NoMouse |
			ImGuiConfigFlags_NoMouseCursorChange;

		if (cursorEnabled) {
			ImGui::GetIO().ConfigFlags &= ~flags;
		} else {
			ImGui::GetIO().ConfigFlags |= flags;
		}
	}


	if (Input::isKeyDown(KeyCode::X)) {
		Window::close();
	}

	const auto focused = Input::isKeyHeld(KeyCode::LEFT_SHIFT);

	auto view = Mat4::identity;
	Vec3 cameraPosition(0.0f);
	auto playerVelocity = SurfaceTangent::zero();
	switch (cameraMode) {
		using enum CameraMode;
	case ON_SURFACE: {

		surfaceCamera.angleToTangentPlane = PI<f32> / 2.0f - 0.01f;
		surfaceCamera.height = cameraHeight;
		const auto speed = focused ? shiftPlayerSpeed : playerSpeed;
		const auto r = surfaceCamera.update(surface, directionInput, Constants::dt, speed);
		playerVelocity = surface.tangentVectorFromPolar(surfaceCamera.position, directionInput.angle() - PI<f32> / 2.0f, speed * directionInput.length());
		view = r;
		cameraPosition = surfaceCamera.cameraPosition(surface);
		playerPosition3 = surface.position(surfaceCamera.position);
		break;
	}

	case IN_SPACE: {
		fpsCamera.update(Constants::dt);
		view = fpsCamera.viewMatrix();
		cameraPosition = fpsCamera.position;
		break;
	}
		
	}

	if (currentWave.has_value()) {
		const auto end = (*currentWave)->update(*this);
		if (end) {
			currentWave = std::nullopt;
		}
	}

	auto updateLifetime = [](f32& lifetime) {
		lifetime -= Constants::dt;
		return lifetime < 0.0f;
	};

	auto collision = [this](const SurfacePosition& a, f32 aRadius, const SurfacePosition& b, f32 bRadius) {
		return (surface.position(a).distanceTo(surface.position(b)) < aRadius + bRadius);
	};

	for (auto bullet : bullets) {
		surface.integrateParticle(bullet->position, bullet->velocity);
		if (collision(bullet->position, bulletSize / 2.0f, surfaceCamera.position, playerSize)) {
			ImGui::Text("hit");
		}
		if (updateLifetime(bullet->lifetime)) {
			bullets.destroy(bullet.id);
		}
	}
	for (auto e : bulletCollections) {
		surface.integrateParticle(e->position, e->velocity);

		for (const auto& angle : PeriodicUniformPartition(0.0f, TAU<f32>, e->bullets.size())) {
			auto bullet = bullets.get(e->bullets[angle.i]);
			if (!bullet.has_value()) {
				continue;
			}

			const auto bulletPos = surface.moveForward(e->position, surface.tangentVectorFromPolar(e->position, angle, 1.0f), 0.05f);
			bullet->position = bulletPos;
			//bullet->position = e->position;
		}
	}


	//iterateAndRemove(
	//	bullets,
	//	[&](Bullet& bullet) {
	//		surface.integrateParticle(bullet.position, bullet.velocity);
	//		bullet.lifetime -= Constants::dt;
	//		const auto remove = bullet.lifetime < 0.0f;
	//		if (surface.position(bullet.position).distanceTo(surface.position(surfaceCamera.position)) <= playerSize + bulletSize) {
	//			ImGui::Text("hit");
	//		}
	//		return remove;
	//	}
	//);

	iterateAndRemove(
		playerBullets,
		[&](PlayerBullet& bullet) {
			surface.integrateParticle(bullet.position, bullet.velocity);
			bullet.lifetime -= Constants::dt;
			auto remove = bullet.lifetime < 0.0f;
			return remove;
		}
	);

	for (auto enemy : enemies) {
		for (const auto& bullet : playerBullets) {
			if (collision(enemy->position, enemySize, bullet.position, playerBulletSize)) {
				enemy->hp -= Constants::dt;
			}
		}

		if (enemy->hp < 0.0f) {
			enemies.destroy(enemy.id);
		}

		if (frame % 60 * 3 == 0) {
			const auto offset = randomAngle();
			for (const auto& a : PeriodicUniformPartition(offset, offset + TAU<f32>, 15)) {

				//auto b = bullets.create();
				//
				//b->position = enemy->position;
				//b->velocity = surface.tangentVectorFromPolar(b->position, a, 0.25f);
				//b->lifetime = 3.0f; 
			}

 		//	const auto playerPos = surface.position(surfaceCamera.position);
			//const auto enemyPos = surface.position(enemy->position);
			//const auto dir = vectorInTangentSpaceBasis(playerPos - enemyPos, surface.tangentU(enemy->position), surface.tangentV(enemy->position));
			//auto b = bullets.create();
			//b->position = enemy->position;
			//b->velocity = surface.scaleTangent(surface.tangentVectorNormalize(enemy->position, SurfaceTangent(dir)), 0.5f);
			//b->lifetime = 1.0f;
		}
	}

	frame++;

	if (Input::isMouseButtonHeld(MouseButton::LEFT) && frame % 3 == 0) {
		Camera camera;
		camera.aspectRatio = Window::aspectRatio();
		const auto cursorPos = Input::cursorPosClipSpace() * camera.clipSpaceToWorldSpace();
		const auto& position = surfaceCamera.position;

		const auto tangentU = surface.tangentU(surfaceCamera.position);
		const auto tangentV = surface.tangentV(surfaceCamera.position);
		const auto normal = cross(tangentU, tangentV).normalized();
		const auto v0 = tangentU.normalized();
		const auto v1 = cross(tangentU, normal).normalized();
		auto toOrthonormalBasis = [&](Vec3 v) -> Vec2 {
			return Vec2(dot(v, v0), dot(v, v1));
		};
		const auto uvDir = Vec2::oriented(surfaceCamera.uvForwardAngle);
		const auto t = toOrthonormalBasis(tangentU * uvDir.x + tangentV * uvDir.y);

		// TODO: Make function orthonormal basis at -> std::array<Vec3>
		// uvTangentVectorToPolar. 

		auto velocityAtAngle = [&](f32 angle) {
			return SurfaceTangent::makeUv(
				surface.tangentVectorFromPolar(position, angle + cursorPos.angle() - PI<f32> / 2.0f + t.angle(), 1.0f).uv
				//+playerVelocity.uv
			);
		};
		const auto halfSpreadAngle = focused ? 0.05f : 0.2f;
		const auto lifetime = 1.0f;
		auto bullet = [&](f32 angle) {
			playerBullets.push_back(PlayerBullet{
				.position = position,
				.velocity = velocityAtAngle(angle),
				.lifetime = lifetime,
			});
		};
		bullet(0.0f);
		bullet(halfSpreadAngle);
		bullet(-halfSpreadAngle);

		cursorPos.angle();
	}

	//auto spawnCircle = [&](SurfacePosition position, i32 angleCount, f32 angleOffset) {
	//	// TODO: I think this spawns to bullets in one point.
	//	for (auto& angle : UniformPartition(0.0f, TAU<f32>, angleCount)) {
	//		bullets.push_back(Bullet{
	//			.position = position,
	//			.velocity = surface.tangentVectorFromPolar(position, angle.x + angleOffset, bulletSpeed),
	//			.lifetime = 5.0f
	//		});
	//	}
	//};

	//std::vector<i32> playerBulletsToRemove;

	enemies.update();
	bullets.update();
	render(view, cameraPosition);
}

void Game::renderOpaque() {
	renderer.sphere(surface.position(surfaceCamera.position), playerSize, Color3::WHITE);
	renderer.renderHemispheres();

	renderer.cube(Color3::BLUE);
	renderer.renderCubes();
}

void Game::renderTransparent(const Mat4& view) {
	{
		//const auto meshOpacity = 0.7f;
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
	}

}

void Game::render(const Mat4& view, Vec3 cameraPosition) {
	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
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

	for (const auto& bullet : bullets) {
		const auto position = surface.position(bullet->position) + surface.normal(bullet->position) * 0.03f;
		renderer.sphere(surface.position(bullet->position), bulletSize, bullet->color);
	}

	//for (const auto& bullet : immediateBullets) {
	//	const auto position = surface.position(bullet.position) + surface.normal(bullet.position) * 0.03f;
	//	renderer.sphere(surface.position(bullet.position), bulletSize, Color3::RED);
	//	//renderer.bullet(bulletSize, position, Vec4(Color3::RED));
	//}

	for (const auto& bullet : playerBullets) {
		const auto position = surface.position(bullet.position) + surface.normal(bullet.position) * 0.03f;
		renderer.sphere(surface.position(bullet.position), playerSize / 2.0f, Color3::GREEN);
	}

	for (const auto& enemy : enemies) {
		renderer.sphere(surface.position(enemy->position), bulletSize * 2.0f, Color3::MAGENTA);
	}

	bool weightedBlendedOit = false;

	if (weightedBlendedOit) {
		// opaque
		{
			renderer.opaqueFbo.bind();
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			glDepthMask(GL_TRUE);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glDisable(GL_BLEND);

			renderOpaque();

			// Can't just put alpha blended transaprent objects, here, because if a depth value is written then the surface won't be drawn behind it and if it's not output then the the surface will be blended on top of the alpha blended surface.

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

			renderTransparent(view);

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

			glDepthMask(GL_FALSE);
			glEnable(GL_BLEND);
			//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			//for (const auto& bullet : bullets) {
			//	const auto position = surface.position(bullet.position) + surface.normal(bullet.position) * 0.03f;
			//	//renderer.sphere(surface.position(bullet.position), bulletSize / 2.0f, Color3::RED);
			//	renderer.bullet(bulletSize, position, Vec4(Color3::RED));
			//}
			renderer.renderBullets(view.inversed());
			glDisable(GL_BLEND);
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
	} else {
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		////glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		//glDepthMask(GL_FALSE);
		//renderer.renderBullets(view.inversed());
		//glDisable(GL_BLEND);
		//glDepthMask(GL_TRUE);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_BLEND);
		renderOpaque();

		const auto isVisible = meshOpacity > 0.0f;
		const auto isTransparent = meshOpacity < 1.0f;
		if (isTransparent) {
			surface.sortTriangles(cameraPosition);
		}

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

		if (isTransparent) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDepthMask(GL_FALSE);
		}

		renderTransparent(view);

		if (isTransparent) {
			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);
		}

		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		renderer.renderBullets(view.inversed());
 	}

	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

EntityArrayPair<Bullet> Game::spawnBullet(SurfacePosition position, SurfaceTangent velocity, f32 lifetime, Vec3 color) {
	auto b = bullets.create();
	b->position = position;
	b->velocity = velocity;
	b->lifetime = lifetime;
	b->color = color;
	return b;
}

void Game::bulletCircle(SurfacePosition position, f32 initialAngle, i32 angleCount, f32 velocity, f32 lifetime, Vec3 color) {
	for (const auto& angle : PeriodicUniformPartition(initialAngle, initialAngle + TAU<f32>, angleCount)) {
		spawnBullet(position, surface.tangentVectorFromPolar(position, angle, velocity), lifetime, color);
	}
}

EntityArrayPair<Enemy> Game::spawnEnemy(SurfacePosition position, f32 hp) {
	auto e = enemies.create();
	e->position = position;
	e->hp = hp;
	return e;
}

SurfaceTangent Game::tryAimAtPlayer(SurfacePosition sourcePosition, f32 velocity) {
	const auto playerPos = surface.position(surfaceCamera.position);
	const auto sourcePos = surface.position(sourcePosition);
	const auto dir = vectorInTangentSpaceBasis(playerPos - sourcePos, surface.tangentU(sourcePosition), surface.tangentV(sourcePosition));
	return surface.scaleTangent(surface.tangentVectorNormalize(sourcePosition, SurfaceTangent(dir)), velocity);
}

f32 Game::tryAimAtPlayerAngle(SurfacePosition sourcePosition) {
	const auto tangentU = surface.tangentU(sourcePosition);
	const auto tangentV = surface.tangentV(sourcePosition);
	const auto normal = cross(tangentU, tangentV).normalized();
	const auto v0 = tangentU.normalized();
	const auto v1 = cross(tangentU, normal).normalized();
	auto toOrthonormalBasis = [&](Vec3 v) -> Vec2 {
		return Vec2(dot(v, v0), dot(v, v1));
	};
	const auto playerPos = surface.position(surfaceCamera.position);
	const auto sourcePos = surface.position(sourcePosition);
	const auto t = toOrthonormalBasis(playerPos - sourcePos);
	return t.angle();
}

EntityArrayPair<BulletCollection> Game::bulletCollection(SurfacePosition position, SurfaceTangent velocity, f32 lifetime, Vec3 color) {
	auto e = bulletCollections.create();
	e->bullets.clear();
	e->lifetime = lifetime;
	e->position = position;
	e->velocity = velocity;
	for (i32 i = 0; i < 10; i++) {
		e->bullets.push_back(spawnBullet(position, velocity, lifetime, color).id);
	}
	return e;
}

f32 Game::random01() {
	return surface.uniform01(surface.rng);
}

f32 Game::randomMinus1To1() {
	return (random01() - 0.5f) * 2.0f;
}

f32 Game::randomAngle() {
	return random01() * TAU<f32>;
}

void RandomEnemiesShootingWave::initialize() {
	frame = 0;
}

bool RandomEnemiesShootingWave::update(Game& c) {
	frame++;

	iterateAndRemove(spawnedEnemies, [&](const EnemyId& id) {
		return !c.enemies.isAlive(id);
	});

	if (frame % 40 == 0 && spawnedEnemies.size() < 5) {
		auto enemy = c.enemies.create();
		enemy->position = c.surface.randomPointOnSurface();
		enemy->hp = 2.0f;
		spawnedEnemies.push_back(enemy.id);
	}
	for (const auto& id : spawnedEnemies) {
		const auto e = c.enemies.get(id);
		if (!e.has_value()) {
			continue;
		}

		if (frame % 50 == 0) {
			const auto initialAngle = c.randomAngle();
			c.bulletCircle(
				e->position,
				initialAngle,
				10,
				0.25f,
				5.0f
			);
		}
		//for (const auto& a : PeriodicUniformPartition(offset, offset + TAU<f32>, 15)) {

		//	//auto b = bullets.create();
		//	//
		//	//b->position = enemy->position;
		//	//b->velocity = surface.tangentVectorFromPolar(b->position, a, 0.25f);
		//	//b->lifetime = 3.0f; 
		//}
	}

	return false;
}

void SpiralEnemyWave::initialize(Game& c) {
	angle = 0.0f;
	const auto pointAitpodalToPlayer = SurfacePosition::makeUv(c.surfaceCamera.position.uv.x + PI<f32>, 0.0f);
	enemy = c.spawnEnemy(pointAitpodalToPlayer, 80.0f).id;
}

bool SpiralEnemyWave::update(Game& c) {

	const auto e = c.enemies.get(enemy);
	if (!e.has_value()) {
		return true;
	}

	const auto lifetime = 4.0f;

	auto shotDirectional = [&]() {
		if (frame % 35 == 0 && (shotType == ShotType::DIPOLE || shotType == ShotType::SPIRAL)) {
			auto direction = c.tryAimAtPlayerAngle(e->position);
			direction += c.randomMinus1To1() * 0.4f;
			const auto angleSpread = 0.1f;
			for (const auto& angle : UniformPartition(direction - angleSpread / 2.0f, direction + angleSpread / 2.0f, 3)) {
				c.spawnBullet(
					e->position,
					//c.surface.tangentVectorFromPolar(position, angle, 0.5f),
					c.surface.tangentVectorFromPolar(e->position, angle, 0.4f),
					lifetime,
					Color3::BLUE
				);
			}

			/*const auto angle = c.randomAngle();
			for (const auto& angle : UniformPartition(angle, angle + TAU<f32> / 8.0f, 6)) {
				c.spawnBullet(
					position,
					c.surface.tangentVectorFromPolar(position, angle, 0.25f),
					lifetime,
					Color3::BLUE
				);
			}*/
		}
	};

	switch (shotType) {
		using enum ShotType;
	case SPIRAL: {
		angle += Constants::dt * TAU<f32> / 3.0f;
		if (frame % 8 == 0) {
			angle += 0.1f;
			c.bulletCircle(
				e->position,
				angle,
				3,
				0.25f,
				lifetime,
				Color3::RED
			);
		}
		shotDirectional();
		break;
	}
		
	case DIPOLE: {
		angle += Constants::dt * TAU<f32> / 3.0f;
		if (frame % 8 == 0) {
			angle += 0.02f;
			c.bulletCircle(
				e->position,
				angle,
				2,
				0.25f,
				lifetime,
				Color3::RED
			);
			c.bulletCircle(
				e->position,
				-angle,
				2,
				0.25f,
				lifetime,
				Color3::YELLOW
			);
		}
		shotDirectional();
		break;
	}

	case WALLS: {
		// macrododging
		angle += Constants::dt * 10.0f;
		if (frame % 35 == 0) {
			for (const auto& centerDirection : PeriodicUniformPartition(0.0f, TAU<f32>, 5)) {
				const auto halfSpread = 0.2f;
				for (const auto& direction : UniformPartition(centerDirection - halfSpread, centerDirection + halfSpread, 5)) {
					c.spawnBullet(
						e->position,
						c.surface.tangentVectorFromPolar(e->position, direction + angle, 0.2f),
						lifetime,
						Color3::RED
					);
				}
			}
		}
		break;
	}

	case CIRCLE_COLLECTIONS: {
		angle += Constants::dt * TAU<f32> / 6.0f;
		if (frame % 120 == 0) {
			c.bulletCircle(
				e->position,
				//sin(angle / 5.0f)* PI<f32>,
				0.0f,
				17,
				0.07f,
				lifetime * 3,
				Color3::YELLOW
			);
		}
		if (frame % 35 == 0) {
			for (const auto& direction : PeriodicUniformPartition(0.0f, TAU<f32>, 3)) {
				c.bulletCollection(
					e->position,
					c.surface.tangentVectorFromPolar(e->position, angle + direction, 0.2f),
					lifetime,
					Color3::RED
				);
			}

			/*for (const auto& centerDirection : PeriodicUniformPartition(0.0f, TAU<f32>, 5)) {
				const auto halfSpread = 0.2f;
				for (const auto& direction : UniformPartition(centerDirection - halfSpread, centerDirection + halfSpread, 5)) {
					
				}
			}*/
		}
		break;
	}

	}

	frame++;

	return false;
}

void CircleSpawning::initialize(Game& c) {
	u = 0.0f;
	v = 0.0f;
	frame = 0;
	enemyCount = 6;

	for (i32 i = 0; i < enemyCount; i++) {
		const auto id = c.spawnEnemy(SurfacePosition::makeUv(0.0f, 0.0f), 10.0f).id;
		enemies.push_back(id);
	}
}

bool CircleSpawning::update(Game& c) {
	u += Constants::dt * 0.5f;
	v += Constants::dt;
	frame++;

	for (const auto& ua : PeriodicUniformPartition(0.0f, TAU<f32>, enemyCount)) {
		auto enemy = c.enemies.get(enemies[ua.i]);
		if (!enemy.has_value()) {
			continue;
		}

		const auto position = SurfacePosition::makeUv(u + ua, ua.i % 2 == 0 ? 0.0f : PI<f32>);
		enemy->position = position;
		
		const auto angleOffset = c.random01();
		if (frame % 30 == 0) {
			c.bulletCircle(
				position,
				//SurfacePosition::makeUv(u + ua + offset, 0.0f),
				//SurfacePosition::makeUv(u + ua + offset, v + ua.i % 2 == 0 ? 0.0f : PI<f32>),
				angleOffset,
				10,
				0.2f,
				4.0f
			);
		}
	}

	return false;
}

void SpawningAroundThePlayer::initialize(Game& c) {
}

bool SpawningAroundThePlayer::update(Game& c) {
	frame++;
	const auto count = 20;
	const auto offset = 5;
	//if (frame % (count * offset) == angle.i * offset) 
	static i32 directionFrame = 0;
	if (frame % offset == 0) {
		directionFrame++;
		const auto directionIndex = directionFrame % count;
		const auto angle = f32(directionIndex) / f32(count) * TAU<f32>;
		const auto result = c.surface.moveForwardAndReturnDirection(
			c.surfaceCamera.position,
			c.surface.tangentVectorFromPolar(c.surfaceCamera.position, angle, 1.0f),
			0.5f
		);
		c.spawnBullet(
			result.position,
			c.surface.scaleTangent(result.finalDirection, -0.2f),
			4.0f,
			Color3::RED
		);
	}
	//for (i32 i = 0; i < )
	c.surfaceCamera.position;
	return false;
}
