////////////////////////////////////////////////////////////////////////////////
//         Distributed under the Boost Software License, Version 1.0.         //
//            (See accompanying file LICENSE or copy at                       //
//                 https://www.boost.org/LICENSE_1_0.txt)                     //
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <ranges>
#include <cmath>
#include <string>

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

/**
 * Helper function to create a row of 10 bricks.
 *
 * @param entities
 *   Collection to add new entities to.
 *
 * @param y
 *   Y coordinate of row.
 *
 * @param colour
 *   Colour of bricks.
 */
void create_brick_row(std::vector<cpp::Entity> &entities, float y, const cpp::Colour &colour)
{
    auto x = 20.0f;

    for (auto i = 0u; i < 10u; ++i)
    {
        entities.push_back({{{x, y}, 58.0f, 20.0f}, colour});
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
 * @param entities
 *   Collection of all entities, brick entities will be removed if a collision is detected.
 */
void check_collisions(
    const cpp::Entity &ball,
    cpp::Vector2 &ball_velocity,
    const cpp::Entity &paddle,
    std::vector<cpp::Entity> &entities)
{
    // check and handle ball and paddle collision
    if (paddle.intersects(ball))
    {
        const auto ball_pos = ball.rectangle().position;
        const auto paddle_pos = paddle.rectangle().position;

        if (ball_pos.x < paddle_pos.x + 100.0f)
        {
            ball_velocity.x = -1.4f;
            ball_velocity.y = -1.4f;
        }
        else if (ball_pos.x < paddle_pos.x + 200.0f)
        {
            ball_velocity.x = 0.0f;
            ball_velocity.y = -2.0f;
        }
        else
        {
            ball_velocity.x = 1.4f;
            ball_velocity.y = -1.4f;
        }
    }
    else
    {
        // only check brick intersections if we didn't intersect the paddle, unlikely these will both happen in the same
        // frame due to the layout of the game

        // iterate over all entities, skipping the first two as these are the paddle and ball
        auto bricks_view = entities | std::views::drop(2u);
        auto hit_brick =
            std::ranges::find_if(bricks_view, [&ball](const auto &brick) { return ball.intersects(brick); });

        if (hit_brick != std::ranges::end(bricks_view))
        {
            // Add score when brick is destroyed
            current_score += BRICK_POINTS;
            
            ball_velocity.y *= -1.0f;
            entities.erase(hit_brick);
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

    if ((ball_pos.y > 800.0f) || (ball_pos.y < 0.0f))
    {
        velocity.y *= -1.0f;
    }

    if ((ball_pos.x > 800.0f) || (ball_pos.x < 0.0f))
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
    paddle.translate(velocity);
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
    ball_velocity = cpp::Vector2{0.0f, 2.0f};
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
 * @param entities
 *   Collection of all entities
 *
 * @returns
 *   True if no bricks remain, false otherwise
 */
bool check_win_condition(const std::vector<cpp::Entity> &entities)
{
    // Skip first two entities (paddle and ball)
    return entities.size() <= 2;
}

}

int main()
{
    std::cout << "hello world\n";

    const cpp::Window window{};
    auto running = true;

    // Initialize game state to title screen
    GameState game_state = GameState::TITLE_SCREEN;

    std::vector<cpp::Entity> entities{
        {{{300.0f, 780.0f}, 300.0f, 20.0f}, 0xFFFFFF}, {{{420.0f, 400.0f}, 10.0f, 10.0f}, 0xFFFFFF}};

    create_brick_row(entities, 50.0f, 0xff0000);
    create_brick_row(entities, 80.0f, 0xff0000);
    create_brick_row(entities, 110.0f, 0xffa500);
    create_brick_row(entities, 140.0f, 0xffa500);
    create_brick_row(entities, 170.0f, 0x00ff00);
    create_brick_row(entities, 200.0f, 0x00ff00);

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
                        reset_ball(entities[1], ball_velocity);
                        reset_paddle(entities[0], paddle_velocity);
                        reset_score();
                        left_press = false;
                        right_press = false;
                    }
                    else if (game_state == GameState::GAME_OVER || game_state == GameState::WIN)
                    {
                        game_state = GameState::PLAYING;
                        // Reset everything
                        entities.clear();  // Clear all entities
                        entities.push_back({{{300.0f, 780.0f}, 300.0f, 20.0f}, 0xFFFFFF});  // Add paddle
                        entities.push_back({{{420.0f, 400.0f}, 10.0f, 10.0f}, 0xFFFFFF});  // Add ball
                        
                        // Recreate all bricks
                        create_brick_row(entities, 50.0f, 0xff0000);
                        create_brick_row(entities, 80.0f, 0xff0000);
                        create_brick_row(entities, 110.0f, 0xffa500);
                        create_brick_row(entities, 140.0f, 0xffa500);
                        create_brick_row(entities, 170.0f, 0x00ff00);
                        create_brick_row(entities, 200.0f, 0x00ff00);
                        
                        reset_ball(entities[1], ball_velocity);
                        reset_paddle(entities[0], paddle_velocity);
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
                // Render animated title screen with game entities in background
                render_title_screen(window, title_animation_time, entities);
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

                {
                    auto &paddle = entities[0];
                    auto &ball = entities[1];

                    update_paddle(paddle, paddle_velocity);
                    update_ball(ball, ball_velocity);
                    check_collisions(ball, ball_velocity, paddle, entities);

                    if (is_game_over(ball))
                    {
                        game_state = GameState::GAME_OVER;
                    }
                    else if (check_win_condition(entities))
                    {
                        game_state = GameState::WIN;
                    }
                }

                // Render game entities
                window.render(entities);

                // Render score at top of screen
                std::string score_text = "Score: " + std::to_string(current_score);
                window.render_text(score_text, 10, 10, 24, 0xFFFFFF);  // White color
                break;
            }
            case GameState::GAME_OVER:
            {
                // Render game entities in background
                window.render(entities);
                
                // Render game over screen with final score
                window.render_text("Game Over!", 200, 250, 72);
                std::string final_score = "Final Score: " + std::to_string(current_score);
                window.render_text(final_score, 200, 350, 48, 0xFFFF00);  // Yellow color
                window.render_text("Press Space to Restart", 200, 450, 36);
                break;
            }
            case GameState::WIN:
            {
                // Render game entities in background
                window.render(entities);
                
                // Semi-transparent overlay
                window.render_overlay(0x000000, 180);

                // Render win screen with final score
                window.render_text("YOU WIN!", 200, 250, 72, 0x00FF00);  // Green color
                std::string final_score = "Final Score: " + std::to_string(current_score);
                window.render_text(final_score, 200, 350, 48, 0xFFFF00);  // Yellow color
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
