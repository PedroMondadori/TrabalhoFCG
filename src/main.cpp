// TRABALHO FINAL FCG
// JOGUINHO DE TIRO POW POW

// Alexadre de Rosso Crestani   00312980
// Airton Hoch Junior           00275852
// Pedro Lago Mondadori         00301506

#include <stack>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <mmsystem.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <tiny_obj_loader.h>
#include <stb_image.h>

#include "utils.h"
#include "matrices.h"

// INCLUSAO DO HEADER "colliosions.hpp"
#include "collisions.hpp"

#define PI 3.1415

struct ObjModel
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    ObjModel(const char *filename, const char *basepath = NULL, bool triangulate = true)
    {
        printf("Carregando modelo \"%s\"... ", filename);

        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename, basepath, triangulate);

        if (!err.empty())
            fprintf(stderr, "\n%s\n", err.c_str());

        if (!ret)
            throw std::runtime_error("Erro ao carregar modelo.");

        printf("OK.\n");
    }
};

void PushMatrix(glm::mat4 M);
void PopMatrix(glm::mat4 &M);

void BuildTrianglesAndAddToVirtualScene(ObjModel *);
void ComputeNormals(ObjModel *model);
void LoadShadersFromFiles();
void LoadTextureImage(const char *filename);
void DrawVirtualObject(const char *object_name);
GLuint LoadShader_Vertex(const char *filename);
GLuint LoadShader_Fragment(const char *filename);
void LoadShader(const char *filename, GLuint shader_id);
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id);

void TextRendering_Init();
float TextRendering_LineHeight(GLFWwindow *window);
float TextRendering_CharWidth(GLFWwindow *window);
void TextRendering_PrintString(GLFWwindow *window, const std::string &str, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrix(GLFWwindow *window, glm::mat4 M, float x, float y, float scale = 1.0f);
void TextRendering_PrintVector(GLFWwindow *window, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProduct(GLFWwindow *window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductMoreDigits(GLFWwindow *window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductDivW(GLFWwindow *window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);

void FramebufferSizeCallback(GLFWwindow *window, int width, int height);
void ErrorCallback(int error, const char *description);
void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow *window, double xpos, double ypos);
void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset);

struct SceneObject
{
    std::string name;
    size_t first_index;
    size_t num_indices;
    GLenum rendering_mode;
    GLuint vertex_array_object_id;
    glm::vec3 bbox_min;
    glm::vec3 bbox_max;
};

std::map<std::string, SceneObject> g_VirtualScene;

std::stack<glm::mat4> g_MatrixStack;

glm::vec4 bezier(float t, glm::vec4 p1, glm::vec4 p2, glm::vec4 p3, glm::vec4 p4);

float g_ScreenRatio = 1.0f;

float g_AngleX = 0.0f;
float g_AngleY = 0.0f;
float g_AngleZ = 0.0f;

bool g_LeftMouseButtonPressed = false;
bool g_RightMouseButtonPressed = false;
bool g_MiddleMouseButtonPressed = false;

float g_CameraTheta = 0.0f;
float g_CameraPhi = 0.0f;
float g_CameraDistance = 3.5f;

float r = g_CameraDistance;
float y = r * sin(g_CameraPhi);
float z = r * cos(g_CameraPhi) * cos(g_CameraTheta);
float x = r * cos(g_CameraPhi) * sin(g_CameraTheta);

float g_ForearmAngleZ = 0.0f;
float g_ForearmAngleX = 0.0f;

float g_TorsoPositionX = 0.0f;
float g_TorsoPositionY = 0.0f;

bool g_ShowInfoText = true;

GLuint vertex_shader_id;
GLuint fragment_shader_id;
GLuint program_id = 0;
GLint model_uniform;
GLint view_uniform;
GLint projection_uniform;
GLint object_id_uniform;
GLint bbox_min_uniform;
GLint bbox_max_uniform;

GLuint g_NumLoadedTextures = 0;

glm::vec4 bbox_min_world_monster[5];
glm::vec4 bbox_max_world_monster[5];

// ====================================================================================================================================================
// ====================================================================================================================================================
//                                                               FUNCOES DOS GURI
// ====================================================================================================================================================
// ====================================================================================================================================================

// STATUS DO JOGADOR
int PLAYER_HP = 100;
int PLAYER_SCORE = 0;
int PLAYER_FINAL_SCORE = 0;
bool PLAYER_INVENCIVEL = false;
bool PLAYER_PODE_CURAR = false;
float PLAYER_TIME;

// STATUS DO MONSTRO
int MONSTER_HP[5] = {1};

float MONSTER_SPEED = 10;

// FLAGS PARA CORRIGIR A TAXA DE TIROS E MANDAR APENAS 1 POR CLICK
float SHOT_ANIMATION = 0.0f;
bool ATIROU = false;
float SHOT_RATE = 0.0f;

// FLAGS DE CONTROLE DE MOVIMENTO
bool W_KEY_ON = false;
bool A_KEY_ON = false;
bool S_KEY_ON = false;
bool D_KEY_ON = false;
bool RESTART = false;
bool SPRINT = false;

// FLAGS PARA MANTER A VELOCIDADE CONSTANTE INDEPENDENTE DO COMPUTADOR
float DELTA_TIME = 0.0f;
float LAST_FRAME = 0.0f;

// DEFINIMOS AS VARIAVEIS QUE DEFINEM A CAMERA VIRTUAL (semelhante ao lab 5)
glm::vec4 camera_position_c = glm::vec4(0.0f, 1.5f, 3.0f, 1.0f); // Ponto "c", centro da câmera
glm::vec4 camera_lookat_l = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);   // Ponto "l", para onde a câmera (look-at) estará sempre olhando
glm::vec4 camera_view_vector = glm::vec4(-x, 0.0f, -z, 0.0f);    // Vetor "view", sentido para onde a câmera está virada
glm::vec4 camera_up_vector = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);  // Vetor "up" fixado para apontar para o "céu" (eixo Y global)

