#include <glm/gtc/type_ptr.hpp>

bool Collision(glm::vec4 pos1, glm::vec4 pos2, glm::vec3 sz1, glm::vec3 sz2);
bool ShotCollision(glm::vec4 posPoint, glm::vec4 bbox_min, glm::vec4 bbox_max);
bool TreeCollision(glm::vec4 pos1, glm::vec3 sz1, glm::vec4 bbox_max_world_tree, glm::vec4 bbox_min_world_tree);
