
#include "texture.hpp"
#include "src\render\dx\texture.hpp"
#include "src\render\dx\direct.hpp"

#include "src\main.hpp"

void TextureManager::initialize() noexcept {
	game.getRenderer();
}
void TextureManager::finalize() noexcept {}
