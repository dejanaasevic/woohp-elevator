#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
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

// define floor height
float floorHeight[] = { -0.875f, -0.625f, -0.375f, -0.125f, 0.125f, 0.375f, 0.625f, 0.875f };

// define selected floor
bool floorSelected[8] = { false };

// define elevator state
enum ElevatorState { IDLE, MOVING, DOOR_OPENING, DOOR_OPEN, DOOR_CLOSING };

// define elevator direction
enum Direction { NONE, UP, DOWN };

// define floor label
const char* floorLabels[] = { "SU","PR","01","02", "03", "04", "05", "06" };

// define elevator variables
int currentFloorIndex = 2;
int targetFloorIndex = 2;
float elevatorY = floorHeight[currentFloorIndex];
float elevatorSpeed = 0.4f;
float delta = 0.0;
ElevatorState state = IDLE;
Direction direction = NONE;
float doorTimer = 0.0f;   
float doorTimerLimit = 5.0f;
bool timerExtend = false;
float doorOpenAmount = 0.0f;
float doorSpeed = 2.0f; 

enum Button {
    BTN_NONE = -1, BTN_SU, BTN_PR, BTN_1, BTN_2, BTN_3, BTN_4, 
    BTN_5, BTN_6, BTN_OPEN, BTN_CLOSE, BTN_STOP, BTN_VENT
};

struct ButtonRectangle { float x1, y1, x2, y2; };
ButtonRectangle buttonRects[] = {
    {-0.920f,  0.200f, -0.730f,  0.370f},  // SU
    {-0.710f,  0.200f, -0.520f,  0.370f},  // PR
    {-0.500f,  0.200f, -0.310f,  0.370f},  // 1
    {-0.290f,  0.200f, -0.100f,  0.370f},  // 2
    {-0.920f, -0.020f, -0.730f,  0.150f},  // 3
    {-0.710f, -0.020f, -0.520f,  0.150f},  // 4
    {-0.500f, -0.020f, -0.310f,  0.150f},  // 5
    {-0.290f, -0.020f, -0.100f,  0.150f},  // 6
    {-0.920f, -0.480f, -0.510f, -0.310f},  // OPEN
    {-0.490f, -0.480f, -0.080f, -0.310f},  // CLOSE
    {-0.920f, -0.700f, -0.510f, -0.530f},  // STOP
    {-0.490f, -0.700f, -0.080f, -0.530f},  // VENT
};

Button getPressedButton(float x, float y) {
    for (int i = 0; i < 12; i++) {
        const ButtonRectangle& button = buttonRects[i];
        if (x >= button.x1 && x <= button.x2 && y >= button.y1 && y <= button.y2)
        {
            return (Button)i;
        }
    }
    return BTN_NONE;
}

std::string getCurrentFloorLabel() {
    for (int i = 7; i >= 0; i--) {
        if (elevatorY >= floorHeight[i]) {
            return floorLabels[i];
        }
    }
    return "";
}

int findSelectedFloor() {
    if (direction == NONE) {
        int nearestFloorIndex= -1;
        int nearestDistance = 100;
        for (int i = 0; i < 8; i++) {
            if (i != currentFloorIndex && floorSelected[i]) {
                if (abs(currentFloorIndex - i) < nearestDistance) {
                    nearestDistance = abs(currentFloorIndex - i);
                    nearestFloorIndex = i;
                }
            }
        }

        if (nearestDistance == 100) {
            direction = NONE;
            return -1;
        }

        if (currentFloorIndex > nearestFloorIndex) {
            direction = DOWN;
            return nearestFloorIndex;
        }

        else {
            direction = UP;
            return nearestFloorIndex;
        }
    }
    if (direction == UP) {
        for (int i = currentFloorIndex + 1; i < 8; i++) {
            if (floorSelected[i]) {
                return i;
            }
        }

        for (int i = currentFloorIndex - 1; i >= 0; i--) {
            if (floorSelected[i]) {
                direction = DOWN;
                return i;
            }
        }
    }
    else {
        for (int i = currentFloorIndex - 1; i >= 0; i--) {
            if (floorSelected[i]) {
                return i;
            }
        }
        for (int i = currentFloorIndex + 1; i < 8; i++) {
            if (floorSelected[i]) {
                direction = UP;
                return i;
            }
        }
    }
    direction = NONE;
    return -1;
}

