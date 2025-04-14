////////////////////////////////////////////////////////////////////////////////
//         Distributed under the Boost Software License, Version 1.0.         //
//            (See accompanying file LICENSE or copy at                       //
//                 https://www.boost.org/LICENSE_1_0.txt)                     //
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <optional>
#include <vector>
#include <string>

#include "entity.h"
#include "key_event.h"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
struct _TTF_Font;
typedef struct _TTF_Font TTF_Font;

using SDLWindowDelete = void (*)(SDL_Window *);
using SDLRendererDelete = void (*)(SDL_Renderer *);

namespace cpp
{

/**
 * Window is responsible for creating and destroying a platform window as well as rendering to it and getting events.
 */
class Window
{
  public:
    /**
     * Construct a new Window.
     */
    Window();
    ~Window() = default;
    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    Window(Window &&) = default;
    Window &operator=(Window &&) = default;

    /**
     * Get an event if one is available.
     *
     * @returns
     *   A KeyEvent if an event was available, otherwise an empty optional.
     */
    std::optional<KeyEvent> get_event() const;

    /**
     * Render a collection of entities.
     *
     * @param entities
     *   Entities to render.
     */
    void render(const std::vector<Entity> &entities) const;

    /**
     * Render text on screen
     * 
     * @param text
     *   Text to render
     * 
     * @param x
     *   X position
     * 
     * @param y
     *   Y position
     */
    void render_text(const std::string& text, int x, int y) const;

    /**
     * Render text on screen with specific size
     * 
     * @param text
     *   Text to render
     * 
     * @param x
     *   X position
     * 
     * @param y
     *   Y position
     * 
     * @param size
     *   Font size
     */
    void render_text(const std::string& text, int x, int y, int size) const;

    /**
     * Render text on screen with specific size and color
     * 
     * @param text
     *   Text to render
     * 
     * @param x
     *   X position
     * 
     * @param y
     *   Y position
     * 
     * @param size
     *   Font size
     * 
     * @param color
     *   Text color
     */
    void render_text(const std::string& text, int x, int y, int size, uint32_t color) const;

    /**
     * Clear the render surface
     */
    void clear() const;

    /**
     * Present the current frame
     */
    void present() const;

    /**
     * Render a semi-transparent overlay
     * 
     * @param color
     *   Color of overlay
     * 
     * @param alpha
     *   Transparency level (0-255)
     */
    void render_overlay(uint32_t color, uint8_t alpha) const;

  private:
    /** SDL window object. */
    std::unique_ptr<SDL_Window, SDLWindowDelete> window_;

    /** SDL renderer object. */
    std::unique_ptr<SDL_Renderer, SDLRendererDelete> renderer_;

    std::unique_ptr<TTF_Font, void(*)(TTF_Font*)> font_;
};

}