glm::vec4 antigaPos;
// Variável que controla se a câmera é lookat ou livre
bool g_UseLookatCamera = false;

// DEFINE A QUANTIDADE DE ARVORES NO CENARIO E ARENA
#define TREES 50
glm::vec3 g_Tree_Positions[TREES];
float g_Tree_Size[TREES];
void BuildTrees()
{
    for (int i = 0; i < TREES; i++)
    {
        float x = rand() % 100 - 50;
        float z = rand() % 100 - 50;

        while (abs(x) < 3)
            x = rand() % 100 - 50;
        while (abs(z) < 3)
            z = rand() % 100 - 50;

        g_Tree_Positions[i] = glm::vec3(x, -1.0f, z);
        g_Tree_Size[i] = (50 + rand() % 50) / (float)100;
    }
}

// DESENHA O CHAO
void DrawFloor(glm::mat4 model, int object)
{
    model = Matrix_Translate(0.0f, -1.1f, 0.0f);
    model *= Matrix_Scale(40.0f, 1.0f, 40.0f);
    glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(object_id_uniform, object);
    DrawVirtualObject("plane");
}

// DESENHA AS 4 PAREDES
void DrawAllWalls(glm::mat4 model, int wall1, int wall2)
{
    // Parede 1
    model = Matrix_Translate(0.0f, 0.9f, 40.0f);
    model *= Matrix_Rotate_X(-1.57f);
    model *= Matrix_Scale(40.f, 1.0f, 2.0f);
    glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(object_id_uniform, wall1);
    DrawVirtualObject("wall");

    // Parede 2
    model = Matrix_Translate(0.0f, 0.9f, -40.0f);
    model *= Matrix_Rotate_X(1.57f);
    model *= Matrix_Scale(40.f, 1.0f, 2.0f);
    glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(object_id_uniform, wall1);
    DrawVirtualObject("wall");

    // Parede 3
    model = Matrix_Translate(40.0f, 0.9f, 0.0f);
    model *= Matrix_Rotate_Z(1.57f);
    model *= Matrix_Scale(2.0f, 1.0f, 40.5f);
    glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(object_id_uniform, wall2);
    DrawVirtualObject("wall");

    // Parede 4
    model = Matrix_Translate(-40.0f, 0.9f, 0.0f);
    model *= Matrix_Rotate_Z(-1.57f);
    model *= Matrix_Scale(2.0f, 1.0f, 40.0f);
    glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(object_id_uniform, wall2);
    DrawVirtualObject("wall");
}

// DESENHA A ARMA
void DrawGun(glm::mat4 model, glm::vec4 c, int GUN)
{
    PushMatrix(model);
    model *= Matrix_Translate(c[0], c[1] - 0.1f, c[2]);
    model *= Matrix_Scale(0.05f, 0.05f, 0.05f);
    // model *= Matrix_Rotate_X(3.14f/2.0f);
    // model *= Matrix_Rotate_Z(-3.14f/2.0f);
    model *= Matrix_Rotate_Y(-PI);
    glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(object_id_uniform, GUN);
    DrawVirtualObject("arma");
    PopMatrix(model);
}

// DESENHA O MONSTRO
void DrawMonster(glm::mat4 model, glm::vec4 monster_pos, GLint model_uniform, int INIMIGO, int index)
{
    model = Matrix_Translate(monster_pos.x, monster_pos.y, monster_pos.z);
    model *= Matrix_Scale(0.1f, 0.1f, 0.1f);

    glm::vec4 monster_view = glm::vec4(monster_pos.x - camera_position_c.x, 0.0f, monster_pos.z - camera_position_c.z, 0.0f);

    float rotation_angle = atan(monster_view.x / monster_view.z);

    if ((monster_pos.z - camera_position_c.z) > 0)
        rotation_angle = rotation_angle + PI;

    model *= Matrix_Rotate_Y(rotation_angle);

    glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(object_id_uniform, INIMIGO);
    DrawVirtualObject("inimigo");

    bbox_min_world_monster[index] = model * glm::vec4(g_VirtualScene["inimigo"].bbox_min.x, g_VirtualScene["inimigo"].bbox_min.y, g_VirtualScene["inimigo"].bbox_min.z, 1.0f);
    bbox_max_world_monster[index] = model * glm::vec4(g_VirtualScene["inimigo"].bbox_max.x, g_VirtualScene["inimigo"].bbox_max.y, g_VirtualScene["inimigo"].bbox_max.z, 1.0f);
}

// DESENHA AS ARVORES
void DrawTrees(glm::mat4 model, GLint model_uniform, int TREE)
{
    for (int i = 0; i < TREES; i++)
    {
        model = Matrix_Translate(g_Tree_Positions[i].x, g_Tree_Positions[i].y, g_Tree_Positions[i].z);
        model *= Matrix_Scale(0.5f, g_Tree_Size[i], 0.5f);

        glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(object_id_uniform, TREE);
        DrawVirtualObject("tree");

        glm::vec4 bbox_min_world_tree = model * glm::vec4(g_VirtualScene["tree"].bbox_min.x, g_VirtualScene["tree"].bbox_min.y, g_VirtualScene["tree"].bbox_min.z, 1.0f);
        glm::vec4 bbox_max_world_tree = model * glm::vec4(g_VirtualScene["tree"].bbox_max.x, g_VirtualScene["tree"].bbox_max.y, g_VirtualScene["tree"].bbox_max.z, 1.0f);

        if (TreeCollision(camera_position_c, glm::vec3(0.1f, 0.1f, 0.1f), bbox_max_world_tree, bbox_min_world_tree))
        {
            camera_position_c = antigaPos;
        }
    }
}

