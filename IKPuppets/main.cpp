#include <iostream>
#include <SFML/Graphics.hpp>
#include <Eigen/Dense>

static constexpr int BONE_COUNT = 8;
static Eigen::Translation2f offset(640, 640);

template<int N>
struct Bones {
    int parent[N];
    float length[N];
    Eigen::Matrix<float, N, 1> rotation;
    Eigen::Transform<float, 2, Eigen::Affine> worldTransform[N];
    size_t size = N;
};

template <int N>
Eigen::Rotation2D<float> localTransform(const Bones<N>& bones, int i) {
    return Eigen::Rotation2D<float>(bones.rotation(i));
}

template <int N>
void updateTransforms(Bones<N>& bones) {
    Eigen::Transform<float, 2, Eigen::Affine> parentTransform = Eigen::Transform<float, 2, Eigen::Affine>(offset);
    float parentBoneLength = 0.0f;
    for (int i = 0; i < N; i++) {
        bones.worldTransform[i] = parentTransform * Eigen::Translation<float, 2>(parentBoneLength, 0) * Eigen::Rotation2D<float>(bones.rotation(i));
        parentTransform = bones.worldTransform[i];
        parentBoneLength = bones.length[i];
    }
}

template<int N>
void drawBones(const Bones<N>& bones, const std::vector<sf::ConvexShape>& boneShapes, sf::RenderTarget& target) {
    for (size_t i = 0; i < bones.size; i++) {
        const Eigen::Transform<float, 2, Eigen::Affine>& transform = bones.worldTransform[i];
        sf::Transform t(
            transform(0,0), transform(0,1), transform(0,2),
            transform(1,0), transform(1,1), transform(1,2),
            transform(2,0), transform(2,1), transform(2,2)
        );
        target.draw(boneShapes[i], t);
    }
}

template <int N>
Eigen::Vector2f jointPosition(Bones<N>& bones, int i) {
    // todo: if we give bones.length the same treatment we did bones.rotation then we can just make this a single mul
    const Eigen::Transform<float, 2, Eigen::Affine>& world = bones.worldTransform[i];
    const Eigen::Vector3f point(bones.length[i], 0, 1);
    const Eigen::Vector3f transformed = world * point;
    return Eigen::Vector2f(transformed.x(), transformed.y());
}

template <int N>
void jacobianIK(Bones<N>& bones, sf::Vector2f target) {
    const Eigen::Vector2f mousePos(target.x, target.y);
    Eigen::Vector2f endPos;

    int giveUp = 500;
    for (;;) {
        endPos = jointPosition(bones, N - 1);
        Eigen::Matrix<float, 2, N> jacobian;
        for (int i = 0; i < N; i++)
        {
            const Eigen::Vector2f jointPos = jointPosition(bones, i);
            const Eigen::Vector2f jointToEnd = endPos - jointPos;
            jacobian.col(i) = Eigen::Vector2f(-jointToEnd.y(), jointToEnd.x());
        }
        
        const Eigen::Matrix<float, N, 1> d0 = jacobian.transpose() * (mousePos - endPos);
        bones.rotation += d0 * .000001f;
        updateTransforms(bones);

        const Eigen::Vector2f endToMouseVector = mousePos - endPos;
        const float magnitude = endToMouseVector.norm();
        if (magnitude < 10)
            break;
        if (giveUp-- == 0)
            break;
    }
}

int main() {
    // Init
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    sf::RenderWindow window(sf::VideoMode(1280, 1280), "IK Puppets", sf::Style::Default, settings);

    sf::Clock clock;
    sf::Vector2f target;

    bool wiggleMode = true;
    
    // init bones
    Bones<BONE_COUNT> bones;
    for (size_t i = 0; i < bones.size; i++) {
        bones.parent[i] = (int)i - 1;
        bones.length[i] = 100.f - 10 * i;
    }

    // init boneshapes
    std::vector<sf::ConvexShape> boneShapes;
    for (size_t i = 0; i < bones.size; i++) {
        sf::ConvexShape shape;
        shape.setFillColor(sf::Color::Cyan);
        shape.setPointCount(4);
        shape.setPoint(0, sf::Vector2f(0, 0));
        shape.setPoint(1, sf::Vector2f(10, 10));
        shape.setPoint(2, sf::Vector2f(bones.length[i], 0));
        shape.setPoint(3, sf::Vector2f(10, -10));
        boneShapes.emplace_back(shape);
    }

    // mouse target dot
    const float rad = 10.f;
    sf::CircleShape dot(rad);
    dot.setFillColor(sf::Color::Yellow);

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
            case sf::Event::KeyPressed:
                switch (event.key.code) {
                case sf::Keyboard::Space:
                    wiggleMode = !wiggleMode;
                    break;
                }
                break;
            case sf::Event::MouseMoved:
                target = sf::Vector2f{ (float)event.mouseMove.x, (float)event.mouseMove.y };
                break;
            default:
                break;
            }
        }

        // render stuff
        {
            window.clear(sf::Color(32, 32, 32, 255));

            const float dt = clock.getElapsedTime().asSeconds();

            if (wiggleMode) {
                for (size_t i = 0; i < bones.size; i++) {
                    bones.rotation(i) = 10.f * i * cos(dt * (1.0f + i));
                }
                bones.rotation(0) = dt * 4.f;
                updateTransforms(bones);
            }
            else {
                dot.setPosition(target - sf::Vector2f{ rad, rad });
                window.draw(dot);
                jacobianIK(bones, target);
            }            

            drawBones(bones, boneShapes, window);

            window.display();
        }
    }

    return 0;
}
