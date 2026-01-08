#include "MyGame.h"

// -------------------------------------------------------------- My Game --------------------------------------------------------------

MyGame::MyGame() : AbstractGame(), score(0), lives(1), gameWon(false), gameLost(false) {

	// Fonts
	bigFont = ResourceManager::loadFont("res/fonts/arial.ttf", 72);
	smallFont = ResourceManager::loadFont("res/fonts/arial.ttf", 20);
	gfx->setVerticalSync(true);

	// Campfire Setup 
	campfire_sprite.SetMidPointToCentre();
	campfire_sprite.SetAnimatedSprite(true);
	campfire_sprite.LoadJSON("campfire");
	campfire_sprite.CreateAnimationFromJSON("Fire", true, "FireIdle");
	campfire_sprite.SetAnimationFrameChange("FireIdle", 8);
	campfire_sprite.SelectAnimation("FireIdle");

	// Background Setup
	mySystem->DeregisterSprite(&background_sprite); // No need to call increment frame since it isn't animated

	// Audio
	fire_loop = Mix_LoadMUS("res/audio/fire_loop.wav");
	eerie_loop = Mix_LoadMUS("res/audio/eerie_ghost_song.wav");
	victory_sfx = Mix_LoadWAV("res/audio/Game_Fail_Fanfare.wav");
	loss_sfx = Mix_LoadWAV("res/audio/Fanfare_4.wav");
	sfx->playMP3(fire_loop, -1);
}

MyGame::~MyGame() {
	
}

void MyGame::handleKeyEvents() {

	// Pause
	if (pauseDelay > 0) pauseDelay--;
	if (eventSystem->isPressed(Key::SPACE))
	{
		if (pauseDelay == 0)
		{
			paused = !paused;
			pauseDelay = 20;
		}
	}

	// Move
	int speed = 3;
	if (eventSystem->isPressed(Key::W)) {
		velocity.y = -speed;
		pc_sprite.TryPunch1();
	}

	if (eventSystem->isPressed(Key::S)) {
		velocity.y = speed;
		pc_sprite.TryPunch2();
	}

	if (eventSystem->isPressed(Key::A)) {
		velocity.x = -speed;
	}

	if (eventSystem->isPressed(Key::D)) {
		velocity.x = speed;
	}
}

void MyGame::update() {

	// Movement
	if (burnt) velocity.x *= -1; // Flip movement controls once bat is burnt
	if (burnt && pc_sprite.GetRotation() != 180) pc_sprite.SetRotation(180); // Rotate 180 degrees once bat is burnt

	pc_sprite.update(velocity.x); // Update PC

	bat_sprite.update(&pc_sprite); //  Update NPC

	if (pc_sprite.BodyCollide(&campfire_sprite)) pc_sprite.Damage(10); // PC Collide with Fire => Damage

	// Burning Bat
	if (bat_sprite.GetDead() && bat_sprite.Overlaps(&campfire_sprite) && !burnt)
	{
		burnt = bat_sprite.Burn(); // Return true once in fire long enough

		if (burnt) // Calls following code only once
		{
			sfx->playMP3(eerie_loop, -1);
			campfire_sprite.SetTexture("res/images/cursedCampfireSheet.png");
		}
	}


	if (pc_sprite.PunchCollide(&bat_sprite))
	{
		bat_sprite.Punched(&pc_sprite); // Hit Bat

		// Increase Score
		if (!bat_sprite.GetDead()) 
		{
			if (pc_sprite.GetPunchType() == 1) score += 20; // Heavy Punch
			else score += 5; // Jab
		}
	}

	if (!pc_sprite.GetActive() && !gameLost) // PC Died
	{
		if (!gameWon) waitFrames = 100;
		else waitFrames = 1;
		gameWon = false;
		gameLost = true;
		
	}
	if (score >= 250 && pc_sprite.GetActive() && !gameWon) // 250 Score Reached
	{
		bat_sprite.Die();
		gameWon = true;
		waitFrames = 100;
	}

	// Delay on win/loss sfx
	if (waitFrames > -1) waitFrames--;
	if (waitFrames == 0)
	{
		if (gameWon)
		{
			sfx->playSound(victory_sfx);
		}
		else if (gameLost)
		{
			sfx->playSound(loss_sfx);
		}
	}

	// Reset Velocity
	velocity.x = 0;
	velocity.y = 0;
}




