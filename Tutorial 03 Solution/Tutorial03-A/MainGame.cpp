#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

constexpr float DISPLAY_WIDTH{ 1280 };
constexpr float DISPLAY_HEIGHT{ 720 };
constexpr int DISPLAY_SCALE{ 1 };
constexpr int BALL_RADIUS{ 48 };

// Define a value for our ball's default velocity
const Vector2D BALL_VELOCITY_DEFAULT(5.0f, 0.f);

const Vector2D BALL_ACCELERATION(0.f, 0.5f);

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
};

GameState gameState;


// Forward-declaration of Draw

void Draw();

void UpdateBall();
void UpdatePaddle();
void ResetBall();
void UpdatePlayerControls();

bool IsBallColliding();


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

	// Example Code for drawing text with an integer value to the screen
	int val{0};
	Play::DrawFontText("64px", "High Score: " + std::to_string(val), Point2D(DISPLAY_WIDTH - 150, DISPLAY_HEIGHT - 100), Play::CENTRE);
	
	// 'Paste' our drawing buffer to the visible screen so we can see it
	Play::PresentDrawingBuffer();
}


void UpdateBall()
{
	GameObject& ballObj{ Play::GetGameObjectByType(TYPE_BALL) };

	ballObj.velocity += ballObj.acceleration;
	ballObj.pos += ballObj.velocity;

	ballObj.acceleration.y = std::clamp(ballObj.acceleration.y, 0.f, 0.1f);

	if (ballObj.pos.x < 0 || ballObj.pos.x > DISPLAY_WIDTH)
	{
		ballObj.pos.x = std::clamp(ballObj.pos.x, 0.f, DISPLAY_WIDTH);
		ballObj.velocity.x *= -1;
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
		ballObj.pos.y = std::clamp(ballObj.pos.y, 0.f, DISPLAY_HEIGHT);
		ballObj.acceleration *= -1;
		ballObj.velocity.y *= -1;
	}
}


void UpdatePaddle()
{
	GameObject& paddleObj{ Play::GetGameObjectByType(TYPE_PADDLE) };
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
				return true;
			}
		}
	}

	if ( ballObj.pos.y - BALL_AABB.y < paddleObj.pos.y + PADDLE_AABB.y &&
		ballObj.pos.y + BALL_AABB.y > paddleObj.pos.y - PADDLE_AABB.y )

	{
		if (ballObj.pos.x + BALL_AABB.x > paddleObj.pos.x - PADDLE_AABB.x &&
			ballObj.pos.x - BALL_AABB.x < paddleObj.pos.x + PADDLE_AABB.x)
		{
			return true;
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

	