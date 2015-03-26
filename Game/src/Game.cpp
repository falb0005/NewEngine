#include "Game.h"
#include <GameObject.h>
#include <SDL.h>
#include <math.h>
#include <SDL_image.h>
#include <SDL_opengl.h>
#include <InputManager.h>

#include "Cube.h"
#include "Enemy.h"
#include <Cameras/Camera.h>
#include <Cameras/PerspectiveCamera.h>
#include <Cameras/OrthographicCamera.h>
#include <Windows.h>
#include <time.h>

// Initializing our static member pointer.
GameEngine* GameEngine::_instance = nullptr;

GameEngine* GameEngine::CreateInstance()
{
	if (_instance == nullptr)
	{
		_instance = new Game();
	}
	return _instance;
}

Game::Game() : GameEngine()
{
	_windowString = (char*)malloc(sizeof(char)* 100);
}

Game::~Game()
{
	delete(_playerCube);
	//free visited cubes memory
	for (int i = 0; i < _gridHeight; i++)
	{
		free(_visitedCubes[i]);
	}
	free(_visitedCubes);
	for (int i = 0; i < _gridHeight; i++)
	{
		delete[](_worldCubes[i]);
	}
	delete[](_worldCubes);
	delete[](_enemies);
	Mix_FreeChunk(_moveSound);
	Mix_FreeChunk(_dieSound);
	Mix_FreeChunk(_clearLevelSound);
	Mix_FreeChunk(_visitedNewBlockSound);
	Mix_FreeChunk(_enemySpawnSound);
	Mix_FreeChunk(_enemyMovementSound);
	Mix_CloseAudio();
	Mix_Quit();
	free(_windowString);
}

