#include "GameWorld.h"
#include "BackgroundManager.h"
#include "Player.h"
#include "Enemy.h"
#include "Shield.h"
#include "Bomb.h"
#include "Blast.h"
#include "MissileLauncher.h"
#include "Missile.h"
#include "Popups.h"

GameWorld::GameWorld()
{
	background_ = NULL;
	boundary_rect_ = CCRectZero;
	player_ = NULL;
	enemies_ = NULL;
	powerups_ = NULL;
	blasts_ = NULL;
	missiles_ = NULL;
	score_label_ = NULL;
	seconds_ = 0;
	enemies_killed_total_ = 0;
	enemies_killed_combo_ = 0;
	combo_timer_ = 0;
	score_ = 0;
	is_popup_active_ = false;
}

GameWorld::~GameWorld()
{
	CC_SAFE_RELEASE_NULL(enemies_);
}

CCScene* GameWorld::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    
    // 'layer' is an autorelease object
    GameWorld *layer = GameWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool GameWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !CCLayer::init() )
    {
        return false;
    }
    
    setAccelerometerEnabled(true);
    setTouchEnabled(true);

	CreateGame();

    return true;
}

void GameWorld::CreateGame()
{
	seconds_ = 0;
	enemies_killed_total_ = 0;
	enemies_killed_combo_ = 0;
	combo_timer_ = 0;
	score_ = 0;
	is_popup_active_ = false;

	background_ = BackgroundManager::create();
	addChild(background_, E_LAYER_BACKGROUND);

	CreateBoundary();
	CreatePlayer();
	CreateContainers();
	CreateHUD();

	AddEnemyFormation();
	AddPowerUp();

	scheduleUpdate();
	schedule(schedule_selector(GameWorld::Tick), 1.0f);
}

void GameWorld::CreateBoundary()
{
	float offset = 0.025f;
	boundary_rect_.origin.x = SCREEN_SIZE.width * offset;
	boundary_rect_.origin.y = SCREEN_SIZE.height * offset;
	boundary_rect_.size.width = SCREEN_SIZE.width - SCREEN_SIZE.width * offset * 2;
	boundary_rect_.size.height = SCREEN_SIZE.height - SCREEN_SIZE.height * offset * 4;

	CCPoint vertices[4] = {CCPoint(boundary_rect_.origin.x, boundary_rect_.origin.y), 
		CCPoint(boundary_rect_.origin.x, boundary_rect_.origin.y + boundary_rect_.size.height), 
		CCPoint(boundary_rect_.origin.x + boundary_rect_.size.width, boundary_rect_.origin.y + boundary_rect_.size.height), 
		CCPoint(boundary_rect_.origin.x + boundary_rect_.size.width, boundary_rect_.origin.y)};

	CCDrawNode* boundary = CCDrawNode::create();
	boundary->drawPolygon(vertices, 4, ccc4f(0, 0, 0, 0), 1, ccc4f(1, 1, 1, 0.3f));
	addChild(boundary, E_LAYER_FOREGROUND);
}

void GameWorld::CreatePlayer()
{
	player_ = Player::create();
	player_->game_world_ = this;
	player_->setPosition(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.5f);
	addChild(player_, E_LAYER_PLAYER);
}

void GameWorld::CreateContainers()
{
	enemies_ = CCArray::createWithCapacity(MAX_ENEMIES);
	enemies_->retain();
	powerups_ = CCArray::createWithCapacity(MAX_POWERUPS);
	powerups_->retain();
	blasts_ = CCArray::createWithCapacity(MAX_BLASTS);
	blasts_->retain();
	missiles_ = CCArray::createWithCapacity(MAX_MISSILES);
	missiles_->retain();
}

void GameWorld::CreateHUD()
{
	score_label_ = CCLabelBMFont::create("Score: 0", "infont.fnt");
	score_label_->setAnchorPoint(CCPointZero);
	score_label_->setPosition(CCPoint(SCREEN_SIZE.width * 0.1f, boundary_rect_.getMaxY() + boundary_rect_.getMinY()));
	addChild(score_label_, E_LAYER_HUD);
}

