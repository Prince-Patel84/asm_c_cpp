////////////////////////////////////////////////////////////////////////////////
//         Distributed under the Boost Software License, Version 1.0.         //
//            (See accompanying file LICENSE or copy at                       //
//                 https://www.boost.org/LICENSE_1_0.txt)                     //
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <ranges>
#include <cmath>
#include <string>
#include <fstream>
#include <algorithm>

#include "colour.h"
#include "entity.h"
#include "key_event.h"
#include "rectangle.h"
#include "vector2.h"
#include "window.h"

namespace
{

enum class GameState
{
    TITLE_SCREEN,
    PLAYING,
    GAME_OVER,
    WIN
};

float title_animation_time = 0.0f;
const float ANIMATION_SPEED = 0.05f;

// Add these near other global variables
int current_score = 0;
const int BRICK_POINTS = 100;  // Points per brick destroyed
int high_score = 0;
const std::string HIGH_SCORE_FILE = "highscore.txt";

// Add near other global variables
struct Brick {
    cpp::Entity entity;
    int hits_required;
};

// Different brick colors based on hits required
const uint32_t BRICK_COLORS[] = {
    0x00FF00,  // Green (1 hit)
    0xFFA500,  // Orange (2 hits)
    0xFF0000   // Red (3 hits)
};

// Points awarded based on brick strength
const int BRICK_POINTS_BY_STRENGTH[] = {
    100,  // 1-hit brick
    200,  // 2-hit brick
    300   // 3-hit brick
};

// Add near other global variables
const float INITIAL_BALL_SPEED = 2.0f;
const float MAX_BALL_SPEED = 4.0f;
const float SPEED_INCREMENT = 0.1f;
float current_ball_speed = INITIAL_BALL_SPEED;

// Add this new variable to scale paddle speed with ball speed
const float PADDLE_SPEED_MULTIPLIER = 0.5f; // Adjust this multiplier as needed

/**
 * Helper function to create a row of 10 bricks.
 *
 * @param bricks
 *   Collection to add new entities to.
 *
 * @param y
 *   Y coordinate of row.
 *
 * @param hits_required
 *   Number of hits required to destroy the brick
 */
void create_brick_row(std::vector<Brick> &bricks, float y, int hits_required)
{
    auto x = 20.0f;
    
    for (auto i = 0u; i < 10u; ++i)
    {
        cpp::Entity entity{{{x, y}, 58.0f, 20.0f}, BRICK_COLORS[hits_required - 1]};
        bricks.push_back({entity, hits_required});
        x += 78.0f;
    }
}

/**
 * Helper function to check for and resolve collisions between the ball and other entities.
 *
 * @param ball
 *   Ball to check for collisions.
 *
 * @param ball_velocity
 *   Velocity of ball, might get mutated during collision response.
 *
 * @param paddle
 *   Paddle to check for collisions with.
 *
 * @param bricks
 *   Collection of all entities, brick entities will be removed if a collision is detected.
 */
void check_collisions(
    const cpp::Entity &ball,
    cpp::Vector2 &ball_velocity,
    const cpp::Entity &paddle,
    std::vector<Brick> &bricks)
{
    // Paddle collision
    if (paddle.intersects(ball))
    {
        const auto ball_pos = ball.rectangle().position;
        const auto paddle_pos = paddle.rectangle().position;

        if (ball_pos.x < paddle_pos.x + 100.0f)
        {
            ball_velocity.x = -current_ball_speed * 0.7f;
            ball_velocity.y = -current_ball_speed * 0.7f;
        }
        else if (ball_pos.x < paddle_pos.x + 200.0f)
        {
            // ball_velocity.x = 0.0f;
            ball_velocity.y = -current_ball_speed;
        }
        else
        {
            ball_velocity.x = current_ball_speed * 0.7f;
            ball_velocity.y = -current_ball_speed * 0.7f;
        }
    }
    else
    {
        // Check brick collisions
        for (auto it = bricks.begin(); it != bricks.end(); ++it)
        {
            if (it->entity.intersects(ball))
            {
                // Reduce hits required and update color
                it->hits_required--;
                
                if (it->hits_required <= 0)
                {
                    // Add score based on original brick strength
                    current_score += BRICK_POINTS_BY_STRENGTH[it->hits_required];
                    bricks.erase(it);
                    
                    // Increase ball speed
                    current_ball_speed = std::min(current_ball_speed + SPEED_INCREMENT, MAX_BALL_SPEED);
                    
                    // Adjust current velocity to match new speed while maintaining direction
                    float current_speed = std::sqrt(ball_velocity.x * ball_velocity.x + 
                                                  ball_velocity.y * ball_velocity.y);
                    ball_velocity.x = (ball_velocity.x / current_speed) * current_ball_speed;
                    ball_velocity.y = (ball_velocity.y / current_speed) * current_ball_speed;
                }
                else
                {
                    // Update brick color based on remaining hits
                    it->entity = cpp::Entity{
                        it->entity.rectangle(),
                        BRICK_COLORS[it->hits_required - 1]
                    };
                }
                
                ball_velocity.y *= -1.0f;
                break;
            }
        }
    }
}

/**
 * Helper function to update the ball. Will check if the ball leaves the window and adjust the velocity so it "bounces"
 * off walls.
 *
 * @param ball
 *   Ball entity to update.
 *
 * @param velocity
 *   Ball velocity, will be mutated if ball goes off screen.
 */
void update_ball(cpp::Entity &ball, cpp::Vector2 &velocity)
{
    ball.translate(velocity);

    const auto ball_pos = ball.rectangle().position;

    if (ball_pos.y < 0.0f)
    {
        velocity.y *= -1.0f;
    }

    if ((ball_pos.x + 10.0f > 800.0f) || (ball_pos.x < 0.0f))
    {
        velocity.x *= -1.0f;
    }
}

/**
 * Helper function to update the paddle.
 *
 * @param paddle
 *   Paddle entity to update.
 *
 * @param velocity
 *   Paddle velocity.
 */
void update_paddle(cpp::Entity &paddle, const cpp::Vector2 &velocity)
{
    // Calculate paddle speed based on current ball speed
    float paddle_speed = current_ball_speed * PADDLE_SPEED_MULTIPLIER;
    paddle.translate(cpp::Vector2{velocity.x * paddle_speed, 0.0f});
}

/**
 * Helper function to check if the game is over (ball falls below paddle)
 *
 * @param ball
 *   Ball entity to check position
 *
 * @returns
 *   True if game is over, false otherwise
 */
bool is_game_over(const cpp::Entity &ball)
{
    return ball.rectangle().position.y > 800.0f;
}

/**
 * Helper function to reset the ball to initial position
 *
 * @param ball
 *   Ball entity to reset
 *
 * @param ball_velocity
 *   Ball velocity to reset
 */
void reset_ball(cpp::Entity &ball, cpp::Vector2 &ball_velocity)
{
    ball = cpp::Entity{{{420.0f, 400.0f}, 10.0f, 10.0f}, 0xFFFFFF};
    ball_velocity = cpp::Vector2{0.0f, current_ball_speed};
}

/**
 * Helper function to reset the paddle to initial position
 *
 * @param paddle
 *   Paddle entity to reset
 *
 * @param paddle_velocity
 *   Paddle velocity to reset
 */
void reset_paddle(cpp::Entity &paddle, cpp::Vector2 &paddle_velocity)
{
    paddle = cpp::Entity{{{300.0f, 780.0f}, 300.0f, 20.0f}, 0xFFFFFF};
    paddle_velocity = cpp::Vector2{0.0f, 0.0f};
}

/**
 * Helper function to render the animated title screen
 *
 * @param window
 *   Window to render to
 *
 * @param animation_time
 *   Current animation time
 */
void render_title_screen(const cpp::Window& window, float& animation_time, const std::vector<cpp::Entity> &entities)
{
    // Update animation time
    animation_time += ANIMATION_SPEED;
    
    // Render game entities in background with dimmed effect
    window.render(entities);
    
    // Semi-transparent overlay to dim the background
    window.render_overlay(0x000000, 180);  // Black with alpha 180 (partially transparent)
    
    // Rainbow colors for BREAKOUT letters
    const uint32_t colors[] = {
        0xFF0000,  // Red
        0xFF7F00,  // Orange
        0xFFFF00,  // Yellow
        0x00FF00,  // Green
        0x0000FF,  // Blue
        0x4B0082,  // Indigo
        0x9400D3,  // Violet
        0xFF1493   // Pink
    };
    
    // Render each letter of BREAKOUT with different colors and offsets
    const char* letters = "BREAKOUT";
    int base_x = 150;
    int letter_spacing = 70;
    
    for (int i = 0; letters[i] != '\0'; i++)
    {
        char letter[2] = {letters[i], '\0'};
        float letter_offset = std::sin(animation_time + i * 0.5f) * 15.0f;
        window.render_text(letter, 
                         base_x + i * letter_spacing, 
                         200 + static_cast<int>(letter_offset), 
                         100, 
                         colors[i % 8]);
    }

    // Only show the essential text with pulsing effect
    float scale = 1.0f + std::sin(animation_time * 2.0f) * 0.1f;
    window.render_text("Press SPACE to Start", 
                      200, 
                      500 + static_cast<int>(std::sin(animation_time) * 20.0f), 
                      48, 
                      0x00FF00);  // Green
}

// Add this helper function to reset score
void reset_score()
{
    current_score = 0;
}

/**
 * Helper function to check if all bricks are destroyed
 *
 * @param bricks
 *   Collection of all entities
 *
 * @returns
 *   True if no bricks remain, false otherwise
 */
bool check_win_condition(const std::vector<Brick> &bricks)
{
    return bricks.empty();
}

// Helper function to load high score
void load_high_score() {
    std::ifstream file(HIGH_SCORE_FILE);
    if (file.is_open()) {
        file >> high_score;
        file.close();
    }
}

// Helper function to save high score
void save_high_score() {
    std::ofstream file(HIGH_SCORE_FILE);
    if (file.is_open()) {
        file << high_score;
        file.close();
    }
}

// Helper function to update high score
void update_high_score() {
    if (current_score > high_score) {
        high_score = current_score;
        save_high_score();
    }
}

// Add this helper function in the anonymous namespace
void render_entity(const cpp::Window& window, const cpp::Entity& entity) {
    std::vector<cpp::Entity> temp{entity};
    window.render(temp);
}

// Helper function to reset ball speed
void reset_ball_speed() {
    current_ball_speed = INITIAL_BALL_SPEED;
}

}

