/**
* \class Game.h
* \brief A class that represents the game implmentation
* \author Johnathan Falbo
* \date March 23, 2014
*/
#pragma once

#include <GameEngine.h>
#include <SDL_mixer.h>

//forward declarations
union SDL_Event;
class Graphics;
class Camera;
class Cube;
class Enemy;
struct Mix_Chunk;

class Game : public GameEngine
{
	friend class GameEngine;

public:
	/**
	* \fn void Game::~Game()
	* \brief de-constructor for class Game
	*/
	~Game();

protected:
	/**
	* \fn void Game::Game()
	* \brief Default constructor for class Game
	*/
	Game();

	/**
	* \fn void Game::InitializeImpl(Graphics *graphics)
	* \brief A function that is used to initialize the game
	* \param graphics The Graphics object used to draw the game.
	*/
	void InitializeImpl(Graphics *graphics);

	/**
	* \fn void Game::UpdateImpl(Graphics * graphics, float dt)
	* \brief A function that is used to update the games state every tick
	* \param graphics The Graphics object used to draw the game.
	* \param dt The time in fractions of a second since the last pass.
	*/
	void UpdateImpl(Graphics * graphics, float dt);

	/**
	* \fn void Game::DrawImpl(Graphics *graphics, float dt)
	* \brief A function that is used to draw the games state every tick
	* \param graphics The Graphics object used to draw the game.
	* \param dt The time in fractions of a second since the last pass.
	*/
	void DrawImpl(Graphics *graphics, float dt);

	/**
	* \fn void Game::DrawImpl(Graphics *graphics, float dt)
	* \brief A function that is used to calculate the order GameObjects should be drawn
	* \param std::vector<GameObject *>& drawOrder the draw order of the GameObjects
	*/
	void CalculateDrawOrder(std::vector<GameObject *>& drawOrder);

	/**
	* \fn void Game::CalculateCameraViewpoint()
	* \brief A function that is used to calculate the game cameras viewpoint
	*/
	void CalculateCameraViewpoint();

	/**
	* \fn int Game::UpdateCubeVisitState()
	* \brief A function that is used to check if cubes have been visited and notifies the game when all have been visited
	* \return 1 if all cubes have been visited 0 if some still need to be visted
	*/
	int UpdateCubeVisitState();

	/**
	* \fn int Game::NextGameLevel(Graphics *graphics)
	* \brief A function that is used to increment the games difficulty to the next level
	* \param graphics The Graphics object used to draw the game.
	*/
	void NextGameLevel(Graphics *graphics);

	/**
	* \fn int Game::ResetGame(Graphics *graphics)
	* \brief A function that is used to reset the game to its starting state
	* \param graphics The Graphics object used to draw the game.
	*/
	void ResetGame(Graphics *graphics);

	/**
	* \fn int Game::DeployEnemy()
	* \brief A function that is called to deploy the first available enemy to the game world
	*/
	void DeployEnemy();

	/**
	* \fn int Game::CheckPlayerEnemyCollisions()
	* \brief A function that is called to check if the player has collided with an enemy
	*/
	void CheckPlayerEnemyCollisions();

	//game camera
	Camera *_camera;

	//two dimensional array of ints used to track which cubes have been visited
	int **_visitedCubes;

	//game grid width
	float _gridWidth;

	//game grid height
	float _gridHeight;

	//cube representing the player
	Cube *_playerCube;

	//two dimensionl array of cubes
	Cube **_worldCubes;

	//sound played when player moves
	Mix_Chunk *_moveSound;

	//sound played when player loses a life
	Mix_Chunk *_dieSound;

	//sound played when player clears a level
	Mix_Chunk *_clearLevelSound;

	//sound played when player clears a level
	Mix_Chunk *_visitedNewBlockSound;

	//sound played when an enemy spawns
	Mix_Chunk *_enemySpawnSound;

	//sound played when enemy moves
	Mix_Chunk *_enemyMovementSound;

	//player score
	int _playerScore;

	//player lives
	int _playerLives;

	//player game grid 2D position
	Vector2 _playerGridPos;

	//string used for window title
	char* _windowString;

	//array of enemies
	Enemy *_enemies;

	//number of enemies in array
	int _numEnemies;

	//enemy deploy rate in number of seconds
	float _enemyDeployRate;

	//time in seconds since last enemy spawn
	float _timeSinceLastEnemySpawn;

	//time in seconds since last enemy movement
	float _timeSinceLastEnemyMoveMent;

	//interval in seconds that the enemies should move at
	float _enemyMovementSpeed;
};