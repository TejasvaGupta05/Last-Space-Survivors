
#include "window.hpp"
#include "player.hpp"
#include "enemy.hpp"
#include "Background.hpp"
#include "effects.hpp"
#include "TitleScreen.hpp"
#include "MusicGenerator.hpp"
#include "Coin.hpp"
#include "HUD.hpp" // NEW
#include <vector>
#include <random>
#include <algorithm>
#include <iostream>
#include <map>
#include <optional>
#include <cmath>
#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/Text.hpp>
#include <limits>
#include <cstdint>
#include <memory>

using namespace std;

inline bool containsPoint(const sf::FloatRect &rect, const sf::Vector2f &point)
{
    return point.x >= rect.position.x && point.x <= rect.position.x + rect.size.x &&
           point.y >= rect.position.y && point.y <= rect.position.y + rect.size.y;
}

// Helper for ray-box intersection
bool rayBoxIntersect(sf::Vector2f rayOrigin, sf::Vector2f rayDir, sf::FloatRect box, float& distance) {
    float tmin = 0.0f;
    float tmax = std::numeric_limits<float>::max();

    sf::Vector2f min = {box.position.x, box.position.y};
    sf::Vector2f max = {box.position.x + box.size.x, box.position.y + box.size.y};

    // X axis
    if (std::abs(rayDir.x) < 0.0001f) {
        if (rayOrigin.x < min.x || rayOrigin.x > max.x) return false;
    } else {
        float ood = 1.0f / rayDir.x;
        float t1 = (min.x - rayOrigin.x) * ood;
        float t2 = (max.x - rayOrigin.x) * ood;
        if (t1 > t2) std::swap(t1, t2);
        tmin = std::max(tmin, t1);
        tmax = std::min(tmax, t2);
        if (tmin > tmax) return false;
    }

    // Y axis
    if (std::abs(rayDir.y) < 0.0001f) {
        if (rayOrigin.y < min.y || rayOrigin.y > max.y) return false;
    } else {
        float ood = 1.0f / rayDir.y;
        float t1 = (min.y - rayOrigin.y) * ood;
        float t2 = (max.y - rayOrigin.y) * ood;
        if (t1 > t2) std::swap(t1, t2);
        tmin = std::max(tmin, t1);
        tmax = std::min(tmax, t2);
        if (tmin > tmax) return false;
    }

    distance = tmin;
    return true;
}

// Static RNG for orb drop probability
static std::mt19937 rng(std::random_device{}());
static std::uniform_real_distribution<float> orbDropChance(0.f, 1.f);
constexpr float ORB_DROP_PROBABILITY = 0.25f;

enum class GameState {
    PRECREDIT,
    TITLE,
    GAME,
    GAMEOVER
};