void Game::InitializeImpl(Graphics *graphics)
{
	srand(time(NULL));

	//initialize array of enemies
	_numEnemies = 10;

	_enemyDeployRate = 5;//deploy every 5 seconds

	_enemyMovementSpeed = 1;//move down every second

	_enemies = new Enemy[_numEnemies]();
	for (int i = 0; i < _numEnemies; i++)
	{
		_enemies[i].Initialize(graphics);
		_enemies[i].GetTransform().position = Vector3(0, 0, 0);
	}

	//set too zero since no enemies have spawned yet
	_timeSinceLastEnemySpawn = 0;
	_timeSinceLastEnemyMoveMent = 0;

	//set player starting score and lives
	_playerScore = 0;
	_playerLives = 5;

	//initial player game grid position
	_playerGridPos.x = 0;
	_playerGridPos.y = 0;

	//set initial window title
	sprintf_s(_windowString, 80, "Cubert   Score: %d   Lives: %d", _playerScore, _playerLives);
	SDL_SetWindowTitle(_window, _windowString);

	//initialize player
	_playerCube = new Cube();
	_playerCube->Initialize(graphics);
	_playerCube->SetVertex(0, /*pos*/-0.5f, 0.5f, 0.5f,  /*color*/ 1.0f, 1.0f, 0.0f, 1.0f);
	_playerCube->SetVertex(1, /*pos*/0.5f, 0.5f, 0.5f,   /*color*/ 1.0f, 1.0f, 0.0f, 1.0f);
	_playerCube->SetVertex(2, /*pos*/-0.5f, -0.5f, 0.5f, /*color*/ 1.0f, 1.0f, 0.0f, 1.0f);
	_playerCube->SetVertex(3, /*pos*/0.5f, -0.5f, 0.5f,  /*color*/ 1.0f, 1.0f, 0.0f, 1.0f);
	_playerCube->SetVertex(4, /*pos*/-0.5f, 0.5f, -0.5f, /*color*/ 1.0f, 1.0f, 0.0f, 1.0f);
	_playerCube->SetVertex(5, /*pos*/0.5f, 0.5f, -0.5f,  /*color*/ 1.0f, 1.0f, 0.0f, 1.0f);
	_playerCube->SetVertex(6, /*pos*/-0.5f, -0.5f, -0.5f,/*color*/ 1.0f, 1.0f, 0.0f, 1.0f);
	_playerCube->SetVertex(7, /*pos*/0.5f, -0.5f, -0.5f, /*color*/ 1.0f, 1.0f, 0.0f, 1.0f);
	_playerCube->GetTransform().position = Vector3(0, 1, 0);

	//load audio
	int channel;
	Uint16 audio_format = AUDIO_U8;
	int audio_channels = 2;
	int audio_buffers = 4096;
	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, audio_format, audio_channels, audio_buffers) != 0) {
		printf("Unable to initialize audio: %s\n", Mix_GetError());
		Sleep(2000);
		exit(EXIT_FAILURE);
	}
	_moveSound = Mix_LoadWAV("res/boing2.wav");
	if (!_moveSound) {
		printf("Mix_LoadWAV: %s\n", Mix_GetError());
		Sleep(2000);
		exit(EXIT_FAILURE);
	}
	_dieSound = Mix_LoadWAV("res/car_crash.wav");
	if (!_dieSound) {
		printf("Mix_LoadWAV: %s\n", Mix_GetError());
		Sleep(2000);
		exit(EXIT_FAILURE);
	}
	_clearLevelSound = Mix_LoadWAV("res/applause.wav");
	if (!_clearLevelSound) {
		printf("Mix_LoadWAV: %s\n", Mix_GetError());
		Sleep(2000);
		exit(EXIT_FAILURE);
	}
	_visitedNewBlockSound = Mix_LoadWAV("res/bottle_x.wav");
	if (!_visitedNewBlockSound) {
		printf("Mix_LoadWAV: %s\n", Mix_GetError());
		Sleep(2000);
		exit(EXIT_FAILURE);
	}
	_enemySpawnSound = Mix_LoadWAV("res/bowling.wav");
	if (!_enemySpawnSound) {
		printf("Mix_LoadWAV: %s\n", Mix_GetError());
		Sleep(2000);
		exit(EXIT_FAILURE);
	}
	_enemyMovementSound = Mix_LoadWAV("res/cannon_x.wav");
	if (!_enemyMovementSound) {
		printf("Mix_LoadWAV: %s\n", Mix_GetError());
		Sleep(2000);
		exit(EXIT_FAILURE);
	}

	//initialize world grid
	_gridHeight = 4;
	_gridWidth = _gridHeight;

	//create arrays of ints used for boolean logic wether or not cubes have been visited
	_visitedCubes = (int**)malloc(sizeof(int*)* _gridHeight);
	for (int i = 0; i < _gridHeight; i++)
	{
		_visitedCubes[i] = (int*)malloc(sizeof(int)* _gridWidth);
		for (int x = 0; x < _gridWidth; x++)
		{
			_visitedCubes[i][x] = 0;
		}
	}

	//initialize camera
	float nearPlane = 0.01f;
	float farPlane = 100.0f;
	Vector4 position(3.5f, 2.5f, 5.5f, 0.0f);
	Vector4 lookAt = Vector4::Normalize(Vector4::Difference(Vector4(0.0f, 0.0f, 0.0f, 0.0f), position));
	Vector4 up(0.0f, 1.0f, 0.0f, 0.0f);

	//_camera = new PerspectiveCamera(50.0f, 1.0f, nearPlane, farPlane, position, lookAt, up);
	_camera = new OrthographicCamera(-10.0f, 10.0f, 10.0f, -10.0f, nearPlane, farPlane, position, lookAt, up);


	//initialize 2 dimensional array of world cubes

	//allocate space on the heap to hold pointers to the world cubes
	_worldCubes = (Cube**)malloc(sizeof(Cube*)* _gridHeight);

	//initialize world cubes
	int index = 0;
	for (int gridX = 0; gridX < _gridHeight; gridX++)
	{
		_worldCubes[gridX] = new Cube[(int)_gridWidth]();
		for (int gridZ = 0; gridZ < _gridWidth; gridZ++)
		{
			//world coordinates
			float worldX = gridX;
			float worldY = -(gridX + gridZ);
			float worldZ = gridZ;

			_worldCubes[gridX][gridZ].GetTransform().position = Vector3(worldX, worldY, worldZ);
			_worldCubes[gridX][gridZ].Initialize(graphics);
			index++;
		}
	}

	//set the world cube the player starts on to visited
	_worldCubes[0][0].SetVertex(0, /*pos*/-0.5f, 0.5f, 0.5f,  /*color*/ 1.0f, 1.0f, 0.0f, 1.0f);
	_worldCubes[0][0].SetVertex(1, /*pos*/0.5f, 0.5f, 0.5f,   /*color*/ 1.0f, 0.0f, 1.0f, 1.0f);
	_worldCubes[0][0].SetVertex(2, /*pos*/-0.5f, -0.5f, 0.5f, /*color*/ 1.0f, 0.0f, 0.0f, 1.0f);
	_worldCubes[0][0].SetVertex(3, /*pos*/0.5f, -0.5f, 0.5f,  /*color*/ 1.0f, 1.0f, 0.0f, 1.0f);
	_worldCubes[0][0].SetVertex(4, /*pos*/-0.5f, 0.5f, -0.5f, /*color*/ 0.0f, 0.0f, 1.0f, 1.0f);
	_worldCubes[0][0].SetVertex(5, /*pos*/0.5f, 0.5f, -0.5f,  /*color*/ 1.0f, 1.0f, 0.0f, 1.0f);
	_worldCubes[0][0].SetVertex(6, /*pos*/-0.5f, -0.5f, -0.5f,/*color*/ 0.0f, 0.0f, 1.0f, 1.0f);
	_worldCubes[0][0].SetVertex(7, /*pos*/0.5f, -0.5f, -0.5f, /*color*/ 1.0f, 1.0f, 0.0f, 1.0f);
}

