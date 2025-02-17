#include <SFML/Graphics.hpp>
#include <Eigen/Dense>
#include <vector>
#include <iostream>
#include <imgui.h>
#include <imgui-SFML.h>
#include <cmath>

// Définition de la structure Planète
struct Planet {
    Eigen::Vector2d position;  // Position de la planète
    Eigen::Vector2d velocity;  // Vitesse de la planète
    float mass;                // Masse de la planète (modifiée en float)
    sf::CircleShape shape;     // Forme graphique de la planète

    // Constructeur de la planète
    Planet(double x, double y, double vx, double vy, float m, sf::Color color) 
        : position(x, y), velocity(vx, vy), mass(m) {
        shape.setRadius(5);          // Définir le rayon de la planète
        shape.setFillColor(color);   // Définir la couleur de la planète
        shape.setOrigin(5, 5);       // Définir l'origine de la forme au centre
    }

    // Mettre à jour la position de la planète en fonction de sa vitesse
    void updatePosition(double dt) {
        position += velocity * dt;
        shape.setPosition(position.x(), position.y());
    }

    // Mettre à jour la position de la planète directement
    void setPosition(double x, double y) {
        position = Eigen::Vector2d(x, y);
        shape.setPosition(position.x(), position.y());
    }
};

// Variable globale pour la constante gravitationnelle
float G = 1.0;

// Fonction pour calculer les forces gravitationnelles entre deux planètes
Eigen::Vector2d computeGravity(const Planet& p1, const Planet& p2) {
    Eigen::Vector2d diff = p2.position - p1.position;  // Différence de position entre les deux planètes
    double dist = diff.norm() + 1e-6;  // Distance entre les deux planètes (ajout d'une petite valeur pour éviter la division par zéro)
    return G * (p1.mass * p2.mass) / (dist * dist) * diff.normalized();  // Force gravitationnelle
}

// Fonction pour dessiner une flèche
void drawArrow(sf::RenderWindow& window, const sf::Vector2f& start, const sf::Vector2f& end, const sf::Color& color) {
    sf::Vertex line[] = {
        sf::Vertex(start, color),
        sf::Vertex(end, color)
    };
    window.draw(line, 2, sf::Lines);

    // Calculer la direction de la flèche
    sf::Vector2f direction = end - start;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    direction /= length;

    // Définir les points de la tête de la flèche
    sf::Vector2f arrowHead1 = end - direction * 10.f + sf::Vector2f(-direction.y, direction.x) * 5.f;
    sf::Vector2f arrowHead2 = end - direction * 10.f - sf::Vector2f(-direction.y, direction.x) * 5.f;

    sf::Vertex arrowHead[] = {
        sf::Vertex(end, color),
        sf::Vertex(arrowHead1, color),
        sf::Vertex(end, color),
        sf::Vertex(arrowHead2, color)
    };
    window.draw(arrowHead, 4, sf::Lines);
}

