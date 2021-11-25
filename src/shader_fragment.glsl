#version 330 core

in vec4 position_world;
in vec4 normal;

in vec4 position_model;

in vec2 texcoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform int object_id;

uniform vec4 bbox_min;
uniform vec4 bbox_max;

uniform sampler2D TextureImage0;
uniform sampler2D TextureImage1;
uniform sampler2D TextureImage2;

out vec3 color;

#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

// DEFINE AS CONSTANTES QUE REPRESENTAM NOSSOS OBJETOS
#define PLANE 0
#define INIMIGO  1
#define TREE  2
#define GUN   3
#define WALL  4
#define WALL2 5

void main() {
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

    vec4 p = position_world;

    vec4 n = normalize(normal);

    vec4 l = normalize(vec4(1.0,1.0,0.0,0.0));

    vec4 v = normalize(camera_position - p);

    float U = 0.0;
    float V = 0.0;

    if (object_id == PLANE) {
        U = texcoords.x;
        V = texcoords.y;

        // Obtemos a refletância difusa a partir da leitura da imagem TextureImage0
        vec3 Kd0 = texture(TextureImage2, vec2(U,V)).rgb ;

        // Equação de Iluminação
        float lambert = max(0,dot(n,l));

        color = Kd0 * (lambert + 0.01);

        color = pow(color, vec3(1.0,1.0,1.0)/1.0);
    }

    else if (object_id == INIMIGO) {
        // Espectro da fonte de iluminação
        vec3 I = vec3(1.0,1.0,1.0);

        vec3 Kd = vec3(1.0,0.0,0.0);
        vec3 Ka = vec3(1.0,0.0,0.0);
        float q = 1.0;

        vec3 Ia = vec3(0.2,0.2,0.2);
        vec3 ambient_term = Ka * Ia;

        vec3 lambert_diffuse_term = Kd * I * max(0,dot(n,l));
        color = lambert_diffuse_term + ambient_term;
    }

    else if (object_id == TREE) {
        // Espectro da fonte de iluminação
        vec3 I = vec3(1.0,1.0,1.0);

        vec3 Kd = vec3(0.0,0.1,0.0);
        vec3 Ks = vec3(0.5,0.5,0.5);
        vec3 Ka = vec3(0.1,0.3,0.1);
        float q = 64.0;

        vec3 Ia = vec3(0.2,0.2,0.2);
        vec3 ambient_term = Ka * Ia;

        vec4 r = -l + (2 * n * dot(l, n));

        vec4 h = normalize(v + l);

        vec3 phong_specular_term  = Ks * I * pow(max(0, dot(n,h)), q);

        vec3 lambert_diffuse_term = Kd * I * max(0,dot(n,l));

        color = lambert_diffuse_term + ambient_term + phong_specular_term;
    }

    else if (object_id == GUN) {
        vec3 I = vec3(1.0,1.0,1.0);

        vec3 Kd = vec3(0.05,0.05,0.05);
        vec3 Ks = vec3(0.05,0.05,0.05);
        vec3 Ka = vec3(0.0,0.0,0.0);
        float q = 64.0;

        vec3 Ia = vec3(0.2,0.2,0.2);
        vec3 ambient_term = Ka * Ia;

        vec4 r = -l + (2 * n * dot(l, n));
        vec4 h = normalize(v + l);

        vec3 phong_specular_term  = Ks * I * pow(max(0, dot(n,h)), q);
        vec3 lambert_diffuse_term = Kd * I * max(0,dot(n,l));

        color = lambert_diffuse_term + ambient_term + phong_specular_term;
    }

    else if (object_id == WALL) {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;

        // Obtemos a refletância difusa a partir da leitura da imagem TextureImage0
        vec3 Kd0 = texture(TextureImage1, vec2(U,V)).rgb ;

        // Equação de Iluminação
        float lambert = max(0,dot(n,l));

        color = Kd0 * (lambert + 0.01);

        color = pow(color, vec3(1.0,1.0,1.0)/2.1);
    }

    else if (object_id == WALL2) {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.y;
        V = texcoords.x;

        // Obtemos a refletância difusa a partir da leitura da imagem TextureImage0
        vec3 Kd0 = texture(TextureImage1, vec2(U,V)).rgb ;

        // Equação de Iluminação
        float lambert = max(0,dot(n,l));

        color = Kd0 * (lambert + 0.01);

        color = pow(color, vec3(1.0,1.0,1.0)/2.1);
    }

    color = pow(color, vec3(1.0,1.0,1.0)/2.2);
}