void Game::UpdateImpl(Graphics * graphics, float dt)
{
	static float fps = 0;
	static float timeSinceLastFPS = 0;
	timeSinceLastFPS += dt;
	if (timeSinceLastFPS > 0.2f){
		fps = 1.0f / dt;
		timeSinceLastFPS = 0;
	}
	sprintf_s(_windowString, 80, "Cubert   Score: %d   Lives: %d   FPS: %f", _playerScore, _playerLives, fps);
	SDL_SetWindowTitle(_window, _windowString);
	if ((_playerGridPos.x < _gridHeight && _playerGridPos.y < _gridWidth) && (_playerGridPos.x > -1 && _playerGridPos.y > -1))
	{
		InputManager::GetInstance()->Update(dt);
		if (InputManager::GetInstance()->GetKeyState(SDLK_UP, SDL_KEYUP) == true)
		{
			_playerCube->GetTransform().position.y = _playerCube->GetTransform().position.y + 1;
			_playerCube->GetTransform().position.z = _playerCube->GetTransform().position.z - 1;
			_playerGridPos.y--;
			Mix_PlayChannel(-1, _moveSound, 0);
		}
		else if (InputManager::GetInstance()->GetKeyState(SDLK_DOWN, SDL_KEYUP) == true)
		{
			_playerCube->GetTransform().position.y = _playerCube->GetTransform().position.y - 1;
			_playerCube->GetTransform().position.z = _playerCube->GetTransform().position.z + 1;
			_playerGridPos.y++;
			Mix_PlayChannel(-1, _moveSound, 0);
		}
		else if (InputManager::GetInstance()->GetKeyState(SDLK_RIGHT, SDL_KEYUP) == true)
		{
			_playerCube->GetTransform().position.y = _playerCube->GetTransform().position.y - 1;
			_playerCube->GetTransform().position.x = _playerCube->GetTransform().position.x + 1;
			_playerGridPos.x++;
			Mix_PlayChannel(-1, _moveSound, 0);
		}
		else if (InputManager::GetInstance()->GetKeyState(SDLK_LEFT, SDL_KEYUP) == true)
		{
			_playerCube->GetTransform().position.y = _playerCube->GetTransform().position.y + 1;
			_playerCube->GetTransform().position.x = _playerCube->GetTransform().position.x - 1;
			_playerGridPos.x--;
			Mix_PlayChannel(-1, _moveSound, 0);
		}
	}
	else
	{
		Mix_PlayChannel(-1, _dieSound, 0);
		_playerLives -= 1;
		_playerGridPos.x = 0;
		_playerGridPos.y = 0;
		_playerCube->GetTransform().position = Vector3(0, 1, 0);
	}

	//deploy enemies
	_timeSinceLastEnemySpawn += dt;
	if (_timeSinceLastEnemySpawn > _enemyDeployRate)
	{
		DeployEnemy();
	}

	//move enemies
	_timeSinceLastEnemyMoveMent += dt;
	if (_timeSinceLastEnemyMoveMent > _enemyMovementSpeed)
	{
		for (int i = 0; i < _numEnemies; i++)
		{
			if (_enemies[i].GetIsAlive() == true)
			{
				_enemies[i].MoveDownGameWorld();
				Mix_PlayChannel(-1, _enemyMovementSound, 0);
			}
		}
		_timeSinceLastEnemyMoveMent = 0;
	}

	//check if enemies have fallen off
	for (int i = 0; i < _numEnemies; i++)
	{
		if (_enemies[i].GetIsAlive() == true)
		if (_enemies[i].GetGridPos().x >= _gridHeight || _enemies[i].GetGridPos().y >= _gridWidth)
			_enemies[i].SetIsAlive(false);
	}


	//update array of enemies
	for (int i = 0; i < _numEnemies; i++)
	{
		if (_enemies[i].GetIsAlive() == true)
			_enemies[i].Update(dt);
	}

	//check cubes if they are visited
	int allVisted = UpdateCubeVisitState();
	if (allVisted == 1)
	{
		NextGameLevel(graphics);
		allVisted = 0;
	}
	
	//check for player collisions with enemies
	CheckPlayerEnemyCollisions();

	if (_playerLives < 0)
	{
		ResetGame(graphics);
	}

	//update player
	_playerCube->Update(dt);

	for (int i = 0; i < _gridHeight; i++)
	{
		for (int x = 0; x < _gridWidth; x++)
		{
			_worldCubes[i][x].Update(dt);
		}
	}
}