void GameWorld::AddEnemyFormation()
{
	EEnemyFormation type = GetEnemyFormation();
	vector<CCPoint> formation = GameGlobals::GetEnemyFormation(type, boundary_rect_, player_->getPosition());
	int num_enemies_to_create = formation.size();
	
	int num_enemies_on_screen = enemies_->count();
	if(num_enemies_on_screen + num_enemies_to_create >= MAX_ENEMIES)
	{
		num_enemies_to_create = MAX_ENEMIES - num_enemies_on_screen;
	}

	for(int i = 0; i < num_enemies_to_create; ++i)
	{
		Enemy* enemy = Enemy::create(this);
		enemy->setPosition(formation[i]);
		enemy->Spawn(i * ENEMY_SPAWN_DELAY);
		addChild(enemy, E_LAYER_ENEMIES);
		enemies_->addObject(enemy);
	}
}

EEnemyFormation GameWorld::GetEnemyFormation()
{
	if(seconds_ > E_SKILL6)
	{
		int random_index = CCRANDOM_0_1() * GameGlobals::skill6_formations_size;
		return (EEnemyFormation)(GameGlobals::skill6_formations[random_index]);
	}
	else if(seconds_ > E_SKILL5)
	{
		int random_index = CCRANDOM_0_1() * GameGlobals::skill5_formations_size;
		return (EEnemyFormation)(GameGlobals::skill5_formations[random_index]);
	}
	else if(seconds_ > E_SKILL4)
	{
		int random_index = CCRANDOM_0_1() * GameGlobals::skill4_formations_size;
		return (EEnemyFormation)(GameGlobals::skill4_formations[random_index]);
	}
	else if(seconds_ > E_SKILL3)
	{
		int random_index = CCRANDOM_0_1() * GameGlobals::skill3_formations_size;
		return (EEnemyFormation)(GameGlobals::skill3_formations[random_index]);
	}
	else if(seconds_ > E_SKILL2)
	{
		int random_index = CCRANDOM_0_1() * GameGlobals::skill2_formations_size;
		return (EEnemyFormation)(GameGlobals::skill2_formations[random_index]);
	}
	else if(seconds_ > E_SKILL1)
	{
		int random_index = CCRANDOM_0_1() * GameGlobals::skill1_formations_size;
		return (EEnemyFormation)(GameGlobals::skill1_formations[random_index]);
	}
	else
	{
		return E_FORMATION_RANDOM_EASY;
	}
}

void GameWorld::AddPowerUp()
{
	if(powerups_->count() >= MAX_POWERUPS)
		return;

	PowerUp* powerup = NULL;

	int random_index = CCRANDOM_0_1() * GameGlobals::powerup_frequency_size;
	EPowerUpType powerup_type = (EPowerUpType)GameGlobals::powerup_frequency[random_index];

	switch(powerup_type)
	{
	case E_POWERUP_BOMB:
		powerup = Bomb::create(this);
		break;
	case E_POWERUP_MISSILE_LAUNCHER:
		powerup = MissileLauncher::create(this);
		break;
	case E_POWERUP_SHIELD:
		powerup = Shield::create(this);
		break;
	default:
		powerup = Bomb::create(this);
	}

	powerup->setPosition(ccp(boundary_rect_.origin.x + CCRANDOM_0_1() * boundary_rect_.size.width, boundary_rect_.origin.y + CCRANDOM_0_1() * boundary_rect_.size.height));
	powerup->Spawn();
	addChild(powerup, E_LAYER_POWERUPS);
	powerups_->addObject(powerup);
}

void GameWorld::AddBlast(Blast* blast)
{
	addChild(blast, E_LAYER_BLASTS);
	blasts_->addObject(blast);
}

void GameWorld::AddMissile(Missile* missile)
{
	addChild(missile, E_LAYER_MISSILES);
	missiles_->addObject(missile);
}

void GameWorld::update(float dt)
{
	if(player_->is_dying_)
		return;

	CCObject* object = NULL;
	CCARRAY_FOREACH(enemies_, object)
	{
		Enemy* enemy = (Enemy*)object;
		if(enemy)
		{
			enemy->Update(player_->getPosition(), player_->GetShield() == NULL);
		}
	}

	object = NULL;
	CCARRAY_FOREACH(powerups_, object)
	{
		PowerUp* powerup = (PowerUp*)object;
		if(powerup)
		{
			powerup->Update();
		}
	}

	CheckCollisions();
	CheckRemovals();
}

