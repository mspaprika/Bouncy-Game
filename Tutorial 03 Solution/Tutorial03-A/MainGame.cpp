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

float minY = 0.f;
float maxY = 0.f;

float minX = 0.f;
float maxX = 0.f;

const int CHEST_SPACING{ 90 };

enum GameObjectType
{
	TYPE_BALL = 0,
	TYPE_PADDLE = 1,
	TYPE_CHEST = 2,
	TYPE_DESTROYED = 3,
};

enum GameFlow
{
	STATE_PLAY = 0,
	STATE_GAMEOVER = 1,
	STATE_PAUSED = 2,
	STATE_WON = 3,
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
	int lives = 3;
	int score = 0;
	GameFlow state = STATE_PLAY;
	
};

GameState gameState;

void DrawGameOver();
void DrawGamePlay();
void DrawPaused();
void DrawGameWon();

void StartGame();
void RestartAndRestore();

void UpdateBall();
void UpdatePaddle();
void ResetBall();
void UpdatePlayerControls();
void UpdateDestroyed();

void RestartGame();
void DestroyObjects();

bool IsBallColliding(GameObject& object);

void CalculateMinMax(const GameObject& object, float& minX, float& maxX, float& minY, float& maxY);
void RedirectBall(const GameObject& object);
void AdjustBallAndPaddle();

// The entry point for a PlayBuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	// Setup PlayBuffer
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	Play::CentreAllSpriteOrigins(); // this function makes it so that obj.pos values represent the center of a sprite instead of its top-left corner

	StartGame();
	DrawGamePlay();			
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate(float elapsedTime)
{
	switch (gameState.state)
	{
		case STATE_PLAY:
		{
			UpdateBall();
			UpdatePaddle();
			//ResetBall();
			UpdatePlayerControls();			
			UpdateDestroyed();
			DrawGamePlay();
			break;
		}
		case STATE_GAMEOVER:
		{
			DrawGameOver();

			if (Play::KeyDown(VK_SPACE))
			{
				RestartAndRestore();
			}
			break;
		}
		case STATE_WON:
		{
			DrawGameWon();

			if (Play::KeyDown(VK_SPACE))
			{
				RestartAndRestore();
			 }
			break;
		}
		case STATE_PAUSED:
		{
			DrawPaused();

			if (Play::KeyDown(VK_SPACE))
			{	
				gameState.state = STATE_PLAY;
			}
			if (Play::KeyDown(VK_TAB))
			{
				RestartAndRestore();
			}
			break;
		}
	}
	
	return Play::KeyDown(VK_ESCAPE);
}

// Gets called once when the player quits the game 
int MainGameExit(void)
{
	Play::DestroyManager();
	return PLAY_OK; 
}

void StartGame()
{
	// Create a ball object and a paddle object
	Play::CreateGameObject(TYPE_BALL, { (DISPLAY_WIDTH / 2) - 200, DISPLAY_HEIGHT / 2 }, BALL_RADIUS, "ball");
	Play::CreateGameObject(TYPE_PADDLE, { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 100 }, BALL_RADIUS, "spanner");
	// ... "ball", "spanner" etc. are the filenames of sprites stored in the Data folder alongside this solution, you can put any .PNGs in there if you feel creative :)

	// Set initial velocity for ball
	GameObject& ballObj = Play::GetGameObjectByType(TYPE_BALL);
	ballObj.velocity = BALL_VELOCITY_DEFAULT;
	ballObj.acceleration = BALL_ACCELERATION;

	int j{ -1 };


	for (int i = 0; i < 13; i++)
	{
		if (gameState.x > DISPLAY_WIDTH - CHEST_SPACING * 2)
		{
			gameState.yCount += CHEST_SPACING;
			gameState.xCount = 0;
			j = 0;
			gameState.x = (CHEST_SPACING * gameState.xCount) + gameState.offsetX;
		}

		j++;
		gameState.x = (CHEST_SPACING * gameState.xCount) + gameState.offsetX + CHEST_SPACING * j;
		gameState.y = gameState.yCount + gameState.offsetY;
		Play::CreateGameObject(TYPE_CHEST, { gameState.x, gameState.y }, 10, "box");
		gameState.xCount++;
	}
}