void Game::DrawImpl(Graphics *graphics, float dt)
{
	std::vector<GameObject *> renderOrder = _objects;
	//CalculateDrawOrder(renderOrder);

	glPushMatrix();
	glTranslatef(0, 9, 0);
	CalculateCameraViewpoint();

	_playerCube->Draw(graphics, _camera->GetProjectionMatrix(), dt);

	for (int i = 0; i < _gridHeight; i++)
	{
		for (int x = 0; x < _gridWidth; x++)
		{
			_worldCubes[i][x].Draw(graphics, _camera->GetProjectionMatrix(), dt);
		}
	}

	//draw array of enemies
	for (int i = 0; i < _numEnemies; i++)
	{
		if (_enemies[i].GetIsAlive() == true)
			_enemies[i].Draw(graphics, _camera->GetProjectionMatrix(), dt);
	}
	glPopMatrix();
}

void Game::NextGameLevel(Graphics *graphics)
{
	Mix_PlayChannel(-1, _clearLevelSound, 0);
	//free visited cubes memory
	for (int i = 0; i < _gridHeight; i++)
	{
		free(_visitedCubes[i]);
	}
	free(_visitedCubes);

	//free world cube memory
	for (int i = 0; i < _gridHeight; i++)
	{
		delete[](_worldCubes[i]);
	}
	delete[](_worldCubes);

	//increment grid size
	if (_gridHeight < 9)
	{
		_gridHeight++;
		_gridWidth++;
	}
	else{
		if (_enemyDeployRate > 1)
		{
			_enemyDeployRate -= 1;
		}
		if (_enemyMovementSpeed > 0.1)
		{
			_enemyDeployRate -= 0.1;
		}
	}

	//set player game pos to origin
	_playerGridPos.x = 0;
	_playerGridPos.y = 0;

	//set player draw pos too start pos
	_playerCube->GetTransform().position = Vector3(0, 1, 0);

	//create arrays of chars used for bolean logic wether or not cubes have been visited
	_visitedCubes = (int**)malloc(sizeof(int*)* _gridHeight);
	for (int i = 0; i < _gridHeight; i++)
	{
		_visitedCubes[i] = (int*)malloc(sizeof(int)* _gridWidth);
		for (int x = 0; x < _gridWidth; x++)
		{
			_visitedCubes[i][x] = 0;
		}
	}

	//initialize array of world cubes

	//allocate space on the heap to hold pointers to the world cubes
	_worldCubes = (Cube**)malloc(sizeof(Cube*)* _gridHeight);
	int index = 0;
	for (int gridX = 0; gridX < _gridHeight; gridX++)
	{
		_worldCubes[gridX] = new Cube[(int)_gridWidth]();
		for (int gridZ = 0; gridZ < _gridWidth; gridZ++)
		{
			//world coordinates
			float worldX = gridX;
			float worldY = -(gridX + gridZ);
			float worldZ = gridZ;

			_worldCubes[gridX][gridZ].GetTransform().position = Vector3(worldX, worldY, worldZ);
			_worldCubes[gridX][gridZ].Initialize(graphics);
			index++;
		}
	}

	//set the world cube the player starts on to visited
	_worldCubes[0][0].SetVertex(0, /*pos*/-0.5f, 0.5f, 0.5f,  /*color*/ 1.0f, 1.0f, 0.0f, 1.0f);
	_worldCubes[0][0].SetVertex(1, /*pos*/0.5f, 0.5f, 0.5f,   /*color*/ 1.0f, 0.0f, 1.0f, 1.0f);
	_worldCubes[0][0].SetVertex(2, /*pos*/-0.5f, -0.5f, 0.5f, /*color*/ 1.0f, 0.0f, 0.0f, 1.0f);
	_worldCubes[0][0].SetVertex(3, /*pos*/0.5f, -0.5f, 0.5f,  /*color*/ 1.0f, 1.0f, 0.0f, 1.0f);
	_worldCubes[0][0].SetVertex(4, /*pos*/-0.5f, 0.5f, -0.5f, /*color*/ 0.0f, 0.0f, 1.0f, 1.0f);
	_worldCubes[0][0].SetVertex(5, /*pos*/0.5f, 0.5f, -0.5f,  /*color*/ 1.0f, 1.0f, 0.0f, 1.0f);
	_worldCubes[0][0].SetVertex(6, /*pos*/-0.5f, -0.5f, -0.5f,/*color*/ 0.0f, 0.0f, 1.0f, 1.0f);
	_worldCubes[0][0].SetVertex(7, /*pos*/0.5f, -0.5f, -0.5f, /*color*/ 1.0f, 1.0f, 0.0f, 1.0f);

	_playerLives++;
}

