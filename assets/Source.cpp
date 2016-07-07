#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
sf::Color colors[2]{ sf::Color::Blue, sf::Color::Red};
static unsigned color;
sf::Text text;
sf::Text text2;
sf::Font font;
float zoom;

sf::Texture texture;
struct Platform
{
	
	std::vector<sf::Vector2i> points;
	std::vector<sf::Vertex> drawPoints;
	void addPoint(sf::Vector2i point)
	{
		sf::Vertex vertex = sf::Vertex(sf::Vector2f(point));
		vertex.color = colors[color];
		drawPoints.push_back(vertex);
		points.push_back(point);
	};
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

void saveRaw()
{

	std::ofstream myfile;
	myfile.open("coordinatesRaw.txt");
	std::string text;
	for (int i = 0; i < platforms.size(); i++)
	{
		text += std::to_string(i);
		text += "\n";
		for (unsigned j = 0; j < platforms[i].points.size(); j++)
		{
			text += std::to_string(platforms[i].points[j].x);
			text += ", ";
			text += std::to_string(platforms[i].points[j].y);
			text += "\n";
		}
	}
	myfile << text;

	myfile.close();
}
int main()
{
	zoom = 1;
	texture.loadFromFile("Map.png");
	font.loadFromFile("asd.ttf");
	sf::Sprite sprite(texture);
	color = 0;
	sf::RenderWindow window(sf::VideoMode(1500, 1000), "SFML works!");

	text.setFont(font);
	text2.setFont(font);
	text.setCharacterSize(35);
	text.setColor(sf::Color::White);
	sf::Vertex vertex;
	vertex.color = colors[color];
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
		vertex.color = colors[color];
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
					platforms.push_back(platform);
					platform.clear();
					platform.drawPoints.push_back(vertex);
					color++;
					if (color == 2)
						color = 0;
				}
			}
		}
		else
			purkkaright = true;
		
	
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		{
			view1.move(-1, 0);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		{
			view1.move(1, 0);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		{
			view1.move(0, -1);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			view1.move(0, 1);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
		{

			view1.zoom(1.001);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
		{

			view1.zoom(0.999);
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
		{
			save();
			saveRaw();
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