
#pragma once

#include "src\utils\math.hpp"
#include "src\render\dx\def.hpp"
#include "src\render\texture.hpp"

class Camera {
	class DirectCamera* camera = nullptr;
public:
	Camera(DirectCamera* camera) noexcept : camera(camera) {}
	~Camera() = default;

	[[nodiscard]] Vector3D getDirection() const noexcept;
	[[nodiscard]] Vector3D getPosition() const noexcept;
	void setDirection(const Vector3D& d) const noexcept;
	void setDirection(double pitch, double yaw) const noexcept;
	void setDirection(double x, double y, double z) const noexcept;
	void setPosition(const Vector3D& p) const noexcept;
	void setPosition(double x, double y, double z) const noexcept;
	void move(const Vector3D& v) const noexcept;
	[[nodiscard]] double getCenterDistance() const noexcept;
	[[nodiscard]] double getFoV() const noexcept;
	[[nodiscard]] double getFarthestDistance() const noexcept;
	[[nodiscard]] double getNearestDistance() const noexcept;
	void setCenterDistance(double distance) const noexcept;
	void setFoV(double fov) const noexcept;
	void setFarthestDistance(double fd) const noexcept;
	void setNearestDistance(double nd) const noexcept;
	void calculate(double width, double height) const noexcept;
	void rotate(double pitch, double yaw) const noexcept;
	void usePerspectiveProjection() const noexcept;
	void useOrthographicProjection() const noexcept;
	void useProjectionSwitch() const noexcept;
};

class Renderer {
	DirectRenderer* renderer = nullptr;

public:
	Renderer(DirectRenderer* renderer) noexcept : renderer(renderer) {}
	~Renderer() = default;

	void debugDefault() const noexcept;
	void debugCustom() const noexcept;
	void assertThread() const noexcept(false);
	void setViewport(int width, int height) const noexcept;

	void drawTexture(const DirectTextureContext& context) const;
	void drawColor(ColoredSet&&) const noexcept(false);
	void drawColor(const ColoredSet&) const noexcept(false);
};
