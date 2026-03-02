
#include "src\render\render.hpp"
#include "src\render\dx\direct.hpp"

Vector3D Camera::getDirection() const noexcept { return camera->getDirection(); }
Vector3D Camera::getPosition() const noexcept { return camera->getPosition(); }
void Camera::setDirection(const Vector3D& d) const noexcept { camera->setDirection(d); }
void Camera::setDirection(const double pitch, const double yaw) const noexcept { camera->setDirection(pitch, yaw); }
void Camera::setDirection(const double x, const double y, const double z) const noexcept { camera->setDirection(x, y, z); }
void Camera::setPosition(const Vector3D& p) const noexcept { camera->setPosition(p); }
void Camera::setPosition(const double x, const double y, const double z) const noexcept { camera->setPosition(x, y, z); }
void Camera::move(const Vector3D& v) const noexcept { camera->move(v); }
double Camera::getCenterDistance() const noexcept { return camera->getCenterDistance(); }
double Camera::getFoV() const noexcept { return camera->getFoV(); }
double Camera::getFarthestDistance() const noexcept { return camera->getFarthestDistance(); }
double Camera::getNearestDistance() const noexcept { return camera->getNearestDistance(); }
void Camera::setCenterDistance(const double distance) const noexcept { camera->setCenterDistance(distance); }
void Camera::setFoV(const double fov) const noexcept { camera->setFoV(fov); }
void Camera::setFarthestDistance(const double fd) const noexcept { camera->setFarthestDistance(fd); }
void Camera::setNearestDistance(const double nd) const noexcept { camera->setNearestDistance(nd); }
void Camera::calculate(const double width, const double height) const noexcept { camera->calculate(width, height); }
void Camera::rotate(const double pitch, const double yaw) const noexcept { camera->rotate(pitch, yaw); }
void Camera::usePerspectiveProjection() const noexcept { camera->usePerspectiveProjection(); }
void Camera::useOrthographicProjection() const noexcept { camera->useOrthographicProjection(); }
void Camera::useProjectionSwitch() const noexcept { camera->useProjectionSwitch(); }

void Renderer::debugDefault() const noexcept { renderer->debugDefault(); }
void Renderer::debugCustom() const noexcept { renderer->debugCustom(); }
void Renderer::assertThread() const noexcept(false) { renderer->assertThread(); }
void Renderer::setViewport(const int width, const int height) const noexcept { return renderer->setViewport(width, height); }
void Renderer::drawTexture(const DirectTextureContext& context) const { renderer->drawTexture(context); }
void Renderer::drawColor(ColoredSet&& set) const noexcept(false) { renderer->drawColor(set); }
void Renderer::drawColor(const ColoredSet& set) const noexcept(false) { renderer->drawColor(set); }
