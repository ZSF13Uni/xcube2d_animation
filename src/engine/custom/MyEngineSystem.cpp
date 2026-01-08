#include "MyEngineSystem.h"


// My Sprite --------------------------------------------------------------------------------

MySprite::MySprite(int x, int y, int w, int h, std::string filePath, std::shared_ptr<MyEngineSystem> systemInstance) : Rect(x, y, w, h) // Constructor
{

	SDL_Color col;
	col.a = 255;
	texture = ResourceManager::loadTexture(filePath, col);

	UpdateTextureSize();
	defaultRect = SDL_Rect{0,0,textureSize.x,textureSize.y};
	
	systemInstance->RegisterSprite(this);
}



void MySprite::UpdateTextureSize()
{
	Vector2i size;
	SDL_QueryTexture(texture, NULL, NULL, &size.x, &size.y);
	textureSize = size;
}

void MySprite::render(std::shared_ptr<GraphicsEngine> gfx)
{
	if (!isActive) return;
	SDL_Rect* destRect = &getSDLRect(); // Position to fill on screen


	if (animated && selectedAnimation != -1 && animations.size() > 0) // If in an Animation
	{
		if (animations[selectedAnimation].GetNOFrames() > 0 && animations[selectedAnimation].inAnimation()) // If Animation has frames
		{
			SDL_Rect* sourceRect = &animations[selectedAnimation].GetFrame(animations[selectedAnimation].GetCurrentFrame());
			gfx->drawTexture(texture, sourceRect, destRect, rotation, &midPoint, flipped);
			return;
		}
	}
	gfx->drawTexture(texture, &defaultRect, destRect, rotation, &midPoint, flipped);
}

void MySprite::SetTexture(char* path)
{
	SDL_Color col;
	col.a = 255;
	texture = ResourceManager::loadTexture(path, col);

	UpdateTextureSize();
}



void MySprite::SetMidPoint(SDL_Point point)
{
	midPoint = point;
}

void MySprite::SetMidPoint(int X, int Y)
{
	SDL_Point point = { X, Y };
	midPoint = point;
}



void MySprite::ChangeXY(int X, int Y)
{
	x += X;
	y += Y;
}

void MySprite::SetWidthHeight(int W, int H)
{
	float dw = w - W;
	float dh = h - H;

	float xMidRatio = (float)midPoint.x / w;
	float yMidRatio = (float)midPoint.y / h;

	x += dw * xMidRatio;
	y += dh * yMidRatio;

	w = W;
	h = H;

	midPoint.x = xMidRatio * W;
	midPoint.y = yMidRatio * H;
}

void MySprite::Scale(float scaleX, float scaleY)
{
	float newW = (float)w * scaleX;
	float newH = (float)h * scaleY;
	SetWidthHeight((int)newW, (int)newH);
}

// Collision

void MySprite::CollisionRotationNotice()
{
	if (rotateWarning) return;
	if (rotation != 0)
	{
		SDL_LogWarn(0, "Sprite is rotated, so this collision detection function may not work as intended.\n");
		rotateWarning = true;
	}
	
}

bool MySprite::Overlaps(MySprite* sprite)
{
	CollisionRotationNotice();
	SDL_Rect rect1 = getSDLRect();
	SDL_Rect rect2 = sprite->getSDLRect();

	return (SDL_HasIntersection(&rect1, &rect2) == SDL_TRUE);
}

bool MySprite::Overlaps(Rect& rect)
{
	CollisionRotationNotice();
	SDL_Rect rect1 = getSDLRect();
	SDL_Rect rect2 = rect.getSDLRect();

	return (SDL_HasIntersection(&rect1, &rect2) == SDL_TRUE);
}

bool MySprite::Overlaps(SDL_Rect rect)
{
	CollisionRotationNotice();
	SDL_Rect rect1 = getSDLRect();

	return (SDL_HasIntersection(&rect1, &rect) == SDL_TRUE);
}

// Default Frame

void MySprite::ResetDefaultFrame()
{
	defaultRect = SDL_Rect{ 0,0,textureSize.x,textureSize.y };
}

