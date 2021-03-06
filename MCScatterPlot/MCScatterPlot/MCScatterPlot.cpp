#include "stdafx.h"
#include <SFML/Graphics.hpp>
#include <random>
#include <iostream>
#include <algorithm>
#include <vector>
#include "windows.h"
using namespace std;
const int data_count = 10000;
const int radius = 200;
const int width = radius * 2;
int main()
{
	sf::RenderWindow window(sf::VideoMode(width, width), "Sprites!");

	sf::CircleShape targetCircle;
	targetCircle.setFillColor(sf::Color(0, 0, 255));
	targetCircle.setRadius(radius);
	targetCircle.setPosition(0, 0);

	sf::CircleShape sampleShape;
	sampleShape.setFillColor(sf::Color(255, 0, 0));
	sampleShape.setRadius(1);

	std::default_random_engine rng;
	std::uniform_int_distribution<int> distr(0, width);
	bool onTarget[data_count];
	sf::Vector2f samplePoint;
	float distFromRadius;
	float totalArea;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear();
		window.draw(targetCircle);
		for (int i = 0; i < data_count; i++) {
			samplePoint.x = distr(rng);
			samplePoint.y = distr(rng);
			sampleShape.setPosition(distr(rng), distr(rng));
			distFromRadius = std::sqrt(std::pow(samplePoint.x - radius, 2) + std::pow(samplePoint.y - radius, 2));
			if (distFromRadius < radius) {
				onTarget[i] = true;
			}
			else {
				onTarget[i] = false;
			}
			window.draw(sampleShape);
			
			//Sleep(1);
		}
		window.display();
		int area = std::count(onTarget, onTarget + data_count, true);
		totalArea = width * width * float(area) / float(data_count);
		std::cout << totalArea << std::endl;
		Sleep(1000);
	}
}