void Game::ResetGame(Graphics *graphics)
{
	Mix_PlayChannel(-1, _dieSound, 0);

	//reset enemy speeds to default
	_enemyMovementSpeed = 1;
	_enemyDeployRate = 5;

	//free visited cubes memory
	for (int i = 0; i < _gridHeight; i++)
	{
		free(_visitedCubes[i]);
	}
	free(_visitedCubes);

	//free world cube memory
	for (int i = 0; i < _gridHeight; i++)
	{
		delete[](_worldCubes[i]);
	}
	delete[](_worldCubes);

	_gridHeight = 4;
	_gridWidth = 4;

	//set player game pos to origin
	_playerGridPos.x = 0;
	_playerGridPos.y = 0;

	//set player lives and score too default
	_playerScore = 0;
	_playerLives = 5;

	//set player draw pos too start pos
	_playerCube->GetTransform().position = Vector3(0, 1, 0);

	//create arrays of chars used for bolean logic wether or not cubes have been visited
	_visitedCubes = (int**)malloc(sizeof(int*)* _gridHeight);
	for (int i = 0; i < _gridHeight; i++)
	{
		_visitedCubes[i] = (int*)malloc(sizeof(int)* _gridWidth);
		for (int x = 0; x < _gridWidth; x++)
		{
			_visitedCubes[i][x] = 0;
		}
	}

	//initialize array of world cubes

	//allocate space on the heap to hold pointers to the world cubes
	_worldCubes = (Cube**)malloc(sizeof(Cube*)* _gridHeight);
	int index = 0;
	for (int gridX = 0; gridX < _gridHeight; gridX++)
	{
		_worldCubes[gridX] = new Cube[(int)_gridWidth]();
		for (int gridZ = 0; gridZ < _gridWidth; gridZ++)
		{
			//world coordinates
			float worldX = gridX;
			float worldY = -(gridX + gridZ);
			float worldZ = gridZ;

			_worldCubes[gridX][gridZ].GetTransform().position = Vector3(worldX, worldY, worldZ);
			_worldCubes[gridX][gridZ].Initialize(graphics);
			index++;
		}
	}

	//set the world cube the player starts on to visited
	_worldCubes[0][0].SetVertex(0, /*pos*/-0.5f, 0.5f, 0.5f,  /*color*/ 1.0f, 1.0f, 0.0f, 1.0f);
	_worldCubes[0][0].SetVertex(1, /*pos*/0.5f, 0.5f, 0.5f,   /*color*/ 1.0f, 0.0f, 1.0f, 1.0f);
	_worldCubes[0][0].SetVertex(2, /*pos*/-0.5f, -0.5f, 0.5f, /*color*/ 1.0f, 0.0f, 0.0f, 1.0f);
	_worldCubes[0][0].SetVertex(3, /*pos*/0.5f, -0.5f, 0.5f,  /*color*/ 1.0f, 1.0f, 0.0f, 1.0f);
	_worldCubes[0][0].SetVertex(4, /*pos*/-0.5f, 0.5f, -0.5f, /*color*/ 0.0f, 0.0f, 1.0f, 1.0f);
	_worldCubes[0][0].SetVertex(5, /*pos*/0.5f, 0.5f, -0.5f,  /*color*/ 1.0f, 1.0f, 0.0f, 1.0f);
	_worldCubes[0][0].SetVertex(6, /*pos*/-0.5f, -0.5f, -0.5f,/*color*/ 0.0f, 0.0f, 1.0f, 1.0f);
	_worldCubes[0][0].SetVertex(7, /*pos*/0.5f, -0.5f, -0.5f, /*color*/ 1.0f, 1.0f, 0.0f, 1.0f);
}

