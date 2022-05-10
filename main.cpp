#include <SFML/Graphics.hpp>
#include <math.h>
#include <iostream>

using namespace std;
using namespace sf;

#define SCR_WIDTH 1600
#define SCR_HEIGHT 900
#define BOID_SPEED 200
#define LOCAL_RADIUS 70
#define CLOSEST_RADIUS 10
#define AVOIDANCE_RADIUS 250
#define BOID_SIZE 5

#define ALIGNMENT_FORCE 5
#define COHESION_FORCE 1
#define SEPARATION_FORCE 20
#define AVOIDANCE_FORCE 10

#define NUMBER_OF_BOIDS

struct Boid
{
    double x, y;
    double velX, velY, rotation;
    Color color;
};

double degToRad(int deg)
{
    return M_PI*deg/180;
}

double radToDeg(double rad)
{
    return rad*180/M_PI;
}

int dist(int x1, int y1, int x2, int y2)
{
    return sqrt((x2-x1) * (x2-x1) + (y2-y1) * (y2-y1));
}

void normalize(double& vx, double& vy, double speed)
{
    double velocity = sqrt(vx*vx + vy*vy);
    vx *= speed / velocity;
    vy *= speed / velocity;
}

void rotate(double& vx, double& vy, double& r)
{
    r = radToDeg(atan2(vy, vx)) + 90;
}