void MySprite::SetDefaultFrame(SDL_Rect rect)
{
	defaultRect = rect;
}

void MySprite::SetDefaultFrame(int X, int Y, int W, int H)
{
	SDL_Rect rect{ X,Y,W,H };
	defaultRect = rect;
}

void MySprite::SetDefaultFrameFromJSON(int index)
{
	defaultRect = json.GetRectFromIndex(index);
}

// Animation Methods

void MySprite::IncrementFrame()
{
	if (animated && selectedAnimation != -1)
	{
		animations[selectedAnimation].IncrementFrameCount(animationPaused);
	}
}

void MySprite::SelectAnimation(int animID)
{
	if (animated && animID > -2 && animID < animations.size())
	{
		selectedAnimation = animID;
		if (selectedAnimation != -1)
		{
			animations[selectedAnimation].StartAnimation();
			return;
		}
		std::cout << "Setting animation to anim with ID '-1'. Was this intended behavior?\n";
		return;
	}
	std::cout << "Anim ID exceeded bounds.\n";
}

void MySprite::SelectAnimation(std::string animName)
{
	if (animationNames.count(animName) == 0)
	{
		std::cout << "Failed to select animation with name '" << animName << "'. No such animation exists.\n";
		return;
	}
	SelectAnimation(animationNames[animName]);
}



int MySprite::AddAnimation(Animation anim)
{
	animations.push_back(anim);
	return animations.size() - 1;
}

int MySprite::AddAnimation(Animation anim, std::string animName)
{
	if (animName == "N/A")
	{
		std::cout << "Could not create animation with name 'N/A'";
		return -1;
	}
	if (animationNames.count(animName) != 0) {

		std::cout << "Tried to create animation with name " << animName << ", but that name was already taken.\n";
		return -1;
	}

	animations.push_back(anim); // Add animation

	int ID = animations.size() - 1;
	animationNames[animName] = ID; // Link it to animName
	return ID;
}



bool MySprite::InAnimation()
{
	if (!animated || selectedAnimation == -1 || !animations.size() > 0)
	{
		return false;
	}

	if (animations[selectedAnimation].GetNOFrames() == 0 || !animations[selectedAnimation].inAnimation())
	{
		return false;
	}

	return true;
}

void MySprite::SetAnimationFrameChange(int animID, int frames)
{
	if (animID > -1 && animID < animations.size())
	{
		animations[animID].SetFrameChange(frames);
	}
	else
	{
		std::cout << "Failed to set animation frame because animId was out of bounds.\n";
	}
}

void MySprite::SetAnimationFrameChange(std::string animName, int frames)
{
	if (animationNames.count(animName) == 0)
	{
		std::cout << "Failed to set animation frame for animation '" << animName << "', as an animation by that name cannot be found.\n";
		return;
	}
	SetAnimationFrameChange(animationNames[animName], frames);
}





void MySprite::ChangeAnimName(std::string animName, std::string newName)
{
	if (newName == "N/A")
	{
		std::cout << "Could not change name of anim '" << animName << "' to 'N/A', as it is exempt.\n";
		return;
	}

	if (animationNames.count(animName) != 0 && animationNames.count(newName) == 0) // If there is an entry for animName, and newName isn't already taken
	{
		// Erase old pair, and add new one
		int ID = animationNames[animName];
		animationNames.erase(animName);
		animationNames[newName] = ID;
	}
}

void MySprite::ChangeAnimName(int id, std::string newName)
{
	// Get animName
	if (newName == "N/A")
	{
		std::cout << "Could not change name of anim with ID '" << id << "' to 'N/A', as it is exempt.\n";
		return;
	}
	std::string animName = "";
	for (auto pair : animationNames)
	{
		if (pair.second == id)
		{
			animName = pair.first;
		}
	}

	if (animationNames.count(animName) != 0 && animationNames.count(newName) == 0) // If there is an entry for animName, and newName isn't already taken
	{
		// Erase old pair, and add new one
		animationNames.erase(animName);
		animationNames[newName] = id;
	}

}



std::string MySprite::GetSelectedAnimationName()
{
	for (auto pair : animationNames)
	{
		if (pair.second == selectedAnimation)
		{
			return pair.first;
		}
	}
	return ("N/A");
}



