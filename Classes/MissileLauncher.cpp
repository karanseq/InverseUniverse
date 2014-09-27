#include "MissileLauncher.h"
#include "GameWorld.h"
#include "Missile.h"
#include "Player.h"
#include "Enemy.h"

MissileLauncher* MissileLauncher::create(GameWorld* instance)
{
	MissileLauncher* missile_launcher = new MissileLauncher();
	if(missile_launcher && missile_launcher->init(instance))
	{
		missile_launcher->autorelease();
		return missile_launcher;
	}
	CC_SAFE_DELETE(missile_launcher);
	return NULL;
}

bool MissileLauncher::init(GameWorld* instance)
{
	if(!PowerUp::init(instance))
		return false;
	
	vector<CCPoint> vertices_vector1;
	vector<CCPoint> vertices_vector2;
	vector<CCPoint> vertices;

	GameGlobals::GetRegularPolygonVertices(vertices_vector1, 5, POWERUP_ICON_INNER_RADIUS - 6, M_PI * -2/20);
	GameGlobals::GetRegularPolygonVertices(vertices_vector2, 5, POWERUP_ICON_INNER_RADIUS, M_PI * 2/20);
	
	for(int i = 0; i < 5; ++i)
	{
		vertices.push_back(vertices_vector1[i]);
		vertices.push_back(vertices_vector2[i]);
	}

	drawPolygon(&vertices[0], 10, ccc4f(0, 0, 0, 0), 2, ccc4f(0.88235, 0.96078, 0, 1));

	return true;
}

void MissileLauncher::Update()
{
	if(!is_active_)
	{
		PowerUp::Update();
	}
}

void MissileLauncher::Activate()
{
	if(is_active_)
		return;

	PowerUp::Activate();

	vector<CCPoint> target = GenerateTargets();
	vector<CCPoint> control_points;
	GameGlobals::GetRegularPolygonVertices(control_points, 5, SCREEN_SIZE.width/4, M_PI * 2/20);

	for(int i = 0; i < 5; ++i)
	{
		Missile* missile = Missile::createWithTarget(game_world_, target[i], ccpMult(control_points[i].normalize(), MISSILE_SPEED));
		missile->setPosition(m_obPosition);
		game_world_->AddMissile(missile);
	}

	SOUND_ENGINE->playEffect("missile.wav");
	
	PowerUp::Deactivate();
}

vector<CCPoint> MissileLauncher::GenerateTargets()
{
	vector<CCPoint> target_points;
	target_points.clear();

	int targets_found = 0;

	int num_enemies = game_world_->enemies_->count();
	for(int i = 0; i < num_enemies && targets_found < 5; ++i)
	{
		Enemy* enemy = (Enemy*)game_world_->enemies_->objectAtIndex(i);
		target_points.push_back(enemy->getPosition());
		++ targets_found;
	}

	while(targets_found < 5)
	{
		target_points.push_back(CCPoint(CCRANDOM_0_1() * (game_world_->boundary_rect_.origin.x + game_world_->boundary_rect_.size.width) , CCRANDOM_0_1() * (game_world_->boundary_rect_.origin.y + game_world_->boundary_rect_.size.height)));
		++ targets_found;
	}

	return target_points;
}
