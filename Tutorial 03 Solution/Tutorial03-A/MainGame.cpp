#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

constexpr float DISPLAY_WIDTH{ 1280 };
constexpr float DISPLAY_HEIGHT{ 720 };
constexpr int DISPLAY_SCALE{ 1 };
constexpr int BALL_RADIUS{ 48 };

// Define a value for our ball's default velocity
const Vector2D BALL_VELOCITY_DEFAULT(5.0f, 0.f);

const Vector2D BALL_ACCELERATION(0.f, 0.2f);

// Define a size for the paddle's AABB box
const Vector2D PADDLE_AABB{100.f, 20.f};
const Vector2D BALL_AABB{ 48.f, 48.f };
const Vector2D CHEST_AABB{ 50.f, 50.f };

const int CHEST_SPACING{ 100 };

enum GameObjectType
{
	TYPE_BALL = 0,
	TYPE_PADDLE = 1,
	TYPE_CHEST = 2,
	TYPE_DESTROYED = 3,
};

struct GameState
{
	int yCount = 0;
	int xCount = 0;
	int x = 0;
	int y = 0;
	int offsetX = 80;
	int offsetY = 70;
	int collisionCount = 0;
	int ballRotation = 0;
	
};

GameState gameState;

int reversed = 0;


// Forward-declaration of Draw

void Draw();

void UpdateBall();
void UpdatePaddle();
void ResetBall();
void UpdatePlayerControls();

bool IsBallColliding();
bool IsBallCollidingChest();


// The entry point for a PlayBuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	// Setup PlayBuffer
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	Play::CentreAllSpriteOrigins(); // this function makes it so that obj.pos values represent the center of a sprite instead of its top-left corner

	// Create a ball object and a paddle object
	Play::CreateGameObject(TYPE_BALL, { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 }, BALL_RADIUS, "ball");
	Play::CreateGameObject(TYPE_PADDLE, { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 100 }, BALL_RADIUS, "spanner");
	// ... "ball", "spanner" etc. are the filenames of sprites stored in the Data folder alongside this solution, you can put any .PNGs in there if you feel creative :)

	// Set initial velocity for ball
	GameObject& ballObj = Play::GetGameObjectByType(TYPE_BALL);
	ballObj.velocity = BALL_VELOCITY_DEFAULT;
	ballObj.acceleration = BALL_ACCELERATION;

	
	for (int i = 0; i < 24; i++)
	{
		if (gameState.x > DISPLAY_WIDTH - CHEST_SPACING * 2)
		{
			gameState.yCount += CHEST_SPACING;
			gameState.xCount = 0;
			gameState.x = (CHEST_SPACING * gameState.xCount) + gameState.offsetX;
		}

		gameState.x = (CHEST_SPACING * gameState.xCount) + gameState.offsetX;
		gameState.y = gameState.yCount + gameState.offsetY;
		Play::CreateGameObject(TYPE_CHEST, { gameState.x, gameState.y }, 10, "box");
		gameState.xCount++;
	}

}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate(float elapsedTime)
{
	
	UpdateBall();
	UpdatePaddle();
	ResetBall();
	UpdatePlayerControls();
	Draw();

	return Play::KeyDown(VK_ESCAPE);
}

// Gets called once when the player quits the game 
int MainGameExit(void)
{
	Play::DestroyManager();
	return PLAY_OK; 
}

