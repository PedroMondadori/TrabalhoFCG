#include <glm/gtc/type_ptr.hpp>

// VERIFICA SE A POSICAO x E z DO PLAYER PASSOU DOS LIMITES DA PAREDE
bool Collision(glm::vec4 pos1, glm::vec4 pos2, glm::vec3 size1, glm::vec3 size2)
{
    glm::vec3 halfsize1 = glm::vec3(size1[0] / 2, size1[1] / 2, size1[2] / 2);
    glm::vec3 halfsize2 = glm::vec3(size2[0] / 2, size2[1] / 2, size2[2] / 2);

    bool collisionX = (pos1[0] + halfsize1[0] >= pos2[0] && pos2[0] + halfsize2[0] >= pos1[0]) || (pos2[0] - halfsize2[0] <= pos1[0] && pos1[0] - halfsize1[0] <= pos2[0]);
    bool collisionY = (pos1[1] + halfsize1[1] >= pos2[1] && pos2[1] + halfsize2[1] >= pos1[1]) || (pos2[1] - halfsize2[1] <= pos1[1] && pos1[1] - halfsize1[1] <= pos2[1]);
    bool collisionZ = (pos1[2] + halfsize1[2] >= pos2[2] && pos2[2] + halfsize2[2] >= pos1[2]) || (pos2[2] - halfsize2[2] <= pos1[2] && pos1[2] - halfsize1[2] <= pos2[2]);

    return collisionX && collisionY && collisionZ;
}

// ANALISA SE O ALVO ESTA NO VETOR DE VISAO DA CAMERA
bool ShotCollision(glm::vec4 posPoint, glm::vec4 bbox_min, glm::vec4 bbox_max)
{
    bool collisionX, collisionY, collisionZ;
    collisionY = (bbox_max.y >= posPoint.y) && (bbox_min.y <= posPoint.y);

    // TIVEMOS QUE FAZER CONDICIONAIS PARA CADA CASO, POIS A BOUNDING BOX DO
    // MONSTRO ROTACIONA JUNTO.

    if (bbox_max.x > bbox_min.x)
    {
        collisionX = (bbox_max.x >= posPoint.x) && (bbox_min.x <= posPoint.x);
        if (bbox_max.z > bbox_min.z)
            collisionZ = (bbox_max.z >= posPoint.z) && (bbox_min.z <= posPoint.z);
        else
            collisionZ = (bbox_max.z <= posPoint.z) && (bbox_min.z >= posPoint.z);
    }
    else
    {
        collisionX = (bbox_max.x <= posPoint.x) && (bbox_min.x >= posPoint.x);
        if (bbox_max.z > bbox_min.z)
            collisionZ = (bbox_max.z >= posPoint.z) && (bbox_min.z <= posPoint.z);
        else
            collisionZ = (bbox_max.z <= posPoint.z) && (bbox_min.z >= posPoint.z);
    }

    return collisionX && collisionY && collisionZ;
}

// VERIFICA SE O PERSONAGEM ESTA DENTRO DO BOUNDING BOX DA ARVORE
bool TreeCollision(glm::vec4 pos1, glm::vec3 size1, glm::vec4 bbox_max_world_tree, glm::vec4 bbox_min_world_tree)
{
    glm::vec3 halfsize1 = glm::vec3(size1[0] / 2, size1[1] / 2, size1[2] / 2);

    bool collisionX = (pos1[0] + halfsize1[0] >= bbox_min_world_tree.x && bbox_max_world_tree.x >= pos1[0]);
    bool collisionY = (pos1[1] + halfsize1[1] >= bbox_min_world_tree.y && bbox_max_world_tree.y >= pos1[1]);
    bool collisionZ = (pos1[2] + halfsize1[2] >= bbox_min_world_tree.z && bbox_max_world_tree.z >= pos1[2]);

    return collisionX && collisionY && collisionZ;
}