// MOSTRA NA TELA:
// A VIDA ATUAL E MAXIMA DO PLAYER
// A CONTAGEM DE SCORE
// A VELOCIDADE ATUAL DO MONSTRO
void TextRendering_ShowGameStatus(GLFWwindow *window)
{
    if (!g_ShowInfoText)
    {
        return;
    }
    float padding = TextRendering_LineHeight(window);

    char buffer1[20];
    snprintf(buffer1, 20, "HP:      %d/100", PLAYER_HP);
    TextRendering_PrintString(window, buffer1, -1.0f, 1.0f - padding, 1.0f);

    char buffer2[20];
    snprintf(buffer2, 20, "Score:   %d", PLAYER_SCORE);
    TextRendering_PrintString(window, buffer2, -1.0f, 1.0f - 2 * padding, 1.0f);

    char buffer3[20];
    snprintf(buffer3, 20, "M Speed: %.0f", MONSTER_SPEED);
    TextRendering_PrintString(window, buffer3, -1.0f, 1.0f - 3 * padding, 1.0f);

    char buffer4[20];
    snprintf(buffer4, 20, "View: %.3f", camera_position_c.x);
    TextRendering_PrintString(window, buffer4, -1.0f, 1.0f - 4 * padding, 1.0f);

    char buffer5[20];
    snprintf(buffer5, 20, "View: %.3f", camera_position_c.z);
    TextRendering_PrintString(window, buffer5, -1.0f, 1.0f - 5 * padding, 1.0f);
}

// MOSTRA NA TELA UMA MENSAGEM DE GAME OVER
// AVISANDO QUE O JOGADOR MORREU
// E MOSTRANDO A QUANTIDADE DE PONTOS FEITOS
void TextRendering_ShowDeathMessage(GLFWwindow *window)
{
    float padding = TextRendering_LineHeight(window);

    char buffer1[15];
    snprintf(buffer1, 15, "Voce Morreu");
    TextRendering_PrintString(window, buffer1, -0.5f, 1.0f - 12 * padding, 4.0f);

    char buffer2[15];
    snprintf(buffer2, 15, " Game Over ");
    TextRendering_PrintString(window, buffer2, -0.5f, 1.0f - 17 * padding, 4.0f);

    char buffer3[16];
    snprintf(buffer3, 16, "  Score: %d", PLAYER_FINAL_SCORE);
    TextRendering_PrintString(window, buffer3, -0.5f, 1.0f - 22 * padding, 4.0f);
}

// MOSTRA O CURSOR / MIRA DO PLAYER NO MEIO DA TELA
void TextRendering_PrintCursor(GLFWwindow *window)
{
    char buffer[2];
    snprintf(buffer, 2, "X");

    TextRendering_PrintString(window, buffer, -0.0f, -0.0f, 2.0f);
}

// MOSTRA NA TELA O FPS ATUAL
void TextRendering_ShowFramesPerSecond(GLFWwindow *window)
{
    if (!g_ShowInfoText)
        return;

    // Variáveis estáticas (static) mantém seus valores entre chamadas
    // subsequentes da função!
    static float old_seconds = (float)glfwGetTime();
    static int ellapsed_frames = 0;
    static char buffer[20] = "?? fps";
    static int numchars = 7;

    ellapsed_frames += 1;

    // Recuperamos o número de segundos que passou desde a execução do programa
    float seconds = (float)glfwGetTime();

    // Número de segundos desde o último cálculo do fps
    float ellapsed_seconds = seconds - old_seconds;

    if (ellapsed_seconds > 1.0f)
    {
        numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);

        old_seconds = seconds;
        ellapsed_frames = 0;
    }

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, buffer, 1.0f - (numchars + 1) * charwidth, 1.0f - lineheight, 1.0f);
}

// CURVAS DE BEZIER PARA O COICE DA ARMA
glm::vec4 bezier(float t, glm::vec4 p1, glm::vec4 p2, glm::vec4 p3, glm::vec4 p4)
{
    glm::vec3 a = glm::vec3(pow(1 - t, 3) * p1[0], pow(1 - t, 3) * p1[1], pow(1 - t, 3) * p1[2]);
    glm::vec3 b = glm::vec3(3 * t * pow(1 - t, 2) * p2[0], 3 * t * pow(1 - t, 2) * p2[1], 3 * t * pow(1 - t, 2) * p2[2]);
    glm::vec3 c = glm::vec3(3 * t * t * (1 - t) * p3[0], 3 * t * t * (1 - t) * p3[1], 3 * t * t * (1 - t) * p3[2]);
    glm::vec3 d = glm::vec3(pow(t, 3) * p4[0], pow(t, 3) * p4[1], pow(t, 3) * p4[2]);

    return glm::vec4(a[0] + b[0] + c[0] + d[0], a[1] + b[1] + c[1] + d[1], a[2] + b[2] + c[2] + d[2], 1.0f);
}

