#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdint.h>
sf::Color colors[4]{ sf::Color::Blue, sf::Color::Red, sf::Color::Green, sf::Color::Magenta};

sf::Text text;
sf::Text text2;
sf::Font font;
float zoom;

sf::Texture texture;

enum PlatformType
{
	CollideAll = uint32_t(0),
	CollideGladiator =uint32_t(1),
	LadderLeft = uint32_t(2),
	LadderRight = uint32_t(3)
};

enum PlatformMode
{
	Shapeless,
	Polygon
};

struct Platform
{
	PlatformType type;
	std::vector<sf::Vector2i> points;
	std::vector<sf::Vertex> drawPoints;
	void addPoint(sf::Vector2i point)
	{
		sf::Vertex vertex = sf::Vertex(sf::Vector2f(point));
		vertex.color = colors[3];
		drawPoints.push_back(vertex);
		points.push_back(point);
	};
	void reColor(PlatformType color)
	{
		for (unsigned i = 0; i < drawPoints.size(); i++)
		{
			drawPoints[i].color = colors[color];
		}
	}
	void clear()
	{
		points.clear();
		drawPoints.clear();
	}
};

sf::Vertex limits[5]
{
	sf::Vertex(sf::Vector2f(0,0)),
	sf::Vertex(sf::Vector2f(7680,0)),
	sf::Vertex(sf::Vector2f(7680,2160)),
	sf::Vertex(sf::Vector2f(0,2160)),
	sf::Vertex(sf::Vector2f(0,0))
};
std::vector<Platform> platforms;
void save()
{

	std::ofstream myfile;
	myfile.open("coordinates.txt");
	std::string text;
	for (int i = 0; i < platforms.size(); i++)
	{
		text += "\nPlatform platform";
		text += std::to_string(i);
		text += ";\n";
		for (unsigned j = 0; j < platforms[i].points.size(); j++)
		{
			text += "vec = glm::vec2(";
			text += std::to_string(platforms[i].points[j].x);
			text += ", ";
			text += std::to_string(platforms[i].points[j].y);
			text += ");\n";
	
			text += "platform";
			text += std::to_string(i);
			text += ".points.push_back(vec);\n";

		}
		text += "m_physics.createPlatform(platform";
		text += std::to_string(i);
		text += ".points);\n";

		text += "m_platformVector.push_back(platform";
		text += std::to_string(i);
		text += ");\n";


	}
	myfile << text;
	
	myfile.close();
}


struct platformObject
{

	uint32_t size;
	std::vector<float> pointsX;
	std::vector<float> pointsY;
	PlatformType type;
};

void saveRaw()
{

	std::ofstream myfile;
	myfile.open("coordinatesRawData.dat",  std::ios::out| std::ios::binary );
	uint32_t size = platforms.size();
	

	myfile.write(reinterpret_cast<char *>(&size), sizeof(uint32_t));
	for (int i = 0; i < platforms.size(); i++)
	{
		platformObject object;
		unsigned type = platforms[i].type;

		uint32_t pointSize = platforms[i].points.size();
		for (unsigned j = 0; j < platforms[i].points.size(); j++)
		{
			object.pointsX.push_back(platforms[i].points[j].x);
			object.pointsY.push_back(platforms[i].points[j].y);
		}
		myfile.write(reinterpret_cast<char *>(&pointSize), sizeof(uint32_t));
		myfile.write(reinterpret_cast<char *>(&type), sizeof(uint32_t));
		myfile.write(reinterpret_cast<char *>(object.pointsX.data()), sizeof(float)*object.pointsX.size());
		myfile.write(reinterpret_cast<char *>(object.pointsY.data()), sizeof(float)*object.pointsY.size());
	}
	
	myfile.close();

	std::ifstream file;

	file.open("coordinatesRawData.dat", std::ios::in | std::ios::binary);
	
	uint32_t sizes;
	file.read(reinterpret_cast<char*>(&sizes), sizeof(uint32_t));
	printf("size: %d\n", sizes);
	std::vector<platformObject> objects;
	for (unsigned i = 0; i < sizes; i++)
	{
		platformObject object;
		file.read(reinterpret_cast<char*>(&object.size), sizeof(uint32_t));
		file.read(reinterpret_cast<char*>(&object.type), sizeof(uint32_t));
		printf("object type: %d object size: %d\n", object.type,object.size);

		for (unsigned i = 0; i < object.size; i++)
		{
			float x;
			file.read(reinterpret_cast<char*>(&x), sizeof(float));
			object.pointsX.push_back(x);
		}
		for (unsigned i = 0; i < object.size; i++)
		{
			float y;
			file.read(reinterpret_cast<char*>(&y), sizeof(float));
			object.pointsX.push_back(y);
		}
		objects.push_back(object);
	}
	file.close();


}

