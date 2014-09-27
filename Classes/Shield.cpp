#include "Shield.h"
#include "GameWorld.h"
#include "Player.h"

Shield* Shield::create(GameWorld* instance)
{
	Shield* shield = new Shield();
	if(shield && shield->init(instance))
	{
		shield->autorelease();
		return shield;
	}
	CC_SAFE_DELETE(shield);
	return NULL;
}

bool Shield::init(GameWorld* instance)
{
	if(!PowerUp::init(instance))
		return false;
	
	vector<CCPoint> vertices_vector;
	//CCPoint vertices_array[6];

	GameGlobals::GetRegularPolygonVertices(vertices_vector, 6, POWERUP_ICON_INNER_RADIUS, M_PI/6);
	//GameGlobals::ConvertPointArray(vertices_vector, vertices_array);
	//drawPolygon(vertices_array, 6, ccc4f(0, 0, 0, 0), 3, ccc4f(0, 0.96862f, 1, 1));
	drawPolygon(&vertices_vector[0], 6, ccc4f(0, 0, 0, 0), 3, ccc4f(0, 0.96862f, 1, 1));

	return true;
}

void Shield::Update()
{
	if(!is_active_)
	{
		PowerUp::Update();
	}
	else
	{
		setPosition(game_world_->player_->getPosition());
		setRotation(game_world_->player_->getRotation());
	}
}

void Shield::Tick()
{
	if(is_active_)
	{
		-- shield_time_left_;

		if(shield_time_left_ <= 0)
		{
			Deactivate();
		}
		else if(shield_time_left_ == 2)
		{
			CCActionInterval* blink = CCBlink::create(2.0f, 8);
			blink->setTag(SHIELD_BLINK_TAG);
			runAction(blink);
		}
	}
	else
	{
		PowerUp::Tick();
	}
}

void Shield::Activate()
{
	if(is_active_)
		return;

	if(game_world_->player_->GetShield())
	{
		game_world_->player_->GetShield()->Reset();
		Deactivate();
		removeFromParentAndCleanup(true);
	}
	else
	{
		PowerUp::Activate();

		shield_time_left_ = SHIELD_DURATION;
		setScale(0);

		vector<CCPoint> vertices;
		GameGlobals::GetRegularPolygonVertices(vertices, 6, PLAYER_RADIUS * 2.5f);
		drawPolygon(&vertices[0], 6, ccc4f(0, 0, 0, 0), 4, ccc4f(0, 0.96862f, 1, 1));

		runAction(CCEaseBounceOut::create(CCScaleTo::create(0.25f, 1.0f)));
		runAction(CCRepeatForever::create(CCSequence::createWithTwoActions(CCEaseSineOut::create(CCScaleTo::create(0.25f, 1.15f)), CCEaseSineOut::create(CCScaleTo::create(0.25f, 1.0f)))));

		game_world_->player_->SetShield(this);
	}

	SOUND_ENGINE->playEffect("shield.wav");
}

void Shield::Deactivate()
{
	PowerUp::Deactivate();

	if(is_active_)
	{
		game_world_->player_->SetShield(NULL);
	}
}


void Shield::Reset()
{
	shield_time_left_ = SHIELD_DURATION;
	stopActionByTag(SHIELD_BLINK_TAG);
	setVisible(true);
}

