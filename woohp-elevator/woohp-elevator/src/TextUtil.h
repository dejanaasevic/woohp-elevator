#pragma once
#include <string>

// initializes the text rendering system
bool initText(const char* fontPath, unsigned int pixelSize, unsigned int windowWidth,
			  unsigned int windowHeight, const char* vertShaderPath = "shaders/text.vert",
			  const char* fragShaderPath = "shaders/text.frag");

// render the text
void renderText(const std::string& text, float x, float y, float scale, float r, float g, float b);

// render the centered the text
void renderTextCentered(const std::string& text, float cx, float cy, float scale, float r, float g, float b);

// calculate text width
float getTextWidth(const std::string& text, float scale);

// release resources
void cleanupText();
