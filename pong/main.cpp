#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
using namespace sf;
using namespace std;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int SCORE_LIMIT = 10;
const string FONT_NAME = "ARCADECLASSIC.TTF";

enum {LEFT_PLAYER, RIGHT_PLAYER};

class Score {
public:
	const int marginTop = 0;
	const int textSize = 85;
	const int space = 50;
	int left = 0, right = 0;
	Text textLeft, textRight;
	Font font;

	Score(Color leftColor, Color rightColor) {
		font.loadFromFile(FONT_NAME);

		textLeft.setColor(leftColor);
		textLeft.setFont(font);
		textLeft.setPosition(Vector2f(SCREEN_WIDTH / 2 - textSize/2 - space / 2, marginTop));
		textLeft.setCharacterSize(textSize);

		textRight.setColor(rightColor);
		textRight.setFont(font);
		textRight.setPosition(Vector2f(SCREEN_WIDTH / 2 + space / 2, marginTop));
		textRight.setCharacterSize(textSize);
	}

	void draw(RenderWindow& window) {
		textLeft.setString(to_string(left));
		textRight.setString(to_string(right));
		window.draw(textLeft);
		window.draw(textRight);
	}
};

class Racket {
public:
	static const int height = SCREEN_HEIGHT/8;
	static const int width = height/5;
	static const int shift = width*3;
	
	const int startY = SCREEN_HEIGHT / 2 - height / 2;
	float speed = 0.3;

	Color color;
	float x, y = startY; 
	float hitBoundX, centerY;
	int player;
	Keyboard::Key upKey, downKey;
	RectangleShape rectangle = RectangleShape(Vector2f(width, height));

	Racket(Color color, int player) {
		this->color = color;
		rectangle.setFillColor(color);
		switch (player)
		{
		case LEFT_PLAYER:
			x = shift; 
			hitBoundX = x + width;
			upKey = Keyboard::W;
			downKey = Keyboard::S;
			break;
		case RIGHT_PLAYER:
			x = SCREEN_WIDTH - shift - width;
			hitBoundX = x;
			upKey = Keyboard::Up;
			downKey = Keyboard::Down;
		}
		this->player = player;
	}

	void update(float dt) {
		float y = this->y;

		if (Keyboard::isKeyPressed(upKey)) y -= speed * dt;
		else if (Keyboard::isKeyPressed(downKey)) y += speed * dt;

		if (y < 0) y = 0;
		else if (y > SCREEN_HEIGHT - height) y = SCREEN_HEIGHT - height;
		
		this->centerY = y + height / 2;
		this->y = y;
	}

	void draw(RenderWindow &window) {
		rectangle.setPosition(x, y);
		window.draw(rectangle);
	}
};

class Ball {
public:
	static const int radius = SCREEN_HEIGHT/35;
	float speed = 0.4;
	
	const int startY = SCREEN_HEIGHT / 2;
	float x, y, vx, vy;

	CircleShape circle = CircleShape(radius);
	Color color;
	
	Score* score;

	Thread blinker;

	Ball(Color color, Score* score)
	: blinker(&Ball::blink, this)
	{
		this->color = color;
		this->score = score;
	}

	void setToLeftPlayer() {
		this->x = Racket::shift + Racket::width + radius;
		this->y = startY;
		this->vx = speed;
		this->vy = 0;
	}

	void setToRightPlayer() {
		this->x = SCREEN_WIDTH - Racket::shift - Racket::width - radius;
		this->y = startY;
		this->vx = -speed;
		this->vy = 0;
	}

	void blink() {
		Time delay = milliseconds(150);
		int times = 2;
		Color savedColor = color;
		for (int i = 0; i <= times; i++) {
			sleep(delay);
			color = Color::Transparent;
			sleep(delay);
			color = savedColor;
		}
	}

	void rightScored() {
		score->right++;
		setToLeftPlayer();
		blinker.launch();
	}

	void leftScored() {
		score->left++;
		setToRightPlayer();
		blinker.launch();
	}

	float dt;
	void update(float dt) {
		x += vx*dt; y += vy*dt;
		this->dt = dt;
	}

	void checkBoundariesHit() {
		if (y + radius >= SCREEN_HEIGHT || y - radius <= 0) {
			vy = -vy;
			update(dt);
		}
		if (x - radius >= SCREEN_WIDTH) leftScored();
		if (x + radius <= 0) rightScored();
		
	}

	void handlePlayerHit(float offsetY) {
		vx = -vx; this->vy = offsetY / (Racket::height / 2) * speed;
		update(dt);
	}