void RestartAndRestore()
{
	RestartGame();
	DrawGamePlay();
	gameState.lives = 3;
	gameState.state = STATE_PLAY;
}

void DrawPaused()
{
	Play::ClearDrawingBuffer(Play::cWhite);
	Play::DrawBackground();
	Play::DrawFontText("64px", "PAUSED", Point2D(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2), Play::CENTRE);
	Play::DrawFontText("64px", "Press Space to Continue", Point2D(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 + 100), Play::CENTRE);
	Play::DrawFontText("64px", "Press TAB to Restart", Point2D(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 + 200), Play::CENTRE);
	Play::PresentDrawingBuffer();
}

void DrawGameWon()
{
	Play::ClearDrawingBuffer(Play::cWhite);
	Play::DrawBackground();
	Play::DrawFontText("64px", "YOU WON", Point2D(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2), Play::CENTRE);
	Play::DrawFontText("64px", "Press Space to Restart", Point2D(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 + 100), Play::CENTRE);
	Play::DrawFontText("64px", "Highscore: " + std::to_string(gameState.score), Point2D(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 + 200), Play::CENTRE);
	Play::PresentDrawingBuffer();
}

void DrawGameOver()
{
	Play::ClearDrawingBuffer(Play::cWhite);
	Play::DrawBackground();
	Play::DrawFontText("64px", "GAME OVER", Point2D(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2), Play::CENTRE);
	Play::DrawFontText("64px", "Press Space to Restart", Point2D(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 + 100), Play::CENTRE);
	Play::PresentDrawingBuffer();
}

void DrawGamePlay()
{
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

	Play::DrawFontText("64px", "High Score: " + std::to_string(gameState.score), Point2D(DISPLAY_WIDTH - 150, DISPLAY_HEIGHT - 100), Play::CENTRE);
	Play::DrawFontText("64px", "Lives: " + std::to_string(gameState.lives), Point2D(DISPLAY_WIDTH - 150, DISPLAY_HEIGHT - 200), Play::CENTRE);
	Play::DrawFontText("64px", "Collisions: " + std::to_string(gameState.collisionCount), Point2D(DISPLAY_WIDTH - 150, DISPLAY_HEIGHT - 300), Play::CENTRE);
	Play::DrawFontText("64px", "Velocity x: " + std::to_string(velocity), Point2D(DISPLAY_WIDTH - 150, DISPLAY_HEIGHT - 400), Play::CENTRE);
	Play::DrawFontText("64px", "Velocity y: " + std::to_string(acceleration), Point2D(DISPLAY_WIDTH - 150, DISPLAY_HEIGHT - 500), Play::CENTRE);

	Play::PresentDrawingBuffer();
}

