/**
* \class Enemy.h
* \brief A class that represents a Enemy GameObject
* \author Johnathan Falbo
* \date March 23, 2014
*/
#pragma once

#include <GameObject.h>

struct Vertex;

class Enemy : public GameObject
{
public:
	/**
	* \fn Enemy::Enemy()
	* \brief Default Constructor
	*/
	Enemy();

	/**
	* \fn Enemy::Enemy(Vector3 position)
	* \brief Alternate Constructor that sets default position
	* \param Vector3 position The position you want to set the enemy to start at
	*/
	Enemy(Vector3 position);

	/**
	* \fn void Enemy::Initialize(Graphics *graphics)
	* \brief A function that is to initialize the Enemy to its default state
	* \param graphics The Graphics object used to draw the game.
	*/
	void Initialize(Graphics *graphics);

	/**
	* \fn void Enemy::Update(float dt)
	* \brief A function that is used to update the game
	* \param float dt The time since the last game tick
	*/
	void Update(float dt);

	/**
	* \fn void Enemy::Draw(Graphics *graphics, Matrix4x4 relativeTo, float dt)
	* \brief A function that is used to draw the enemies into the game
	* \param graphics The Graphics object used to draw the game.
	* \param relativeTo 
	* \param dt the time in seconds since the last game tick
	*/
	void Draw(Graphics *graphics, Matrix4x4 relativeTo, float dt);

	/**
	* \fn void Enemy::SetVertex(int index, float x, float y, float z, float r, float g, float b, float a);
	* \brief A function that is used to set vertex positions and colours
	* \param index of the vertex
	* \param x position
	* \param y position
	* \param z position
	* \param r red colour value
	* \param g green colour value
	* \param b blue colour value
	* \param a opacity value
	*/
	void SetVertex(int index, float x, float y, float z, float r, float g, float b, float a);

	/**
	* \fn void Enemy::ResetGame(Graphics *graphics)
	* \brief A function that is used to move the enemies positions down the game world randomly left or right
	*/
	void MoveDownGameWorld();

	/**
	* \fn bool Enemy::GetIsAlive()
	* \brief A function that is used to get the alive state of an enemy
	* \return true is enemy is alive false if not
	*/
	bool GetIsAlive();

	/**
	* \fn void Enemy::SetIsAlive(bool isAlive)
	* \brief A function that is to set the alive state of an enemy
	* \param isAlive true if alive false if not
	*/
	void SetIsAlive(bool isAlive);

	/**
	* \fn Vector2 Enemy::GetGridPos()
	* \brief A function that is used to set the gridPosition of the enemy
	* \return Vector2 representing the enemies position on the grid
	*/
	Vector2 GetGridPos();

	/**
	* \fn void Enemy::SetGridPos(Vector2)
	* \brief A function that is used to set the grid position of an enemy
	* \param gridPos a Vector2 representing the position fo the grid you would liek to set the enemy too
	*/
	void SetGridPos(Vector2 gridPos);

protected:
	//void *_vertexPointer;

	Vector3 *vertices;
	Vector4 *colours;

	int size;
	int offset;

	//Vertex *vertices;
	unsigned int* indices;

	//Enemy Grid Positions
	Vector2 _enemyPosGrid;

	//reperesents if the enemy is alive or not
	bool _isAlive;
};