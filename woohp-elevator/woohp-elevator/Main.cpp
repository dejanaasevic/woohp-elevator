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

// define application constants
constexpr unsigned int WINDOW_WIDTH = 1200;
constexpr unsigned int WINDOW_HEIGHT = 800;
constexpr float CONTROL_PANEL_RATIO = 0.5f;
constexpr float ELEVATOR_SCENE_RATIO = 0.5f;


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
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "WOOHP Elevator", nullptr, nullptr);

    if (window == nullptr)
    {
        return terminateWithError("Failed to create the application window.");
    }

    // calculate the control panel and elevator scene dimensions
    unsigned int controlPanelWidth = static_cast<unsigned int> (CONTROL_PANEL_RATIO * WINDOW_WIDTH);
    unsigned int elevatorSceneWidth = static_cast<unsigned int> (ELEVATOR_SCENE_RATIO * WINDOW_WIDTH);

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

    // control panel definition
    float controlPanelVertices[] = {
        // control panel area background
        -1.0f, -1.0f, 0.910f, 0.914f, 0.922f,
        -1.0f,  1.0f, 0.910f, 0.914f, 0.922f,
         0.0f,  1.0f, 0.910f, 0.914f, 0.922f,
         0.0f, -1.0f, 0.910f, 0.914f, 0.922f,
    };

    unsigned int controlPanelVAO;
    unsigned int controlPanelVBO;

    glGenVertexArrays(1, &controlPanelVAO);
    glGenBuffers(1, &controlPanelVBO);

    glBindVertexArray(controlPanelVAO);
    glBindBuffer(GL_ARRAY_BUFFER, controlPanelVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(controlPanelVertices), controlPanelVertices, GL_STATIC_DRAW);

   // position attribute 
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);


    float elevatorSceneVertices[] = {
        // elevator scene background
         0.0f, -1.0f, 0.125f, 0.137f, 0.153f,
         0.0f,  1.0f, 0.125f, 0.137f, 0.153f,
         1.0f,  1.0f, 0.125f, 0.137f, 0.153f,
         1.0f, -1.0f, 0.125f, 0.137f, 0.153f,
    };


    unsigned int elevatorSceneVAO;
    unsigned int elevatorSceneVBO;

    glGenVertexArrays(1, &elevatorSceneVAO);
    glGenBuffers(1, &elevatorSceneVBO);

    glBindVertexArray(elevatorSceneVAO);
    glBindBuffer(GL_ARRAY_BUFFER, elevatorSceneVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(elevatorSceneVertices), elevatorSceneVertices, GL_STATIC_DRAW);

    // position attribute 
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // set the background color
    glClearColor(0.08f, 0.09f, 0.10f, 1.0f);

    // main rendering loop
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // draw the control panel area background
        glBindVertexArray(controlPanelVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // draw the elevator scene background
        glBindVertexArray(elevatorSceneVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // release OpenGL resources
    glDeleteVertexArrays(1, &controlPanelVAO);
    glDeleteBuffers(1, &controlPanelVBO);
    glDeleteVertexArrays(1, &elevatorSceneVAO);
    glDeleteBuffers(1, &elevatorSceneVBO);
    glDeleteProgram(shaderProgram);

    // close the application.
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}