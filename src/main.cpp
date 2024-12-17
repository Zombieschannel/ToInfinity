#include <SFML/Graphics.hpp>
#include "ZLE.h"
#include <vector>
using namespace sf;
using namespace std;
//#define TIMERMODE
//#define CONTROLLABLE
//#define SWAPCOLORS
//#define BORNAMODE
#define FANCYMODE
bool Circle_Rectangle(const sf::Vector2f& pos1, const float radius1, const sf::FloatRect& rectangle)
{
    sf::Vector2f tests = pos1;
    if (pos1.x < rectangle.left)
        tests.x = rectangle.left;
    else if (pos1.x > rectangle.left + rectangle.width)
        tests.x = rectangle.left + rectangle.width;

    if (pos1.y < rectangle.top)
        tests.y = rectangle.top;
    else if (pos1.y > rectangle.top + rectangle.height)
        tests.y = rectangle.top + rectangle.height;

    sf::Vector2f dist = sf::Vector2f(pos1.x - tests.x, pos1.y - tests.y);
    float distanceSqr = (dist.x * dist.x) + (dist.y * dist.y);
    if (distanceSqr <= radius1 * radius1)
        return 1;
    return 0;
}
Vector2f normalize(const Vector2f& arg)
{
    float len = arg.x * arg.x + arg.y * arg.y;
    if (len == 0.f)
        return Vector2f();
    return arg / sqrt(len);
}
class ToInfinity
{
    struct Ball
    {
        CircleShape ball;
        Vector2f dir;
        bool dead = false;
    };
    int ballCount = 4;
    //vector<Color> ballColors = { Color(255, 50, 40), Color(255, 255, 255), Color(242, 174, 14), Color(5, 107, 14) };
    //vector<Color> bgColor = { Color(0, 0, 0), Color(200, 30, 0), Color(200, 200, 200), Color(207, 154, 10), Color(11, 87, 18) };
    vector<Color> ballColors = { Color(255, 50, 40), Color(0x5AFFFFFF), Color(242, 174, 14), Color(5, 107, 14) };
    vector<Color> bgColor = { Color(0, 0, 0), Color(200, 30, 0), Color(0x99E6E6FF), Color(207, 154, 10), Color(11, 87, 18) };
    //vector<Color> ballColors;
    //vector<Color> bgColor;
    //vector<Vector2f> ballPos;
    const float ballSpeed = 800.f;
    const Vector2u canvasSize = Vector2u(1920, 1080);
    const Vector2u mapSize = Vector2u(16, 9) * 2U;
    const float ballRadius = (static_cast<float>(canvasSize.x) / mapSize.x) / 4;
    vector<Vector2f> ballPos = { Vector2f(canvasSize.x / 4, canvasSize.y / 4), Vector2f(canvasSize.x / 4 * 3, canvasSize.y / 4),
        Vector2f(canvasSize.x / 4, canvasSize.y / 4 * 3), Vector2f(canvasSize.x / 4 * 3, canvasSize.y / 4 * 3) };
    vector<vector<Uint8>> map;
    RenderWindow window;
    VertexArray arr;
    VertexBuffer buff;
    vector<Ball> balls;
    View view;
    Font font;
    vector<Text> counters;
    vector<int> totalTiles;
    int highestID = 0;
    int lowestID = 0;
    vector<zle::ParticleSystem> wallBreak;
    vector<zle::ParticleSystem> ballTrail;
    zle::ParticleSystem snowFlakeSystem;
    Clock snowFlakeClock;
    Texture circle;
    Texture snowFlakeTexture;
    Image img;

    const int totalTimerCnt = 60;
    const int swapColorsCnt = 40;
    Text timerText;
    Time timer = seconds(totalTimerCnt);
    Time swapColors = seconds(swapColorsCnt);

