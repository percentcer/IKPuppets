#include <iostream>
#include <SFML/Graphics.hpp>

struct Joint {
    Joint* parent;
    float length;
    float rotation;
};

sf::Transform jointTransform(const Joint& joint) {
    float parentLength = joint.parent ? joint.parent->length : 0.0f;
    return sf::Transform()
        .translate({ parentLength, 0.0f })
        .rotate(joint.rotation);
}

void drawBone(const Joint& joint, sf::RenderTarget& target) {
    sf::ConvexShape shape;
    shape.setFillColor(sf::Color::Cyan);
    shape.setPointCount(4);
    shape.setPoint(0, sf::Vector2f(-10, 0));
    shape.setPoint(1, sf::Vector2f(0, 10));
    shape.setPoint(2, sf::Vector2f(joint.length - 10.0f, 0));
    shape.setPoint(3, sf::Vector2f(0, -10));

    sf::Transform myTransform = jointTransform(joint);
    const Joint* current = &joint;
    while (current = current->parent) {
        myTransform = jointTransform(*current) * myTransform;
    }

    target.draw(shape, myTransform);
}

int main() {
    // Init
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    sf::RenderWindow window(sf::VideoMode(1280, 720), "IK Puppets", sf::Style::Default, settings);

    sf::Clock clock;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            default:
                break;
            }
        }

        // render stuff
        {
            window.clear(sf::Color(32, 32, 32, 255));

            const float dt = clock.getElapsedTime().asSeconds();
            Joint root = { nullptr, 200.0f, dt };
            Joint upper = { &root, 80.0f, 20.f * cos(dt / 2.f)};
            Joint lower = { &upper, 60.0f, 40.f * cos(dt) };
            Joint end = { &lower, 50.0f, 50.0f * sin(dt) };
            
            drawBone(root, window);
            drawBone(upper, window);
            drawBone(lower, window);
            drawBone(end, window);

            window.display();
        }
    }

    return 0;
}