// Fonction pour détecter les collisions entre deux planètes
bool detectCollision(const Planet& p1, const Planet& p2) {
    double dist = (p2.position - p1.position).norm();
    return dist < (p1.shape.getRadius() + p2.shape.getRadius());
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Three-Body Problem");  // Créer une fenêtre SFML
    if (!ImGui::SFML::Init(window)) {  // Initialiser ImGui-SFML et vérifier le retour
        std::cerr << "Failed to initialize ImGui-SFML" << std::endl;
        return -1;
    }

    // Initialiser les planètes avec des valeurs prédéfinies
    std::vector<Planet> planets = {
        Planet(200, 300, 0, 1, 10.0f, sf::Color::Red),
        Planet(400, 300, 0, -1, 10.0f, sf::Color::Green),
        Planet(600, 300, 0, 0, 10.0f, sf::Color::Blue)
    };

    double dt = 0.1;  // Intervalle de temps pour la simulation
    bool isRunning = false;  // Indicateur pour savoir si la simulation est en cours
    bool collisionOccurred = false;  // Indicateur pour savoir si une collision a eu lieu
    sf::CircleShape collisionCircle;  // Cercle pour représenter la collision
    collisionCircle.setRadius(50);  // Rayon du cercle de collision
    collisionCircle.setFillColor(sf::Color(255, 255, 255, 255));  // Couleur du cercle de collision avec transparence
    collisionCircle.setOrigin(50, 50);  // Définir l'origine du cercle au centre
    float collisionAlpha = 255.0f;  // Transparence du cercle de collision

    // Correction : sf::VertexArray au lieu de std::vector<sf::VertexArray>
    std::vector<sf::VertexArray> trajectories(planets.size());

    // Initialiser chaque VertexArray avec le type de primitive
    for (auto& va : trajectories) {
        va.setPrimitiveType(sf::LineStrip);
    }

    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);  // Traiter les événements ImGui
            if (event.type == sf::Event::Closed) {
                window.close();  // Fermer la fenêtre si l'événement de fermeture est détecté
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) {
                    isRunning = !isRunning;  // Basculer entre pause et lecture
                } else if (event.key.code == sf::Keyboard::R) {
                    // Réinitialiser les planètes avec des valeurs prédéfinies
                    planets = {
                        Planet(200, 300, 0, 1, 10.0f, sf::Color::Red),
                        Planet(400, 300, 0, -1, 10.0f, sf::Color::Green),
                        Planet(600, 300, 0, 0, 10.0f, sf::Color::Blue)
                    };
                    trajectories.clear();  // Effacer les trajectoires
                    trajectories.resize(planets.size());  // Redimensionner les trajectoires
                    collisionOccurred = false;  // Réinitialiser l'indicateur de collision
                    collisionAlpha = 255.0f;  // Réinitialiser la transparence du cercle de collision
                }
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());  // Mettre à jour ImGui-SFML

        ImGui::Begin("Planet Parameters");  // Créer une fenêtre ImGui
        bool valuesChanged = false;  // Indicateur pour savoir si les valeurs ont changé
        for (size_t i = 0; i < planets.size(); ++i) {
            ImGui::PushID(static_cast<int>(i));  // Ajouter un identifiant pour chaque planète
            ImGui::Text("Planet %d", static_cast<int>(i + 1));
            float position[2] = { static_cast<float>(planets[i].position.x()), static_cast<float>(planets[i].position.y()) };
            float velocity[2] = { static_cast<float>(planets[i].velocity.x()), static_cast<float>(planets[i].velocity.y()) };
            valuesChanged |= ImGui::SliderFloat2("Position", position, 0.0f, 800.0f);
            valuesChanged |= ImGui::SliderFloat2("Velocity", velocity, -1.0f, 1.0f);
            valuesChanged |= ImGui::SliderFloat("Mass", &planets[i].mass, 1.0f, 100.0f);
            planets[i].setPosition(position[0], position[1]);  // Mettre à jour la position en temps réel
            planets[i].velocity = Eigen::Vector2d(velocity[0], velocity[1]);
            ImGui::PopID();  // Retirer l'identifiant
        }
        valuesChanged |= ImGui::SliderFloat("Gravitational Constant (G)", &G, 0.1f, 10.0f);  // Ajouter un slider pour G
        ImGui::End();

        if (valuesChanged) {
            // Effacer les trajectoires si les valeurs ont changé
            trajectories.clear();
            trajectories.resize(planets.size());
        }

        if (isRunning) {
            std::vector<Eigen::Vector2d> forces(planets.size(), Eigen::Vector2d(0, 0));  // Initialiser les forces

            for (size_t i = 0; i < planets.size(); ++i) {
                for (size_t j = 0; j < planets.size(); ++j) {
                    if (i != j) {
                        forces[i] += computeGravity(planets[i], planets[j]);  // Calculer les forces gravitationnelles
                    }
                }
            }

            for (size_t i = 0; i < planets.size(); ++i) {
                planets[i].velocity += forces[i] / planets[i].mass * dt;  // Mettre à jour la vitesse des planètes
                planets[i].updatePosition(dt);  // Mettre à jour la position des planètes

                // Ajouter un point à la VertexArray
                sf::Vertex point;
                point.position = sf::Vector2f(planets[i].position.x(), planets[i].position.y());
                point.color = planets[i].shape.getFillColor();
                trajectories[i].append(point);  // Ajouter le point à la trajectoire
            }

            // Détecter les collisions
            for (size_t i = 0; i < planets.size(); ++i) {
                for (size_t j = i + 1; j < planets.size(); ++j) {
                    if (detectCollision(planets[i], planets[j])) {
                        collisionOccurred = true;
                        collisionCircle.setPosition(planets[i].position.x(), planets[i].position.y());
                        planets.erase(planets.begin() + j);
                        planets.erase(planets.begin() + i);
                        trajectories.erase(trajectories.begin() + j);
                        trajectories.erase(trajectories.begin() + i);
                        break;
                    }
                }
                if (collisionOccurred) {
                    break;
                }
            }
        }

        window.clear();  // Effacer la fenêtre

        // Dessiner chaque VertexArray
        for (const auto& va : trajectories) {
            window.draw(va);
        }

        for (const auto& planet : planets) {
            window.draw(planet.shape);  // Dessiner chaque planète
        }

        // Dessiner les flèches de vélocité si la simulation est en pause
        if (!isRunning) {
            for (const auto& planet : planets) {
                sf::Vector2f start(planet.position.x(), planet.position.y());
                sf::Vector2f end = start + sf::Vector2f(planet.velocity.x(), planet.velocity.y()) * 100.0f;  // Échelle de la flèche
                drawArrow(window, start, end, planet.shape.getFillColor());
            }
        }

        // Dessiner le cercle de collision si une collision a eu lieu
        if (collisionOccurred) {
            collisionAlpha -= 1.0f;  // Diminuer la transparence du cercle de collision
            if (collisionAlpha < 0.0f) {
                collisionAlpha = 0.0f;
                collisionOccurred = false;  // Réinitialiser l'indicateur de collision lorsque le cercle a complètement disparu
            }
            collisionCircle.setFillColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(collisionAlpha)));
            window.draw(collisionCircle);
        }

        ImGui::SFML::Render(window);  // Rendre ImGui-SFML
        window.display();  // Afficher le contenu de la fenêtre
    }

    ImGui::SFML::Shutdown();  // Arrêter ImGui-SFML
    return 0;
}