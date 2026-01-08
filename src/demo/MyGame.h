#ifndef __TEST_GAME_H__
#define __TEST_GAME_H__

#include "../engine/AbstractGame.h"


// -------------------------------------------------------------- PC --------------------------------------------------------------
class PC : MySprite
{
	friend class NPC;
private:
	int xVel = 0;
	bool tryPunch1 = false;
	bool tryPunch2 = false;

	int health = 100;
	int iframes = 0;
	int damageCooldown = 70;

	Rect bodyHitbox = Rect(1, 1, 1, 1);
	Rect punchHitbox = Rect(1, 1, 1, 1);
	int punchType = 0;
	int punchRangeOffset = 30;
	int punch1Cooldown = 0;
	int punch2Cooldown = 0;

	// Audio
	std::shared_ptr<AudioEngine> sfx;
	Mix_Chunk* punch1_sfx;
	Mix_Chunk* punch2_sfx;
	Mix_Chunk* damage_sfx;

public:
	PC(std::shared_ptr<MyEngineSystem> system, std::shared_ptr<AudioEngine> SFX);
	void update(int xVelocity);
	void HandleAnimations();
	void TryPunch1() { tryPunch1 = true; }
	void TryPunch2() { tryPunch2 = true; }
	bool GetActive() { return isActive; }
	void SetActive(bool act) { isActive = act; }

	int GetMidX() { return MySprite::GetX() + MySprite::GetW() / 2; }
	int GetX() { return MySprite::GetX(); }
	void render(std::shared_ptr<GraphicsEngine> gfx) { MySprite::render(gfx); }

	SDL_RendererFlip GetFlip() { return flipped; }

	void SetRotation(double rotate) { rotation = rotate; }
	double GetRotation() { return rotation; }
	void Rotate(double rotate) { rotate += rotate; }

	// Collision
	bool Overlaps(MySprite* sprite) { return MySprite::Overlaps(sprite); }
	bool BodyCollide(MySprite* sprite) { return sprite->Overlaps(bodyHitbox); }
	bool PunchCollide(NPC* npc);
	void Damage(int amount);

	// Gameplay
	void SetDamageCooldown(int cd) { damageCooldown = cd; }
	int GetDamageCooldown() { return damageCooldown; }
	int GetHealth() { return health; }
	int GetPunchType() { return punchType; }
};




// -------------------------------------------------------------- NPC --------------------------------------------------------------

class NPC : MySprite
{
private:
	int chaseRange = 70;
	int xVel = 0;
	void HandleAnimations();
	void HandleMovement(PC* pc);
	void HandleKnockback();
	bool knockedBack = true;
	bool dead = false;
	int burnFrames = 0;
	bool escape = false;

	// Audio
	std::shared_ptr<AudioEngine> sfx;
	Mix_Chunk* death_sfx;
public:
	// Character Methods
	NPC(std::shared_ptr<MyEngineSystem> system, std::shared_ptr<AudioEngine> SFX);
	void update(PC* pc);
	
	bool GetActive() { return isActive; }
	void SetActive(bool act) { isActive = act; }

	int GetMidX() { return MySprite::GetX() + MySprite::GetW() / 2; }
	int GetX() { return MySprite::GetX(); }
	void render(std::shared_ptr<GraphicsEngine> gfx) { MySprite::render(gfx); }

	// Gameplay
	void Knockback(int strength, int direction);
	void Punched(PC* pc);
	void Die();
	bool GetDead() { return dead; }
	bool Burn();

	// Collision
	bool Overlaps(Rect rect) { return MySprite::Overlaps(rect); }
	bool Overlaps(MySprite* sprite) { return MySprite::Overlaps(sprite); }

	// Customization
	void SetChaseRange(int range) { chaseRange = range; }
};




// -------------------------------------------------------------- My Game --------------------------------------------------------------

class MyGame : public AbstractGame {
	private:
		// Sprites
		PC pc_sprite = PC(mySystem, sfx);
		NPC bat_sprite = NPC(mySystem, sfx);

		MySprite campfire_sprite = MySprite(50, 350, 100, 100, "res/images/campfireSheet.png", mySystem);
		MySprite background_sprite = MySprite(0,0,DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "res/images/background.png", mySystem);

		// Audio
		Mix_Music* fire_loop;
		Mix_Music* eerie_loop;
		Mix_Chunk* victory_sfx;
		Mix_Chunk* loss_sfx;
		int waitFrames = -1;

		// Velocity Input
		Vector2i velocity;

		/* GAMEPLAY */
		int score, lives;
		bool gameWon;
		bool gameLost;

		int pauseDelay = 0;
		bool burnt = false; // Bat burnt on campfire?

		// Fonts
		TTF_Font* bigFont; 
		TTF_Font* smallFont; 

		// AbstractGame Function Overrides
		void handleKeyEvents();
		void update();
		void render();
		void renderUI();
	public:
        MyGame(); // Constructor
		~MyGame(); // Destructor
};



#endif