// FUNCAO MODIFICADA RESGATADA DO LAB 3 OU 4...
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mod)
{
    // ==============
    // Não modifique este loop! Ele é utilizando para correção automatizada dos
    // laboratórios. Deve ser sempre o primeiro comando desta função KeyCallback().
    for (int i = 0; i < 10; ++i)
        if (key == GLFW_KEY_0 + i && action == GLFW_PRESS && mod == GLFW_MOD_SHIFT)
            std::exit(100 + i);
    // ==============

    // SE O USUÁRIO PRESSIONA 'ESC', SAI
    // W, A, S, D, MOVIMENTA O PLAYER
    // R REINICIA O JOGO
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;

        case GLFW_KEY_W:
            W_KEY_ON = true;
            break;

        case GLFW_KEY_A:
            A_KEY_ON = true;
            break;

        case GLFW_KEY_S:
            S_KEY_ON = true;
            break;

        case GLFW_KEY_D:
            D_KEY_ON = true;
            break;

        case GLFW_KEY_R:
            RESTART = true;
            break;

        case GLFW_KEY_LEFT_SHIFT:
            SPRINT = true;
            break;
        }
    }

    // SE O USUARIO SOLTA UMA TECLA, PARA O MOVIMENTO
    if (action == GLFW_RELEASE)
    {
        switch (key)
        {
        case GLFW_KEY_W:
            W_KEY_ON = false;
            break;

        case GLFW_KEY_A:
            A_KEY_ON = false;
            break;

        case GLFW_KEY_S:
            S_KEY_ON = false;
            break;

        case GLFW_KEY_D:
            D_KEY_ON = false;
            break;

        case GLFW_KEY_LEFT_SHIFT:
            SPRINT = false;
            break;
        }
    }

    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        g_UseLookatCamera = !g_UseLookatCamera;
    }

    // Se o usuário apertar a tecla espaço, resetamos os ângulos de Euler para zero.
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        g_AngleX = 0.0f;
        g_AngleY = 0.0f;
        g_AngleZ = 0.0f;
        g_ForearmAngleX = 0.0f;
        g_ForearmAngleZ = 0.0f;
        g_TorsoPositionX = 0.0f;
        g_TorsoPositionY = 0.0f;
    }

    // Se o usuário apertar a tecla H, fazemos um "toggle" do texto informativo mostrado na tela.
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
    {
        g_ShowInfoText = !g_ShowInfoText;
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        LoadShadersFromFiles();
        fprintf(stdout, "Shaders recarregados!\n");
        fflush(stdout);
    }
}

