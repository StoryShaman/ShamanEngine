#pragma once
#include <memory>

#include "SeModel.h"

namespace SE {

struct Transform2dComponent
{
    glm::vec2 translation{};
    glm::vec2 scale{1.f, 1.f};
    float rotation{};
    glm::mat2 mat2()
    {
        const float s = glm::sin(rotation);
        const float c = glm::cos(rotation);
        glm::mat2 rotMatrix{{c, s}, {-s, c}};
        
        glm::mat2 scaleMat{{scale.x, 0.f}, {0.f, scale.y}};
        return rotMatrix * scaleMat;
    }
};

class SeObject
{
public:
    using id_t = unsigned int;

    static SeObject createObject()
    {
        static id_t currentId = 0;
        return SeObject{currentId++};
    }

    SeObject(const SeObject &) = delete;
    SeObject &operator=(const SeObject &) = delete;
    SeObject(SeObject &&) = default;
    SeObject &operator=(SeObject &&) = default;

    id_t getId() const { return id; }

    std::shared_ptr<SeModel> model{};
    glm::vec3 color{};
    Transform2dComponent transform2d{};

private:
    SeObject(id_t objId) : id{objId} {}

    id_t id;
    
};

}

