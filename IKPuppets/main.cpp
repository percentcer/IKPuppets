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
    shape.setPoint(0, sf::Vector2f(-1, 0));
    shape.setPoint(1, sf::Vector2f(0, 1));
    shape.setPoint(2, sf::Vector2f(joint.length - 1.0f, 0));
    shape.setPoint(3, sf::Vector2f(0, -1));
    shape.scale({ 20.f, 20.f });

    sf::Transform myTransform = joint.transform;
    const Joint* current = &joint;
    while (current = current->parent) {
        myTransform = current->transform * myTransform;
    }

    target.draw(shape, myTransform);
}

int main() {
    // Init
    sf::RenderWindow window(sf::VideoMode(1280, 720), "IK Puppets");
    sf::RenderTexture frame;
    frame.create(window.getSize().x, window.getSize().y);

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
            window.clear();
            frame.clear(sf::Color(32,32,32,255));

            sf::Transform jointTransform = sf::Transform();
            jointTransform
                .translate({ 100, 100 })
                .rotate(10.f);
            
            Joint root = { nullptr, 7.0f, jointTransform };
            Joint child = { &root, 7.0f, jointTransform };
            Joint end = { &child, 7.0f, jointTransform };
            
            drawBone(root, frame);
            drawBone(child, frame);
            drawBone(end, frame);

            frame.display();
            sf::Sprite sframe{ frame.getTexture() };

            window.draw(sframe);
            
            window.display();
        }
    }

    return 0;
}