Animation MySprite::ReturnAnimation(std::string animName)
{
	if (animationNames.count(animName) != 0)
	{
		int ID = animationNames[animName];
		return animations[ID];
	}
}

Animation MySprite::ReturnAnimation(int animID)
{
	return animations[animID];
}



// Json

void MySprite::LoadJSON(char* name)
{
	json = StoredJSON(name);
	jsonLoaded = true;

}

// Create animation from order of appearance in JSON file. Start inclusive, end exclusive. Starts from 0.
int MySprite::CreateAnimationFromJSON(int start, int end, bool loop)
{
	if (jsonLoaded)
	{
		Animation anim = Animation();
		for (int i = start; i < end; i++)
		{
			anim.AddFrame(json.GetRectFromIndex(i));
		}
		anim.loop = loop;
		return AddAnimation(anim);
	}
}

// Creates animation using JSON file based on input name-scheme ("run" will create Animation from "run01", "run02", "run03" etc.)
int MySprite::CreateAnimationFromJSON(char* animName, bool loop) 
{
	if (!jsonLoaded) return -1;
	std::map<int, int> indexMap;

	
	// Get all matching frames
	for (int i = 0; i < json.GetNOFrames(); i++)
	{
		const char* filename = json.GetFileNameFromIndex(i);

		// Check if filename begins with animName
		bool matches = true;
		for (int j = 0; j < strlen(animName); j++)
		{
			if (filename[j] != animName[j]) { matches = false; break; }
		}

		if (matches) // Filename begins with animName parameter
		{
			// Get number at end of filename
			std::string nameNumberStr = "";
			for (int j = 0; j < strlen(filename); j++)
			{
				if (j >= strlen(animName)) // If at index past animNames length (the number portion)
				{
					//if (name[j] == '.' || name[j] == 'p' || name[j] == 'n' || name[j] == 'g') continue; // Parse out the final .png
					if (filename[j] == '.') break; // Stop once reaching .png
					nameNumberStr += filename[j];
				}
			}
			const char* nameNumber = nameNumberStr.c_str();
			int num = std::atoi(nameNumber); // Number at end of filename;

			//std::cout << num-1 << "\n";
			indexMap[num-1] = i; // Map fileNumber to index position
		}
	}

	// Put together the Animation
	Animation anim = Animation();
	anim.loop = loop;

	
	for (int i = 0; i < indexMap.size(); i++)
	{
		if (indexMap.count(i) == 0) // Check there is an entry for this index
		{
			std::cout << "Attempted to create animation from JSON file '" << json.GetFileName() << "', but the frame with filename '" << animName << (i + 1) << "' was not found.\n";
			return -1;
		}
		anim.AddFrame(json.GetRectFromIndex(indexMap[i]));
	}

	if (anim.GetNOFrames() > 0) // If animation was successfully created
	{
		return AddAnimation(anim);
	}
	else return -1;
}

// Create animation from order of appearance in JSON file. Start inclusive, end exclusive. Starts from 0.
int MySprite::CreateAnimationFromJSON(int start, int end, bool loop, std::string animName)
{
	if (animName == "N/A")
	{
		std::cout << "Could not create animation with name 'N/A'";
		return -1;
	}
	if (animationNames.count(animName) == 0)
	{
		int ID = CreateAnimationFromJSON(start, end, loop);
		if (ID != -1) animationNames[animName] = ID;
		return ID;
	}
	else
	{
		std::cout << "Failed to create animation. Animation name '" << animName << "', is already taken.\n";
		return -1;
	}
}

int MySprite::CreateAnimationFromJSON(char* animName, bool loop, std::string optionalName)
{
	if (optionalName == "N/A")
	{
		std::cout << "Could not create animation with name 'N/A'";
		return -1;
	}
	if (animationNames.count(optionalName) == 0)
	{
		int ID = CreateAnimationFromJSON(animName, loop);
		if (ID != -1) animationNames[optionalName] = ID;
		return ID;
	}
	else
	{
		std::cout << "Failed to create animation. Animation name '" << optionalName << "', is already taken.\n";
		return -1;
	}
}


