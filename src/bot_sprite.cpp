#include "bot_sprite.h"

void BotSprite::take_action(PolicyConfigs::Action a)
{
	if (a == PolicyConfigs::Action::UP)
		position.y -= 1;
	if (a == PolicyConfigs::Action::DOWN)
		position.y += 1;
	if (a == PolicyConfigs::Action::LEFT)
		position.x -= 1;
	if (a == PolicyConfigs::Action::RIGHT)
		position.x += 1;
}