int main(int argc, char* argv[])
{
    RenderWindow window(sf::VideoMode(SCR_WIDTH, SCR_HEIGHT), "Boid test");
    CircleShape boidShape(BOID_SIZE, 3);
    CircleShape obstacleShape(50);
    boidShape.setOrigin(BOID_SIZE, BOID_SIZE);
    obstacleShape.setOrigin(50, 50);
    obstacleShape.setFillColor(sf::Color(33, 69, 17));
    boidShape.setScale(1, 2);
    boidShape.setFillColor(sf::Color::Blue);
    Clock clock;
    clock.restart();
    window.setFramerateLimit(60);
    int lastClicked = 100;

    int numberOfBoids = NUMBER_OF_BOIDS;
    if(argc > 1)
    {
        numberOfBoids = atoi(argv[1]);
    }

    vector<Boid> Boids(numberOfBoids);
    vector<Vector2i> obstacles;

    for(auto boid = Boids.begin(); boid != Boids.end(); boid++)
    {
        boid->x = rand()%SCR_WIDTH;
        boid->y = rand()%SCR_HEIGHT;
        boid->velX = rand()%200 - 100;
        boid->velY = rand()%200 - 100;
        normalize(boid->velX, boid->velY, BOID_SPEED);
        boid->color = Color(rand()%100 + 50, rand()%100 + 50, rand()%100 + 50);
    }

    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
        }
        if(Mouse::isButtonPressed(Mouse::Left) && lastClicked >= 2)
        {
            obstacles.push_back(Vector2i(Mouse::getPosition(window).x, Mouse::getPosition(window).y));
            lastClicked = 0;
        }
        lastClicked++;
        if(lastClicked > 10000) lastClicked = 200;

        window.clear(sf::Color(10, 43, 97));
        double elapsed = clock.getElapsedTime().asSeconds();
        clock.restart();
        for(auto obstacle = obstacles.begin(); obstacle != obstacles.end(); obstacle++)
        {
            obstacleShape.setPosition(obstacle->x, obstacle->y);
            window.draw(obstacleShape);
        }
        for(auto boid = Boids.begin(); boid != Boids.end(); boid++)
        {
            // Calculations
            double averageVelocityX = 0, averageVelocityY = 0;
            double averagePositionX = 0, averagePositionY = 0;
            double closestAveragePositionX = 0, closestAveragePositionY = 0;
            int count = 0, closestCount = 0;
            for(auto other = Boids.begin(); other != Boids.end(); other++)
            {
                double radius = dist(boid->x, boid->y, other->x, other->y);
                if(other != boid && radius <= LOCAL_RADIUS)
                {
                    averageVelocityX += other->velX;
                    averageVelocityY += other->velY;
                    averagePositionX += other->x;
                    averagePositionY += other->y;
                    count++;
                    if(radius <= CLOSEST_RADIUS)
                    {
                        closestCount++;
                        closestAveragePositionX += other->x;
                        closestAveragePositionY += other->y;
                    }
                }
            }
            if(count > 0)
            {
                averageVelocityX /= count;
                averageVelocityY /= count;
                averagePositionX /= count;
                averagePositionY /= count;

                double steeringAlignmentX = averageVelocityX - boid->velX;
                double steeringAlignmentY = averageVelocityY - boid->velY;
                double steeringCohesionX = averagePositionX - boid->x;
                double steeringCohesionY = averagePositionY - boid->y;

                normalize(steeringAlignmentX, steeringAlignmentY, ALIGNMENT_FORCE);
                normalize(steeringCohesionX, steeringCohesionY, COHESION_FORCE);

                boid->velX += steeringAlignmentX + steeringCohesionX;
                boid->velY += steeringAlignmentY + steeringCohesionY;
                normalize(boid->velX, boid->velY, BOID_SPEED);
            }

            if(closestCount > 0)
            {
                closestAveragePositionX /= closestCount;
                closestAveragePositionY /= closestCount;

                double steeringSeparationX = boid->x - closestAveragePositionX;
                double steeringSeparationY = boid->y - closestAveragePositionY;

                normalize(steeringSeparationX, steeringSeparationY, SEPARATION_FORCE);

                boid->velX += steeringSeparationX;
                boid->velY += steeringSeparationY;
                normalize(boid->velX, boid->velY, BOID_SPEED);
            }

            if(dist(boid->x, boid->y, Mouse::getPosition(window).x, Mouse::getPosition(window).y) <= AVOIDANCE_RADIUS)
            {
                double steeringAvoidanceX = boid->x - Mouse::getPosition(window).x;
                double steeringAvoidanceY = boid->y - Mouse::getPosition(window).y;

                normalize(steeringAvoidanceX, steeringAvoidanceY, AVOIDANCE_FORCE);

                boid->velX += steeringAvoidanceX;
                boid->velY += steeringAvoidanceY;
                normalize(boid->velX, boid->velY, BOID_SPEED);
            }

            for(auto obstacle = obstacles.begin(); obstacle != obstacles.end(); obstacle++)
            {
                if(dist(boid->x, boid->y, obstacle->x, obstacle->y) <= AVOIDANCE_RADIUS)
                {
                    double steeringAvoidanceX = boid->x - obstacle->x;
                    double steeringAvoidanceY = boid->y - obstacle->y;

                    normalize(steeringAvoidanceX, steeringAvoidanceY, AVOIDANCE_FORCE);

                    boid->velX += steeringAvoidanceX;
                    boid->velY += steeringAvoidanceY;
                    normalize(boid->velX, boid->velY, BOID_SPEED);
                }
            }
            
            // Movement
            boid->x += boid->velX * elapsed;
            boid->y += boid->velY * elapsed;

            if(boid->x < -BOID_SIZE*2 ) boid->x = SCR_WIDTH + BOID_SIZE;
            if(boid->x > SCR_WIDTH + BOID_SIZE*2) boid->x = -BOID_SIZE;
            if(boid->y < -BOID_SIZE*2) boid->y = SCR_HEIGHT + BOID_SIZE;
            if(boid->y > SCR_HEIGHT + BOID_SIZE*2) boid->y = -BOID_SIZE;

            // Rendering
            boidShape.setPosition(boid->x, boid->y);
            rotate(boid->velX, boid->velY, boid->rotation);
            boidShape.setRotation(boid->rotation);
            boidShape.setFillColor(boid->color);
            window.draw(boidShape);
        }
        window.display();
    }

    return 0;
}