// -------------------------------------------------------------- Rendering --------------------------------------------------------------
void MyGame::render() {
	background_sprite.render(gfx);
	if (campfire_sprite.isActive) campfire_sprite.render(gfx);
	if (pc_sprite.GetActive()) pc_sprite.render(gfx);
	if (bat_sprite.GetActive()) bat_sprite.render(gfx);
}

void MyGame::renderUI() {

	// Paused UI
	if (paused)
	{
		gfx->useFont(bigFont);
		gfx->setDrawColor(SDL_COLOR_WHITE);
		gfx->drawText("Game Paused", 170, 150);

		gfx->useFont(smallFont);
		gfx->drawText("Press Space to Toggle Pause", 275, 260);
		return;
	}

	// Playing UI

	gfx->useFont(smallFont); // Set Font to smaller one
	// Disclaimer
	gfx->setDrawColor(SDL_COLOR_WHITE);
	gfx->drawText("(This game does not endorse animal cruelty.)", 198, 50);

	// Score Title
	gfx->setDrawColor(SDL_COLOR_AQUA);
	gfx->drawText("Score", 700-24, 10);

	// Health Title
	gfx->setDrawColor(SDL_COLOR_RED);
	gfx->drawText("Health", 76, 10);
	

	gfx->useFont(bigFont); // Set Font to larger one
	// Score
	gfx->setDrawColor(SDL_COLOR_AQUA);
	std::string scoreStr = std::to_string(score);
	gfx->drawText(scoreStr, 700 - scoreStr.length() * 20, 25);

	// Health
	gfx->setDrawColor(SDL_COLOR_RED);
	std::string hpScore = std::to_string(pc_sprite.GetHealth());
	gfx->drawText(hpScore, 100 - hpScore.length() * 18, 25);


	if (gameWon)
	{
		// Victory
		if (burnt) gfx->setDrawColor(SDL_COLOR_PURPLE);
		else gfx->setDrawColor(SDL_COLOR_RED);
		gfx->drawText("YOU WON", 250, 500);
	}
	else if (gameLost)
	{
		// Loss
		gfx->setDrawColor(SDL_COLOR_BLUE);
		gfx->drawText("You Lost...", 250, 500);
	}
}




// -------------------------------------------------------------- Game Demo Class Functions --------------------------------------------------------------




// -------------------------- PC --------------------------
PC::PC(std::shared_ptr<MyEngineSystem> system, std::shared_ptr<AudioEngine> SFX) : MySprite(100,350,120,100, "res/images/brawlerSheet.png", system)
{
	SetMidPointToCentre();
	SetAnimatedSprite(true);

	// Create animations From JSON:
	LoadJSON("brawler");

	// Idle
	int idleID = CreateAnimationFromJSON("Idle", true, "Idleee");
	ChangeAnimName(idleID, "Iadu"); // Change anim name by ID
	ChangeAnimName("Iadu", "Idle"); // Change anim name by name
	SetAnimationFrameChange(idleID, 8);

	// Walk
	CreateAnimationFromJSON("Walk", true, "Walk");
	SetAnimationFrameChange("Walk", 12);

	// Damage
	CreateAnimationFromJSON("Damage", false, "Damage");
	SetAnimationFrameChange("Damage", 18);

	// Punch 1
	CreateAnimationFromJSON(6, 7, false, "Punch1");
	SetAnimationFrameChange("Punch1", 40);

	// Punch 2
	CreateAnimationFromJSON(7, 8, false, "Punch2");
	SetAnimationFrameChange("Punch2", 20);


	// Select Animation
	SetDefaultFrameFromJSON(2);
	SelectAnimation("Idle");

	// Audio
	sfx = SFX;
	punch1_sfx = Mix_LoadWAV("res/audio/knock_out.wav");
	punch2_sfx = Mix_LoadWAV("res/audio/Fist_Punch_or_kick.wav");
	damage_sfx = Mix_LoadWAV("res/audio/Retro_video_game_sfx_Ouch.wav");
	

	// Setup Hitboxes
	bodyHitbox.w = 10;
	bodyHitbox.h = GetH();
	bodyHitbox.y = GetY();

	punchHitbox.w = 1;
	punchHitbox.h = GetH();
	punchHitbox.y = GetY();
}

