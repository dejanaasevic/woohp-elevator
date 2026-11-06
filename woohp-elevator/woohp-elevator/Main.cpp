#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "src/Util.h"

// handles errors and terminates the application
int terminateWithError(const char* message)
{
    std::cerr << message << std::endl;
    glfwTerminate();
    return -1;
}

int main()
{
    // initialize GLFW
    if (!glfwInit())
    {
        return terminateWithError("Failed to initialize GLFW.");
    }

    // configure the OpenGL context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // create the application window
    GLFWwindow* window = glfwCreateWindow(1000, 1000, "WOOHP Elevator", nullptr, nullptr);

    if (window == nullptr)
    {
        return terminateWithError("Failed to create the application window.");
    }

    glfwMakeContextCurrent(window);

    // initialize GLEW
    if (glewInit() != GLEW_OK)
    {
        return terminateWithError("Failed to initialize GLEW.");
    }

    // load the shader program
    unsigned int shaderProgram = createShader(
        "shaders/basic.vert",
        "shaders/basic.frag"
    );

    // define vertex data: position (x, y) and color (r, g, b)
    float vertices[] = {
         0.0f,  0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.0f, 0.0f, 1.0f
    };

    // create vertex array and buffer objects
    unsigned int VAO;
    unsigned int VBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // configure vertex attributes
    // TODO: define position and color attributes

    // set the background color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // main rendering loop
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        // TODO: render the scene

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // release OpenGL resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // close the application.
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}