// Animation --------------------------------------------------------------------------------

Animation::Animation()
{
}
void Animation::AddFrame(SDL_Rect frame)
{
	frames.push_back(frame);
}

void Animation::AddFrame(int X, int Y, int W, int H)
{
	SDL_Rect frame{ X,Y,W,H };
	frames.push_back(frame);
}

int Animation::StartAnimation()
{
	currentAnimationFrame = 0;
	frameCount = 0;
	return currentAnimationFrame;
}

int Animation::GetCurrentFrame()
{
	return currentAnimationFrame;
}

int Animation::GetNextFrame()
{
	int frame = currentAnimationFrame;
	frame++;
	if (frame >= frames.size())
	{
		if (loop)
		{
			frame = 0;
		}
		else
		{
			frame = -1;
		}
	}
	currentAnimationFrame = frame;
	return currentAnimationFrame;
}

void Animation::IncrementFrameCount(bool paused)
{
	if (loop == false && currentAnimationFrame == -1) return;
	if (paused) return;

	frameCount++;
	if (frameCount >= frameChange) // If ready to change animation frames
	{
		frameCount = 0;
		GetNextFrame();
	}
}

bool Animation::inAnimation()
{
	if (currentAnimationFrame == -1)
	{
		return false;
	}
	else
	{
		return true;
	}
}


// Stored JSON --------------------------------------------------------------------------------

StoredJSON::StoredJSON(char* name)
{
	std::string pathStart = "res/json/";
	std::string pathEnd = ".json";
	std::string path = pathStart + name + pathEnd;


	//https://www.geeksforgeeks.org/cpp/rapidjson-file-read-write-in-cpp/ - Source
	std::ifstream file(path);

	// Read the entire file into a string
	std::string json((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());
	const char* jsonChar = json.c_str();

	document.Parse(jsonChar);

	if (document.HasParseError()) {
		switch (document.GetParseError()) {
		case rapidjson::kParseErrorNone:
			std::cout << "No error" << std::endl;
			break;
		case rapidjson::kParseErrorDocumentEmpty:
			std::cout << "Error: Document is empty" << std::endl;
			return;
		case rapidjson::kParseErrorDocumentRootNotSingular:
			std::cout << "Error: Document root is not singular"
				<< std::endl;
			return;
			// ...
		}
	}


	assert(document.IsObject());
	const rapidjson::Value& frames = document["frames"];
	assert(frames.IsArray());

	for (rapidjson::SizeType i = 0; i < frames.Size(); i++) // Uses SizeType instead of size_t
	{
		assert(frames[i].IsObject());
		const rapidjson::Value& frame = frames[i];

		frameList.push_back(&frame);
	}

	fileName = name;

}
SDL_Rect StoredJSON::GetRectFromIndex(int index)
{
	if (frameList.empty()) { SDL_LogWarn(0, "Attempted to read from from empty json file."); return SDL_Rect{ 0,0,0,0 }; }
	const rapidjson::Value& frame = *frameList[index];
	const rapidjson::Value& frameInfo = frame["frame"].GetObject();
	int x = frameInfo["x"].GetInt();
	int y = frameInfo["y"].GetInt();
	int w = frameInfo["w"].GetInt();
	int h = frameInfo["h"].GetInt();
	return SDL_Rect{x,y,w,h};

}

const char* StoredJSON::GetFileNameFromIndex(int index)
{
	const rapidjson::Value& frame = *frameList[index];
	const char* name = frame["filename"].GetString();
	return name;
}

// My Engine System --------------------------------------------------------------------------------
void MyEngineSystem::RegisterSprite(MySprite* sprite)
{
	allSprites.push_back(sprite);
}

void MyEngineSystem::update()
{
	for (int i = 0; i < allSprites.size(); i++)
	{
		if (allSprites[i]->isActive) allSprites[i]->IncrementFrame();
	}
}

void MyEngineSystem::DeregisterSprite(MySprite* sprite)
{
	for (int i = 0; i < allSprites.size(); i++)
	{
		if (allSprites[i] == sprite)
		{
			allSprites.erase(allSprites.begin() + i);
		}
	}
}