	void checkPlayerHit(Racket* racket) {
		if (x - radius <= racket->hitBoundX && x + radius >= racket->x &&
			y + radius >= racket->y && y - radius <= racket->y + Racket::height) {
			handlePlayerHit(abs(racket->centerY-y));
		}
	}

	/*void checkRightPlayerHit(Racket* racket) {
		if (x + radius >= racket->hitBoundX && x - radius <= racket->x &&
			y + radius >= racket->y && y - radius <= racket->y + Racket::height) {
			handlePlayerHit(abs(racket->centerY-y));
		}
	}*/

	void draw(RenderWindow &window) {
		circle.setFillColor(color);
		circle.setPosition(x-radius, y-radius);
		window.draw(circle);
	}
};

class StartMenu {
public:
	const int textSize = 70;
	Text text;
	Font font;
	string textStr = "PRESS  ENTER  TO  START";

	StartMenu(Color color) {
		font.loadFromFile(FONT_NAME);

		text.setColor(color);
		text.setFont(font);
		text.setString(textStr);
		text.setCharacterSize(textSize);

		FloatRect textRect = text.getLocalBounds();
		text.setPosition(Vector2f(SCREEN_WIDTH / 2 - textRect.width / 2, SCREEN_HEIGHT / 2 - textSize));
	}

	void draw(RenderWindow& window) {
		window.draw(text);
	}
};

class WinnerMenu {
public:
	const int textSize = 70;
	Text text;
	Font font;

	WinnerMenu(Color color, int player) {
		font.loadFromFile(FONT_NAME);
		text.setColor(color);
		text.setFont(font);
		text.setCharacterSize(textSize);
		
		string textStr = "PLAYER  WIN!";
		textStr = player == LEFT_PLAYER ? "RED  " + textStr : "BLUE  " + textStr;
		text.setString(textStr);

		FloatRect textRect = text.getLocalBounds();
		text.setPosition(Vector2f(SCREEN_WIDTH / 2 - textRect.width / 2, SCREEN_HEIGHT / 2 - textSize));
	}

	void draw(RenderWindow& window) {
		window.draw(text);
	}
};

void START(RenderWindow& window);
void GAME(RenderWindow& window);
void WINNER(RenderWindow& window, int leftScore, int rightScore);

void GAME(RenderWindow &window) {
	sf::Clock clock;
	float time;

	Racket* leftRacket = new Racket(Color::Red, LEFT_PLAYER);
	Racket* rightRacket = new Racket(Color::Blue, RIGHT_PLAYER);
	Score* score = new Score(Color::Red, Color::Blue);
	Ball* ball = new Ball(Color::White, score);
	ball->setToLeftPlayer();

	while (window.isOpen())
	{
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed) {
				window.close();
			}
		}

		time = clock.getElapsedTime().asMilliseconds();
		clock.restart();
		leftRacket->update(time);
		rightRacket->update(time);
		ball->update(time);
		ball->checkPlayerHit(leftRacket);
		ball->checkPlayerHit(rightRacket);
		ball->checkBoundariesHit();

		if (score->left >= SCORE_LIMIT || score->right >= SCORE_LIMIT) {
			WINNER(window, score->left, score->right);
			return;
		}

		window.clear();

		leftRacket->draw(window);
		rightRacket->draw(window);
		score->draw(window);
		ball->draw(window);

		window.display();
	}
}

void START(RenderWindow& window) {
	StartMenu* menu = new StartMenu(Color::White);
	while (window.isOpen())
	{
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed) {
				window.close();
			}

			if (event.type == Event::KeyPressed && event.key.code == Keyboard::Return) {
				GAME(window);
				return;
			}
		}
		window.clear();
		menu->draw(window);
		window.display();
	}
}

void WINNER(RenderWindow& window, int leftScore, int rightScore) {
	int player = leftScore > rightScore ? LEFT_PLAYER : RIGHT_PLAYER;
	Color color = leftScore > rightScore ? Color::Red : Color::Blue;
	WinnerMenu* menu = new WinnerMenu(color, player);
	while (window.isOpen())
	{
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed) {
				window.close();
			}

			if (event.type == Event::KeyPressed && event.key.code == Keyboard::Return) {
				GAME(window);
				return;
			}
		}
		window.clear();
		menu->draw(window);
		window.display();
	}
}

int main()
{
	ContextSettings settings;
	settings.antialiasingLevel = 8;
	RenderWindow window(VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "PING-PONG", Style::Default, settings);
	window.setVerticalSyncEnabled(true);
	
	START(window);

	return 0;
}