void GameWorld::CheckCollisions()
{
	CCPoint player_position = player_->getPosition();
	float player_radius = player_->getRadius();

	CCObject* object = NULL;
	CCARRAY_FOREACH(enemies_, object)
	{
		Enemy* enemy = (Enemy*)object;
		if(enemy)
		{
			CCPoint enemy_position = enemy->getPosition();

			// check with Player
			if(CIRCLE_INTERSECTS_CIRCLE(player_position, player_radius, enemy_position, ENEMY_RADIUS))
			{
				if(player_->GetShield())
				{
					enemy->Die();
					EnemyKilled();
				}
				else if(!enemy->getIsSpawning())
					player_->Die();
			}

			// check with Blast
			CCObject* object2 = NULL;
			CCARRAY_FOREACH(blasts_, object2)
			{
				Blast* blast = (Blast*)object2;
				if(blast)
				{
					if(CIRCLE_INTERSECTS_CIRCLE(blast->getPosition(), blast->getRadius(), enemy_position, ENEMY_RADIUS))
					{
						enemy->Die();
						EnemyKilled();
					}
				}
			}

			object2 = NULL;
			CCARRAY_FOREACH(missiles_, object2)
			{
				Missile* missile = (Missile*)object2;
				if(missile)
				{
					if(CIRCLE_INTERSECTS_CIRCLE(missile->getPosition(), MISSILE_RADIUS, enemy_position, ENEMY_RADIUS))
					{
						missile->Explode();
					}
				}
			}
		}
	}

	object = NULL;
	CCARRAY_FOREACH(powerups_, object)
	{
		PowerUp* powerup = (PowerUp*)object;
		if(powerup && !powerup->getIsActive())
		{
			if(CIRCLE_INTERSECTS_CIRCLE(player_position, player_radius, powerup->getPosition(), POWERUP_ICON_OUTER_RADIUS))
			{
				powerup->Activate();
			}
		}
	}
}

void GameWorld::CheckRemovals()
{
	CCObject* object = NULL;
	CCARRAY_FOREACH(enemies_, object)
	{
		Enemy* enemy = (Enemy*)object;
		if(enemy && enemy->getMustBeRemoved())
		{
			enemies_->removeObject(enemy);
		}
	}

	object = NULL;
	CCARRAY_FOREACH(powerups_, object)
	{
		PowerUp* powerup = (PowerUp*)object;
		if(powerup && powerup->getMustBeRemoved())
		{
			powerups_->removeObject(powerup);
		}
	}

	object = NULL;
	CCARRAY_FOREACH(blasts_, object)
	{
		Blast* blast = (Blast*)object;
		if(blast && blast->getMustBeRemoved())
		{
			blasts_->removeObject(blast);
		}
	}

	object = NULL;
	CCARRAY_FOREACH(missiles_, object)
	{
		Missile* missile = (Missile*)object;
		if(missile && missile->getMustBeRemoved())
		{
			missiles_->removeObject(missile);
		}
	}
}

void GameWorld::Tick(float dt)
{
	if(player_->is_dying_)
		return;

	++ seconds_;

	-- combo_timer_;
	if(combo_timer_ < 0)
		combo_timer_ = 0;
	else if(combo_timer_ == 0)
		ComboTimeUp();

	CCObject* object = NULL;
	CCARRAY_FOREACH(enemies_, object)
	{
		Enemy* enemy = (Enemy*)object;
		if(enemy)
		{
			enemy->Tick();
		}
	}

	object = NULL;
	CCARRAY_FOREACH(powerups_, object)
	{
		PowerUp* powerup = (PowerUp*)object;
		if(powerup)
		{
			powerup->Tick();
		}
	}

	if(seconds_ % 5 == 0)
		AddEnemyFormation();
	else if(seconds_ % 4 == 0)
		AddPowerUp();
}

void GameWorld::EnemyKilled()
{
	++ enemies_killed_total_;
	++ enemies_killed_combo_;
	combo_timer_ = COMBO_TIME;	
	score_ += 7;

	char buf[16] = {0};
	sprintf(buf, "Score: %d", score_);
	score_label_->setString(buf);
}