int main()
{
    
    Window Game("SHADE");
    
    // Systems initialized via pointers for async loading
    std::unique_ptr<Player> player;
    std::unique_ptr<Background> background;
    std::unique_ptr<ScreenShake> screenShake;
    std::unique_ptr<ParticleSystem> particleSystem;
    std::unique_ptr<TitleScreen> titleScreen;

    // Initialize PreCredit Scene
    sf::Texture preCreditTexture;
    if (!preCreditTexture.loadFromFile("resources/PreCredit.png")) {
        std::cerr << "Failed to load PreCredit.png" << std::endl;
    }
    sf::Sprite preCreditSprite(preCreditTexture);
    sf::FloatRect pcBounds = preCreditSprite.getLocalBounds();
    preCreditSprite.setOrigin({pcBounds.size.x / 2.f, pcBounds.size.y / 2.f});
    preCreditSprite.setPosition({Game.center.x, Game.center.y});

    // Load Blast Sounds
    sf::SoundBuffer blastBuffer1;
    if (!blastBuffer1.loadFromFile("resources/Blast1.wav")) {
        std::cerr << "Failed to load Blast1.wav" << std::endl;
    }
    sf::Sound blastSound1(blastBuffer1);

    sf::SoundBuffer blastBuffer2;
    if (!blastBuffer2.loadFromFile("resources/Blast2.wav")) {
        std::cerr << "Failed to load Blast2.wav" << std::endl;
    }
    sf::Sound blastSound2(blastBuffer2);

    sf::SoundBuffer laserShootBuffer;
    if (!laserShootBuffer.loadFromFile("resources/LaserShoot.wav")) {
        std::cerr << "Failed to load LaserShoot.wav" << std::endl;
    }
    sf::Sound laserShootSound(laserShootBuffer);
    laserShootSound.setVolume(50.f);

    sf::SoundBuffer coinPickupBuffer;
    if (!coinPickupBuffer.loadFromFile("resources/1_Coins.ogg")) {
        std::cerr << "Failed to load 1_Coins.ogg" << std::endl;
    }
    sf::Sound coinPickupSound(coinPickupBuffer);
    
    sf::SoundBuffer powerupBuffer;
    if (!powerupBuffer.loadFromFile("resources/bell.wav")) {
         std::cerr << "Failed to load bell.wav" << std::endl;
    }
    sf::Sound powerupSound(powerupBuffer);

    sf::Font preCreditFont;
    if (!preCreditFont.openFromFile("resources/Font/Jumps Winter.ttf")) {
        std::cerr << "Failed to load Jumps Winter.ttf" << std::endl;
    }
    sf::Text preCreditText(preCreditFont, "OR ANY OTHER GAME ENGINE", 30);
    sf::FloatRect textBounds = preCreditText.getLocalBounds();
    preCreditText.setOrigin({textBounds.size.x / 2.f, textBounds.size.y / 2.f});
    preCreditText.setPosition({Game.center.x, Game.center.y + pcBounds.size.y / 2.f + 40.f});
    preCreditText.setFillColor(sf::Color::White);

    // Game Over UI
    sf::Text gameOverText(preCreditFont, "GAME OVER", 60);
    sf::FloatRect goBounds = gameOverText.getLocalBounds();
    gameOverText.setOrigin({goBounds.size.x / 2.f, goBounds.size.y / 2.f});
    gameOverText.setPosition({Game.center.x, Game.center.y});
    gameOverText.setFillColor(sf::Color::Red);

    float preCreditTimer = 0.f;
    GameState currentState = GameState::PRECREDIT;
    std::unique_ptr<HUD> hud;
    int loadStage = 0;

    std::vector<Laser> lasers;
    lasers.reserve(32);

    std::unique_ptr<HitSplash> hitSplashEffect;
    // Initialize with dummy values, will be updated
    hitSplashEffect = std::make_unique<HitSplash>(sf::Vector2f(0,0), 0.f);

    std::vector<ShockwaveOrb> orbs;
    orbs.reserve(16);
    std::vector<ShockwaveRipple> shockwaveRipples;
    shockwaveRipples.reserve(4);
    
    std::vector<FloatingText> floatingTexts;
    
    std::vector<Coin> coins;
    coins.reserve(128);
    int totalCoins = 0;
    
    std::vector<Enemy> enemies;
    constexpr std::size_t baseEnemyCount = 4;
    constexpr std::size_t maxEnemyCount = 32;
    enemies.reserve(maxEnemyCount);
    sf::Clock difficultyClock;
    sf::Clock frameClock;
    float difficulty = 1.f;
    float difficultyTimeOffset = 0.f;

    while (Game.window.isOpen())
    {
        bool isLaserHitting = false;
        bool didShoot = false;
        static bool wasShooting = false;
        float dt = frameClock.restart().asSeconds();
        dt = std::clamp(dt, 0.f, 0.05f);

        sf::Vector2i mousePixel = sf::Mouse::getPosition(Game.window);
        sf::Vector2f mouseWorld = Game.window.mapPixelToCoords(mousePixel);

        // SFML 3.0-style polling returns std::optional<sf::Event>
        std::optional<sf::Event> event;
        while (event = Game.window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                Game.window.close();
            }

            else if (currentState == GameState::PRECREDIT) {
                // Consume events but do nothing interactive
            }
            else if (currentState == GameState::TITLE) {
                if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                    int selection = titleScreen->handleInput(keyEvent->code);
                    if (selection == 0) { // Play
                        currentState = GameState::GAME;
                        difficultyClock.restart(); // Reset difficulty timer
                    } else if (selection == 3) { // Exit
                        Game.window.close();
                    }
                } else if (const auto* mouseMove = event->getIf<sf::Event::MouseMoved>()) {
                    sf::Vector2f mouseUI = Game.window.mapPixelToCoords(mouseMove->position, Game.uiView);
                    titleScreen->handleMouseMove(mouseUI);
                } else if (const auto* mousePress = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mousePress->button == sf::Mouse::Button::Left) {
                        sf::Vector2f mouseUI = Game.window.mapPixelToCoords(mousePress->position, Game.uiView);
                        int selection = titleScreen->handleClick(mouseUI);
                        if (selection == 0) { // Play
                            currentState = GameState::GAME;
                            difficultyClock.restart();
                        } else if (selection == 3) { // Exit
                            Game.window.close();
                        }
                    }
                }
            }
        }

        if (currentState == GameState::PRECREDIT) {
            preCreditTimer += dt;
            
            // Incremental Loading
            switch (loadStage) {
                case 0:
                    player = std::make_unique<Player>(Game.center.x, Game.center.y);
                    loadStage++;
                    break;
                case 1:
                    background = std::make_unique<Background>("resources");
                    loadStage++;
                    break;
                case 2:
                    screenShake = std::make_unique<ScreenShake>();
                    loadStage++;
                    break;
                case 3:
                    particleSystem = std::make_unique<ParticleSystem>();
                    loadStage++;
                    break;
                case 4:
                    // ribbonTrail removed
                    loadStage++;
                    break;
                case 5:
                    titleScreen = std::make_unique<TitleScreen>();
                    if (!titleScreen->init(Game.worldView.getSize().x, Game.worldView.getSize().y)) {
                        std::cerr << "Failed to initialize Title Screen" << std::endl;
                    }
                    loadStage++;
                    break;
                case 6:
                    for (std::size_t i = 0; i < baseEnemyCount; ++i)
                    {
                        enemies.emplace_back(Game);
                        enemies.back().applyDifficulty(difficulty);
                    }
                    loadStage++;
                    break;
                case 7: 
                    hud = std::make_unique<HUD>(Game.UiFont, sf::Vector2f(Game.width, Game.height));
                    loadStage++;
                    break;
            }

            float alpha = 255.f;
            if (preCreditTimer > 3.f) {
                float fadeProgress = (preCreditTimer - 3.f) / 1.f; // 0 to 1 over last second
                alpha = 255.f * (1.f - fadeProgress);
                if (alpha < 0.f) alpha = 0.f;
            }

            sf::Color c = sf::Color::White;
            c.a = static_cast<std::uint8_t>(alpha);
            preCreditSprite.setColor(c);
            preCreditText.setFillColor(c);

            if (preCreditTimer >= 4.f && loadStage >= 8) { // Updated check
                currentState = GameState::TITLE;
            }

            Game.window.clear(sf::Color::Black);
            Game.window.setView(Game.uiView);
            Game.window.draw(preCreditSprite);
            Game.window.draw(preCreditText);
            Game.window.display();
            continue;
        }

        static sf::SoundBuffer bgmBuffer;
        static std::unique_ptr<sf::Sound> bgm;
        static bool bgmInitialized = false;
        
        if (!bgmInitialized && currentState != GameState::PRECREDIT) {
             bgmBuffer = MusicGenerator::generateSpaceTrack();
             bgm = std::make_unique<sf::Sound>(bgmBuffer);
             bgm->setLooping(true);
             bgm->setVolume(30.f); 
             bgm->play();
             bgmInitialized = true;
        }

        if (currentState == GameState::TITLE) {
            titleScreen->update(dt);
            
            Game.window.clear(sf::Color::Black);
            // Draw background behind title for nice effect
            Game.window.setView(Game.worldView);
            background->update({0.f, 0.f}, Game.worldView.getSize()); // Static background for title
            background->draw(Game.window);
            
            // Draw Title UI
            Game.window.setView(Game.uiView);
            titleScreen->draw(Game.window);
            Game.window.display();
            continue;
        }

        if (currentState == GameState::GAMEOVER) {
            // Update background (slowly) for effect
            background->update({0.f, 0.f}, Game.worldView.getSize()); // Static or slowly moving could be nice, let's keep it static relative to last view
            
            Game.window.clear(sf::Color::Black);
            
            // Draw world in background (maybe darkened?)
            Game.window.setView(Game.worldView);
            background->draw(Game.window);
            for (const auto &enemy : enemies) Game.window.draw(enemy.body);
            // Don't draw player if they exploded, or maybe draw debris?
            
            // UI Overlay
            Game.window.setView(Game.uiView);
            
            // Darken background
            sf::RectangleShape overlay(sf::Vector2f(Game.uiView.getSize().x, Game.uiView.getSize().y));
            overlay.setFillColor(sf::Color(0, 0, 0, 150));
            overlay.setPosition({0.f, 0.f});
            // Center the overlay on the UI view
            overlay.setPosition(Game.uiView.getCenter() - Game.uiView.getSize()/2.f);
            
            Game.window.draw(overlay);
            Game.window.draw(gameOverText);
            Game.window.display();

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::R)) {
                // Restart Game
                player = std::make_unique<Player>(Game.center.x, Game.center.y);
                enemies.clear();
                lasers.clear();
                orbs.clear();
                coins.clear();
                shockwaveRipples.clear();
                particleSystem->particles.clear();
                floatingTexts.clear();
                
                totalCoins = 0;
                difficultyClock.restart();
                difficulty = 1.f;
                difficultyTimeOffset = 0.f;
                
                // Initial enemies
                for (std::size_t i = 0; i < baseEnemyCount; ++i)
                {
                    enemies.emplace_back(Game);
                    enemies.back().applyDifficulty(difficulty);
                }

                currentState = GameState::GAME;
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Escape)) {
                currentState = GameState::TITLE;
            }
            continue;
        }

        // --- GAME LOOP ---

        float elapsedSeconds = difficultyClock.getElapsedTime().asSeconds();
        difficulty = 1.f + (elapsedSeconds - difficultyTimeOffset) / 60.f;
        if (difficulty < 1.f) difficulty = 1.f; // Ensure difficulty doesn't drop below 1
        difficulty = std::min(difficulty, 5.f);

        std::size_t targetEnemyCount = std::min<std::size_t>(maxEnemyCount, static_cast<std::size_t>(baseEnemyCount * difficulty));
        while (enemies.size() < targetEnemyCount)
        {
            enemies.emplace_back(Game);
            enemies.back().applyDifficulty(difficulty);
        }

        Game.worldView.setCenter(player->body.getPosition());
        Game.window.setView(Game.worldView);
        mouseWorld = Game.window.mapPixelToCoords(mousePixel);

        // Update Background
        background->update(player->body.getPosition(), Game.worldView.getSize());
        
        // Update Effects
        screenShake->update(dt);
        particleSystem->update(dt);

        bool wantsNitro = sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Space);
        bool nitroActive = player->updateNitro(wantsNitro, dt);
        player->maxSpeed = nitroActive ? player->nitroMaxSpeed : player->baseMaxSpeed;
        float accelBoost = nitroActive ? player->nitroAccelMultiplier : 1.f;
        float thrust = player->baseAcceleration * accelBoost * dt;
        float frictionBase = nitroActive ? player->baseFriction * player->nitroFrictionFactor : player->baseFriction;
        float friction = frictionBase * (dt * 60.f);
        friction = std::clamp(friction, 0.01f, 0.3f);

        bool isMoving = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::W))
        {
            isMoving = true;
            if (player->velY > -player->maxSpeed)
                player->accY = -(0.05f * player->velY) - thrust;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::S))
        {
            isMoving = true;
            if (player->velY < player->maxSpeed)
                player->accY = -(0.05f * player->velY) + thrust;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::A))
        {
            isMoving = true;
            if (player->velX > -player->maxSpeed)
                player->accX = -(0.05f * player->velX) - thrust;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::D))
        {
            isMoving = true;
            if (player->velX < player->maxSpeed)
                player->accX = -(0.05f * player->velX) + thrust;
        }
        
        player->updateTexture(isMoving);
        player->updateLaserEnergy(dt);

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        {
            if (player->canShoot()) {
                sf::Vector2f playerPos = player->body.getPosition();
                sf::Vector2f dir = mouseWorld - playerPos;
                float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
                if (len > 0.0001f) dir /= len;
                
                float angleDeg = player->body.getRotation().asDegrees() - 90.f;
                
                // Raycast
                float minDistance = 2000.f; // Max laser length
                Enemy* hitEnemy = nullptr;
                
                // Check enemies
                for(auto& enemy : enemies) {
                    float dist = 0.f;
                    if(rayBoxIntersect(playerPos, dir, enemy.body.getGlobalBounds(), dist)) {
                        if(dist < minDistance) {
                            minDistance = dist;
                            hitEnemy = &enemy;
                        }
                    }
                }
                
                // Check screen bounds (simple approximation)
                // We just let it go to max distance if no enemy hit, or clamp to screen edge if we wanted to be precise.
                // For now, let's just use minDistance.
                
                lasers.emplace_back(playerPos, angleDeg, minDistance);
                
                if(hitEnemy) {
                    hitEnemy->takeDamage(2); // Damage based on visible time (reduced for continuous fire)
                    hitEnemy->applyKnockback(dir, 0.5f); // Reduced knockback per frame
                    
                    // Update Hit Splash
                    hitSplashEffect->setPosition(playerPos + dir * (minDistance - 10.f));
                    hitSplashEffect->setRotation(angleDeg);
                    hitSplashEffect->update(dt);
                    isLaserHitting = true;
                }

                
                player->consumeLaserEnergy();
                screenShake->addTrauma(0.01f); // Reduced trauma per frame
                didShoot = true;
            }
        }

        if (didShoot && !wasShooting) {
             laserShootSound.play();
        }
        wasShooting = didShoot;


        // Handle shockwave activation (right mouse button)
        static bool rightMousePressed = false;
        bool rightMouseCurrentlyPressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
        if (rightMouseCurrentlyPressed && !rightMousePressed)
        {
            // Right mouse button just pressed
            if (player->activateShockwave()) {
                shockwaveRipples.emplace_back(player->body.getPosition());
                
                // Multiply the difficulty by 3/4
                // Current difficulty: D = 1 + (T - Offset) / 60
                // Target difficulty: D' = D * 0.75
                // D' = 1 + (T - NewOffset) / 60
                // D*0.75 - 1 = (T - NewOffset) / 60
                // 60 * (D*0.75 - 1) = T - NewOffset
                // NewOffset = T - 60 * (D*0.75 - 1)
                
                float currentT = difficultyClock.getElapsedTime().asSeconds();
                float targetD = difficulty * 0.75f;
                if (targetD < 1.f) targetD = 1.f;
                
                float newOffset = currentT - (targetD - 1.f) * 60.f;
                difficultyTimeOffset = newOffset;
            }
        }
        rightMousePressed = rightMouseCurrentlyPressed;

        // Update shockwave timer
        player->updateShockwave(dt);
        
        // Update Ripples
        shockwaveRipples.erase(std::remove_if(shockwaveRipples.begin(), shockwaveRipples.end(),
            [&](ShockwaveRipple& r) { return !r.update(dt); }),
            shockwaveRipples.end());

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Escape))
        {
            // Return to title screen instead of closing
            currentState = GameState::TITLE;
            continue;
        }

        // Repair Mechanic
        static bool rKeyPressed = false;
        bool rKeyCurrentlyPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::R);
        if (rKeyCurrentlyPressed && !rKeyPressed) {
            if (totalCoins >= 500 && player->HP < player->maxHP) {
                totalCoins -= 500;
                player->HP = player->maxHP;
                powerupSound.play();
                floatingTexts.emplace_back(Game.UiFont, "Repaired! -500", player->body.getPosition(), sf::Color::Green);
            }
        }
        rKeyPressed = rKeyCurrentlyPressed;

        player->deaccelerate(player->velY, player->accY, false, friction);
        player->deaccelerate(player->velX, player->accX, true, friction);
        player->RotateTowards(mouseWorld);

        Game.worldView.setCenter(player->body.getPosition());
        Game.window.setView(Game.worldView);

        player->diagonalhandle();
        // Update HUD
        if(hud) hud->update(*player, totalCoins, dt);

        // Update lasers
        sf::Vector2f currentPlayerPos = player->body.getPosition();
        float currentPlayerRotation = player->body.getRotation().asDegrees();
        
        lasers.erase(std::remove_if(lasers.begin(), lasers.end(),
                                     [&](Laser &l)
                                     {
                                         l.body.setPosition(currentPlayerPos);
                                         l.body.setRotation(sf::degrees(currentPlayerRotation - 90.f));
                                         return !l.update(dt);
                                     }),
                      lasers.end());

        // Update Hit Splashes - Removed as we use single instance now
        // hitSplashes.erase(...)

        // Remove bullets that went off-screen (with margin)
        sf::FloatRect viewBounds = Game.getViewBounds(64.f);
        sf::FloatRect tightViewBounds = Game.getViewBounds();
        float leftBound = viewBounds.position.x;
        float rightBound = viewBounds.position.x + viewBounds.size.x;
        float topBound = viewBounds.position.y;
        float bottomBound = viewBounds.position.y + viewBounds.size.y;

        // Update orbs, check for collection, and remove off-screen orbs in one optimized pass
        sf::Vector2f playerPos = player->body.getPosition();
        orbs.erase(std::remove_if(orbs.begin(), orbs.end(),
                                  [&](ShockwaveOrb &orb)
                                  {
                                      orb.update(playerPos);
                                      if (orb.isCollected(playerPos))
                                      {
                                          player->addShockwaveCharge();
                                          powerupSound.play();
                                          return true; // Remove collected orb
                                      }
                                      sf::Vector2f p = orb.body.getPosition();
                                      return p.x < leftBound || p.y < topBound || p.x > rightBound || p.y > bottomBound;
                                  }),
                   orbs.end());

        // Update Coins
        coins.erase(std::remove_if(coins.begin(), coins.end(),
                                   [&](Coin &c)
                                   {
                                       c.update(playerPos, dt);
                                       if (c.isCollected(playerPos))
                                       {
                                           totalCoins++;
                                           if (coinPickupSound.getStatus() != sf::Sound::Status::Playing) {
                                               coinPickupSound.play();
                                           }
                                           return true;
                                       }
                                       // Despawn if too far
                                       if (c.isTooFar(playerPos)) {
                                            return true;
                                       }
                                       return false;
                                   }),
                                   coins.end());
         
        float playerSpeedSquared = player->velX * player->velX + player->velY * player->velY;
        for (auto &enemy : enemies)
        {
            enemy.update(dt);
            if (enemy.HP <= 0)
            {
                // Enemy Death Effects
                if (std::uniform_int_distribution<int>(0, 1)(rng) == 0) {
                    blastSound1.play();
                } else {
                    blastSound2.play();
                }
                screenShake->addTrauma(0.4f);
                particleSystem->emit(enemy.body.getPosition(), 20, sf::Color::Red, 150.f);
                
                // Spawn shockwave orb when enemy is killed (25% probability)
                if (orbDropChance(rng) < ORB_DROP_PROBABILITY)
                {
                    orbs.emplace_back(enemy.body.getPosition());
                }
                
                // Spawn Coins
                int coinCount = std::uniform_int_distribution<int>(1, 3)(rng);
                for(int i=0; i<coinCount; ++i) {
                     sf::Vector2f offset = {static_cast<float>(std::uniform_int_distribution<int>(-20, 20)(rng)), 
                                            static_cast<float>(std::uniform_int_distribution<int>(-20, 20)(rng))};
                     coins.emplace_back(enemy.body.getPosition() + offset);
                }
                
                enemy.applyDifficulty(difficulty);
                enemy.respawn(Game.worldView, 50.f);
                continue;
            }

            bool enemyOffscreen = !containsPoint(tightViewBounds, enemy.body.getPosition());
            if (enemyOffscreen && playerSpeedSquared > 16.f)
            {
                sf::Vector2f toEnemy = enemy.body.getPosition() - player->body.getPosition();
                float dot = player->velX * toEnemy.x + player->velY * toEnemy.y;
                if (dot < 0.f)
                {
                    enemy.applyDifficulty(difficulty);
                    enemy.respawn(Game.worldView, 50.f);
                    continue;
                }
            }

            // Check if enemy is close enough to explode
            sf::Vector2f playerPos = player->body.getPosition();

            // Update visual state (warning colors)
            enemy.updateVisualState(playerPos);

            if (enemy.shouldExplode(playerPos))
            {
                // Enemy explodes, dealing damage to player
                player->takeDamage(enemy.explosionDamage);
                
                // Player Hit Effects
                screenShake->addTrauma(0.8f);
                particleSystem->emit(playerPos, 30, sf::Color::Red, 200.f);
                
                // Respawn the enemy after explosion
                enemy.applyDifficulty(difficulty);
                enemy.respawn(Game.worldView, 50.f);
                
                if (player->HP <= 0) {
                     currentState = GameState::GAMEOVER;
                     blastSound2.play();
                     break;
                }
                continue;
            }

            // Apply shockwave repulsion if active
            if (player->shockwaveActive)
            {
                enemy.applyShockwave(playerPos, player->shockwaveForce, dt);
            }
            else
            {
                enemy.moveTowards(playerPos);
            }
        }
        
        Game.window.clear(sf::Color::Black);
        
        // Apply Screen Shake
        screenShake->apply(Game.worldView, elapsedSeconds);
        Game.window.setView(Game.worldView);
        
        // Draw Background
        background->draw(Game.window);
        
        // Draw Ripples (behind entities but above background)
        for (auto& ripple : shockwaveRipples) {
            ripple.draw(Game.window);
        }
        
        // Draw Particles (behind entities)
        particleSystem->draw(Game.window);

        for (const auto &l : lasers)
            Game.window.draw(l.body);
        for (const auto &orb : orbs)
            Game.window.draw(orb.body);
            
        for (auto &coin : coins)
            coin.draw(Game.window);

        Game.window.draw(player->body);
        for (auto &enemy : enemies)
            enemy.trail.draw(Game.window, sf::Color(255, 50, 50));
        for (const auto &enemy : enemies)
            Game.window.draw(enemy.body);
        if (isLaserHitting)
            Game.window.draw(hitSplashEffect->sprite);
        Game.window.setView(Game.uiView);
        // Draw HUD
        if(hud) hud->draw(Game.window);

        Game.window.display();
    }
}

