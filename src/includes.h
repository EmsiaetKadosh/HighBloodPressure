//
// Created by EmsiaetKadosh on 25-1-14.
//

#pragma once

#include "warnings.h"

// Any include
#include "def.h"
#include "utils\math.h" // required by Location.h
#include "utils\File.h"
#include "utils\exception.h"
#include "utils\Chars.h"

// utils serialized
#include "utils\gc.h" // required by utils.h
#include "utils\utils.h" // required by Task.h
#include "utils\Task.h"
#include "hbp.h"
#include "game\Animation.h"

// game basic
#include "interact\InteractManager.h"
#include "utils\IText.h"
#include "game\world\Location.h" // required by Renderer.h
#include "render\Renderer.h"
#include "render\TextureManager.h"
#include "ui\Hud.h"
#include "ui\Window.h"

// game
#include "game\Game.h"

// game extension
#include "ui\xWindows.h"
#include "game\gameDef.h"
#include "game\entity\Entity.h"
#include "game\world\Block.h"
#include "game\world\World.h"
#include "game\entity\Player.h"