// ====================================================================================================================================================
// ====================================================================================================================================================
//                                                           FIM DAS FUNCOES DOS GURI
// ====================================================================================================================================================
// ====================================================================================================================================================
int main(int argc, char *argv[])
{
    MONSTER_HP[0] = 1;
    MONSTER_HP[1] = 1;
    MONSTER_HP[2] = 1;
    MONSTER_HP[3] = 1;
    MONSTER_HP[4] = 1;

    // CONSTRUIMOS OS DADOS DE DEFINICAO DAS ARVORES
    BuildTrees();

    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    glfwSetErrorCallback(ErrorCallback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // DEFINIMOS UMA JANELA DE TAMANHO 800x600 COM O TITULO DO JOGO
    GLFWwindow *window;

    int height, width, top, left;

    top = 50;
    left = 50;

    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    width = mode->width - 100;
    height = mode->height - 100;

    window = glfwCreateWindow(width, height, "JOGUINHO DE TIRO POW POW", NULL, NULL);
    glfwSetWindowPos(window, left, top);

    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    FramebufferSizeCallback(window, width, height);

    const GLubyte *vendor = glGetString(GL_VENDOR);
    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *glversion = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);
    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    LoadShadersFromFiles();

    // CARREGAMOS 2 IMAGENS PARA USAR COMO TEXTURA
    LoadTextureImage("../../data/parede.jpg");
    LoadTextureImage("../../data/chao.jpg");

    // ASSIM COMO NO LAB 5, CONSTRUIMOS A REPRESENTACAO DOS OBJETOS POR MALHAS DE TRIANGULOS
    ObjModel planemodel("../../data/plane.obj");
    ComputeNormals(&planemodel);
    BuildTrianglesAndAddToVirtualScene(&planemodel);

    ObjModel treemodel("../../data/tree.obj");
    ComputeNormals(&treemodel);
    BuildTrianglesAndAddToVirtualScene(&treemodel);

    ObjModel wallmodel("../../data/wall.obj");
    ComputeNormals(&wallmodel);
    BuildTrianglesAndAddToVirtualScene(&wallmodel);

    ObjModel gunmodel("../../data/arma.obj");
    ComputeNormals(&gunmodel);
    BuildTrianglesAndAddToVirtualScene(&gunmodel);

    ObjModel monstromodel("../../data/inimigo.obj");
    ComputeNormals(&monstromodel);
    BuildTrianglesAndAddToVirtualScene(&monstromodel);

    glm::vec4 monster_pos[5];
    monster_pos[0] = glm::vec4(40.0f, 1.0f, 40.0f, 1.0f);
    monster_pos[1] = glm::vec4(-40.0f, 1.0f, 40.0f, 1.0f);
    monster_pos[2] = glm::vec4(40.0f, 1.0f, -40.0f, 1.0f);
    monster_pos[3] = glm::vec4(-40.0f, 1.0f, -40.0f, 1.0f);
    monster_pos[4] = glm::vec4(20.0f, 1.0f, 20.0f, 1.0f);
    bool played_sound = false;

    if (argc > 1)
    {
        ObjModel model(argv[1]);
        BuildTrianglesAndAddToVirtualScene(&model);
    }

    TextRendering_Init();

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glm::mat4 the_projection;
    glm::mat4 the_model;
    glm::mat4 the_view;

    // RENDERIZA ATE QUE O PLAYER FECHE A JANELA 'ESC'
    while (!glfwWindowShouldClose(window))
    {
        // VALORES (RGB, transparencia) DO PLANO DE FUNDO (CEU)
        glClearColor(0.53f, 0.81f, 1.0f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program_id);

        const float currentFrame = glfwGetTime();
        DELTA_TIME = currentFrame - LAST_FRAME;
        LAST_FRAME = currentFrame;

        float SPEED = 10.0f * DELTA_TIME;

        if (SPRINT)
            SPEED *= 1.5f;

        float r = g_CameraDistance;
        float y = r * sin(g_CameraPhi);
        float z = r * cos(g_CameraPhi) * cos(g_CameraTheta);
        float x = r * cos(g_CameraPhi) * sin(g_CameraTheta);

        glm::mat4 view;

        if (RESTART)
        {
            g_UseLookatCamera = false;
            RESTART = false;
            played_sound = false;
            MONSTER_SPEED = 10;
            PLAYER_HP = 100;
            PLAYER_SCORE = 0;
            monster_pos[0] = glm::vec4(40.0f, 1.0f, 40.0f, 1.0f);
            monster_pos[1] = glm::vec4(-40.0f, 1.0f, 40.0f, 1.0f);
            monster_pos[2] = glm::vec4(40.0f, 1.0f, -40.0f, 1.0f);
            monster_pos[3] = glm::vec4(-40.0f, 1.0f, -40.0f, 1.0f);
            monster_pos[4] = glm::vec4(20.0f, 1.0f, 20.0f, 1.0f);
        }

        // DEFINIMOS AS "INTERACOES" COM A CAMERA (usuario)
        if (!g_UseLookatCamera) {

            camera_view_vector = glm::vec4(x, -y, z, 0.0f);

            const glm::vec4 u = crossproduct(camera_up_vector, -camera_view_vector);

            const float camera_y_before = camera_position_c.y;

            antigaPos = camera_position_c;

            // PRESSIONAR UMA TECLA FAZ A FUNCAO KeyCallBack SETAR A VARIAVEL CORRESPONDENTE PARA true, SETA PRA false AO SOLTAR A TECLA
            if (W_KEY_ON)
                camera_position_c += SPEED * camera_view_vector / norm(camera_view_vector);
            if (S_KEY_ON)
                camera_position_c -= SPEED * camera_view_vector / norm(camera_view_vector);
            if (A_KEY_ON)
                camera_position_c -= u / norm(u) * SPEED;
            if (D_KEY_ON)
                camera_position_c += u / norm(u) * SPEED;

            camera_position_c.y = camera_y_before;

            // COLISAO COM AS PAREDES
            // SE BATEU RETORNA PRA POSICAO ANTERIOR (pos no momento anterior a batida)
            bool bateu = Collision(camera_position_c, glm::vec4(0.0f, 0.0f, 39.8f, 1.0f), glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(80.0f, 10.0f, 0.0f));
            bateu = bateu || Collision(camera_position_c, glm::vec4(0.0f, 0.0f, -39.8f, 1.0f), glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(80.0f, 10.0f, 0.0f));
            bateu = bateu || Collision(camera_position_c, glm::vec4(39.8f, 0.0f, 0.0f, 1.0f), glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.0f, 10.0f, 80.0f));
            bateu = bateu || Collision(camera_position_c, glm::vec4(-39.8f, 0.0f, 0.0f, 1.0f), glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.0f, 10.0f, 80.0f));
            if (bateu)
            {
                camera_position_c = antigaPos;
            }

            bool toma_dano[5];

            // COLISAO COM O INIMIGO
            // HABILITA A POSSIBILIDADE DE TOMAR DANO CASO ESTEJA EM CONTATO
            for (int i = 0; i < 5; i++)
                toma_dano[i] = Collision(camera_position_c, monster_pos[i], glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(2.0f, 2.0f, 2.0f));

            // DANO
            // VERIFICACAO QUE IMPEDE QUE O USUARIO TOME DANO A TODO MOMENTO, HABILITANDO O DANO A CADA 1 SEGUNDO DE CONTATO
            // O INIMIGO TEM DANO ALEATORIO ENTRE 20 E 30
            // O PLAYER REGENERA 5 DE VIDA A CADA 2 SEGUNDOS SEM TOMAR DANO (maximo de vida a regenerar é 99)
            if (glfwGetTime() - PLAYER_TIME > 1.0)
            {
                PLAYER_INVENCIVEL = false;
            }

            for (int i = 0; i < 5; i++)
            {
                if (toma_dano[i] && !PLAYER_INVENCIVEL)
                {
                    PLAYER_HP -= rand() % 10 + 20;
                    PLAYER_INVENCIVEL = true;
                    PLAYER_TIME = glfwGetTime();
                }

                PLAYER_PODE_CURAR = glfwGetTime() - PLAYER_TIME > 2.0;
                if (!toma_dano[i] && PLAYER_PODE_CURAR)
                {
                    if (PLAYER_HP <= 94)
                    {
                        PLAYER_HP += 5;
                    }
                    else
                    {
                        PLAYER_HP = 100;
                    }
                    PLAYER_PODE_CURAR = false;
                    PLAYER_TIME = glfwGetTime();
                }
            }

            view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);
        }
        else
        {
            // AJUSTA A CAMERA
            glm::vec4 camera_position_c_lookat = glm::vec4(x, y, z, 1.0f);   // centro da camera
            camera_lookat_l = glm::vec4(0.0f, -1.1f, 0.0f, 1.0f);            // look-at
            camera_view_vector = camera_lookat_l - camera_position_c_lookat; // sentido para onde a câmera esta virada
            camera_up_vector = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);            // vetor up, aponta pra cima

            view = Matrix_Camera_View(camera_position_c_lookat, camera_view_vector, camera_up_vector);
        }

        glm::mat4 projection;

        float nearplane = -0.1f;
        float farplane = -100.0f;

        float field_of_view = 3.141592 / 3.0f;
        projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);

        glm::mat4 model = Matrix_Identity();

        glUniformMatrix4fv(view_uniform, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform, 1, GL_FALSE, glm::value_ptr(projection));