void UpdateBall()
{
	GameObject& ballObj{ Play::GetGameObjectByType(TYPE_BALL) };
	GameObject& paddleObj{ Play::GetGameObjectByType(TYPE_PADDLE) };
	
	ballObj.acceleration.y = std::clamp(ballObj.acceleration.y, 0.f, 0.1f);

	if (ballObj.velocity.y > 9.0f)
	{
		ballObj.velocity = { 5.0f, 8.0f };
	}

	ballObj.rotSpeed = ballObj.velocity.x * 0.01f;

	if (ballObj.pos.x < 0 || ballObj.pos.x > DISPLAY_WIDTH)
	{
		ballObj.pos.x = std::clamp(ballObj.pos.x, 0.f, DISPLAY_WIDTH);
		ballObj.velocity.x *= -0.9f;
		ballObj.velocity.y *= 0.9f;
	}

	if (ballObj.pos.y > DISPLAY_HEIGHT)
	{
		gameState.lives--;
		if (gameState.lives > 0)
		{
			DestroyObjects();
			RestartGame();
			DrawGamePlay();
		}
		else
		{
			gameState.state = STATE_GAMEOVER;
		}
	}

	if (ballObj.pos.y < 0)
	{
		ballObj.pos.y = std::clamp(ballObj.pos.y, 0.f, DISPLAY_HEIGHT);
		ballObj.acceleration *= -1;
		ballObj.velocity.y *= -1;
	}

	CalculateMinMax(paddleObj, minX, maxX, minY, maxY);

	if (IsBallColliding(paddleObj))
	{
		RedirectBall(paddleObj);
	}

	std::vector<int> chestIds{ Play::CollectGameObjectIDsByType(TYPE_CHEST) };

	if (chestIds.size() == 0)
	{
		gameState.state = STATE_WON;
	}

	for (int chest : chestIds)
	{
		GameObject& chestObj{ Play::GetGameObject(chest) };

		CalculateMinMax(chestObj, minX, maxX, minY, maxY);

		if (IsBallColliding(chestObj))
		{
			RedirectBall(chestObj);		
			chestObj.type = TYPE_DESTROYED;
		}
	}
	Play::UpdateGameObject(ballObj);
}

void CalculateMinMax(const GameObject& object, float& minX, float& maxX, float& minY, float& maxY)
{
	switch (object.type)
	{
		case TYPE_PADDLE:
		{
			minX = object.pos.x - PADDLE_AABB.x;
			maxX = object.pos.x + PADDLE_AABB.x;

			minY = object.pos.y - PADDLE_AABB.y;
			maxY = object.pos.y + PADDLE_AABB.y;

			break;
		}
		case TYPE_CHEST:
		{
			minX = object.pos.x - CHEST_AABB.x;
			maxX = object.pos.x + CHEST_AABB.x;

			minY = object.pos.y - CHEST_AABB.y;
			maxY = object.pos.y + CHEST_AABB.y;

			break;
		}
	}
}

void RedirectBall(const GameObject& object)
{
	GameObject& ball{ Play::GetGameObjectByType(TYPE_BALL) };

	float velocityChange = 0.0f;
	float yChange = 0.0f;

	if (object.type == TYPE_PADDLE)
	{
		velocityChange = 1.1f;
		yChange = 1.0f;
	}
	else if (object.type == TYPE_CHEST)
	{
		velocityChange = 1.0f;
		yChange = -1.0f;
	}
		
	if (ball.oldPos.y < minY && ball.oldPos.x > minX && ball.oldPos.x < maxX)
	{
		ball.acceleration *= -1;
		ball.velocity.y *= velocityChange * (-1);
		ball.velocity.x *= velocityChange;
	}
	else if (ball.oldPos.x < minX && ball.oldPos.y > minY && ball.oldPos.y < maxY)
	{
		AdjustBallAndPaddle();

		ball.acceleration *= -1;
		ball.velocity.x *= velocityChange * (-1);
		ball.velocity.y *= velocityChange * yChange;
	}
	else if (ball.oldPos.x > minX && ball.oldPos.x < maxX && ball.oldPos.y > maxY)
	{
		ball.acceleration *= -1;
		ball.velocity.y *= velocityChange * (-1);
		ball.velocity.x *= velocityChange;
	}
	else if (ball.oldPos.x > maxX && ball.oldPos.y > minY && ball.oldPos.y < maxY)
	{	
		AdjustBallAndPaddle();

		ball.acceleration *= -1;
		ball.velocity.x *= velocityChange * (-1);
		ball.velocity.y *= velocityChange * yChange;
	}
	else
	{
		AdjustBallAndPaddle();

		ball.velocity *= velocityChange * (-1);
		ball.velocity.x *= yChange;
		ball.acceleration *= -1;
	}
}

