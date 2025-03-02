#include "SeController.h"

#include <iostream>
#include <GLM/vec3.hpp>
#include "SeObject.h"
#include "vulkancontext.h"

namespace SE {
void SeController::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
     SeController* controller = static_cast<SeController*>(glfwGetWindowUserPointer(window));
     if (!controller) return;
     if (button == GLFW_MOUSE_BUTTON_RIGHT) {
          if (action == GLFW_PRESS) {
               // Disable cursor when right mouse button is pressed
               glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
               controller->mouseLook = true;
          } else if (action == GLFW_RELEASE) {
               // Restore cursor when right mouse button is released
               glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
               controller->mouseLook = false;
          }
     }
}

void SeController::cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
     SeController* controller = static_cast<SeController*>(glfwGetWindowUserPointer(window));
     if (!controller) return;
     // Calculate mouse movement delta
     // 
     controller->cX = ypos * -1;
     controller->cY = xpos;
     
     
}

SeController::SeController(std::shared_ptr<VulkanContext> inctx)
{
     ctx = inctx;
     glfwSetWindowUserPointer(ctx->Se_window->window, this);
     glfwSetMouseButtonCallback(ctx->Se_window->window, mouse_button_callback);
     glfwSetCursorPosCallback(ctx->Se_window->window, cursor_position_callback);
}



void SeController::moveInPlaneXZ(GLFWwindow* window, float deltaTime, SeObject& object)
{
     
     glm::vec3 rotate{0};
     // Mouselook
     if (mouseLook)
     {
          float dx = (float)(cX - lX);
          float dy = (float)(cY - lY);

          // Sensitivity (adjust as needed)
          float sensitivity = 0.8f;
          dx *= sensitivity;
          dy *= sensitivity;
          rotate.x += dx;
          rotate.y += dy;
          if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
          {
               object.transform.rotation += lookSpeed * deltaTime * rotate;
          }

          //if (dx > 0 || dy > 0) std::cout << dx << ", " << dy << std::endl;
     }
     lX = cX; lY = cY;
     
     if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
     if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;
     if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
     if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;
     if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
     {
          object.transform.rotation += lookSpeed * deltaTime * glm::normalize(rotate);
     }

     object.transform.rotation.x = glm::clamp(object.transform.rotation.x, -34.5f, 34.5f);
     object.transform.rotation.y = glm::mod(object.transform.rotation.y, glm::two_pi<float>());

     float yaw = object.transform.rotation.y;
     const glm::vec3 forwardDir{sin(yaw), 0, cos(yaw)};
     const glm::vec3 upDir{0.f, -1.f, 0.f};
     const glm::vec3 rightDir = glm::normalize(glm::cross(forwardDir, upDir)); // Correct right vector

     glm::vec3 moveDir{0.f};
     if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
     if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
     if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
     if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
     if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
     if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;
     
     if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
     {
          object.transform.translation += moveSpeed * deltaTime * glm::normalize(moveDir);
     }
}

}