// DEFINIMOS CONTANTES PARA IDENTIFICAR NOSSOS OBJETOS SEMELHANTE AO LAB 5
#define PLANE 0
#define INIMIGO 1
#define TREE 2
#define GUN 3
#define WALL 4
#define WALL2 5

        DrawFloor(model, PLANE);
        DrawAllWalls(model, WALL, WALL2);

        // FLAGS DE CONTROLE PRA NÃO HAVER INCONTAVEIS TIROS NUM UNICO CLICK DO MOUSE
        if (ATIROU)
        {
            if (SHOT_ANIMATION > 1.0f)
            {
                ATIROU = false;
                SHOT_ANIMATION = 0.0f;
            }
            else
            {
                SHOT_ANIMATION += SPEED / 2;
            }
        }

        // FAZ ANIMACAO DO TIRO COM CURVAS DE BEZIER
        glm::vec4 p1 = glm::vec4(-0.22f, -0.2f, 0.5f, 0.1f);
        glm::vec4 p2 = glm::vec4(-0.22f, -0.2f, 0.3f, 0.1f);
        glm::vec4 p3 = glm::vec4(-0.22f, -0.1f, 0.4f, 0.1f);
        glm::vec4 c = bezier(SHOT_ANIMATION, p1, p2, p3, p1);

        model = Matrix_Identity();
        model *= Matrix_Translate(camera_position_c[0], camera_position_c[1], camera_position_c[2]);
        model *= Matrix_Rotate_Y(g_CameraTheta);
        model *= Matrix_Rotate_X(g_CameraPhi);

        // POSICIONA E DESENHA A ARMA EM RELAÇÃO AO JOGADOR
        DrawGun(model, c, GUN);

        // MOVIMENTA O MONSTRO ALTERANOD A POSICAO DELE DE ACORDO COM A 'TAXA' VELOCIDADE
        for (int i = 0; i < 5; i++)
        {
            if (sqrt(pow(camera_position_c.x - monster_pos[i].x, 2) + pow(camera_position_c.z - monster_pos[i].z, 2)) > 1)
            {
                if (camera_position_c.x > monster_pos[i].x)
                {
                    monster_pos[i].x += SPEED * (MONSTER_SPEED / 50);
                }
                else
                {
                    monster_pos[i].x -= SPEED * (MONSTER_SPEED / 50);
                }
                if (camera_position_c.z > monster_pos[i].z)
                {
                    monster_pos[i].z += SPEED * (MONSTER_SPEED / 50);
                }
                else
                {
                    monster_pos[i].z -= SPEED * (MONSTER_SPEED / 50);
                }
            }
            // DESENHA O MONSTRO
            DrawMonster(model, monster_pos[i], model_uniform, INIMIGO, i);
        }

        // DESENHA AS ARVORES
        DrawTrees(model, model_uniform, TREE);

        // CORRIGE O BUG DA TAXA DE DISPAROS PARA NÃO CANCELAR ANIMACAO
        if (SHOT_RATE > 0.0f)
        {
            SHOT_RATE = SHOT_RATE - DELTA_TIME;
            if (SHOT_RATE < 0.0f)
                SHOT_RATE = 0.0f;
        }
        if (g_LeftMouseButtonPressed && SHOT_RATE == 0.0f && PLAYER_HP > 0)
        {
            PlaySound("../../sounds/gunshot.wav", NULL, SND_ASYNC);    
            for (float t = 0.0f; t < 10.0f; t = t + 0.01f)
            {    
                glm::vec4 hitscan = camera_position_c + t * (camera_view_vector);
                for (int i = 0; i < 5; i++)
                {
                    if (ShotCollision(hitscan, bbox_min_world_monster[i], bbox_max_world_monster[i]))
                    {
                        MONSTER_HP[i] -= 1;
                        break;
                    }
                }
            }
            SHOT_RATE = 0.3;
        }

        // SE O MONSTRO MORREU
        // FAZ APARECER MAIS UM MONSTRO NUM LUGAR ALEATORIO
        // INCRIMENTA A VELOCIDADE DO MONSTRO
        // INCREMENTA O SCORE DO PLAYER
        for (int i = 0; i < 5; i++)
        {
            if (MONSTER_HP[i] <= 0)
            {
                MONSTER_HP[i] = 1;

                float monster_pos_x = rand() % 70 - 35;
                float monster_pos_y = 1.0f;
                float monster_pos_z = rand() % 70 - 35;

                monster_pos[i] = glm::vec4(monster_pos_x, monster_pos_y, monster_pos_z, 1.0f);

                if (MONSTER_SPEED < 30)
                {
                    MONSTER_SPEED += 1;
                }

                PLAYER_SCORE += 1;
            }
        }

        // MOSTRAMOS A TAXA DE QUADROS POR SEGUNDO
        TextRendering_ShowFramesPerSecond(window);

        // MOSTRAMOS NA TELA A VIDA ATUAL E MAXIMA DO PLAYER
        TextRendering_ShowGameStatus(window);

        // SE A VIDA DO PLAYER FOR MENOR OU IGUAL A ZERO, AVISA QUE O JOGO ACABOU
        if (PLAYER_HP <= 0)
        {
            if (!played_sound) PlaySound("../../sounds/oof.wav", NULL, SND_ASYNC); 
            played_sound = true;

            g_UseLookatCamera = true;
            PLAYER_HP = 0;
            if (PLAYER_FINAL_SCORE == 0)
            {
                PLAYER_FINAL_SCORE = PLAYER_SCORE;
            }
            TextRendering_ShowDeathMessage(window);
        }

        // MOSTRAMOS A MIRA DO JOGADOR - O CURSOR
        TextRendering_PrintCursor(window);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}

// ====================================================================================================================================================
// ====================================================================================================================================================
//                                          FUNCOES DO SOR / JA IMPLEMENTADAS NO LAB 5
// ====================================================================================================================================================
// ====================================================================================================================================================

