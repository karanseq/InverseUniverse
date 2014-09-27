#include "Bomb.h"
#include "GameWorld.h"
#include "Blast.h"

Bomb* Bomb::create(GameWorld* instance)
{
	Bomb* bomb = new Bomb();
	if(bomb && bomb->init(instance))
	{
		bomb->autorelease();
		return bomb;
	}
	CC_SAFE_DELETE(bomb);
	return NULL;
}

bool Bomb::init(GameWorld* instance)
{
	if(!PowerUp::init(instance))
		return false;
	
	vector<CCPoint> vertices;
	GameGlobals::GetRegularPolygonVertices(vertices, 3, POWERUP_ICON_INNER_RADIUS);
	drawPolygon(&vertices[0], 3, ccc4f(0, 0, 0, 0), 3, ccc4f(0, 1, 0, 1));

	return true;
}

void Bomb::Update()
{
	if(!is_active_)
	{
		PowerUp::Update();
	}
}

void Bomb::Activate()
{
	if(is_active_)
		return;

	PowerUp::Activate();

	Blast* blast = Blast::createWithRadiusAndDuration(PLAYER_RADIUS * 8, 2.0f);
	blast->setPosition(m_obPosition);
	game_world_->AddBlast(blast);
	SOUND_ENGINE->playEffect("big_blast.wav");

	PowerUp::Deactivate();
}