int Game::UpdateCubeVisitState()
{
	int gridX, gridY;

	if ((_playerGridPos.x < _gridHeight && _playerGridPos.y < _gridWidth) && (_playerGridPos.x > -1 && _playerGridPos.y > -1))
	{
		//if the cubes hasnt been visited already
		if (_visitedCubes[(int)_playerGridPos.x][(int)_playerGridPos.y] == 0)
		{
			_playerScore += 5;
			_visitedCubes[(int)_playerGridPos.x][(int)_playerGridPos.y] = 1;
			_worldCubes[(int)_playerGridPos.x][(int)_playerGridPos.y].SetVertex(0, /*pos*/-0.5f, 0.5f, 0.5f,  /*color*/ 1.0f, 1.0f, 0.0f, 1.0f);
			_worldCubes[(int)_playerGridPos.x][(int)_playerGridPos.y].SetVertex(1, /*pos*/0.5f, 0.5f, 0.5f,   /*color*/ 1.0f, 0.0f, 1.0f, 1.0f);
			_worldCubes[(int)_playerGridPos.x][(int)_playerGridPos.y].SetVertex(2, /*pos*/-0.5f, -0.5f, 0.5f, /*color*/ 1.0f, 0.0f, 0.0f, 1.0f);
			_worldCubes[(int)_playerGridPos.x][(int)_playerGridPos.y].SetVertex(3, /*pos*/0.5f, -0.5f, 0.5f,  /*color*/ 1.0f, 1.0f, 0.0f, 1.0f);
			_worldCubes[(int)_playerGridPos.x][(int)_playerGridPos.y].SetVertex(4, /*pos*/-0.5f, 0.5f, -0.5f, /*color*/ 0.0f, 0.0f, 1.0f, 1.0f);
			_worldCubes[(int)_playerGridPos.x][(int)_playerGridPos.y].SetVertex(5, /*pos*/0.5f, 0.5f, -0.5f,  /*color*/ 1.0f, 1.0f, 0.0f, 1.0f);
			_worldCubes[(int)_playerGridPos.x][(int)_playerGridPos.y].SetVertex(6, /*pos*/-0.5f, -0.5f, -0.5f,/*color*/ 0.0f, 0.0f, 1.0f, 1.0f);
			_worldCubes[(int)_playerGridPos.x][(int)_playerGridPos.y].SetVertex(7, /*pos*/0.5f, -0.5f, -0.5f, /*color*/ 1.0f, 1.0f, 0.0f, 1.0f);
			Mix_PlayChannel(-1, _visitedNewBlockSound, 0);
			//printf("Visited X: %d  Y: %d\n", (int)_playerGridPos.x, (int)_playerGridPos.y);
		}
	}

	for (gridX = 0; gridX < _gridWidth; gridX++)
	{
		for (gridY = 0; gridY < _gridHeight; gridY++)
		{

			if (_visitedCubes[gridX][gridY] == 0)
			{
				return 0;
			}
		}
	}
	return 1;
}

