#include <iostream>
#include <SFML/Graphics.hpp>

struct Joint {
    Joint* parent;
    float length;
    sf::Transform transform;
};

void drawBone(const Joint& joint, sf::RenderTarget& target) {
    sf::ConvexShape shape;
    shape.setFillColor(sf::Color::Cyan);
    shape.setPointCount(4);
    shape.setPoint(0, sf::Vector2f(-10, 0));
    shape.setPoint(1, sf::Vector2f(0, 10));
    shape.setPoint(2, sf::Vector2f(joint.length - 10.0f, 0));
    shape.setPoint(3, sf::Vector2f(0, -10));

    sf::Transform myTransform = joint.transform;
    const Joint* current = &joint;
    while (current = current->parent) {
        myTransform = current->transform * myTransform;
    }

    target.draw(shape, myTransform);
}

int main() {
    // Init
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    sf::RenderWindow window(sf::VideoMode(1280, 720), "IK Puppets", sf::Style::Default, settings);

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

            sf::Transform jointTransform = sf::Transform();
            jointTransform
                .translate({ 100, 100 })
                .rotate(10.f);
            
            Joint root = { nullptr, 7.0f, jointTransform };
            Joint child = { &root, 7.0f, jointTransform };
            Joint end = { &child, 7.0f, jointTransform };
            
            drawBone(root, window);
            drawBone(upper, window);
            drawBone(lower, window);
            drawBone(end, window);

            window.display();
        }
    }

    return 0;
}
