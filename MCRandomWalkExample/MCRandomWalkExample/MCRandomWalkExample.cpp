// MCRandomWalkExample.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <iostream>
#include <math.h>
#include "windows.h"
using namespace std;

const int width = 500;
const int res = 50;
const int walk_count = 100;	//Number of walks per node
const int step_count = 5;	//Number of steps per walk
const int show_count = 3;	//Number of walks to show per node
const int node_count = pow((width / res + 1), 2);
const int node_per_row = width / res + 1;
const int show_points_count = node_count * show_count * (step_count + 1);
const int nodes_checked = (width / res - 2) * (width / res - 2);
const double test_time = 10.0;
const int framerate = 60;
vector<double> temp(node_count, 0);	//Function to solve for, u(x, t)
vector<double> new_temp(node_count, 0); //du/dt term to adjust u
int x, y, t = 0;
int roll;
int debugger;
sf::Vector2i dir;
sf::Vector2i pos;
int current_node;
int successful_walks;
vector<sf::Vector2f> show_lines(show_points_count, { 0, 0 });
int show_index;

int getNodeIndex(sf::Vector2i coord) {
	if (coord.x < 0 || coord.x > width || coord.y < 0 || coord.y > width) {
		return -1;
	}
	int N = coord.x / res + (coord.y / res) * node_per_row;
	return N;
}

sf::Vector2i getNodeXY(int N) {
	int x, y;
	x = (N % node_per_row) * res;
	y = (N / node_per_row) * res;
	return sf::Vector2i(x, y);
}

int main()
{
	sf::RenderWindow window(sf::VideoMode(width, width), "Monte Carlo Random Walk");
	sf::Clock clock;
	sf::Time dt = clock.restart();
	double delta_t = 1.0 / framerate;
	window.setFramerateLimit(framerate);
	//Define Top Boundary
	sf::RectangleShape topTemp;
	topTemp.setFillColor(sf::Color(255, 255, 255));
	topTemp.setSize(sf::Vector2f(width, width/100));
	for (int i = 0; i <= width; i += res) {
		pos.x = i;
		pos.y = 0;
		temp[getNodeIndex(pos)] = 200.0;
		new_temp[getNodeIndex(pos)] = 200.0;
	}

	//Define Side Boundary
	sf::RectangleShape sideTemp;
	sideTemp.setFillColor(sf::Color(127, 127, 127));
	sideTemp.setSize(sf::Vector2f(width/100, width));
	for (int i = res; i <= width; i += res) {
		pos.x = 0;
		pos.y = i;
		temp[getNodeIndex(pos)] = 150.0;
		new_temp[getNodeIndex(pos)] = 150.0;
	}

	//Define line drawing object
	sf::RectangleShape show_rect;
	int lineR, lineG;
	sf::Vector2f rect_size;
	double rectx, recty;

	//Define temperature indicator
	sf::CircleShape tempShape;
	tempShape.setOrigin({ 6, 6 });
	tempShape.setRadius(8);
	int tempColor;

	std::default_random_engine rng;
	std::uniform_int_distribution<int> distr(0, 3);

	while (window.isOpen())
	{
		//Draw environment
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear();
		window.draw(topTemp);
		window.draw(sideTemp);
		//window.display();
		lineR = 255;
		lineG = 0;
		//Iterate nodes (range is {res, res} through {width-res, width-res})
		for (int i = res; i < width; i += res) {
			for (int j = res; j < width; j += res) {
				//Set node using pixel coordinates (pos)
				pos.y = i;
				pos.x = j;
				current_node = getNodeIndex(pos);
				tempShape.setPosition(sf::Vector2f(pos.x, pos.y));
				lineR -= 255 / nodes_checked;
				lineG += 255 / nodes_checked;
				new_temp[current_node] = 0; //Reset du/dt
				//Iterate walks
				successful_walks = walk_count; //Reset testable walks
				for (int k = 0; k < walk_count; k++) {
					pos = getNodeXY(current_node);
					if (k < show_count) {	//Check if lines are still being drawn
						show_index = current_node * show_count * (step_count + 1) + k * (step_count + 1);
						show_lines[show_index] = sf::Vector2f(pos.x, pos.y);
					}
					//Iterate steps
					for (int step = 0; step < step_count; step++) {
						roll = distr(rng);
						if (roll == 0) dir = { 1,0 };
						else if (roll == 1) dir = { 0,1 };
						else if (roll == 2) dir = { -1,0 };
						else dir = { 0,-1 };
						pos += (res * dir); //Make a step
						//Show only first few walks for each node
						if (k < show_count) {
							show_lines[show_index + step + 1] = sf::Vector2f(pos.x, pos.y);
						}
					}
					//Add walk (or discard if out-of-bounds)
					if (getNodeIndex(pos) == -1) {
						successful_walks--;
						//cout << successful_walks << endl;
					}
					else if (getNodeIndex(pos) != -1) {
						new_temp[current_node] += temp[getNodeIndex(pos)];
						//cout << new_temp[current_node] << endl;
						//Sleep(50);
					}
						
				}
				//Draw shown lines
				for (int line = 0; line < show_count; line++) {
					for (int step = 0; step < step_count; step++) {
						show_index = current_node * show_count * (step_count + 1) + line * (step_count + 1) + step;
						rectx = show_lines[show_index + 1].x - show_lines[show_index].x;
						recty = show_lines[show_index + 1].y - show_lines[show_index].y;
						rect_size = show_lines[show_index + 1] - show_lines[show_index];
						rect_size += {3, 3};
						show_rect.setSize(rect_size);
						show_rect.setPosition(show_lines[show_index]);
						show_rect.setFillColor(sf::Color(lineR, lineG, 0));
						window.draw(show_rect);
						//window.display();
					}
				}
				//Normalize walks
				new_temp[current_node] = new_temp[current_node] / float(successful_walks);
				//cout << new_temp[current_node] << endl;
				//Set next color
				tempColor = new_temp[current_node] * float(255 / 200);
				tempShape.setFillColor(sf::Color(tempColor, tempColor, tempColor, 255));
				window.draw(tempShape);
				//window.display();
			}
		}
		//Update temperature
		temp = new_temp;
		//cout << temp[12] << endl;
		window.display();
	}
}
