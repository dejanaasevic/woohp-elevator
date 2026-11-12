#include "TextUtil.h"
#include "Util.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <iostream>
#include <map>

namespace
{
    // data needed to draw a single character
    struct Character
    {
        unsigned int texture;   
        int sizeX;
        int sizeY;      
        int bearingX;
        int bearingY;
        unsigned int advance;   
    };

    // map stores the data for all loaded characters
    std::map<unsigned char, Character> characters;

    // VAO, VBO and shader definitions
    unsigned int textVAO = 0;
    unsigned int textVBO = 0;
    unsigned int textShader = 0;

    // variables for converting pixel into normalized coordinates range
    float pixelToNdcX = 0.0f;
    float pixelToNdcY = 0.0f;
}

// initializes the text rendering system
bool initText(const char* fontPath, unsigned int pixelSize, unsigned int windowWidth,
              unsigned int windowHeight, const char* vertShaderPath, const char* fragShaderPath)
{
    // calculate normalized coordinates
    pixelToNdcX = 2.0f / windowWidth;
    pixelToNdcY = 2.0f / windowHeight;

    // load the shader
    textShader = createShader(vertShaderPath, fragShaderPath);

    // initialize FreeType library instance
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cerr << "Failed to initialize FreeType." << std::endl;
        return false;
    }

    // load the font
    FT_Face face;
    if (FT_New_Face(ft, fontPath, 0, &face))
    {
        std::cerr << "Failed to load font: " << fontPath << std::endl;
        FT_Done_FreeType(ft);
        return false;
    }

    // set the font size
    FT_Set_Pixel_Sizes(face, 0, pixelSize);

    // set pixel alignment
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // load all ASCII characters
    for (unsigned int c = 0; c < 128; c++)
    {
        // load character from the font
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            continue;
        }

        // create an OpenGL texture for each character
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, 
                     GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

        // configure the texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // store character information in map
        Character character = {
            texture,
            (int)face->glyph->bitmap.width,
            (int)face->glyph->bitmap.rows,
            face->glyph->bitmap_left,
            face->glyph->bitmap_top,
            (unsigned int)face->glyph->advance.x
        };
        characters[(unsigned char)c] = character;
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // release FreeType resources
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // create VAO and VBO
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}

// calculate text width
float getTextWidth(const std::string& text, float scale)
{
    float width = 0.0f;
    for (char c : text)
    {
        // search character in map
        auto it = characters.find((unsigned char)c);
        
        // skip if character doesn't exists
        if (it == characters.end()) 
        {
            continue;
        }
        
        // add advance to width
        width += (it->second.advance >> 6) * scale * pixelToNdcX;
    }
    return width;
}

// render text
void renderText(const std::string& text, float x, float y, float scale, float r, float g, float b)
{
    // activate the shader and bind VAO and VBO
    glUseProgram(textShader);
    glUniform3f(glGetUniformLocation(textShader, "textColor"), r, g, b);
    glUniform1i(glGetUniformLocation(textShader, "text"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);

    // iterate through the string
    for (char c : text)
    {
        // search character in map
        auto it = characters.find((unsigned char)c);

        // skip if character doesn't exists
        if (it == characters.end()) 
        {
            continue;
        }

        // get its texture and metrics
        const Character& ch = it->second;

        // calculate character position and size
        float xpos = x + ch.bearingX * scale * pixelToNdcX;
        float ypos = y - (ch.sizeY - ch.bearingY) * scale * pixelToNdcY;
        float w = ch.sizeX * scale * pixelToNdcX;
        float h = ch.sizeY * scale * pixelToNdcY;

        // create the vertices - x, y, u, v
        float vertices[6][4] = {
             { xpos,     ypos + h, 0.0f, 0.0f },
             { xpos,     ypos,     0.0f, 1.0f },
             { xpos + w, ypos,     1.0f, 1.0f },

             { xpos,     ypos + h, 0.0f, 0.0f },
             { xpos + w, ypos,     1.0f, 1.0f },
             { xpos + w, ypos + h, 1.0f, 0.0f }
        };

        // bind texture and update vertex data
        glBindTexture(GL_TEXTURE_2D, ch.texture);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // move the cursor
        x += (ch.advance >> 6) * scale * pixelToNdcX;
    }

    // unbind OpenGL objects
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// center the text
void renderTextCentered(const std::string& text, float cx, float cy, float scale, float r, float g, float b)
{
    // calculate text width
    float width = getTextWidth(text, scale);

    // calculate cap height using letter 'H' as reference
    float capHeight = 0.0f;
    auto it = characters.find('H');
    if (it != characters.end())
    {
        capHeight = it->second.bearingY * scale * pixelToNdcY;
    }
    // calculate the starting position and render
    renderText(text, cx - width / 2.0f, cy - capHeight / 2.0f, scale, r, g, b);
}

// release resources
void cleanupText()
{
    // delete texture of each character
    for (auto& pair : characters)
    {
        glDeleteTextures(1, &pair.second.texture);
    }
    
    // remove map
    characters.clear();

    // release the VAO, VBO and shader program
    glDeleteVertexArrays(1, &textVAO);
    glDeleteBuffers(1, &textVBO);
    glDeleteProgram(textShader);
}
