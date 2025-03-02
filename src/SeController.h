#pragma once
#include <memory>
#include <GLFW/glfw3.h>



namespace SE {
struct VulkanContext;

class SeObject;

class SeController
{
public:
    struct KeyMappings
    {
        int moveLeft = GLFW_KEY_A;
        int moveRight = GLFW_KEY_D;
        int moveForward = GLFW_KEY_W;
        int moveBackward = GLFW_KEY_S;
        int moveUp = GLFW_KEY_E;
        int moveDown = GLFW_KEY_Q;
        int lookLeft = GLFW_KEY_LEFT;
        int lookRight = GLFW_KEY_RIGHT;
        int lookUp = GLFW_KEY_UP;
        int lookDown = GLFW_KEY_DOWN;
        int mouseMove = GLFW_MOUSE_BUTTON_2;
    };

    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void cursor_position_callback(GLFWwindow* inwindow, double xpos, double ypos);

    SeController(std::shared_ptr<VulkanContext> inctx);

    void moveInPlaneXZ(GLFWwindow* window, float deltaTime, SeObject& object);

    KeyMappings keys{};
    float moveSpeed{3.f};
    float lookSpeed{1.5f};

    std::shared_ptr<VulkanContext> ctx = nullptr;
    bool mouseLook = false;
    float cX = 0.0f;
    float cY = 0.0f;
    float lX = 0.0f;
    float lY = 0.0f;
};

}