// Our draw function. Called by MainGameUpdate to render each frame. 
void Draw()
{
	// Reset our drawing buffer so it is white
	Play::ClearDrawingBuffer(Play::cWhite);

	Play::DrawBackground();

	// Draw the 'paddle'
	Play::DrawObject(Play::GetGameObjectByType(TYPE_PADDLE));

	// Draw the AABB box for our paddle
	GameObject& paddleObj{ Play::GetGameObjectByType(TYPE_PADDLE) };
	Play::DrawRect(paddleObj.pos - PADDLE_AABB, paddleObj.pos + PADDLE_AABB, Play::cWhite);

	// Draw the ball. This version of the function is slower, but uses the rotation variable stored in GameObjects.
	Play::DrawObjectRotated(Play::GetGameObjectByType(TYPE_BALL));

	std::vector<int> chestIds{ Play::CollectGameObjectIDsByType(TYPE_CHEST) };

	for (int chest : chestIds)
	{
		Play::DrawObject(Play::GetGameObject(chest));
	}

	GameObject& ballObj{ Play::GetGameObjectByType(TYPE_BALL) };
	//Play::DrawRect(ballObj.pos - BALL_AABB, ballObj.pos + BALL_AABB, Play::cWhite);

	float velocity = ballObj.velocity.x;
	float acceleration = ballObj.velocity.y;

	// Example Code for drawing text with an integer value to the screen
	int val{0};
	Play::DrawFontText("64px", "High Score: " + std::to_string(val), Point2D(DISPLAY_WIDTH - 150, DISPLAY_HEIGHT - 100), Play::CENTRE);
	Play::DrawFontText("64px", "Collisions: " + std::to_string(gameState.collisionCount), Point2D(DISPLAY_WIDTH - 150, DISPLAY_HEIGHT - 200), Play::CENTRE);
	Play::DrawFontText("64px", "Reversed: " + std::to_string(reversed), Point2D(DISPLAY_WIDTH - 150, DISPLAY_HEIGHT - 300), Play::CENTRE);
	Play::DrawFontText("64px", "Velocity x: " + std::to_string(velocity), Point2D(DISPLAY_WIDTH - 150, DISPLAY_HEIGHT - 400), Play::CENTRE);
	Play::DrawFontText("64px", "Velocity y: " + std::to_string(acceleration), Point2D(DISPLAY_WIDTH - 150, DISPLAY_HEIGHT - 500), Play::CENTRE);
	
	// 'Paste' our drawing buffer to the visible screen so we can see it
	Play::PresentDrawingBuffer();
}


void UpdateBall()
{
	GameObject& ballObj{ Play::GetGameObjectByType(TYPE_BALL) };
	GameObject& paddleObj{ Play::GetGameObjectByType(TYPE_PADDLE) };

	float minX = paddleObj.pos.x - PADDLE_AABB.x ;
	float maxX = paddleObj.pos.x + PADDLE_AABB.x ;

	float minY = paddleObj.pos.y - PADDLE_AABB.y ;
	float maxY = paddleObj.pos.y + PADDLE_AABB.y ;

	float ballMinX = ballObj.pos.x - BALL_AABB.x;
	float ballMaxX = ballObj.pos.x + BALL_AABB.x;

	float ballMinY = ballObj.pos.y - BALL_AABB.y;
	float ballMaxY = ballObj.pos.y + BALL_AABB.y;

	ballObj.acceleration.y = std::clamp(ballObj.acceleration.y, 0.f, 0.1f);

	if (ballObj.velocity.y > 9.0f)
	{
		ballObj.velocity = { 5.0f, 8.0f };
	}

	
	//ballObj.rotation += ballObj.velocity.x * 0.01f;
	ballObj.rotSpeed = ballObj.velocity.x * 0.01f;

	if (ballObj.pos.x < 0 || ballObj.pos.x > DISPLAY_WIDTH)
	{
		ballObj.pos.x = std::clamp(ballObj.pos.x, 0.f, DISPLAY_WIDTH);
		ballObj.velocity.x *= -0.9f;
		ballObj.velocity.y *= 0.9f;
	}

	if (ballObj.pos.y < 0)
	{
		ballObj.pos.y = std::clamp(ballObj.pos.y, 0.f, DISPLAY_HEIGHT);
		ballObj.acceleration *= -1;
		ballObj.velocity.y *= -1;
	}

	if (ballObj.pos.y > DISPLAY_HEIGHT)
	{
		ballObj.pos.y = std::clamp(ballObj.pos.y, 0.f, DISPLAY_HEIGHT);
		ballObj.acceleration *= -1;
		ballObj.velocity.y *= -1;
	}

	if (IsBallColliding())
	{
		if ( ballObj.oldPos.y < minY && ballObj.oldPos.x > minX && ballObj.oldPos.x < maxX)
		{
			reversed = 1;
			ballObj.acceleration *= -1;
			ballObj.velocity.y *= -1.1;
			ballObj.velocity.x *= 1.1;
		}
		else if (ballObj.oldPos.x < minX && ballObj.oldPos.y > minY && ballObj.oldPos.y < maxY)
		{
			reversed = 2;
			ballObj.acceleration *= -1;
			ballObj.velocity.x *= -1.1;
			ballObj.velocity.y *= 1.1;
		}
		else if (ballObj.oldPos.x > minX && ballObj.oldPos.x < maxX && ballObj.oldPos.y > maxY)
		{
			reversed = 3;
			ballObj.acceleration *= -1;
			ballObj.velocity.y *= -1.1;
			ballObj.velocity.x *= 1.1;
		}
		else if (ballObj.oldPos.x > maxX && ballObj.oldPos.y > minY && ballObj.oldPos.y < maxY)
		{
			reversed = 4;
			ballObj.acceleration *= -1;
			ballObj.velocity.x *= -1.1;
			ballObj.velocity.y *= 1.1;
		}
		else
		{
			reversed = 5;
			ballObj.velocity *= -1.1;
			ballObj.acceleration *= -1;
		}
	}


	if (IsBallCollidingChest())
	{

		if (ballObj.oldPos.y < ballMinY && ballObj.oldPos.x > ballMinX && ballObj.oldPos.x < ballMaxX)
		{
			ballObj.acceleration *= -1;
			ballObj.velocity.y *= -1;
			//ballObj.velocity.x *= 1.05;
		}
		else if (ballObj.oldPos.x < ballMinX && ballObj.oldPos.y > ballMinY && ballObj.oldPos.y < ballMaxY)
		{
			ballObj.acceleration *= -1;
			ballObj.velocity.y *= -1.;
			//ballObj.velocity.x *= 1.05;
		}
		else if (ballObj.oldPos.x > ballMinX && ballObj.oldPos.x < ballMaxX && ballObj.oldPos.y > ballMaxY)
		{
			ballObj.acceleration *= -1;
			ballObj.velocity.y *= -1;
		}
		else if (ballObj.oldPos.x > ballMaxX && ballObj.oldPos.y > ballMinY && ballObj.oldPos.y < ballMaxY)
		{
			ballObj.acceleration *= -1;
			ballObj.velocity.y *= -1.;
		}
		else
		{
			ballObj.velocity *= -1;
			ballObj.acceleration *= -1;
		}
	}


	Play::UpdateGameObject(ballObj);
}