void PC::HandleAnimations()
{
	// Set Flip
	if (xVel < 0)
	{
		SetFlip(SDL_FLIP_HORIZONTAL); // Face Left
		if (rotation == 180) SetFlip(SDL_FLIP_NONE); // Opposite Flip when upside-down
	}
	else if (xVel > 0)
	{
		SetFlip(SDL_FLIP_NONE); // Face right
		if (rotation == 180) SetFlip(SDL_FLIP_HORIZONTAL); // Opposite Flip when upside-down
	}

	// Set Animations
	std::string animName = GetSelectedAnimationName();

	// Switch animation between Idle and Walk
	if (animName == "Idle")
	{
		if (xVel != 0)
		{
			SelectAnimation("Walk");
		}
	}
	else if (animName == "Walk")
	{
		if (xVel == 0)
		{
			SelectAnimation("Idle");
		}
	}
	else if (!InAnimation()) // Idle if finished animation that wasn't idle or walk
	{
		SelectAnimation("Idle");
	}

	// Switch animation from Idle or Walk to Punch1 or Punch2
	if (animName == "Idle" || animName == "Walk")
	{
		if (tryPunch2 && punch2Cooldown == 0)
		{
			SelectAnimation("Punch2");
			punchType = 2;
			punch2Cooldown = 35;
		}
		else if (tryPunch1 && punch1Cooldown == 0)
		{
			SelectAnimation("Punch1");
			punchType = 1;
			punch1Cooldown = 120;
		}

		if (punchType != 0) // Trigger Punch
		{
			int offset = 0;
			if (flipped == SDL_FLIP_HORIZONTAL)
			{
				// Left
				offset = -punchHitbox.w + punchRangeOffset;
			}
			else
			{
				// Right
				offset = GetW() - punchRangeOffset;
			}
			punchHitbox.x = GetX() + offset;

		}
	}
}

void PC::update(int xVelocity)
{
	punchType = 0;
	if (!isActive) return;

	std::string animName = GetSelectedAnimationName();

	bool spriteMove = !(animName == "Damage") && !(animName == "Punch1" || animName == "Punch2");
	if (spriteMove)
	{
		// Move PC
		xVel = xVelocity;
		ChangeXY(xVel, 0);

		// Move PC body hitbox
		int difference = (GetW() - bodyHitbox.w)/2;
		bodyHitbox.x = GetX() + difference;
	}

	HandleAnimations();
	
	tryPunch1 = false;
	tryPunch2 = false;
	if (iframes > 0) iframes--;
	if (punch1Cooldown > 0) punch1Cooldown--;
	if (punch2Cooldown > 0) punch2Cooldown--;
}

void PC::Damage(int amount)
{
	if (iframes > 0) return;
	health -= amount;
	sfx->playSound(damage_sfx, 70);
	SelectAnimation("Damage");
	iframes = damageCooldown;
	if (health <= 0)
	{
		isActive = false;
	}
}

bool PC::PunchCollide(NPC* npc)
{
	if (!npc->GetActive()) return false;
	bool hit = false;
	if (punchType != 0 && npc->Overlaps(punchHitbox)) hit = true;

	// Reset heavy punch cooldown if jab hits, and plays sfx
	if (punchType == 2 && hit)
	{
		punch1Cooldown = 0;
		sfx->playSound(punch2_sfx, 40);
		
	}
	else if (punchType == 1 && hit) // Plays sfx if heavy punch hits
	{
		sfx->playSound(punch1_sfx, 40);
	}

	return hit;
}



