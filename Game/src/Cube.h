/**
* \class Cube.h
* \brief A class that represents a Cube GameObject
* \author Justin Wilkinson
* \date March 23, 2014
*/
#pragma once

#include <GameObject.h>

struct Vertex;

class Cube : public GameObject
{
public:
	/**
	* \fn Cube::Cube()
	* \brief Default Constructor
	*/
	Cube();

	/**
	* \fn Cube::Cube(Vector3 position)
	* \brief Alternate Constructor that sets default position
	* \param Vector3 position The position you want to set the Cube to start at
	*/
	Cube(Vector3 position);

	/**
	* \fn void Cube::Initialize(Graphics *graphics)
	* \brief A function that is to initialize the Cube to its default state
	* \param graphics The Graphics object used to draw the game.
	*/
	void Initialize(Graphics *graphics);

	/**
	* \fn void Cube::Update(float dt)
	* \brief A function that is used to update the game
	* \param float dt The time since the last game tick
	*/
	void Update(float dt);

	/**
	* \fn void Cube::Draw(Graphics *graphics, Matrix4x4 relativeTo, float dt)
	* \brief A function that is used to draw the enemies into the game
	* \param graphics The Graphics object used to draw the game.
	* \param relativeTo
	* \param dt the time in seconds since the last game tick
	*/
	void Draw(Graphics *graphics, Matrix4x4 relativeTo, float dt);

	/**
	* \fn void Cube::SetVertex(int index, float x, float y, float z, float r, float g, float b, float a);
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

protected:
	//void *_vertexPointer;

	Vector3 *vertices;
	Vector4 *colours;

	int size;
	int offset;

	//Vertex *vertices;
	unsigned int* indices;


};