void GameWorld::ComboTimeUp()
{
	if(enemies_killed_combo_ < 5)
		return;

	score_ += enemies_killed_combo_ * 10;

	char buf[16] = {0};
	sprintf(buf, "Score: %d", score_);
	score_label_->setString(buf);

	sprintf(buf, "X%d", enemies_killed_combo_);
	CCLabelBMFont* combo_label = CCLabelBMFont::create(buf, "infont.fnt");
	combo_label->setPosition(player_->getPositionX(), player_->getPositionY() + combo_label->getContentSize().height);
	combo_label->setScale(0.6f);
	combo_label->runAction(CCSequence::create(CCMoveBy::create(1.0f, ccp(0, 50)), CCDelayTime::create(0.5f), CCRemoveSelf::create(true), NULL));
	addChild(combo_label, E_LAYER_HUD);
	
	enemies_killed_combo_ = 0;
}

void GameWorld::PauseGame()
{
	if(is_popup_active_)
		return;

	is_popup_active_ = true;

	pauseSchedulerAndActions();
	
	CCArray* game_world_children = getChildren();
	CCObject* child = NULL;
	CCARRAY_FOREACH(game_world_children, child)
	{
		if(child)
		{
			((CCNode*)child)->pauseSchedulerAndActions();
		}
	}

	PausePopup* pause_popup = PausePopup::create(this);
	addChild(pause_popup, E_LAYER_POPUP);
}

void GameWorld::ResumeGame()
{
	is_popup_active_ = false;

	resumeSchedulerAndActions();
	
	CCArray* game_world_children = getChildren();
	CCObject* child = NULL;
	CCARRAY_FOREACH(game_world_children, child)
	{
		if(child)
		{
			((CCNode*)child)->resumeSchedulerAndActions();
		}
	}
}

void GameWorld::GameOver()
{
	is_popup_active_ = true;

	unscheduleAllSelectors();

	CCArray* game_world_children = getChildren();
	CCObject* child = NULL;
	CCARRAY_FOREACH(game_world_children, child)
	{
		if(child)
		{
			((CCNode*)child)->unscheduleAllSelectors();
			((CCNode*)child)->stopAllActions();
		}
	}

	ComboTimeUp();
	GameOverPopup* game_over_popup = GameOverPopup::create(this, score_, enemies_killed_total_, seconds_);
	addChild(game_over_popup, E_LAYER_POPUP);
	SOUND_ENGINE->playEffect("game_over.wav");
}

void GameWorld::didAccelerate(CCAcceleration* acceleration_value)
{
	float mod_x = acceleration_value->x;
	float mod_y = acceleration_value->y;
	HandleInput(ccp(mod_x, mod_y));
}

void GameWorld::ccTouchesBegan(CCSet* set, CCEvent* event)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	CCTouch* touch = (CCTouch*)(*set->begin());
	CCPoint touch_point = touch->getLocationInView();
	touch_point = CCDirector::sharedDirector()->convertToGL(touch_point);

	if(!boundary_rect_.containsPoint(touch_point))
	{
		PauseGame();
		return;
	}

	CCPoint input = CCPointZero;
	input.x = (touch_point.x - SCREEN_SIZE.width * 0.5f) / (SCREEN_SIZE.width);
	input.y = (touch_point.y - SCREEN_SIZE.height * 0.5f) / (SCREEN_SIZE.height);
	HandleInput(input);
#else
	PauseGame();
#endif	
}

void GameWorld::ccTouchesMoved(CCSet* set, CCEvent* event)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	CCTouch* touch = (CCTouch*)(*set->begin());
	CCPoint touch_point = touch->getLocationInView();
	touch_point = CCDirector::sharedDirector()->convertToGL(touch_point);

	CCPoint input = CCPointZero;
	input.x = (touch_point.x - SCREEN_SIZE.width * 0.5f) / (SCREEN_SIZE.width);
	input.y = (touch_point.y - SCREEN_SIZE.height * 0.5f) / (SCREEN_SIZE.height);
	HandleInput(input);
#endif
}

void GameWorld::ccTouchesEnded(CCSet* set, CCEvent* event)
{
	HandleInput(CCPointZero);
}

void GameWorld::HandleInput(CCPoint input)
{
	if(is_popup_active_ || player_->is_dying_)
		return;

	CCPoint input_abs = CCPoint(fabs(input.x), fabs(input.y));

	// update the player
	player_->speed_.x = input.x * ( (input_abs.x > 0.3f) ? 36 : ( (input_abs.x > 0.2f) ? 28 : 20 ) );
	player_->speed_.y = input.y * ( (input_abs.y > 0.3f) ? 36 : ( (input_abs.y > 0.2f) ? 28 : 20 ) );

	// update the background
	background_->setPosition(ccp(input.x * -30, input.y * -30));
}
