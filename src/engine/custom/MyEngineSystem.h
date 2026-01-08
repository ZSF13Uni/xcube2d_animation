#ifndef __MY_ENGINE_H__
#define __MY_ENGINE_H__

// Standard
#include <vector>
#include <map>

// Engine Components
#include "../EngineCommon.h"
#include "../GameMath.h"
#include  "../GraphicsEngine.h"
#include "../ResourceManager.h"

// Stream
#include <iostream>
#include <fstream>

// rapidJSON
#include "..\..\..\src\include\rapidjson\document.h"
#include "..\..\..\src\include\rapidjson\filereadstream.h"



// -------------------------------- My Engine System --------------------------------

class MyEngineSystem {
	friend class XCube2Engine;
	friend class MySprite;
	private:
		std::vector<MySprite*> allSprites;
	public:
		void update();
		void RegisterSprite(MySprite* sprite);
		void DeregisterSprite(MySprite* sprite);
};





// -------------------------------- Animation --------------------------------

class Animation
{
private:
	std::vector<SDL_Rect> frames;
	int currentAnimationFrame = 0;

	int frameCount = 0;
	int frameChange = 50; // Number of game frames that must pass before changing animation frame

public:
	bool loop = false;

	Animation(); // Constructor

	void AddFrame(SDL_Rect);
	void AddFrame(int X, int Y, int W, int H);

	SDL_Rect GetFrame(int index) { return frames[index]; }
	int GetNOFrames() { return frames.size(); }

	int StartAnimation(); // Resets current animation frame and frameCount
	int GetCurrentFrame();
	int GetNextFrame();

	void SetFrameChange(int change) { frameChange = change; }
	void IncrementFrameCount(bool paused);

	bool inAnimation();
};





// -------------------------------- Stored JSON --------------------------------

class StoredJSON // Using rapidjson library
{
private:
	rapidjson::Document document;
	std::vector<const rapidjson::Value*> frameList;
	char* fileName;
public:
	StoredJSON(char* name);
	StoredJSON() {};

	SDL_Rect GetRectFromIndex(int index); // Returns SDL_Rect of specified frame index
	const char* GetFileNameFromIndex(int index); // Returns filename of specified frame index
	int GetNOFrames() { return frameList.size(); }
	char* GetFileName() { return fileName; }
};






// -------------------------------- My Sprite --------------------------------

class MySprite : Rect
{
protected:
	SDL_RendererFlip flipped = SDL_FLIP_NONE; // Whether the image is flipped
private:
	SDL_Texture* texture;
	Vector2i textureSize;
	void UpdateTextureSize();

	SDL_Point midPoint = { 0,0 }; // Point that the Sprite will be scaled from and rotated around

	bool animated = false; // Whether the Sprite is allowed to enter into animation
	bool animationPaused = false;
	int selectedAnimation = -1; // The index of animations, -1 when not in an animation

	std::vector<Animation> animations; // Vector containing Sprite animations
	std::map<std::string, int> animationNames; // Map for equating animation names to animIDs

	SDL_Rect defaultRect{}; // Default Frame - If the sprite is animated, but has no selected animation, it will fall back to this

	// JSON
	bool jsonLoaded = false; // Set to true when a JSON file is loaded into the Sprite
	StoredJSON json = StoredJSON();

	bool rotateWarning = false;

public:
	MySprite(int x, int y, int w, int h, std::string filePath, std::shared_ptr<MyEngineSystem> systemInstance); // Constructor
	bool isActive = true;

	// Rotation in degrees
	double rotation = 0;

	// -------------------------------- Rect Methods --------------------------------
	int GetX() { return x; }
	int GetY() { return y; }
	void ChangeXY(int X, int Y);

	void SetWidthHeight(int W, int H);
	int GetW() { return w; }
	int GetH() { return h; }
	void Scale(float scaleX, float scaleY);

	void SetMidPoint(SDL_Point point);
	void SetMidPoint(int X, int Y);
	void SetMidPointToOrigin() { SetMidPoint(0, 0); }
	void SetMidPointToCentre() { SetMidPoint(w/2, h/2); }

	// -------------------------------- Render --------------------------------

	void render(std::shared_ptr<GraphicsEngine> gfx);

	void SetTexture(char* path);
	void SetFlip(SDL_RendererFlip flip) { flipped = flip; }
	Vector2i GetTextureSize() { return textureSize; }

	// -------------------------------- Collision Detection -------------------------------- : Note that these will not properly line up with the sprite when rotated. Use other methods of collision in that case. 

	void CollisionRotationNotice();

	bool contains(const Point2& p) { CollisionRotationNotice(); return Rect::contains(p); }
	bool Overlaps(MySprite* sprite);
	bool Overlaps(Rect& rect);
	bool Overlaps(SDL_Rect rect);
	

	// -------------------------------- Animation Methods --------------------------------

	void SetAnimatedSprite(bool Animated) { animated = Animated; }

	void IncrementFrame(); // Called every frame (handled by MyEngineSystem)
	
	void SelectAnimation(int animID); // Plays an animation from animation ID
	void SelectAnimation(std::string animName); // Plays an animation from animation Name

	int GetSelectedAnimationID() { return selectedAnimation; }
	std::string GetSelectedAnimationName();

	int AddAnimation(Animation anim);
	int AddAnimation(Animation anim, std::string animName);

	void SetAnimationFrameChange(int animID, int frames); // Set number of game frames required between each animation frame for a specified animation
	void SetAnimationFrameChange(std::string animName, int frames);

	bool InAnimation(); // Returns true if the sprite is currently in an animation

	void ChangeAnimName(int id, std::string newName);
	void ChangeAnimName(std::string animName, std::string newName);

	// Default Frame - If the sprite is animated, but has no selected animation, it will fall back to this
	void SetDefaultFrame(SDL_Rect rect);
	void SetDefaultFrame(int X, int Y, int W, int H);

	void SetDefaultFrameFromJSON(int index);

	void ResetDefaultFrame(); // Resets to whole texture

	Animation ReturnAnimation(std::string animName);
	Animation ReturnAnimation(int animID);


	// -------------------------------- JSON --------------------------------

	void LoadJSON(char* name);

	int CreateAnimationFromJSON(int start, int end, bool loop); // From Range
	int CreateAnimationFromJSON(char* animName, bool loop); // From Name Scheme

	// -------------------------------- Set Names --------------------------------

	int CreateAnimationFromJSON(int start, int end, bool loop, std::string animName); // From Range
	int CreateAnimationFromJSON(char* animName, bool loop, std::string optionalName); // From Name Scheme
};


#endif