void Game::CalculateDrawOrder(std::vector<GameObject *>& drawOrder)
{
	// SUPER HACK GARBAGE ALGO.
	drawOrder.clear();

	auto objectsCopy = _objects;
	auto farthestEntry = objectsCopy.begin();
	while (objectsCopy.size() > 0)
	{
		bool entryFound = true;
		for (auto itr = objectsCopy.begin(); itr != objectsCopy.end(); itr++)
		{
			if (farthestEntry != itr)
			{
				if ((*itr)->GetTransform().position.y < (*farthestEntry)->GetTransform().position.y)
				{
					entryFound = false;
					farthestEntry = itr;
					break;
				}
			}
		}

		if (entryFound)
		{
			GameObject *farthest = *farthestEntry;

			drawOrder.push_back(farthest);
			objectsCopy.erase(farthestEntry);
			farthestEntry = objectsCopy.begin();
		}
	}
}

void Game::CalculateCameraViewpoint()
{
	Vector4 xAxis(1.0f, 0.0f, 0.0f, 0.0f);
	Vector4 yAxis(0.0f, 1.0f, 0.0f, 0.0f);
	Vector4 zAxis(0.0f, 0.0f, 1.0f, 0.0f);

	Vector3 cameraVector(_camera->GetLookAtVector().x, _camera->GetLookAtVector().y, _camera->GetLookAtVector().z);
	Vector3 lookAtVector(0.0f, 0.0f, -1.0f);

	Vector3 cross = Vector3::Normalize(Vector3::Cross(cameraVector, lookAtVector));
	float dot = MathUtils::ToDegrees(Vector3::Dot(lookAtVector, cameraVector));

	glRotatef(cross.x * dot, 1.0f, 0.0f, 0.0f);
	glRotatef(cross.y * dot, 0.0f, 1.0f, 0.0f);
	glRotatef(cross.z * dot, 0.0f, 0.0f, 1.0f);

	glTranslatef(-_camera->GetPosition().x, -_camera->GetPosition().y, -_camera->GetPosition().z);
}

void Game::DeployEnemy()
{
	for (int i = 0; i < _numEnemies; i++)
	{
		if (_enemies[i].GetIsAlive() == false)
		{
			int posX = rand() % (int)(_gridWidth - 1);
			int posY = rand() % (int)(_gridHeight - 1);
			for (int x = 0; x < _numEnemies; x++)
			{
				if (_enemies[x].GetIsAlive() == true && _enemies[x].GetTransform().position.x == posX && _enemies[x].GetTransform().position.y == posY)
				{
					posX = rand() % (int)(_gridWidth - 1);
					posY = rand() % (int)(_gridHeight - 1);
					x = 0;
				}
				if (posX == 0 && posY == 0)
				{
					posX = rand() % (int)(_gridWidth - 1);
					posY = rand() % (int)(_gridHeight - 1);
					x = 0;
				}
			}
			printf("Enemy deployed X: %d Y: %d\n", posX, posY);
			_timeSinceLastEnemySpawn = 0;
			_enemies[i].SetGridPos(Vector2(posX, posY));
			_enemies[i].GetTransform() = _worldCubes[posX][posY].GetTransform();
			_enemies[i].GetTransform().position.y += 1;
			_enemies[i].SetIsAlive(true);
			_timeSinceLastEnemyMoveMent = 0;
			Mix_PlayChannel(-1, _enemySpawnSound, 0);
			break;
		}
	}
}

void Game::CheckPlayerEnemyCollisions()
{
	for (int i = 0; i < _numEnemies; i++)
	{
		if (_enemies[i].GetIsAlive() == true)
		{
			if (_playerGridPos.x == _enemies[i].GetGridPos().x && _playerGridPos.y == _enemies[i].GetGridPos().y)
			{
				_playerLives -= 1;
				_playerGridPos.x = 0;
				_playerGridPos.y = 0;
				_playerCube->GetTransform().position = Vector3(0, 1, 0);
				Mix_PlayChannel(-1, _dieSound, 0);
				break;
			}
		}
	}
}