int main()
{
	PlatformType currentType = CollideAll;
	PlatformMode mode = Shapeless;
	zoom = 1;
	texture.loadFromFile("Map.png");
	font.loadFromFile("asd.ttf");
	sf::Sprite sprite(texture);

	sf::RenderWindow window(sf::VideoMode(1500, 1000), "SFML works!");

	text.setFont(font);
	text2.setFont(font);
	text.setCharacterSize(35);
	text.setColor(sf::Color::White);
	sf::Vertex vertex;
	vertex.color = colors[3];
	vertex.position = sf::Vector2f(window.mapPixelToCoords(sf::Mouse::getPosition(window)));

	
	Platform platform;
	platform.drawPoints.push_back(vertex);
	sf::View view1(sf::FloatRect(0, 0, 1500, 1000));
	
	bool purkkaleft= 0, purkkaright= 0;

	while (window.isOpen())
	{
		std::string coordinates;
		sf::Vector2i intcoords = sf::Vector2i(window.mapPixelToCoords(sf::Mouse::getPosition(window)));

		coordinates += std::to_string(intcoords.x);
		coordinates += ", ";
		coordinates += std::to_string(intcoords.y);
		text.setString(coordinates);
		vertex.color = colors[3];
		vertex.position = sf::Vector2f(intcoords);
		platform.drawPoints[platform.drawPoints.size() - 1] = vertex;
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			if (purkkaleft)
			{ 
				purkkaleft = false;
				sf::Vector2i mousePosition = sf::Vector2i(window.mapPixelToCoords(sf::Mouse::getPosition(window)));
				printf("%d, %d\n", mousePosition.x, mousePosition.y);
				platform.addPoint(mousePosition);
			}
		}
		else
			purkkaleft = true;


		if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
		{
			if (purkkaright)
			{ 

				purkkaright = false;
				printf("Platform finish\n");
				if (platform.points.size() != 0)
				{
					platform.drawPoints.pop_back();
					if (mode == Polygon)
					{ 
						platform.addPoint(platform.points[0]);
					}
					platform.type = currentType;
					printf("%d\n", platform.type);
					platform.reColor(platform.type);
					platforms.push_back(platform);
					platform.clear();
					platform.drawPoints.push_back(vertex);

				}
			}
		}
		else
			purkkaright = true;
		
	
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		{
			view1.move(-2, 0);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		{
			view1.move(2, 0);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		{
			view1.move(0, -2);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			view1.move(0, 2);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
		{

			view1.zoom(1.005);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
		{

			view1.zoom(0.995);
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
		{
			save();
			saveRaw();
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1))
		{
			currentType = CollideAll;
			printf("Platform collides all\n");
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2))
		{
			currentType = CollideGladiator;
			printf("Platform collides gladiator\n");
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3))
		{
			currentType = LadderLeft;
			printf("Platform is ladder\n");
			mode = Polygon;
			printf("Platform is polygon\n");
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4))
		{
			currentType = LadderRight;
			printf("Platform is LadderRight\n");
			mode = Polygon;
			printf("Platform is polygon\n");
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num5))
		{
			mode = Shapeless;
			printf("Platform is shapeless\n");
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num6))
		{
			mode = Polygon;
			printf("Platform is polygon\n");
		}

		window.clear();

		window.draw(sprite);
		window.draw(&limits[0], 5, sf::LinesStrip);

		for (unsigned i = 0; i < platforms.size(); i++)
		{
			window.draw(&platforms[i].drawPoints[0], platforms[i].drawPoints.size(), sf::LinesStrip);
		
		}
		if (platform.drawPoints.size() != 0)
		{
			window.draw(&platform.drawPoints[0], platform.drawPoints.size(), sf::LinesStrip);
			
		}
		window.draw(text);
		text.setPosition(window.mapPixelToCoords(sf::Vector2i(0, 0)));
		window.display();
		window.setView(view1);
	}

	return 0;
}