﻿// RobotArm3R.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <SFML/Graphics.hpp>
#include <math.h>
#include <iostream>
#define PI 3.14159265

using namespace std;

const int width = 800;		//Available workspace
const int link_size = 300;	//Length of link
const int link_height = 50; //Perpindicular size of link
const int link_dist = link_size - link_height;	//Joint-to-joint length of link
const double spd = 2;		//Degrees per frame
const double v_spd = 100;		//Pixels per frame
const int framerate = 60;

bool move_up = false;
bool move_left = false;
bool move_right = false;
bool move_down = false;
bool l1_cw = false;
bool l1_ccw = false;
bool l2_cw = false;
bool l2_ccw = false;
bool ee_cw = false;
bool ee_ccw = false;

sf::Vector2f l0l1_joint = sf::Vector2f(link_height / 2, width / 2);
sf::RectangleShape link1;
double link1_angle = -45;
double thetad1 = 0;

sf::Vector2f l1l2_joint = sf::Vector2f(link_size - link_height / 2, width / 2);
sf::RectangleShape link2;
double link2_angle = 45;
double thetad2 = 0;

sf::Vector2f l2ee_joint = sf::Vector2f(link_size * 2 - link_height, width / 2);
sf::ConvexShape ee;
double ee_angle = 0;
double thetadee = 0;
sf::Vector2f ee_v = { 0,0 };
double ee_v_mag;

sf::CircleShape jointShape0;
sf::CircleShape jointShape1;
sf::CircleShape jointShape2;

double inv_kin_div;

void update_l1l2() {
	double l1x, l2x, l1y, l2y;
	l1x = l0l1_joint.x;
	l1y = l0l1_joint.y;
	l2x = l1x + cos(link1_angle * PI / 180) * link_dist;
	l2y = l1y + sin(link1_angle * PI / 180) * link_dist;
	l1l2_joint = sf::Vector2f(l2x, l2y);
	link2.setPosition(l1l2_joint);
	link2.setRotation(link2_angle);
	jointShape1.setPosition(l1l2_joint);
}

void update_l2ee() {
	double l2x, eex, l2y, eey;
	l2x = l1l2_joint.x;
	l2y = l1l2_joint.y;
	eex = l2x + cos(link2_angle * PI / 180) * link_dist;
	eey = l2y + sin(link2_angle * PI / 180) * link_dist;
	l2ee_joint = sf::Vector2f(eex, eey);
	ee.setPosition(l2ee_joint);
	ee.setRotation(ee_angle);
	jointShape2.setPosition(l2ee_joint);
}

