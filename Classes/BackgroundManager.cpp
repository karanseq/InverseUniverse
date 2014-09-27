#include "BackgroundManager.h"

bool BackgroundManager::init()
{
	if ( !CCNode::init() )
    {
        return false;
    }

	setContentSize(CCSizeMake(SCREEN_SIZE.width * 1.2f, SCREEN_SIZE.height * 1.2f));

	CCDrawNode* star = CCDrawNode::create();
	addChild(star);

	for(int i = 0; i < MAX_STARS; ++i)
	{
		CCPoint position = ccp(CCRANDOM_0_1() * SCREEN_SIZE.width * 1.2f, CCRANDOM_0_1() * SCREEN_SIZE.height * 1.2f);
		float alpha = 0.25f + CCRANDOM_0_1() * 0.75f;
		star->drawDot(position, 1, ccc4f(1, 1, 1, alpha));
	}

	return true;
}