void UpdatePaddle()
{
	GameObject& paddleObj{ Play::GetGameObjectByType(TYPE_PADDLE) };

	Play::UpdateGameObject(paddleObj);
}


void ResetBall()
{

	if (Play::KeyDown(VK_SPACE))
	{
		GameObject& ballObj{ Play::GetGameObjectByType(TYPE_BALL) };
		ballObj.pos = Vector2D(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2);
		ballObj.velocity = BALL_VELOCITY_DEFAULT;
	}
}

bool IsBallColliding()
{
	GameObject& ballObj{ Play::GetGameObjectByType(TYPE_BALL) };
	GameObject& paddleObj{ Play::GetGameObjectByType(TYPE_PADDLE) };

	if ( ballObj.pos.y - BALL_AABB.y < paddleObj.pos.y + PADDLE_AABB.y &&
		ballObj.pos.y + BALL_AABB.y > paddleObj.pos.y - PADDLE_AABB.y )

	{
		if (ballObj.pos.x + BALL_AABB.x > paddleObj.pos.x - PADDLE_AABB.x &&
			ballObj.pos.x - BALL_AABB.x < paddleObj.pos.x + PADDLE_AABB.x)
		{
			gameState.collisionCount++;
			return true;
		}
	}
	return false;
}

bool IsBallCollidingChest()
{
	GameObject& ballObj{ Play::GetGameObjectByType(TYPE_BALL) };

	std::vector<int> chestIds{ Play::CollectGameObjectIDsByType(TYPE_CHEST) };

	for (int chest : chestIds)
	{
		GameObject& chestObj{ Play::GetGameObject(chest) };
		if (ballObj.pos.y - BALL_AABB.y < chestObj.pos.y + CHEST_AABB.y &&
			ballObj.pos.y + BALL_AABB.y > chestObj.pos.y - CHEST_AABB.y)

		{
			if (ballObj.pos.x + BALL_AABB.x > chestObj.pos.x - CHEST_AABB.x &&
				ballObj.pos.x - BALL_AABB.x < chestObj.pos.x + CHEST_AABB.x)
			{
				chestObj.type = TYPE_DESTROYED;
				//Play::DestroyGameObject(chest);
				gameState.collisionCount++;
				return true;
			}
		}
	}
	return false;
}

void UpdatePlayerControls()
{
	if (Play::KeyDown(VK_LEFT))
	{
		GameObject& paddleObj{ Play::GetGameObjectByType(TYPE_PADDLE) };
		paddleObj.pos.x -= 5;
	}

	if (Play::KeyDown(VK_RIGHT))
	{
		GameObject& paddleObj{ Play::GetGameObjectByType(TYPE_PADDLE) };
		paddleObj.pos.x += 5;
	}
}

void UpdateChests()
{
	std::vector<int> destroyedIds{ Play::CollectGameObjectIDsByType(TYPE_DESTROYED) };

	for (int destroyed : destroyedIds)
	{
		GameObject& chestObj{ Play::GetGameObject(destroyed) };

		Play::DestroyGameObject(destroyed);
	}
}

	