int main()
{
	sf::RenderWindow window(sf::VideoMode(width, width), "Robot Arm");
	double dt = 1 / framerate;
	window.setFramerateLimit(framerate);

	link1.setFillColor(sf::Color(255, 255, 0));
	link1.setSize(sf::Vector2f(link_size, link_height));
	link1.setOrigin(sf::Vector2f(link_height / 2, link_height / 2));
	link1.setPosition(l0l1_joint);
	link1.rotate(link1_angle);

	link2.setFillColor(sf::Color(255, 255, 0));
	link2.setSize(sf::Vector2f(link_size, link_height));
	link2.setOrigin(sf::Vector2f(link_height / 2, link_height / 2));
	link2.setPosition(sf::Vector2f(l1l2_joint));

	ee.setFillColor(sf::Color(0, 255, 0));
	ee.setPointCount(7);
	ee.setPoint(0, sf::Vector2f(0, 0));
	ee.setPoint(1, sf::Vector2f(link_height, 0));
	ee.setPoint(2, sf::Vector2f(link_height, link_height * 0.25));
	ee.setPoint(3, sf::Vector2f(link_height * 0.5, link_height * 0.5));
	ee.setPoint(4, sf::Vector2f(link_height, link_height * 0.75));
	ee.setPoint(5, sf::Vector2f(link_height, link_height));
	ee.setPoint(6, sf::Vector2f(0, link_height));
	ee.setOrigin(sf::Vector2f(0, link_height / 2));
	ee.setPosition(sf::Vector2f(l2ee_joint));

	jointShape0.setFillColor(sf::Color(0, 0, 0));
	jointShape0.setOrigin({ 10,10 });
	jointShape0.setRadius(10);
	jointShape0.setPosition(sf::Vector2f(l0l1_joint));

	jointShape1.setFillColor(sf::Color(0, 0, 0));
	jointShape1.setOrigin({ 10,10 });
	jointShape1.setRadius(10);
	jointShape1.setPosition(sf::Vector2f(l1l2_joint));

	jointShape2.setFillColor(sf::Color(0, 255, 0));
	jointShape2.setOrigin({ 10,10 });
	jointShape2.setRadius(10);
	jointShape2.setPosition(sf::Vector2f(l2ee_joint));

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyPressed) {
				//Workspace Control
				if (event.key.code == sf::Keyboard::W) {
					move_up = true;
				}
				if (event.key.code == sf::Keyboard::A) {
					move_left = true;
				}
				if (event.key.code == sf::Keyboard::S) {
					move_down = true;
				}
				if (event.key.code == sf::Keyboard::D) {
					move_right = true;
				}
				//Jointspace Control
				if (event.key.code == sf::Keyboard::R) {
					l1_ccw = true;
				}
				if (event.key.code == sf::Keyboard::F) {
					l1_cw = true;
				}
				if (event.key.code == sf::Keyboard::T) {
					l2_ccw = true;
				}
				if (event.key.code == sf::Keyboard::G) {
					l2_cw = true;
				}
				if (event.key.code == sf::Keyboard::Y) {
					ee_ccw = true;
				}
				if (event.key.code == sf::Keyboard::H) {
					ee_cw = true;
				}
				//Close program
				if (event.key.code == sf::Keyboard::Escape) {
					window.close();
				}

			}
			if (event.type == sf::Event::KeyReleased) {
				//Workspace Control
				if (event.key.code == sf::Keyboard::W) {
					move_up = false;
				}
				if (event.key.code == sf::Keyboard::A) {
					move_left = false;
				}
				if (event.key.code == sf::Keyboard::S) {
					move_down = false;
				}
				if (event.key.code == sf::Keyboard::D) {
					move_right = false;
				}
				//Jointspace Control
				if (event.key.code == sf::Keyboard::R) {
					l1_ccw = false;
				}
				if (event.key.code == sf::Keyboard::F) {
					l1_cw = false;
				}
				if (event.key.code == sf::Keyboard::T) {
					l2_ccw = false;
				}
				if (event.key.code == sf::Keyboard::G) {
					l2_cw = false;
				}
				if (event.key.code == sf::Keyboard::Y) {
					ee_ccw = false;
				}
				if (event.key.code == sf::Keyboard::H) {
					ee_cw = false;
				}
			}
		}

		//Rotate link1
		if (l1_ccw == true && l1_cw == false) {
			link1_angle += spd;
			link2_angle += spd;
			ee_angle += spd;
			link1.rotate(spd);
		}
		else if (l1_cw == true && l1_ccw == false) {
			link1_angle -= spd;
			link2_angle -= spd;
			ee_angle -= spd;
			link1.rotate(-spd);
		}
		//Rotate link2
		if (l2_ccw == true && l2_cw == false) {
			link2_angle += spd;
			ee_angle += spd;
			link2.rotate(spd);
		}
		else if (l2_cw == true && l2_ccw == false) {
			link2_angle -= spd;
			ee_angle -= spd;
			link2.rotate(-spd);
		}
		//Rotate ee
		if (ee_ccw == true && ee_cw == false) {
			ee_angle += spd;
			ee.rotate(spd);
		}
		else if (ee_cw == true && ee_ccw == false) {
			ee_angle -= spd;
			ee.rotate(-spd);
		}

		//Set ee velocity
		if (move_left == true && move_right == false) {
			ee_v.x = -v_spd;
		}
		else if (move_left == false && move_right == true) {
			ee_v.x = v_spd;
		}
		else {
			ee_v.x = 0;
		}
		if (move_up == true && move_down == false) {
			ee_v.y = -v_spd;
		}
		else if (move_up == false && move_down == true) {
			ee_v.y = v_spd;
		}
		else {
			ee_v.y = 0;
		}
		ee_v_mag = sqrt(pow(ee_v.x, 2) + pow(ee_v.y, 2));
		if (ee_v_mag != 0) {
			//Calculate angle speeds if ee_v is nontrivial
			inv_kin_div = (-cos(link2_angle * PI / 180) * (sin(link1_angle * PI / 180) + sin(link2_angle * PI / 180)) + sin(link2_angle * PI / 180) * (cos(link1_angle * PI / 180) + cos(link2_angle * PI / 180))) * pow(link_dist, 2);
			thetad1 = (cos(link2_angle * PI / 180) * ee_v.x + sin(link2_angle * PI / 180) * ee_v.y) * link_dist / inv_kin_div;
			thetad2 = ((-cos(link1_angle * PI / 180) - cos(link2_angle * PI / 180)) * ee_v.x + -(sin(link1_angle * PI / 180) + sin(link2_angle * PI / 180)) * ee_v.y) * link_dist / inv_kin_div;
			thetadee = -(thetad1 + thetad2);
			//cout << thetad1 << " " << thetad2 << endl;
			link1_angle += thetad1;
			link2_angle += thetad2 + thetad1;
			ee_angle += thetadee + thetad2 + thetad1;
			link1.rotate(thetad1);
			link2.rotate(thetad2);
			ee.rotate(thetadee);
		}
		update_l1l2();
		update_l2ee();
		window.clear();
		window.draw(link1);
		window.draw(link2);
		window.draw(jointShape0);
		window.draw(jointShape1);
		window.draw(jointShape2);
		window.draw(ee);
		window.display();
	}

    return 0;
}