    Shader bgShader;
public:
    bool Collided(int ballID, Vector2i& collisionTile)
    {
        //collision
        const Vector2f tileSize = Vector2f(static_cast<float>(canvasSize.x) / mapSize.x, static_cast<float>(canvasSize.y) / mapSize.y);
        for (int j = -1; j <= 1; j++)
        {
            int coordX = balls[ballID].ball.getPosition().x / tileSize.x + j;
            if (coordX < 0 || coordX >= mapSize.x)
                continue;
            for (int k = -1; k <= 1; k++)
            {
                int coordY = balls[ballID].ball.getPosition().y / tileSize.y + k;
                if (coordY < 0 || coordY >= mapSize.y)
                    continue;
                bool enemyColor = map[coordX][coordY] != ballID + 1;
                bool collision = enemyColor && Circle_Rectangle(balls[ballID].ball.getPosition(), balls[ballID].ball.getRadius(),
                    FloatRect(tileSize.x * coordX, tileSize.y * coordY, tileSize.x, tileSize.y));
                if (collision)
                {
                    collisionTile = Vector2i(coordX, coordY);
                    int index = map[coordX][coordY];
                    for (int i = 0; i < 20; i++)
                    {
                        Vector2f randPos;
                        randPos.x = coordX * tileSize.x + tileSize.x / 2 + (static_cast<float>(rand()) / RAND_MAX - 0.5) * (tileSize.x - wallBreak[index].getStartSize());
                        randPos.y = coordY * tileSize.y + tileSize.y / 2 + (static_cast<float>(rand()) / RAND_MAX - 0.5) * (tileSize.y - wallBreak[index].getStartSize());
                        wallBreak[index].setSpawnPosition(randPos);
                        wallBreak[index].Create();
                    }
                    return true;
                }
            }
        }
        return false;
    }
    void GenCircle()
    {
        img.create(512, 512, Color::Transparent);
        for (int i = 0; i < img.getSize().x; i++)
            for (int j = 0; j < img.getSize().y; j++)
                if ((i - img.getSize().x / 2) * (i - img.getSize().x / 2) + (j - img.getSize().y / 2) * (j - img.getSize().y / 2) < 256 * 256)
                    img.setPixel(i, j, Color::White);
        circle.loadFromImage(img);
    }
    void Start()
    {
#ifdef SFML_SYSTEM_EMSCRIPTEN
        window.create(VideoMode(1920 * 4, 1080 * 4), "ToInfinity", Style::None);
#else
        window.create(VideoMode(1920, 1080), "ToInfinity", Style::None);
#endif
        window.setVerticalSyncEnabled(1);

        //bgColor.emplace_back(Color::Black);
        //for (int i = 0; i < 20; i++)
        //{
        //    ballCount++;
        //    ballPos.emplace_back(Vector2f(canvasSize.x / 100 * (rand() % 100), canvasSize.y / 100 * (rand() % 100)));
        //    ballColors.emplace_back(Color(156 + rand() % 100, 156 + rand() % 100, 156 + rand() % 100));
        //    bgColor.emplace_back(Color(ballColors.back().r - 50, ballColors.back().g - 50, ballColors.back().b - 50));
        //}

        balls.resize(ballCount);
        wallBreak.resize(ballCount + 1);
        ballTrail.resize(ballCount);
        GenCircle();

        snowFlakeTexture.loadFromFile("snowflake.png");
        snowFlakeSystem.setTexture(&snowFlakeTexture);
        snowFlakeSystem.setParticleType(zle::ParticleSystem::ParticleType::QuadsTriangles);
        snowFlakeSystem.setCreateOnUpdate(false);
        snowFlakeSystem.setStartForce(Vector2f(0, 5));
        snowFlakeSystem.setStartSize(50);
        snowFlakeSystem.setEndSize(10);
        snowFlakeSystem.setLifeTime(25);
        snowFlakeSystem.setEndRotation(1080);
        snowFlakeSystem.setRandomStartSize(10);
        snowFlakeSystem.setFading(0.5);

        for (int i = 0; i < balls.size(); i++)
        {
            balls[i].ball.setRadius(ballRadius);
            balls[i].ball.setPosition(ballPos[i]);
            balls[i].ball.setOrigin(balls[i].ball.getRadius(), balls[i].ball.getRadius());
            balls[i].dir = Vector2f(1 + rand() % 2 * -2, 1 + rand() % 2 * -2);
            balls[i].ball.move((static_cast<float>(rand()) / RAND_MAX * 2 - 1) * canvasSize.x / 10.f,
                        (static_cast<float>(rand()) / RAND_MAX * 2 - 1) * canvasSize.y / 10.f);
            balls[i].ball.setFillColor(ballColors[i]);
            ballTrail[i].loadFromFile("ballTrail.psy");
            ballTrail[i].setTexture(&circle);
            ballTrail[i].useBothColors(false);
            ballTrail[i].setStartColor(ballColors[i]);
            ballTrail[i].setCreateOnUpdate(false);
            ballTrail[i].setStartSize(balls[i].ball.getRadius() / 1.3);
            ballTrail[i].setEndSize(balls[i].ball.getRadius() / 10);
            ballTrail[i].setLifeTime(0.3);
        }
        for (int i = 0; i < wallBreak.size(); i++)
        {
            wallBreak[i].loadFromFile("wallBreak.psy");
            wallBreak[i].setCreateOnUpdate(false);
            wallBreak[i].setStartColor(bgColor[i]);
            wallBreak[i].setEndColor(bgColor[i]);
            wallBreak[i].setStartSize(static_cast<float>(canvasSize.x) / mapSize.x / 3);
            wallBreak[i].setMaxParticles(200);
        }
        wallBreak[0].setMaxParticles(1000);

        view.reset(FloatRect(0, 0, canvasSize.x, canvasSize.y));
        map.resize(mapSize.x);
        for (auto& n : map)
            n.resize(mapSize.y);

        counters.resize(balls.size());
        totalTiles.resize(balls.size());

        arr.resize(6 * mapSize.x * mapSize.y);
        arr.setPrimitiveType(Triangles);
        const Vector2f tileSize = Vector2f(static_cast<float>(canvasSize.x) / mapSize.x, static_cast<float>(canvasSize.y) / mapSize.y);
        for (int i = 0; i < mapSize.x; i++)
            for (int j = 0; j < mapSize.y; j++)
            {
                map[i][j] = 0;

                Vertex* ptr = &arr[(j + i * mapSize.y) * 6];
                ptr[0].position = Vector2f(tileSize.x * i, tileSize.y * j);
                ptr[1].position = Vector2f(tileSize.x * (i + 1), tileSize.y * j);
                ptr[2].position = Vector2f(tileSize.x * (i + 1), tileSize.y * (j + 1));
                ptr[3].position = Vector2f(tileSize.x * i, tileSize.y * (j + 1));
                for (int k = 0; k < 4; k++)
                {
                    ptr[k].texCoords = ptr[k].position;
                    ptr[k].color = bgColor[map[i][j]];
                }

                ptr[4] = ptr[0];
                ptr[5] = ptr[2];
            }
        buff.create(arr.getVertexCount());
        buff.setPrimitiveType(Triangles);
        buff.update(&arr[0]);

        font.loadFromFile("Montserrat.ttf");
        timerText.setFont(font);
        timerText.setCharacterSize(50);
        timerText.setPosition(Vector2f(canvasSize.x / 30, canvasSize.y / 30));
        for (int i = 0; i < counters.size(); i++)
        {
            counters[i].setFont(font);
            counters[i].setFillColor(ballColors[i]);
            counters[i].setOutlineColor(Color(255, 255, 255, 96));
            counters[i].setOutlineThickness(0);
            counters[i].setCharacterSize(80);
            counters[i].setPosition(canvasSize.x / 2 + (static_cast<float>(i) / (counters.size() - 1) - 0.5) * canvasSize.x / 2, canvasSize.y / 10 * 9);

            totalTiles[i] = 0;
            counters[i].setString("0");
            counters[i].setOrigin(counters[i].getLocalBounds().width / 2, counters[i].getLocalBounds().height / 2);
        }
#ifdef FANCYMODE
        bgShader.loadFromFile("defaultVertex.glsl", "bgShader.glsl");
        bgShader.setUniform("goldColor", Glsl::Vec4(bgColor[3].r / 255.f, bgColor[3].g / 255.f, bgColor[3].b / 255.f, 1));
        bgShader.setUniform("whiteColor", Glsl::Vec4(bgColor[2].r / 255.f, bgColor[2].g / 255.f, bgColor[2].b / 255.f, 1));
        bgShader.setUniform("greenColor", Glsl::Vec4(bgColor[4].r / 255.f, bgColor[4].g / 255.f, bgColor[4].b / 255.f, 1));
#endif
        Update();
    }
    void Increment(int i, const Vector2i& pos)
    {
        counters[i].setString(to_string(totalTiles[i]));
        counters[i].setOrigin(counters[i].getLocalBounds().width / 2, counters[i].getLocalBounds().height / 2);
    }
    void BallUpdate(const Time& delta)
    {
        bool collided = false;
        for (int i = 0; i < ballCount; i++)
        {
            if (balls[i].dead)
                continue;
            Vector2f prevPos = balls[i].ball.getPosition();
            bool changedX = false;
            bool changedY = false;
            Vector2i tileID;
            balls[i].ball.move(balls[i].dir.x * delta.asSeconds() * ballSpeed, 0);
            if (Collided(i, tileID))
            {
                changedX = true;
                balls[i].dir.x = -balls[i].dir.x;
                collided = true;
                Increment(i, tileID);

                map[tileID.x][tileID.y] = i + 1;
                for (int j = 0; j < 6; j++)
                    arr[(tileID.y + tileID.x * mapSize.y) * 6 + j].color = bgColor[map[tileID.x][tileID.y]];
            }
            if (balls[i].ball.getPosition().x - ballRadius < 0)
            {
                changedX = true;
                balls[i].dir.x = 1;
            }
            if (balls[i].ball.getPosition().x + ballRadius >= canvasSize.x)
            {
                changedX = true;
                balls[i].dir.x = -1;
            }
            if (changedX)
                balls[i].ball.move(balls[i].dir.x * delta.asSeconds() * ballSpeed, 0);

            balls[i].ball.move(0, balls[i].dir.y * delta.asSeconds() * ballSpeed);
            if (Collided(i, tileID))
            {
                changedY = true;
                balls[i].dir.y = -balls[i].dir.y;
                collided = true;
                Increment(i, tileID);

                map[tileID.x][tileID.y] = i + 1;
                for (int j = 0; j < 6; j++)
                    arr[(tileID.y + tileID.x * mapSize.y) * 6 + j].color = bgColor[map[tileID.x][tileID.y]];
            }
            if (balls[i].ball.getPosition().y - ballRadius < 0)
            {
                changedY = true;
                balls[i].dir.y = 1;
            }
            if (balls[i].ball.getPosition().y + ballRadius >= canvasSize.y)
            {
                changedY = true;
                balls[i].dir.y = -1;
            }
            if (changedY)
                balls[i].ball.move(0, balls[i].dir.y * delta.asSeconds() * ballSpeed);
            const float steps = 4;
            for (int k = 0; k < steps; k++)
            {
                ballTrail[i].setSpawnPosition(prevPos + k / steps * (balls[i].ball.getPosition() - prevPos));
                ballTrail[i].Create();
                ballTrail[i].Update(delta / steps);
            }
        }
        for (int i = 0; i < balls.size(); i++)
            totalTiles[i] = 0;
        for (int i = 0; i < mapSize.x; i++)
            for (int j = 0; j < mapSize.y; j++)
            {
                if (map[i][j] > 0)
                    totalTiles[map[i][j] - 1]++;
            }
        for (int i = 0; i < balls.size(); i++)
        {
            if (balls[i].dead)
                continue;
            if (totalTiles[highestID] < totalTiles[i])
            {
                counters[highestID].setOutlineThickness(0);
                counters[i].setOutlineThickness(3);

                highestID = i;
            }
            if (totalTiles[lowestID] > totalTiles[i])
                lowestID = i;
        }
        if (collided)
            buff.update(&arr[0]);
    }
    void Update()
    {
        Clock clock;
        Time delta;
        Clock stopwatch;
        while (window.isOpen())
        {
            delta = clock.restart();
            Event event;
            while (window.pollEvent(event))
            {
#ifndef BORNAMODE
                if (event.type == Event::Closed)
                    window.close();
                if (event.type == Event::KeyReleased)
                {
                    if (event.key.code == Keyboard::Escape)
                        window.close();
                }
#endif
            }
#ifdef FANCYMODE
            bgShader.setUniform("time", stopwatch.getElapsedTime().asSeconds());
            if (snowFlakeClock.getElapsedTime().asSeconds() > 1)
            {
                snowFlakeClock.restart();
                snowFlakeSystem.setSpawnPosition(Vector2f(rand() % canvasSize.x, -50));
                snowFlakeSystem.Create();
            }
            snowFlakeSystem.Update(delta);
#endif
#ifdef CONTROLLABLE
            Vector2f ball0New = Vector2f();
            Vector2f ball1New = Vector2f();
            Vector2f ball2New = Vector2f();
            if (Keyboard::isKeyPressed(Keyboard::W))
                ball0New.y = -1;
            if (Keyboard::isKeyPressed(Keyboard::A))
                ball0New.x = -1;
            if (Keyboard::isKeyPressed(Keyboard::S))
                ball0New.y = 1;
            if (Keyboard::isKeyPressed(Keyboard::D))
                ball0New.x = 1;
            if (Keyboard::isKeyPressed(Keyboard::I))
                ball1New.y = -1;
            if (Keyboard::isKeyPressed(Keyboard::J))
                ball1New.x = -1;
            if (Keyboard::isKeyPressed(Keyboard::K))
                ball1New.y = 1;
            if (Keyboard::isKeyPressed(Keyboard::L))
                ball1New.x = 1;
            if (Keyboard::isKeyPressed(Keyboard::Numpad8))
                ball2New.y = -1;
            if (Keyboard::isKeyPressed(Keyboard::Numpad4))
                ball2New.x = -1;
            if (Keyboard::isKeyPressed(Keyboard::Numpad5))
                ball2New.y = 1;
            if (Keyboard::isKeyPressed(Keyboard::Numpad6))
                ball2New.x = 1;
            if (ball0New.x != 0 || ball0New.y != 0)
                balls[0].dir = normalize(ball0New);
            if (ball1New.x != 0 || ball1New.y != 0)
                balls[1].dir = normalize(ball1New);
            if (ball2New.x != 0 || ball2New.y != 0)
                balls[2].dir = normalize(ball2New);
#endif
#ifdef SWAPCOLORS
            swapColors -= delta;
            if (swapColors < Time::Zero)
            {
                swapColors = seconds(swapColorsCnt);
                bgColor.push_back(bgColor[1]);
                bgColor.erase(bgColor.begin() + 1);
                ballColors.push_back(ballColors[0]);
                ballColors.erase(ballColors.begin());
                for (int i = 0; i < ballCount; i++)
                {
                    balls[i].ball.setFillColor(ballColors[i]);
                    ballTrail[i].setStartColor(ballColors[i]);
                    wallBreak[i + 1].setStartColor(bgColor[i + 1]);
                    wallBreak[i + 1].setEndColor(bgColor[i + 1]);
                    counters[i].setFillColor(ballColors[i]);
                }
                for (int i = 0; i < mapSize.x; i++)
                    for (int j = 0; j < mapSize.y; j++)
                    {
                        int index = map[i][j];
                        if (index == 0)
                            continue;
                        for (int k = 0; k < 6; k++)
                            arr[(i * mapSize.y + j) * 6 + k].color = bgColor[index];
                    }
            }
#endif
#ifdef TIMERMODE
            timer -= delta;
            if (timer < Time::Zero)
            {
                if (lowestID == highestID)
                    timer = Time::Zero;
                else
                {
                    balls[lowestID].dead = true;
                    for (int i = 0; i < mapSize.x; i++)
                        for (int j = 0; j < mapSize.y; j++)
                            if (map[i][j] == lowestID + 1)
                            {
                                map[i][j] = 0;
                                for (int k = 0; k < 6; k++)
                                    arr[(i * mapSize.y + j) * 6 + k].color = bgColor[map[i][j]];
                            }
                    lowestID = highestID;
                    timer = seconds(totalTimerCnt);
                }
            }
            timerText.setString(to_string(timer.asMilliseconds() / 1000) + "." + to_string(timer.asMilliseconds() % 1000 / 100));
#endif
            BallUpdate(delta);

            for (int i = 0; i < wallBreak.size(); i++)
                wallBreak[i].Update(delta);

            window.clear(bgColor[2]);
            window.setView(view);

            window.draw(snowFlakeSystem);
#ifdef FANCYMODE
            if (VertexBuffer::isAvailable())
                window.draw(buff, &bgShader);
            else
                window.draw(arr, &bgShader);
#else
            if (VertexBuffer::isAvailable())
                window.draw(buff);
            else
                window.draw(arr);
#endif

            for (int i = 0; i < wallBreak.size(); i++)
                window.draw(wallBreak[i]);
            for (int i = 0; i < ballTrail.size(); i++)
            {
                if (balls[i].dead)
                    continue;
                window.draw(ballTrail[i]);
            }

            for (auto& n : balls)
            {
                if (n.dead)
                    continue;
                window.draw(n.ball);
            }
            for (int i = 0; i < balls.size(); i++)
            {
                if (balls[i].dead)
                    continue;
                window.draw(counters[i]);
            }
            window.draw(timerText);
            window.display();
        }
    }
};
int main()
{
    ToInfinity app;
    app.Start();
}