// -------------------------- NPC --------------------------

NPC::NPC(std::shared_ptr<MyEngineSystem> system, std::shared_ptr<AudioEngine> SFX) : MySprite(600, 350, 80, 80, "res/images/batSheet.png", system)
{
	// Set Up
	SetMidPointToCentre();
	SetAnimatedSprite(true);

	// Create animations From JSON:
	LoadJSON("bat");

	// Idle
	CreateAnimationFromJSON("Idle", true, "Idle");
	SetAnimationFrameChange("Idle", 8);
	SelectAnimation("Idle");

	// Right
	CreateAnimationFromJSON("Right", true, "MoveRight");
	SetAnimationFrameChange("MoveRight", 8);

	// Left
	CreateAnimationFromJSON("Left", true, "MoveLeft");
	SetAnimationFrameChange("MoveLeft", 8);

	// Dead
	CreateAnimationFromJSON(7, 8, true, "DeadFront");
	CreateAnimationFromJSON(11, 12, true, "DeadLeft");
	CreateAnimationFromJSON(15, 16, true, "DeadRight");

	// Audio
	sfx = SFX;
	death_sfx = Mix_LoadWAV("res/audio/extinct_bird_chirp.wav");
}

void NPC::HandleAnimations()
{
	std::string animName = GetSelectedAnimationName();
	if (!dead) // Alive
	{
		if (xVel == 0) // Stationary
		{
			if (animName != "Idle") SelectAnimation("Idle");
		}
		else if (xVel > 0) // Moving Right
		{
			if (animName != "MoveRight") SelectAnimation("MoveRight");
		}
		else // Moving Left
		{
			if (animName != "MoveLeft") SelectAnimation("MoveLeft");
		}
	}
	else // Dead
	{
		if (xVel == 0) // Stationary
		{
			if (animName != "DeadFront") SelectAnimation("DeadFront");
		}
		else if (xVel > 0) // Moving Right
		{
			if (animName != "DeadRight") SelectAnimation("DeadRight");
		}
		else // Moving Left
		{
			if (animName != "DeadLeft") SelectAnimation("DeadLeft");
		}
	}
}

void NPC::update(PC* pc)
{
	if (!isActive) return;

	if (knockedBack) HandleKnockback();
	else HandleMovement(pc);

	HandleAnimations();
}

void NPC::HandleKnockback()
{
	if (xVel > 0) xVel--;
	else xVel++;

	if (xVel == 0) knockedBack = false;
	ChangeXY(xVel, 0);
}

void NPC::HandleMovement(PC* pc)
{
	if (dead) return;
	if (pc->GetMidX() > GetMidX() + chaseRange)
	{
		xVel = 2;
	}
	else if (pc->GetMidX() < GetMidX() - chaseRange)
	{
		xVel = -2;
	}
	else
	{
		xVel = 0;
	}

	// Escape once PC dies
	if (!pc->GetActive())
	{
		xVel = 4;
		if (!escape)
		{
			escape = true;
			sfx->playSound(death_sfx);
		}
	}
	ChangeXY(xVel, 0);
}

void NPC::Knockback(int strength, int direction)
{
	knockedBack = true;
	int amount = strength * direction;
	xVel += amount;
}

void NPC::Punched(PC* pc)
{
	if (!isActive) return;

	// Get Strength of Punch
	int strength = 0;
	if (pc->GetPunchType() == 1) strength = 20;
	else strength = 14;

	// Get Direction of Punch
	int direction = 1;
	if (pc->GetFlip() == SDL_FLIP_HORIZONTAL) direction = -1;

	Knockback(strength, direction);
}

void NPC::Die()
{
	if (dead) return;
	sfx->playSound(death_sfx);
	dead = true;
	ChangeXY(0, 19); // Fall to ground
}

bool NPC::Burn()
{
	burnFrames++;
	if (burnFrames > 150) // Finished burning
	{
		isActive = false;
		return true;
	}
	return false;
}