void AdjustBallAndPaddle()
{
	GameObject& ballObj{ Play::GetGameObjectByType(TYPE_BALL) };
	GameObject& paddleObj{ Play::GetGameObjectByType(TYPE_PADDLE) };

	if (paddleObj.pos.x > paddleObj.oldPos.x && ballObj.pos.x < ballObj.oldPos.x
		|| paddleObj.pos.x < paddleObj.oldPos.x && ballObj.pos.x > ballObj.oldPos.x)
	{
		paddleObj.pos = paddleObj.oldPos;
		ballObj.pos = ballObj.oldPos;
		ballObj.velocity.x *= -1;
	}	
}

void UpdatePaddle()
{
	GameObject& paddleObj{ Play::GetGameObjectByType(TYPE_PADDLE) };
	paddleObj.pos.x = std::clamp (paddleObj.pos.x, PADDLE_AABB.x, DISPLAY_WIDTH - PADDLE_AABB.x);
	Play::UpdateGameObject(paddleObj);
}

void ResetBall()
{
	if (Play::KeyDown(VK_SPACE))
	{
		GameObject& ballObj{ Play::GetGameObjectByType(TYPE_BALL) };
		ballObj.pos = Vector2D(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2);
		ballObj.velocity = BALL_VELOCITY_DEFAULT;
		ballObj.acceleration = BALL_ACCELERATION;
	}
}

bool IsBallColliding(GameObject& object)
{
	GameObject& ballObj{ Play::GetGameObjectByType(TYPE_BALL) };
	Vector2D AABB{ 0, 0 };

	if (object.type == TYPE_PADDLE)
	{
		AABB = PADDLE_AABB;
	}
	else if (object.type == TYPE_CHEST)
	{
		AABB = CHEST_AABB;
	}

	if ( ballObj.pos.y - BALL_AABB.y < object.pos.y + AABB.y &&
		ballObj.pos.y + BALL_AABB.y > object.pos.y - AABB.y )

	{
		if (ballObj.pos.x + BALL_AABB.x > object.pos.x - AABB.x &&
			ballObj.pos.x - BALL_AABB.x < object.pos.x + AABB.x)
		{
			gameState.collisionCount++;

			if (object.type == TYPE_CHEST)
			{
				gameState.score++;
			}
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
		paddleObj.pos.x -= 10;
	}

	if (Play::KeyDown(VK_RIGHT))
	{
		GameObject& paddleObj{ Play::GetGameObjectByType(TYPE_PADDLE) };
		paddleObj.pos.x += 10;
	}

	if (Play::KeyDown(VK_SHIFT))
	{
		gameState.state = STATE_PAUSED;
	}
}

void UpdateDestroyed()
{
	std::vector<int> destroyedIds{ Play::CollectGameObjectIDsByType(TYPE_DESTROYED) };

	for (int destroyed : destroyedIds)
	{
		Play::DestroyGameObject(destroyed);
	}
}

void RestartGame()
{
	//gameState.lives = 3;
	gameState.score = 0;
	gameState.collisionCount = 0;
	gameState.xCount = 0;
	gameState.yCount = 0;
	gameState.x = 0;
	gameState.y = 0;

	GameObject& ballObj{ Play::GetGameObjectByType(TYPE_BALL) };

	ballObj.pos = Vector2D((DISPLAY_WIDTH / 2) - 200, DISPLAY_HEIGHT / 2);
	ballObj.velocity = BALL_VELOCITY_DEFAULT;
	ballObj.acceleration = BALL_ACCELERATION;

	GameObject& paddleObj{ Play::GetGameObjectByType(TYPE_PADDLE) };
	paddleObj.pos = Vector2D(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 100);
	
	int j{ -1 };

	DestroyObjects();
	StartGame();
}

void DestroyObjects()
{
	std::vector<int> chestIds{ Play::CollectGameObjectIDsByType(TYPE_CHEST) };

	for (int chest : chestIds)
	{
		Play::DestroyGameObject(chest);
	}

	Play::DestroyGameObjectsByType(TYPE_PADDLE);
	Play::DestroyGameObjectsByType(TYPE_BALL);
}

	