void updateElevator() {
    switch (state) {
        case IDLE:{
            floorSelected[currentFloorIndex] = false;
            int selectedFloor = findSelectedFloor();
            if (selectedFloor != -1) {
                targetFloorIndex = selectedFloor;
                state = MOVING;
            }
            break;
        }
        case MOVING: {
            // pick up a floor that was selected on the way to the target
            if (direction == UP) {
                for (int i = currentFloorIndex + 1; i < targetFloorIndex; i++) {
                    if (floorSelected[i] && floorHeight[i] > elevatorY) {
                        targetFloorIndex = i;
                        break;
                    }
                }
            }
            else if (direction == DOWN) {
                for (int i = currentFloorIndex - 1; i > targetFloorIndex; i--) {
                    if (floorSelected[i] && floorHeight[i] < elevatorY) {
                        targetFloorIndex = i;
                        break;
                    }
                }
            }
            
            // move the elevator towards the target floor at constant speed
            float targetY = floorHeight[targetFloorIndex];

            // calculate how far the elevator can move during this frame
            float step = elevatorSpeed * delta;

            // check whether the elevator can reach the destination during this frame
            if (fabs(targetY - elevatorY) <= step)
            {
                // place the elevator at the destination floor
                elevatorY = targetY;

                // update the current floor index because the elevator has arrived
                currentFloorIndex = targetFloorIndex;

                // update elevator state
                floorSelected[currentFloorIndex] = false;
                doorTimer = 0.0f;
                doorTimerLimit = 5.0f;
                timerExtend = false;
                state = DOOR_OPENING;
            }

            //  if destination is above the current elevator position, go up
            else if (targetY > elevatorY)
            {
                elevatorY += step;
            }
            // if destination below the current elevator position, go down
            else
            {
                elevatorY -= step;
            }
            break;
        }
        case DOOR_OPENING:
        {
            doorOpenAmount += doorSpeed * delta;
            if (doorOpenAmount >= 1.0f) {
                doorOpenAmount = 1.0f;
                state = DOOR_OPEN;
            }
            break;
        }
        case DOOR_OPEN:
        {
            doorTimer += delta;
            if (doorTimer >= doorTimerLimit) {
                state = DOOR_CLOSING;
            }
            break;
        }
        case DOOR_CLOSING:
        {
            doorOpenAmount -= doorSpeed * delta;
            if (doorOpenAmount <= 0.0f) {
                doorOpenAmount = 0.0f;
                timerExtend = false;
                state = IDLE;
            }
            break;
        }
        default: break;
    }
}

