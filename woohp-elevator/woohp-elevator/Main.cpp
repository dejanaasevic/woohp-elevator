#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "src/Util.h"
#include "src/TextUtil.h"

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

    // create the application window and context
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "WOOHP Elevator", nullptr, nullptr);

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

    // load the shader programs
    unsigned int shaderProgram = createShader(
        "shaders/basic.vert",
        "shaders/basic.frag"
    );

    // load font
    if (!initText("fonts/segoeui.ttf", 48, WINDOW_WIDTH, WINDOW_HEIGHT))
    {
        return terminateWithError("Failed to load font.");
    }

    // enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // control panel definition
    float controlPanelVertices[] = {
        // control panel area background
        -1.0f, -1.0f, 0.910f, 0.914f, 0.922f,
        -1.0f,  1.0f, 0.910f, 0.914f, 0.922f,
         0.0f,  1.0f, 0.910f, 0.914f, 0.922f,
         0.0f, -1.0f, 0.910f, 0.914f, 0.922f,
    };

    // VAO and VBO definition
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

    // elevator scene definition
    float elevatorSceneVertices[] = {
        // elevator scene background
         0.0f, -1.0f, 0.125f, 0.137f, 0.153f,
         0.0f,  1.0f, 0.125f, 0.137f, 0.153f,
         1.0f,  1.0f, 0.125f, 0.137f, 0.153f,
         1.0f, -1.0f, 0.125f, 0.137f, 0.153f,
    };

    // VAO and VBO definition
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

    // control panel elements definition
    float panelElementsVertices[] = {

        // CURRENT FLOOR background
        -0.430f,  0.760f, 0.204f, 0.224f, 0.251f,
        -0.430f,  0.950f, 0.204f, 0.224f, 0.251f,
        -0.080f,  0.950f, 0.204f, 0.224f, 0.251f,
        -0.080f,  0.760f, 0.204f, 0.224f, 0.251f,

        // SU
        -0.920f,  0.200f, 0.780f, 0.796f, 0.816f,
        -0.920f,  0.370f, 0.780f, 0.796f, 0.816f,
        -0.730f,  0.370f, 0.780f, 0.796f, 0.816f,
        -0.730f,  0.200f, 0.780f, 0.796f, 0.816f,

        // PR
        -0.710f,  0.200f, 0.780f, 0.796f, 0.816f,
        -0.710f,  0.370f, 0.780f, 0.796f, 0.816f,
        -0.520f,  0.370f, 0.780f, 0.796f, 0.816f,
        -0.520f,  0.200f, 0.780f, 0.796f, 0.816f,

        // Floor 1
        -0.500f,  0.200f, 0.780f, 0.796f, 0.816f,
        -0.500f,  0.370f, 0.780f, 0.796f, 0.816f,
        -0.310f,  0.370f, 0.780f, 0.796f, 0.816f,
        -0.310f,  0.200f, 0.780f, 0.796f, 0.816f,

        // Floor 2
        -0.290f,  0.200f, 0.780f, 0.796f, 0.816f,
        -0.290f,  0.370f, 0.780f, 0.796f, 0.816f,
        -0.100f,  0.370f, 0.780f, 0.796f, 0.816f,
        -0.100f,  0.200f, 0.780f, 0.796f, 0.816f,

        // Floor 3
        -0.920f, -0.020f, 0.780f, 0.796f, 0.816f,
        -0.920f,  0.150f, 0.780f, 0.796f, 0.816f,
        -0.730f,  0.150f, 0.780f, 0.796f, 0.816f,
        -0.730f, -0.020f, 0.780f, 0.796f, 0.816f,

        // Floor 4
        -0.710f, -0.020f, 0.780f, 0.796f, 0.816f,
        -0.710f,  0.150f, 0.780f, 0.796f, 0.816f,
        -0.520f,  0.150f, 0.780f, 0.796f, 0.816f,
        -0.520f, -0.020f, 0.780f, 0.796f, 0.816f,

        // Floor 5
        -0.500f, -0.020f, 0.780f, 0.796f, 0.816f,
        -0.500f,  0.150f, 0.780f, 0.796f, 0.816f,
        -0.310f,  0.150f, 0.780f, 0.796f, 0.816f,
        -0.310f, -0.020f, 0.780f, 0.796f, 0.816f,

        // Floor 6
        -0.290f, -0.020f, 0.780f, 0.796f, 0.816f,
        -0.290f,  0.150f, 0.780f, 0.796f, 0.816f,
        -0.100f,  0.150f, 0.780f, 0.796f, 0.816f,
        -0.100f, -0.020f, 0.780f, 0.796f, 0.816f,

        // OPEN DOOR
        -0.920f, -0.310f, 0.204f, 0.224f, 0.251f,
        -0.920f, -0.480f, 0.204f, 0.224f, 0.251f,
        -0.510f, -0.480f, 0.204f, 0.224f, 0.251f,
        -0.510f, -0.310f, 0.204f, 0.224f, 0.251f,

        // CLOSE DOOR
        -0.490f, -0.310f, 0.204f, 0.224f, 0.251f,
        -0.490f, -0.480f, 0.204f, 0.224f, 0.251f,
        -0.080f, -0.480f, 0.204f, 0.224f, 0.251f,
        -0.080f, -0.310f, 0.204f, 0.224f, 0.251f,

        // STOP
        -0.920f, -0.530f, 0.145f, 0.157f, 0.176f,
        -0.920f, -0.700f, 0.145f, 0.157f, 0.176f,
        -0.510f, -0.700f, 0.145f, 0.157f, 0.176f,
        -0.510f, -0.530f, 0.145f, 0.157f, 0.176f,

        // VENTILATION
        -0.490f, -0.530f, 0.204f, 0.224f, 0.251f,
        -0.490f, -0.700f, 0.204f, 0.224f, 0.251f,
        -0.080f, -0.700f, 0.204f, 0.224f, 0.251f,
        -0.080f, -0.530f, 0.204f, 0.224f, 0.251f,

    };

    // VAO and VBO definition
    unsigned int panelElementsVAO;
    unsigned int panelElementsVBO;

    glGenVertexArrays(1, &panelElementsVAO);
    glGenBuffers(1, &panelElementsVBO);

    glBindVertexArray(panelElementsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, panelElementsVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(panelElementsVertices), panelElementsVertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // elevator elements definition
    float elevatorElementsVertices[] = {

        // HALLWAY BACKGROUND
        0.12f, -1.0f, 0.204f, 0.224f, 0.251f,
        0.12f,  1.0f, 0.204f, 0.224f, 0.251f,
        0.74f,  1.0f, 0.204f, 0.224f, 0.251f,
        0.74f, -1.0f, 0.204f, 0.224f, 0.251f,

        // ELEVATOR SHAFT BACKGROUND
        0.78f, -1.0f, 0.125f, 0.137f, 0.153f,
        0.78f,  1.0f, 0.125f, 0.137f, 0.153f,
        0.98f,  1.0f, 0.125f, 0.137f, 0.153f,
        0.98f, -1.0f, 0.125f, 0.137f, 0.153f,

        // FLOOR SEPARATORS - 7 horizontal lines

        // Floor SU / PR
        0.12f, -0.75f, 0.125f, 0.137f, 0.153f,
        0.98f, -0.75f, 0.125f, 0.137f, 0.153f,

        // Floor PR / 1
        0.12f, -0.50f, 0.125f, 0.137f, 0.153f,
        0.98f, -0.50f, 0.125f, 0.137f, 0.153f,

        // Floor 1 / 2
        0.12f, -0.25f, 0.125f, 0.137f, 0.153f,
        0.98f, -0.25f, 0.125f, 0.137f, 0.153f,

        // Floor 2 / 3
        0.12f,  0.00f, 0.125f, 0.137f, 0.153f,
        0.98f,  0.00f, 0.125f, 0.137f, 0.153f,

        // Floor 3 / 4
        0.12f,  0.25f, 0.125f, 0.137f, 0.153f,
        0.98f,  0.25f, 0.125f, 0.137f, 0.153f,

        // Floor 4 / 5
        0.12f,  0.50f, 0.125f, 0.137f, 0.153f,
        0.98f,  0.50f, 0.125f, 0.137f, 0.153f,

        // Floor 5 / 6
        0.12f,  0.75f, 0.125f, 0.137f, 0.153f,
        0.98f,  0.75f, 0.125f, 0.137f, 0.153f,
    };

    // VAO and VBO definition
    unsigned int elevatorElementsVAO;
    unsigned int elevatorElementsVBO;

    glGenVertexArrays(1, &elevatorElementsVAO);
    glGenBuffers(1, &elevatorElementsVBO);

    glBindVertexArray(elevatorElementsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, elevatorElementsVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(elevatorElementsVertices), elevatorElementsVertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // floor doors definition (8 floors, each: frame + door)
    float floorDoorsVertices[] = {

        // SU - frame
        0.80f, -0.98f, 0.204f, 0.224f, 0.251f,
        0.80f, -0.78f, 0.204f, 0.224f, 0.251f,
        0.96f, -0.78f, 0.204f, 0.224f, 0.251f,
        0.96f, -0.98f, 0.204f, 0.224f, 0.251f,
        // SU - door
        0.81f, -0.97f, 0.145f, 0.157f, 0.176f,
        0.81f, -0.79f, 0.145f, 0.157f, 0.176f,
        0.95f, -0.79f, 0.145f, 0.157f, 0.176f,
        0.95f, -0.97f, 0.145f, 0.157f, 0.176f,

        // PR - frame
        0.80f, -0.73f, 0.204f, 0.224f, 0.251f,
        0.80f, -0.53f, 0.204f, 0.224f, 0.251f,
        0.96f, -0.53f, 0.204f, 0.224f, 0.251f,
        0.96f, -0.73f, 0.204f, 0.224f, 0.251f,
        // PR - door
        0.81f, -0.72f, 0.145f, 0.157f, 0.176f,
        0.81f, -0.54f, 0.145f, 0.157f, 0.176f,
        0.95f, -0.54f, 0.145f, 0.157f, 0.176f,
        0.95f, -0.72f, 0.145f, 0.157f, 0.176f,

        // Floor 1 - frame
        0.80f, -0.48f, 0.204f, 0.224f, 0.251f,
        0.80f, -0.28f, 0.204f, 0.224f, 0.251f,
        0.96f, -0.28f, 0.204f, 0.224f, 0.251f,
        0.96f, -0.48f, 0.204f, 0.224f, 0.251f,
        // Floor 1 - door
        0.81f, -0.47f, 0.145f, 0.157f, 0.176f,
        0.81f, -0.29f, 0.145f, 0.157f, 0.176f,
        0.95f, -0.29f, 0.145f, 0.157f, 0.176f,
        0.95f, -0.47f, 0.145f, 0.157f, 0.176f,

        // Floor 2 - frame
        0.80f, -0.23f, 0.204f, 0.224f, 0.251f,
        0.80f, -0.03f, 0.204f, 0.224f, 0.251f,
        0.96f, -0.03f, 0.204f, 0.224f, 0.251f,
        0.96f, -0.23f, 0.204f, 0.224f, 0.251f,
        // Floor 2 - door
        0.81f, -0.22f, 0.145f, 0.157f, 0.176f,
        0.81f, -0.04f, 0.145f, 0.157f, 0.176f,
        0.95f, -0.04f, 0.145f, 0.157f, 0.176f,
        0.95f, -0.22f, 0.145f, 0.157f, 0.176f,

        // Floor 3 - frame
        0.80f,  0.02f, 0.204f, 0.224f, 0.251f,
        0.80f,  0.22f, 0.204f, 0.224f, 0.251f,
        0.96f,  0.22f, 0.204f, 0.224f, 0.251f,
        0.96f,  0.02f, 0.204f, 0.224f, 0.251f,
        // Floor 3 - door
        0.81f,  0.03f, 0.145f, 0.157f, 0.176f,
        0.81f,  0.21f, 0.145f, 0.157f, 0.176f,
        0.95f,  0.21f, 0.145f, 0.157f, 0.176f,
        0.95f,  0.03f, 0.145f, 0.157f, 0.176f,

        // Floor 4 - frame
        0.80f,  0.27f, 0.204f, 0.224f, 0.251f,
        0.80f,  0.47f, 0.204f, 0.224f, 0.251f,
        0.96f,  0.47f, 0.204f, 0.224f, 0.251f,
        0.96f,  0.27f, 0.204f, 0.224f, 0.251f,
        // Floor 4 - door
        0.81f,  0.28f, 0.145f, 0.157f, 0.176f,
        0.81f,  0.46f, 0.145f, 0.157f, 0.176f,
        0.95f,  0.46f, 0.145f, 0.157f, 0.176f,
        0.95f,  0.28f, 0.145f, 0.157f, 0.176f,

        // Floor 5 - frame
        0.80f,  0.52f, 0.204f, 0.224f, 0.251f,
        0.80f,  0.72f, 0.204f, 0.224f, 0.251f,
        0.96f,  0.72f, 0.204f, 0.224f, 0.251f,
        0.96f,  0.52f, 0.204f, 0.224f, 0.251f,
        // Floor 5 - door
        0.81f,  0.53f, 0.145f, 0.157f, 0.176f,
        0.81f,  0.71f, 0.145f, 0.157f, 0.176f,
        0.95f,  0.71f, 0.145f, 0.157f, 0.176f,
        0.95f,  0.53f, 0.145f, 0.157f, 0.176f,

        // Floor 6 - frame
        0.80f,  0.77f, 0.204f, 0.224f, 0.251f,
        0.80f,  0.97f, 0.204f, 0.224f, 0.251f,
        0.96f,  0.97f, 0.204f, 0.224f, 0.251f,
        0.96f,  0.77f, 0.204f, 0.224f, 0.251f,
        // Floor 6 - door
        0.81f,  0.78f, 0.145f, 0.157f, 0.176f,
        0.81f,  0.96f, 0.145f, 0.157f, 0.176f,
        0.95f,  0.96f, 0.145f, 0.157f, 0.176f,
        0.95f,  0.78f, 0.145f, 0.157f, 0.176f,

        // DOOR CENTER LINES (8 vertical lines, one per floor)
        0.88f, -0.97f, 0.204f, 0.224f, 0.251f,   // SU
        0.88f, -0.79f, 0.204f, 0.224f, 0.251f,

        0.88f, -0.72f, 0.204f, 0.224f, 0.251f,   // PR
        0.88f, -0.54f, 0.204f, 0.224f, 0.251f,

        0.88f, -0.47f, 0.204f, 0.224f, 0.251f,   // Floor 1
        0.88f, -0.29f, 0.204f, 0.224f, 0.251f,

        0.88f, -0.22f, 0.204f, 0.224f, 0.251f,   // Floor 2
        0.88f, -0.04f, 0.204f, 0.224f, 0.251f,

        0.88f,  0.03f, 0.204f, 0.224f, 0.251f,   // Floor 3
        0.88f,  0.21f, 0.204f, 0.224f, 0.251f,

        0.88f,  0.28f, 0.204f, 0.224f, 0.251f,   // Floor 4
        0.88f,  0.46f, 0.204f, 0.224f, 0.251f,

        0.88f,  0.53f, 0.204f, 0.224f, 0.251f,   // Floor 5
        0.88f,  0.71f, 0.204f, 0.224f, 0.251f,

        0.88f,  0.78f, 0.204f, 0.224f, 0.251f,   // Floor 6
        0.88f,  0.96f, 0.204f, 0.224f, 0.251f,
    };

    // VAO and VBO definition
    unsigned int floorDoorsVAO;
    unsigned int floorDoorsVBO;

    glGenVertexArrays(1, &floorDoorsVAO);
    glGenBuffers(1, &floorDoorsVBO);

    glBindVertexArray(floorDoorsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorDoorsVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(floorDoorsVertices), floorDoorsVertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // elevator definition (starts on floor 1)
    float elevatorVertices[] = {

        // ELEVATOR outer frame
        0.80f, -0.48f, 0.204f, 0.224f, 0.251f,
        0.80f, -0.28f, 0.204f, 0.224f, 0.251f,
        0.96f, -0.28f, 0.204f, 0.224f, 0.251f,
        0.96f, -0.48f, 0.204f, 0.224f, 0.251f,

        // ELEVATOR cabin interior
        0.81f, -0.47f, 0.145f, 0.157f, 0.176f,
        0.81f, -0.29f, 0.145f, 0.157f, 0.176f,
        0.95f, -0.29f, 0.145f, 0.157f, 0.176f,
        0.95f, -0.47f, 0.145f, 0.157f, 0.176f,

        // ELEVATOR door
        0.81f, -0.47f, 0.651f, 0.725f, 0.718f,
        0.81f, -0.29f, 0.651f, 0.725f, 0.718f,
        0.95f, -0.29f, 0.651f, 0.725f, 0.718f,
        0.95f, -0.47f, 0.651f, 0.725f, 0.718f,

        // ELEVATOR door center line
        0.88f, -0.47f, 0.204f, 0.224f, 0.251f,
        0.88f, -0.29f, 0.204f, 0.224f, 0.251f,
    };


    // VAO and VBO definition
    unsigned int elevatorVAO;
    unsigned int elevatorVBO;

    glGenVertexArrays(1, &elevatorVAO);
    glGenBuffers(1, &elevatorVBO);

    glBindVertexArray(elevatorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, elevatorVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(elevatorVertices), elevatorVertices, GL_STATIC_DRAW);

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

        // draw hallway background
        glBindVertexArray(elevatorElementsVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // draw elevator shaft background
        glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

        // draw floor separators
        glDrawArrays(GL_LINES, 8, 14);

        // draw floor doors
        glBindVertexArray(floorDoorsVAO);
        for (int i = 0; i < 16; i++)
        {
            glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
        }
        // door center lines
        glDrawArrays(GL_LINES, 64, 16);

        // draw elevator
        glBindVertexArray(elevatorVAO);
        for (int i = 0; i < 3; i++)
        {
            glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
        }
        // elevator door center line
        glDrawArrays(GL_LINES, 12, 2);

        // draw control panel elements
        glBindVertexArray(panelElementsVAO);
        for (int i = 0; i < 13; i++)
        {
            glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
        }

        // draw text 
        // header for control panel
        renderText("WOOHP.", -0.92f, 0.86f, 0.8f, 0.125f, 0.137f, 0.153f);
        renderText("ELEVATOR MANAGEMENT SYSTEM", -0.92f, 0.79f, 0.28f, 0.4f, 0.4f, 0.45f);

        // current floor
        renderTextCentered("CURRENT FLOOR", -0.255f, 0.915f, 0.28f, 0.7f, 0.72f, 0.75f);
        renderTextCentered("01", -0.255f, 0.835f, 0.8f, 1.0f, 1.0f, 1.0f);

        // subtitles for control panel
        renderText("Lift control", -0.92f, 0.66f, 0.7f, 0.125f, 0.137f, 0.153f);
        renderText("Select your destination floor", -0.92f, 0.57f, 0.32f, 0.4f, 0.4f, 0.45f);
        renderText("FLOOR SELECTION", -0.92f, 0.44f, 0.3f, 0.4f, 0.4f, 0.45f);

        // buttons
        renderTextCentered("SU", -0.825f, 0.285f, 0.6f, 0.125f, 0.137f, 0.153f);
        renderTextCentered("PR", -0.615f, 0.285f, 0.6f, 0.125f, 0.137f, 0.153f);
        renderTextCentered("1", -0.405f, 0.285f, 0.6f, 0.125f, 0.137f, 0.153f);
        renderTextCentered("2", -0.195f, 0.285f, 0.6f, 0.125f, 0.137f, 0.153f);
        renderTextCentered("3", -0.825f, 0.065f, 0.6f, 0.125f, 0.137f, 0.153f);
        renderTextCentered("4", -0.615f, 0.065f, 0.6f, 0.125f, 0.137f, 0.153f);
        renderTextCentered("5", -0.405f, 0.065f, 0.6f, 0.125f, 0.137f, 0.153f);
        renderTextCentered("6", -0.195f, 0.065f, 0.6f, 0.125f, 0.137f, 0.153f);

        // controls
        renderText("EMERGENCY CONTROL", -0.92f, -0.23f, 0.3f, 0.4f, 0.4f, 0.45f);
        renderTextCentered("OPEN DOOR", -0.715f, -0.395f, 0.45f, 1.0f, 1.0f, 1.0f);
        renderTextCentered("CLOSE DOOR", -0.285f, -0.395f, 0.45f, 1.0f, 1.0f, 1.0f);
        renderTextCentered("STOP", -0.715f, -0.615f, 0.5f, 0.871f, 0.529f, 0.535f);
        renderTextCentered("FAN", -0.285f, -0.575f, 0.4f, 0.651f, 0.725f, 0.718f);
        renderTextCentered("VENTILATION", -0.285f, -0.645f, 0.45f, 0.651f, 0.725f, 0.718f);

        // floor labels
        renderTextCentered("6", 0.075f, 0.875f, 0.45f, 0.910f, 0.914f, 0.922f);
        renderTextCentered("5", 0.075f, 0.625f, 0.45f, 0.910f, 0.914f, 0.922f);
        renderTextCentered("4", 0.075f, 0.375f, 0.45f, 0.910f, 0.914f, 0.922f);
        renderTextCentered("3", 0.075f, 0.125f, 0.45f, 0.910f, 0.914f, 0.922f);
        renderTextCentered("2", 0.075f, -0.125f, 0.45f, 0.910f, 0.914f, 0.922f);
        renderTextCentered("1", 0.075f, -0.375f, 0.45f, 0.910f, 0.914f, 0.922f);
        renderTextCentered("PR", 0.075f, -0.625f, 0.40f, 0.910f, 0.914f, 0.922f);
        renderTextCentered("SU", 0.075f, -0.875f, 0.40f, 0.910f, 0.914f, 0.922f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // release OpenGL resources
    glDeleteVertexArrays(1, &controlPanelVAO);
    glDeleteBuffers(1, &controlPanelVBO);
    glDeleteVertexArrays(1, &elevatorSceneVAO);
    glDeleteBuffers(1, &elevatorSceneVBO);
    glDeleteVertexArrays(1, &panelElementsVAO);
    glDeleteBuffers(1, &panelElementsVBO);
    glDeleteVertexArrays(1, &elevatorElementsVAO);
    glDeleteBuffers(1, &elevatorElementsVBO);
    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &floorDoorsVAO);
    glDeleteBuffers(1, &floorDoorsVBO);
    glDeleteVertexArrays(1, &elevatorVAO);
    glDeleteBuffers(1, &elevatorVBO);
    cleanupText();

    // close the application.
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}