int main()
{
    std::cout << "hello world\n";

    // Load high score at startup
    load_high_score();

    const cpp::Window window{};
    auto running = true;

    // Initialize game state to title screen
    GameState game_state = GameState::TITLE_SCREEN;

    // Change entities vector to separate paddle, ball, and bricks
    cpp::Entity paddle{{{300.0f, 780.0f}, 300.0f, 20.0f}, 0xFFFFFF};
    cpp::Entity ball{{{420.0f, 400.0f}, 10.0f, 10.0f}, 0xFFFFFF};
    std::vector<Brick> bricks;

    // Create rows with different hit requirements
    create_brick_row(bricks, 50.0f, 3);  // Red bricks (3 hits)
    create_brick_row(bricks, 80.0f, 3);
    create_brick_row(bricks, 110.0f, 2); // Orange bricks (2 hits)
    create_brick_row(bricks, 140.0f, 2);
    create_brick_row(bricks, 170.0f, 1); // Green bricks (1 hit)
    create_brick_row(bricks, 200.0f, 1);

    cpp::Vector2 ball_velocity{0.0f, 1.0f};
    cpp::Vector2 paddle_velocity{0.0f, 0.0f};
    const float paddle_speed = 2.0f;
    auto left_press = false;
    auto right_press = false;

    while (running)
    {
        for (;;)
        {
            if (const auto event = window.get_event(); event)
            {
                using enum cpp::Key;
                using enum cpp::KeyState;

                if ((event->key_state == DOWN) && (event->key == ESCAPE))
                {
                    running = false;
                }
                else if ((event->key_state == DOWN) && (event->key == SPACE))
                {
                    if (game_state == GameState::TITLE_SCREEN)
                    {
                        game_state = GameState::PLAYING;
                        reset_ball_speed();
                        reset_ball(ball, ball_velocity);
                        reset_paddle(paddle, paddle_velocity);
                        reset_score();
                        left_press = false;
                        right_press = false;
                    }
                    else if (game_state == GameState::GAME_OVER || game_state == GameState::WIN)
                    {
                        game_state = GameState::PLAYING;
                        // Reset everything
                        bricks.clear();
                        
                        // Recreate all bricks
                        create_brick_row(bricks, 50.0f, 3);
                        create_brick_row(bricks, 80.0f, 3);
                        create_brick_row(bricks, 110.0f, 2);
                        create_brick_row(bricks, 140.0f, 2);
                        create_brick_row(bricks, 170.0f, 1);
                        create_brick_row(bricks, 200.0f, 1);
                        
                        reset_ball_speed();
                        reset_ball(ball, ball_velocity);
                        reset_paddle(paddle, paddle_velocity);
                        reset_score();
                        left_press = false;
                        right_press = false;
                    }
                }
                else if (game_state == GameState::PLAYING)
                {
                    if (event->key == LEFT)
                    {
                        left_press = (event->key_state == DOWN) ? true : false;
                    }
                    else if (event->key == RIGHT)
                    {
                        right_press = (event->key_state == DOWN) ? true : false;
                    }
                }
            }
            else
            {
                break;
            }
        }

        // Clear screen at start of frame
        window.clear();

        switch (game_state)
        {
            case GameState::TITLE_SCREEN:
            {
                // Create a vector with all entities for the background
                std::vector<cpp::Entity> all_entities;
                all_entities.push_back(paddle);
                all_entities.push_back(ball);
                for (const auto& brick : bricks) {
                    all_entities.push_back(brick.entity);
                }
                
                render_title_screen(window, title_animation_time, all_entities);
                // Add high score display on title screen
                std::string high_score_text = "High Score: " + std::to_string(high_score);
                window.render_text(high_score_text, 300, 600, 36, 0xFFD700); // Gold color
                break;
            }
            case GameState::PLAYING:
            {
                // Update game state
                if ((left_press && right_press) || (!left_press && !right_press))
                {
                    paddle_velocity.x = 0.0f;
                }
                else if (left_press)
                {
                    paddle_velocity.x = -paddle_speed;
                }
                else if (right_press)
                {
                    paddle_velocity.x = paddle_speed;
                }

                // Update entities
                update_paddle(paddle, paddle_velocity);
                update_ball(ball, ball_velocity);
                check_collisions(ball, ball_velocity, paddle, bricks);

                if (is_game_over(ball))
                {
                    game_state = GameState::GAME_OVER;
                }
                else if (check_win_condition(bricks))
                {
                    game_state = GameState::WIN;
                }

                // Render entities
                render_entity(window, paddle);
                render_entity(window, ball);
                for (const auto& brick : bricks)
                {
                    render_entity(window, brick.entity);
                }

                // Display scores
                std::string score_text = "Score: " + std::to_string(current_score);
                window.render_text(score_text, 10, 10, 24, 0xFFFFFF);
                std::string high_score_text = "High Score: " + std::to_string(high_score);
                window.render_text(high_score_text, 600, 10, 24, 0xFFD700);
                break;
            }
            case GameState::GAME_OVER:
            {
                render_entity(window, paddle);
                render_entity(window, ball);
                for (const auto& brick : bricks)
                {
                    render_entity(window, brick.entity);
                }
                window.render_text("Game Over!", 200, 250, 72);
                
                // Update high score before displaying
                update_high_score();
                
                std::string final_score = "Final Score: " + std::to_string(current_score);
                window.render_text(final_score, 200, 350, 48, 0xFFFF00);
                
                // Display high score
                std::string high_score_text = "High Score: " + std::to_string(high_score);
                window.render_text(high_score_text, 200, 400, 48, 0xFFD700);
                
                window.render_text("Press Space to Restart", 200, 450, 36);
                break;
            }
            case GameState::WIN:
            {
                render_entity(window, paddle);
                render_entity(window, ball);
                for (const auto& brick : bricks)
                {
                    render_entity(window, brick.entity);
                }
                window.render_overlay(0x000000, 180);
                window.render_text("YOU WIN!", 200, 250, 72, 0x00FF00);
                
                // Update high score before displaying
                update_high_score();
                
                std::string final_score = "Final Score: " + std::to_string(current_score);
                window.render_text(final_score, 200, 350, 48, 0xFFFF00);
                
                // Display high score
                std::string high_score_text = "High Score: " + std::to_string(high_score);
                window.render_text(high_score_text, 200, 400, 48, 0xFFD700);
                
                window.render_text("Press Space to Play Again", 180, 450, 36, 0xFFFFFF);
                break;
            }
        }

        // Present the frame
        window.present();
    }

    std::cout << "goodbye\n";
    return 0;
}