void LoadTextureImage(const char *filename)
{
    printf("Carregando imagem \"%s\"... ", filename);

    stbi_set_flip_vertically_on_load(true);
    int width;
    int height;
    int channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 3);

    if (data == NULL)
    {
        fprintf(stderr, "ERROR: Cannot open image file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }

    printf("OK (%dx%d).\n", width, height);

    GLuint texture_id;
    GLuint sampler_id;
    glGenTextures(1, &texture_id);
    glGenSamplers(1, &sampler_id);

    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    GLuint textureunit = g_NumLoadedTextures;
    glActiveTexture(GL_TEXTURE0 + textureunit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindSampler(textureunit, sampler_id);

    stbi_image_free(data);

    g_NumLoadedTextures += 1;
}

void DrawVirtualObject(const char *object_name)
{
    glBindVertexArray(g_VirtualScene[object_name].vertex_array_object_id);

    glm::vec3 bbox_min = g_VirtualScene[object_name].bbox_min;
    glm::vec3 bbox_max = g_VirtualScene[object_name].bbox_max;
    glUniform4f(bbox_min_uniform, bbox_min.x, bbox_min.y, bbox_min.z, 1.0f);
    glUniform4f(bbox_max_uniform, bbox_max.x, bbox_max.y, bbox_max.z, 1.0f);

    glDrawElements(
        g_VirtualScene[object_name].rendering_mode,
        g_VirtualScene[object_name].num_indices,
        GL_UNSIGNED_INT,
        (void *)(g_VirtualScene[object_name].first_index * sizeof(GLuint)));

    glBindVertexArray(0);
}

void LoadShadersFromFiles()
{
    vertex_shader_id = LoadShader_Vertex("../../src/shader_vertex.glsl");
    fragment_shader_id = LoadShader_Fragment("../../src/shader_fragment.glsl");

    if (program_id != 0)
        glDeleteProgram(program_id);

    program_id = CreateGpuProgram(vertex_shader_id, fragment_shader_id);

    model_uniform = glGetUniformLocation(program_id, "model");
    view_uniform = glGetUniformLocation(program_id, "view");
    projection_uniform = glGetUniformLocation(program_id, "projection");
    object_id_uniform = glGetUniformLocation(program_id, "object_id");
    bbox_min_uniform = glGetUniformLocation(program_id, "bbox_min");
    bbox_max_uniform = glGetUniformLocation(program_id, "bbox_max");

    glUseProgram(program_id);
    glUniform1i(glGetUniformLocation(program_id, "TextureImage1"), 0);
    glUniform1i(glGetUniformLocation(program_id, "TextureImage2"), 1);
    glUseProgram(0);
}

void PushMatrix(glm::mat4 M)
{
    g_MatrixStack.push(M);
}

void PopMatrix(glm::mat4 &M)
{
    if (g_MatrixStack.empty())
    {
        M = Matrix_Identity();
    }
    else
    {
        M = g_MatrixStack.top();
        g_MatrixStack.pop();
    }
}

void ComputeNormals(ObjModel *model)
{
    if (!model->attrib.normals.empty())
        return;

    size_t num_vertices = model->attrib.vertices.size() / 3;

    std::vector<int> num_triangles_per_vertex(num_vertices, 0);
    std::vector<glm::vec4> vertex_normals(num_vertices, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            glm::vec4 vertices[3];
            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3 * triangle + vertex];
                const float vx = model->attrib.vertices[3 * idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3 * idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3 * idx.vertex_index + 2];
                vertices[vertex] = glm::vec4(vx, vy, vz, 1.0);
            }

            const glm::vec4 a = vertices[0];
            const glm::vec4 b = vertices[1];
            const glm::vec4 c = vertices[2];

            const glm::vec4 n = crossproduct(b - a, c - a);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3 * triangle + vertex];
                num_triangles_per_vertex[idx.vertex_index] += 1;
                vertex_normals[idx.vertex_index] += n;
                model->shapes[shape].mesh.indices[3 * triangle + vertex].normal_index = idx.vertex_index;
            }
        }
    }

    model->attrib.normals.resize(3 * num_vertices);

    for (size_t i = 0; i < vertex_normals.size(); ++i)
    {
        glm::vec4 n = vertex_normals[i] / (float)num_triangles_per_vertex[i];
        n /= norm(n);
        model->attrib.normals[3 * i + 0] = n.x;
        model->attrib.normals[3 * i + 1] = n.y;
        model->attrib.normals[3 * i + 2] = n.z;
    }
}

