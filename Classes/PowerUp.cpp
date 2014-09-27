#include "PowerUp.h"
#include "GameWorld.h"

bool PowerUp::init(GameWorld* instance)
{
	if(!CCDrawNode::init())
		return false;
	
	game_world_ = instance;
	time_left_ = MAX_POWERUP_WAIT_ON_SCREEN / 2 + CCRANDOM_0_1() * MAX_POWERUP_WAIT_ON_SCREEN / 2;
	speed_ = CCPoint(CCRANDOM_MINUS1_1() * 2, CCRANDOM_MINUS1_1() * 2);

	drawDot(CCPointZero, POWERUP_ICON_OUTER_RADIUS, ccc4f(0.73725f, 0.5451f, 0, 1));
	drawDot(CCPointZero, POWERUP_ICON_OUTER_RADIUS - 3, ccc4f(0, 0, 0, 1));
	setScale(0.0f);

	return true;
}

void PowerUp::Update()
{
	if(!RECT_CONTAINS_CIRCLE(game_world_->boundary_rect_, m_obPosition, POWERUP_ICON_OUTER_RADIUS))
	{
		// left-right
		if( (m_obPosition.x - POWERUP_ICON_OUTER_RADIUS) < game_world_->boundary_rect_.origin.x ||
			(m_obPosition.x + POWERUP_ICON_OUTER_RADIUS) > (game_world_->boundary_rect_.origin.x + game_world_->boundary_rect_.size.width) )
			speed_.x *= -1;
		// top-bottom
		if( (m_obPosition.y + POWERUP_ICON_OUTER_RADIUS) > (game_world_->boundary_rect_.origin.y + game_world_->boundary_rect_.size.height) ||
			(m_obPosition.y - POWERUP_ICON_OUTER_RADIUS) < game_world_->boundary_rect_.origin.y )
			speed_.y *= -1;
	}

	setPosition(m_obPosition.x + speed_.x, m_obPosition.y + speed_.y);
}

void PowerUp::Tick()
{
	-- time_left_;

	if(time_left_ < 0)
	{
		must_be_removed_ = true;
		runAction(CCSequence::createWithTwoActions(CCEaseBackIn::create(CCScaleTo::create(0.25f, 0.0f)), CCRemoveSelf::create(true)));
	}
}

void PowerUp::Spawn()
{
	runAction(CCEaseElasticOut::create(CCScaleTo::create(1.0f, 1.0f)));
	runAction(CCRepeatForever::create(CCSequence::create(CCDelayTime::create(5 + CCRANDOM_0_1() * 5), CCEaseSineIn::create(CCScaleTo::create(0.1f, 1.2f)), CCEaseSineOut::create(CCScaleTo::create(0.1f, 1.0f)), CCEaseSineIn::create(CCScaleTo::create(0.1f, 1.2f)), CCEaseSineOut::create(CCScaleTo::create(0.1f, 1.0f)), NULL)));
}

void PowerUp::Activate()
{
	is_active_ = true;
	clear();
	stopAllActions();
}

void PowerUp::Deactivate()
{
	runAction(CCSequence::createWithTwoActions(CCDelayTime::create(0.01f), CCRemoveSelf::create(true)));
	must_be_removed_ = true;
}