// callback function for mouse input
void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    // centers the shape if the left mouse button is pressed and the cursor is within the bounds of the shape
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

        // callculates the window size
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        // callculate the cursor position
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // normalize the cursor position to OpenGL range [-1,1]
        float xposNorm = (float) (xpos / width) * 2 - 1;
        float yposNorm = -((float)(ypos / height) * 2 - 1);

        // calculate which button was pressed
        Button pressed = getPressedButton(xposNorm, yposNorm);
        if (pressed >= BTN_SU && pressed <= BTN_6) {
            int floor = (int)pressed;

            // elevator is stopped at this exact floor
            bool elevatorIsHere = (state != MOVING) && (floor == currentFloorIndex) && fabs(elevatorY - floorHeight[floor]) < 0.001f;

            if (elevatorIsHere) {
                // reopen the doors (if already open or opening, do nothing)
                if (state == IDLE || state == DOOR_CLOSING) {
                    doorTimer = 0.0f;
                    doorTimerLimit = 5.0f;
                    timerExtend = false;
                    state = DOOR_OPENING;
                }
            }
            else {
                floorSelected[floor] = true;
            }
        }
        
        else if (pressed == BTN_OPEN) {
            // extend the open period by 5s, only once per open period
            if ((state == DOOR_OPEN || state == DOOR_OPENING) && !timerExtend) {
                timerExtend = true;
                doorTimerLimit += 5.0f;
            }
        }

        else if (pressed == BTN_CLOSE) {
            // close the doors immediately
            if (state == DOOR_OPEN || state == DOOR_OPENING) {
                state = DOOR_CLOSING;
            }
        }
    }
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

    // create the application window and context
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "WOOHP Elevator", nullptr, nullptr);

    if (window == nullptr)
    {
        return terminateWithError("Failed to create the application window.");
    }

    glfwMakeContextCurrent(window);

    // register callback functions
    glfwSetMouseButtonCallback(window, mouseCallback);

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
        -1.0f, -1.0f, 0.865f, 0.868f, 0.876f,
        -1.0f,  1.0f, 0.865f, 0.868f, 0.876f,
         0.0f,  1.0f, 0.865f, 0.868f, 0.876f,
         0.0f, -1.0f, 0.865f, 0.868f, 0.876f,
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

    // floor doors definition (8 floors, each: frame + door + vertical lines)
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

        // DOOR CENTER LINES
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

    // elevator definition 
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

        // ELEVATOR left door half
        0.81f, -0.47f, 0.651f, 0.725f, 0.718f,
        0.81f, -0.29f, 0.651f, 0.725f, 0.718f,
        0.88f, -0.29f, 0.651f, 0.725f, 0.718f,
        0.88f, -0.47f, 0.651f, 0.725f, 0.718f,

        // ELEVATOR right door half
        0.88f, -0.47f, 0.651f, 0.725f, 0.718f,
        0.88f, -0.29f, 0.651f, 0.725f, 0.718f,
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

    // button frames (white outline) definition
    float buttonFrameVertices[] = {
        // SU
        -0.920f,  0.200f, 1.0f, 1.0f, 1.0f,
        -0.920f,  0.370f, 1.0f, 1.0f, 1.0f,
        -0.730f,  0.370f, 1.0f, 1.0f, 1.0f,
        -0.730f,  0.200f, 1.0f, 1.0f, 1.0f,
        // PR
        -0.710f,  0.200f, 1.0f, 1.0f, 1.0f,
        -0.710f,  0.370f, 1.0f, 1.0f, 1.0f,
        -0.520f,  0.370f, 1.0f, 1.0f, 1.0f,
        -0.520f,  0.200f, 1.0f, 1.0f, 1.0f,
        // 1
        -0.500f,  0.200f, 1.0f, 1.0f, 1.0f,
        -0.500f,  0.370f, 1.0f, 1.0f, 1.0f,
        -0.310f,  0.370f, 1.0f, 1.0f, 1.0f,
        -0.310f,  0.200f, 1.0f, 1.0f, 1.0f,
        // 2
        -0.290f,  0.200f, 1.0f, 1.0f, 1.0f,
        -0.290f,  0.370f, 1.0f, 1.0f, 1.0f,
        -0.100f,  0.370f, 1.0f, 1.0f, 1.0f,
        -0.100f,  0.200f, 1.0f, 1.0f, 1.0f,
        // 3
        -0.920f, -0.020f, 1.0f, 1.0f, 1.0f,
        -0.920f,  0.150f, 1.0f, 1.0f, 1.0f,
        -0.730f,  0.150f, 1.0f, 1.0f, 1.0f,
        -0.730f, -0.020f, 1.0f, 1.0f, 1.0f,
        // 4
        -0.710f, -0.020f, 1.0f, 1.0f, 1.0f,
        -0.710f,  0.150f, 1.0f, 1.0f, 1.0f,
        -0.520f,  0.150f, 1.0f, 1.0f, 1.0f,
        -0.520f, -0.020f, 1.0f, 1.0f, 1.0f,
        // 5
        -0.500f, -0.020f, 1.0f, 1.0f, 1.0f,
        -0.500f,  0.150f, 1.0f, 1.0f, 1.0f,
        -0.310f,  0.150f, 1.0f, 1.0f, 1.0f,
        -0.310f, -0.020f, 1.0f, 1.0f, 1.0f,
        // 6
        -0.290f, -0.020f, 1.0f, 1.0f, 1.0f,
        -0.290f,  0.150f, 1.0f, 1.0f, 1.0f,
        -0.100f,  0.150f, 1.0f, 1.0f, 1.0f,
        -0.100f, -0.020f, 1.0f, 1.0f, 1.0f,
    };

    // VAO and VBO definition
    unsigned int buttonFrameVAO;
    unsigned int buttonFrameVBO;

    glGenVertexArrays(1, &buttonFrameVAO);
    glGenBuffers(1, &buttonFrameVBO);

    glBindVertexArray(buttonFrameVAO);
    glBindBuffer(GL_ARRAY_BUFFER, buttonFrameVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(buttonFrameVertices), buttonFrameVertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // set the background color
    glClearColor(0.08f, 0.09f, 0.10f, 1.0f);

    // uniform variable locations
    int uPosLoc = glGetUniformLocation(shaderProgram, "uPos");

    // get the current time before entering the rendering loop
    double lastTime = glfwGetTime();

    // main rendering loop
    while (!glfwWindowShouldClose(window))
    {
        // get the current time
        double now = glfwGetTime();

        // calculate the time elapsed since the previous frame
        delta = (float)(now - lastTime);

        // update the previous frame time
        lastTime = now;

        // elevator vertical position
        updateElevator();

        // calculate the difference between its current position and its original position
        float elevatorOffset = elevatorY - floorHeight[2];

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // set uniform default value
        glUniform2f(uPosLoc, 0.0f, 0.0f);

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

        // set the elevator's vertical position using the calculated offset
                // set the elevator's vertical position using the calculated offset
        glUniform2f(uPosLoc, 0.0f, elevatorOffset);
        glBindVertexArray(elevatorVAO);

        // draw elevator frame (not clipped)
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // calculate elevator clipping rectangle (cabin interior)
        int clipX = (int)((0.81f + 1.0f) / 2.0f * WINDOW_WIDTH);
        int clipY = (int)((-0.47f + elevatorOffset + 1.0f) / 2.0f * WINDOW_HEIGHT);
        int clipWidth = (int)((0.95f - 0.81f) / 2.0f * WINDOW_WIDTH);
        int clipHeight = (int)((-0.29f + 0.47f) / 2.0f * WINDOW_HEIGHT);

        // enable clipping so the door halves don't leave the cabin
        glEnable(GL_SCISSOR_TEST);
        glScissor(clipX, clipY, clipWidth, clipHeight);

        // draw cabin interior
        glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

        float doorMove = doorOpenAmount * 0.07f;

        // draw left half door
        glUniform2f(uPosLoc, -doorMove, elevatorOffset);
        glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

        // draw right half door
        glUniform2f(uPosLoc, doorMove, elevatorOffset);
        glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

        // disable clipping, otherwise glClear and everything else gets clipped
        glDisable(GL_SCISSOR_TEST);

        if (doorOpenAmount == 0) {
            // elevator door center line
            glUniform2f(uPosLoc, 0.0f, elevatorOffset);
            glDrawArrays(GL_LINES, 16, 2);
        }

        // reset offset so the control panel is not moved
        glUniform2f(uPosLoc, 0.0f, 0.0f);

        // draw control panel elements
        glBindVertexArray(panelElementsVAO);
        for (int i = 0; i < 13; i++)
        {
            glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
        }

        // draw white button frame if button is selected
        glBindVertexArray(buttonFrameVAO);
        glLineWidth(3.0f);
        for (int i = 0; i < 8; i++) {
            if (floorSelected[i]) {
                glDrawArrays(GL_LINE_LOOP, i * 4, 4);
            }
        }

        // draw text 
        // header for control panel
        renderText("WOOHP.", -0.92f, 0.86f, 0.8f, 0.125f, 0.137f, 0.153f);
        renderText("ELEVATOR MANAGEMENT SYSTEM", -0.92f, 0.79f, 0.28f, 0.4f, 0.4f, 0.45f);

        // current floor
        renderTextCentered("CURRENT FLOOR", -0.255f, 0.915f, 0.28f, 0.7f, 0.72f, 0.75f);
        renderTextCentered(getCurrentFloorLabel(), -0.255f, 0.835f, 0.8f, 1.0f, 1.0f, 1.0f);

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
    glDeleteVertexArrays(1, &buttonFrameVAO);
    glDeleteBuffers(1, &buttonFrameVBO);
    cleanupText();

    // close the application.
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}