void BuildTrianglesAndAddToVirtualScene(ObjModel *model)
{
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);

    std::vector<GLuint> indices;
    std::vector<float> model_coefficients;
    std::vector<float> normal_coefficients;
    std::vector<float> texture_coefficients;

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t first_index = indices.size();
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        const float minval = std::numeric_limits<float>::min();
        const float maxval = std::numeric_limits<float>::max();

        glm::vec3 bbox_min = glm::vec3(maxval, maxval, maxval);
        glm::vec3 bbox_max = glm::vec3(minval, minval, minval);

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3 * triangle + vertex];

                indices.push_back(first_index + 3 * triangle + vertex);

                const float vx = model->attrib.vertices[3 * idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3 * idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3 * idx.vertex_index + 2];

                model_coefficients.push_back(vx);
                model_coefficients.push_back(vy);
                model_coefficients.push_back(vz);
                model_coefficients.push_back(1.0f);

                bbox_min.x = std::min(bbox_min.x, vx);
                bbox_min.y = std::min(bbox_min.y, vy);
                bbox_min.z = std::min(bbox_min.z, vz);
                bbox_max.x = std::max(bbox_max.x, vx);
                bbox_max.y = std::max(bbox_max.y, vy);
                bbox_max.z = std::max(bbox_max.z, vz);

                if (idx.normal_index != -1)
                {
                    const float nx = model->attrib.normals[3 * idx.normal_index + 0];
                    const float ny = model->attrib.normals[3 * idx.normal_index + 1];
                    const float nz = model->attrib.normals[3 * idx.normal_index + 2];
                    normal_coefficients.push_back(nx);
                    normal_coefficients.push_back(ny);
                    normal_coefficients.push_back(nz);
                    normal_coefficients.push_back(0.0f);
                }

                if (idx.texcoord_index != -1)
                {
                    const float u = model->attrib.texcoords[2 * idx.texcoord_index + 0];
                    const float v = model->attrib.texcoords[2 * idx.texcoord_index + 1];
                    texture_coefficients.push_back(u);
                    texture_coefficients.push_back(v);
                }
            }
        }

        size_t last_index = indices.size() - 1;

        SceneObject theobject;
        theobject.name = model->shapes[shape].name;
        theobject.first_index = first_index;
        theobject.num_indices = last_index - first_index + 1;
        theobject.rendering_mode = GL_TRIANGLES;
        theobject.vertex_array_object_id = vertex_array_object_id;

        theobject.bbox_min = bbox_min;
        theobject.bbox_max = bbox_max;

        g_VirtualScene[model->shapes[shape].name] = theobject;
    }

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model_coefficients.size() * sizeof(float), model_coefficients.data());
    GLuint location = 0;
    GLint number_of_dimensions = 4;
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (!normal_coefficients.empty())
    {
        GLuint VBO_normal_coefficients_id;
        glGenBuffers(1, &VBO_normal_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, normal_coefficients.size() * sizeof(float), normal_coefficients.data());
        location = 1;
        number_of_dimensions = 4;
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if (!texture_coefficients.empty())
    {
        GLuint VBO_texture_coefficients_id;
        glGenBuffers(1, &VBO_texture_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_texture_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, texture_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, texture_coefficients.size() * sizeof(float), texture_coefficients.data());
        location = 2;
        number_of_dimensions = 2;
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), indices.data());

    glBindVertexArray(0);
}

GLuint LoadShader_Vertex(const char *filename)
{
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);

    LoadShader(filename, vertex_shader_id);

    return vertex_shader_id;
}

GLuint LoadShader_Fragment(const char *filename)
{
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    LoadShader(filename, fragment_shader_id);

    return fragment_shader_id;
}

void LoadShader(const char *filename, GLuint shader_id)
{
    std::ifstream file;
    try
    {
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
    }
    catch (std::exception &e)
    {
        fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream shader;
    shader << file.rdbuf();
    std::string str = shader.str();
    const GLchar *shader_string = str.c_str();
    const GLint shader_string_length = static_cast<GLint>(str.length());

    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);

    glCompileShader(shader_id);

    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);

    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

    GLchar *log = new GLchar[log_length];
    glGetShaderInfoLog(shader_id, log_length, &log_length, log);

    if (log_length != 0)
    {
        std::string output;

        if (!compiled_ok)
        {
            output += "ERROR: OpenGL compilation of \"";
            output += filename;
            output += "\" failed.\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }
        else
        {
            output += "WARNING: OpenGL compilation of \"";
            output += filename;
            output += "\".\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }

        fprintf(stderr, "%s", output.c_str());
    }

    delete[] log;
}

GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
{
    GLuint program_id = glCreateProgram();

    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);

    glLinkProgram(program_id);

    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);

    if (linked_ok == GL_FALSE)
    {
        GLint log_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

        GLchar *log = new GLchar[log_length];

        glGetProgramInfoLog(program_id, log_length, &log_length, log);

        std::string output;

        output += "ERROR: OpenGL linking of program failed.\n";
        output += "== Start of link log\n";
        output += log;
        output += "\n== End of link log\n";

        delete[] log;

        fprintf(stderr, "%s", output.c_str());
    }

    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    return program_id;
}

void FramebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    g_ScreenRatio = (float)width / height;
}

double g_LastCursorPosX, g_LastCursorPosY;

void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_LeftMouseButtonPressed = true;
        ATIROU = true;
        SHOT_ANIMATION = 0.0f;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        g_LeftMouseButtonPressed = false;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_RightMouseButtonPressed = true;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        g_RightMouseButtonPressed = false;
    }

    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    {
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_MiddleMouseButtonPressed = true;
    }

    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
    {
        g_MiddleMouseButtonPressed = false;
    }
}

void CursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{

    float dx = xpos - g_LastCursorPosX;
    float dy = ypos - g_LastCursorPosY;

    g_CameraTheta -= 0.01f * dx;
    g_CameraPhi += 0.01f * dy;

    float phimax = 3.141592f / 2;
    float phimin = -phimax;

    if (g_CameraPhi > phimax)
    {
        g_CameraPhi = phimax;
    }

    if (g_CameraPhi < phimin)
    {
        g_CameraPhi = phimin;
    }

    g_LastCursorPosX = xpos;
    g_LastCursorPosY = ypos;

    if (g_RightMouseButtonPressed)
    {
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;

        g_ForearmAngleZ -= 0.01f * dx;
        g_ForearmAngleX += 0.01f * dy;

        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }

    if (g_MiddleMouseButtonPressed)
    {
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;

        g_TorsoPositionX += 0.01f * dx;
        g_TorsoPositionY -= 0.01f * dy;

        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }
}

void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    g_CameraDistance -= 0.1f * yoffset;

    const float verysmallnumber = std::numeric_limits<float>::epsilon();
    if (g_CameraDistance < verysmallnumber)
    {
        g_CameraDistance = verysmallnumber;
    }
}

void ErrorCallback(int error, const char *description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}
