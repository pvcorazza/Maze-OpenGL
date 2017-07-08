#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"                 // para as texturas
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cmath>
#include <string.h>

const unsigned int WIDTH = 1920;
const unsigned int HEIGHT = 1080;

void key_callback_camera_movement(GLFWwindow *window, struct BBox positions[]);
void mouse_callback(GLFWwindow *window, double xposition, double yposition);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void key_callback_close_window(GLFWwindow *window);
bool collision_callback(struct BBox positions[], glm::vec3 cameraFuturePosition);
void showResult_callback(int result);

struct BBox{

    float leftPosition_X;
    float leftPosition_Z;
    float rightPosition_X;
    float rightPosition_Z;

};

glm::vec3 cameraPosition = glm::vec3(0.0f,0.25f,-0.5f);
glm::vec3 cameraFront = glm::vec3(0.0f,0.0f,1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f,1.0f,0.0f);
float deltaTime = 0.0f;
float timeLastFrame = 0.0f;
float timeCurrentFrame = 0.0f;

float timeToComplete = 60.0;

// posições default do cursor do mouse são a metade de cada tamanho da tela
float lastXposition = WIDTH/2.0;
float lastYposition = HEIGHT/2.0;
float pitch = 0.0f;
float yaw = -90.0f;
bool firstMouse = true;
float fov = 45.0f;       // fov = field of view

glm::vec3 lightSource(10.0f, 10.0f, -10.0f);

//dint id_object;    // para a uniform do fragment shader

const char *vertexShaderSource = "#version 330 core \n"
"layout (location = 0) in vec3 position; \n"
//"layout (location = 1) in vec3 inColor; \n"
"layout (location = 1) in vec2 inTex; \n"
"layout (location = 2) in vec3 normalVec; \n"
//"out vec3 finalColor; \n"
"out vec3 normal; \n"
"out vec3 FragmentPosition; \n"
"out vec2 finalTex; \n"
"\n"
"uniform mat4 modelShader; \n"
"uniform mat4 viewShader; \n"
"uniform mat4 projectionShader; \n"
"void main(){ \n"
"   gl_Position = projectionShader * viewShader * modelShader * vec4(position.x, position.y, position.z, 1.0); \n"
"   FragmentPosition = vec3(modelShader * vec4(position.x,position.y,position.z,1.0)); \n"
"   normal = normalVec; \n"
"   finalTex = inTex; \n"
//"   finalColor = inColor; \n"
"}\0";

const char *fragmentShaderSource = "#version 330 core\n"
"in vec2 finalTex;   \n"
"in vec3 normal;     \n"
"in vec3 FragmentPosition; \n"
//"in vec3 finalColor; \n"
"out vec4 FragColor; \n"
"uniform sampler2D TEX; \n"
"uniform float executionTime; \n"
"uniform int isSky; \n"
"uniform vec3 lightPosition; \n"
"uniform vec3 cameraPos; \n"
"uniform vec3 cameraDir; \n"
"uniform int id_object; \n"
"void main(){ \n"
"\n" // para iluminação difusa:
"    if(isSky == 0) { \n"
"       vec3 normalizedNormal = normalize(normal); \n"                                                 // normalizamos o vetor normal
"       vec3 lightVector = normalize(cameraPos - FragmentPosition); \n"             // fazemos a subtração da posição da fonte de luz pela do fragmento para ter o vetor de luz
"       float cosAngle = max(dot(normalizedNormal,lightVector), 0.0); \n"               // não precisa dividir pelas normais pois já estão normalizados
"\n"    // para iluminação especular:
/*
"       vec3 viewerDirection = normalize(cameraPos - FragmentPosition); \n"             // vetor do ponto onde bate o raio e o visualizador
"       vec3 rVector = reflect(-lightVector, normalizedNormal); \n"                     // vetor de reflexão
//"       float alfaAngle = max(dot(viewerDirection, rVector),0.0); \n"                       // pegamos o ângulo entre os 2 vetores definidos nas 2 linhas anteriores
//"       vec3 specular = 0.5 * alfaAngle * vec3(0.3f,0.3f,0.3f); \n"
//"\n"    // para a luz spotlight
*/
"       float attenuation = 1.0; \n"
"       vec3 coneDirection = normalize(cameraDir); \n"
"       float angleSpotlight = degrees(acos(dot(-lightVector, coneDirection))); \n"    // ângulo entre a direção que vai do centro do cone e a direção que vai do fragmento em questão para o centro do cone
"       if(angleSpotlight > (10.0f - (executionTime*0,166))) \n"  // onde 10.0 é o ângulo do cone
"           attenuation = (1000.0 - (executionTime*16.666))/(angleSpotlight * angleSpotlight * angleSpotlight);"   // PARA BOTAR TEMPO, É SÓ FAZER ESSE VALOR DE 1000 BAIXAR E TAMBÉM O 10.0 DO IF PRECISA SER EM FUNÇÃO DELE
"\n"    // ambiente para cada objeto
"       if(id_object == 0) { \n"                          // se for muro
"           FragColor = vec4(0.1f,0.1f,0.1f,1.0f); \n"     // cor ambiente para o objeto
"       } \n"
"\n"
"       else if(id_object == 1) { \n"                    // se for chão
"           FragColor = vec4(0.1f,0.1f,0.1f,1.0f); \n"    // cor ambiente para o objeto
"       } \n"
"\n"
"       FragColor += attenuation * vec4(0.3f,0.3f,0.3f,1.0f) * texture(TEX, finalTex) *  cosAngle; \n"   // * vec4(specular,1.0f)
//       cor final =   SPOTLIGHT!   espectro da luz (noite!)        cor do objeto     ângulo (produto interno de cima)       E ainda tem mais o ambiente que é setado para cada id_object acima e somado com isto
"   } \n"
"   if(isSky == 1) { \n"
"     FragColor = texture(TEX, finalTex); \n"
"   }"
"\n"
"}\0";

int main() {

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Game", nullptr, nullptr);
    if(window == NULL) {
        std::cout << "Failed to create window (GLFW).";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    ///calbacks signature here...
    //key_callback_camera_movement(window);
    key_callback_close_window(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    //pedimos a captura do mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    ///load shaders and create program here...
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // agora que já fizemos o link dos shaders com o programa, não precisamos mais

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    ///load shaders

    glEnable(GL_DEPTH_TEST);   // habilitamos Z-buffer (teste de profundidade)

    float vertices[] = {

    //faces do "cubo"

    //  x       y       z       u     v     normal(x,y,z) pré-computadas (pois sabemos o formato geométrico do modelo de antemão)
       -0.5f, -0.5f, -0.075f,  0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.5f, -0.5f, -0.075f,  1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.5f,  0.5f, -0.075f,  1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
        0.5f,  0.5f, -0.075f,  1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
       -0.5f,  0.5f, -0.075f,  0.0f, 1.0f, 0.0f, 0.0f, -1.0f,
       -0.5f, -0.5f, -0.075f,  0.0f, 0.0f, 0.0f, 0.0f, -1.0f,

       -0.5f, -0.5f,  0.075f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f,  0.075f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f,  0.5f,  0.075f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.5f,  0.5f,  0.075f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
       -0.5f,  0.5f,  0.075f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
       -0.5f, -0.5f,  0.075f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

       -0.5f,  0.5f,  0.075f,  1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
       -0.5f,  0.5f, -0.075f,  1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
       -0.5f, -0.5f, -0.075f,  0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
       -0.5f, -0.5f, -0.075f,  0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
       -0.5f, -0.5f,  0.075f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
       -0.5f,  0.5f,  0.075f,  1.0f, 0.0f, -1.0f, 0.0f, 0.0f,

        0.5f,  0.5f,  0.075f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f,  0.5f, -0.075f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.075f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.075f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f,  0.075f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f,  0.5f,  0.075f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

       -0.5f, -0.5f, -0.075f,  0.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, -0.075f,  1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f,  0.075f,  1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f,  0.075f,  1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
       -0.5f, -0.5f,  0.075f,  0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
       -0.5f, -0.5f, -0.075f,  0.0f, 1.0f, 0.0f, -1.0f, 0.0f,

       -0.5f,  0.5f, -0.075f,  0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.5f,  0.5f, -0.075f,  1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.5f,  0.5f,  0.075f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.5f,  0.5f,  0.075f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
       -0.5f,  0.5f,  0.075f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
       -0.5f,  0.5f, -0.075f,  0.0f, 1.0f, 0.0f, 1.0f, 0.0f
};

float vertices2[] = {

       -0.075f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.075f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.075f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
        0.075f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
       -0.075f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f, 0.0f, -1.0f,
       -0.075f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, -1.0f,

       -0.075f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.075f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.075f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.075f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
       -0.075f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
       -0.075f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

       -0.075f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
       -0.075f,  0.5f, -0.5f,  1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
       -0.075f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
       -0.075f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
       -0.075f, -0.5f,  0.5f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
       -0.075f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f, 0.0f, 0.0f,

        0.075f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.075f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.075f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.075f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.075f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.075f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

       -0.075f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        0.075f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        0.075f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        0.075f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
       -0.075f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
       -0.075f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f, 0.0f,

       -0.075f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.075f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.075f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.075f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
       -0.075f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
       -0.075f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f, 1.0f, 0.0f
};

float vertices3[] = {

       -0.075f, -0.5f, -0.25f,  0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.075f, -0.5f, -0.25f,  1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.075f,  0.5f, -0.25f,  1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
        0.075f,  0.5f, -0.25f,  1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
       -0.075f,  0.5f, -0.25f,  0.0f, 1.0f, 0.0f, 0.0f, -1.0f,
       -0.075f, -0.5f, -0.25f,  0.0f, 0.0f, 0.0f, 0.0f, -1.0f,

       -0.075f, -0.5f,  0.25f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.075f, -0.5f,  0.25f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.075f,  0.5f,  0.25f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.075f,  0.5f,  0.25f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
       -0.075f,  0.5f,  0.25f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
       -0.075f, -0.5f,  0.25f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

       -0.075f,  0.5f,  0.25f,  1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
       -0.075f,  0.5f, -0.25f,  1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
       -0.075f, -0.5f, -0.25f,  0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
       -0.075f, -0.5f, -0.25f,  0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
       -0.075f, -0.5f,  0.25f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
       -0.075f,  0.5f,  0.25f,  1.0f, 0.0f, -1.0f, 0.0f, 0.0f,

        0.075f,  0.5f,  0.25f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.075f,  0.5f, -0.25f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.075f, -0.5f, -0.25f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.075f, -0.5f, -0.25f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.075f, -0.5f,  0.25f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.075f,  0.5f,  0.25f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

       -0.075f, -0.5f, -0.25f,  0.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        0.075f, -0.5f, -0.25f,  1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        0.075f, -0.5f,  0.25f,  1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        0.075f, -0.5f,  0.25f,  1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
       -0.075f, -0.5f,  0.25f,  0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
       -0.075f, -0.5f, -0.25f,  0.0f, 1.0f, 0.0f, -1.0f, 0.0f,

       -0.075f,  0.5f, -0.25f,  0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.075f,  0.5f, -0.25f,  1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.075f,  0.5f,  0.25f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.075f,  0.5f,  0.25f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
       -0.075f,  0.5f,  0.25f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
       -0.075f,  0.5f, -0.25f,  0.0f, 1.0f, 0.0f, 1.0f, 0.0f
};

 float vertices4[] = {     // chão

    //traseira
       -40.5f, -0.01f, -40.5f,  0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        40.5f, -0.01f, -40.5f,  100.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        40.5f,  0.01f, -40.5f,  100.0f, 100.0f, 0.0f, 0.0f, -1.0f,
        40.5f,  0.01f, -40.5f,  100.0f, 100.0f, 0.0f, 0.0f, -1.0f,
       -40.5f,  0.01f, -40.5f,  0.0f, 100.0f, 0.0f, 0.0f, -1.0f,
       -40.5f, -0.01f, -40.5f,  0.0f, 0.0f, 0.0f, 0.0f, -1.0f,

       -40.5f, -0.01f,  40.5f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        40.5f, -0.01f,  40.5f,  100.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        40.5f,  0.01f,  40.5f,  100.0f, 100.0f, 0.0f, 0.0f, 1.0f,
        40.5f,  0.01f,  40.5f,  100.0f, 100.0f, 0.0f, 0.0f, 1.0f,
       -40.5f,  0.01f,  40.5f,  0.0f, 100.0f, 0.0f, 0.0f, 1.0f,
       -40.5f, -0.01f,  40.5f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

       -40.5f,  0.01f,  40.5f,  100.0f, 0.0f, -1.0f, 0.0f, 0.0f,
       -40.5f,  0.01f, -40.5f,  100.0f, 100.0f, -1.0f, 0.0f, 0.0f,
       -40.5f, -0.01f, -40.5f,  0.0f, 100.0f, -1.0f, 0.0f, 0.0f,
       -40.5f, -0.01f, -40.5f,  0.0f, 100.0f, -1.0f, 0.0f, 0.0f,
       -40.5f, -0.01f,  40.5f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
       -40.5f,  0.0f,  40.5f,  100.0f, 0.0f, -1.0f, 0.0f, 0.0f,

        40.5f,  0.01f,  40.5f,  100.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        40.5f,  0.01f, -40.5f,  100.0f, 100.0f, 1.0f, 0.0f, 0.0f,
        40.5f, -0.01f, -40.5f,  0.0f, 100.0f, 1.0f, 0.0f, 0.0f,
        40.5f, -0.01f, -40.5f,  0.0f, 100.0f, 1.0f, 0.0f, 0.0f,
        40.5f, -0.01f,  40.5f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        40.5f,  0.01f,  40.5f,  100.0f, 0.0f, 1.0f, 0.0f, 0.0f,

       -40.5f, -0.01f, -40.5f,  0.0f, 100.0f, 0.0f, -1.0f, 0.0f,
        40.5f, -0.01f, -40.5f,  100.0f, 100.0f, 0.0f, -1.0f, 0.0f,
        40.5f, -0.01f,  40.5f,  100.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        40.5f, -0.01f,  40.5f,  100.0f, 0.0f, 0.0f, -1.0f, 0.0f,
       -40.5f, -0.01f,  40.5f,  0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
       -40.5f, -0.01f, -40.5f,  0.0f, 100.0f, 0.0f, -1.0f, 0.0f,

       -40.5f,  0.01f, -40.5f,  0.0f, 100.0f, 0.0f, 1.0f, 0.0f,
        40.5f,  0.01f, -40.5f,  100.0f, 100.0f, 0.0f, 1.0f, 0.0f,
        40.5f,  0.01f,  40.5f,  100.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        40.5f,  0.01f,  40.5f,  100.0f, 0.0f, 0.0f, 1.0f, 0.0f,
       -40.5f,  0.01f,  40.5f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
       -40.5f,  0.01f, -40.5f,  0.0f, 100.0f, 0.0f, 1.0f, 0.0f
};

float vertices5[] = {                 // sky

       -300.5f, -0.01f, -300.5f,  0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        300.5f, -0.01f, -300.5f,  400.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        300.5f,  0.01f, -300.5f,  400.0f, 400.0f, 0.0f, 0.0f, -1.0f,
        300.5f,  0.01f, -300.5f,  400.0f, 400.0f, 0.0f, 0.0f, -1.0f,
       -300.5f,  0.01f, -300.5f,  0.0f, 400.0f, 0.0f, 0.0f, -1.0f,
       -300.5f, -0.01f, -300.5f,  0.0f, 0.0f, 0.0f, 0.0f, -1.0f,

       -300.5f, -0.01f,  300.5f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        300.5f, -0.01f,  300.5f,  400.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        300.5f,  0.01f,  300.5f,  400.0f, 400.0f, 0.0f, 0.0f, 1.0f,
        300.5f,  0.01f,  300.5f,  400.0f, 400.0f, 0.0f, 0.0f, 1.0f,
       -300.5f,  0.01f,  300.5f,  0.0f, 400.0f, 0.0f, 0.0f, 1.0f,
       -300.5f, -0.01f,  300.5f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

       -300.5f,  0.01f,  300.5f,  400.0f, 0.0f, -1.0f, 0.0f, 0.0f,
       -300.5f,  0.01f, -300.5f,  400.0f, 400.0f, -1.0f, 0.0f, 0.0f,
       -300.5f, -0.01f, -300.5f,  0.0f, 400.0f, -1.0f, 0.0f, 0.0f,
       -300.5f, -0.01f, -300.5f,  0.0f, 400.0f, -1.0f, 0.0f, 0.0f,
       -300.5f, -0.01f,  300.5f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
       -300.5f,  0.01f,  300.5f,  400.0f, 0.0f, -1.0f, 0.0f, 0.0f,

        300.5f,  0.01f,  300.5f,  400.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        300.5f,  0.01f, -300.5f,  400.0f, 400.0f, 1.0f, 0.0f, 0.0f,
        300.5f, -0.01f, -300.5f,  0.0f, 400.0f, 1.0f, 0.0f, 0.0f,
        300.5f, -0.01f, -300.5f,  0.0f, 400.0f, 1.0f, 0.0f, 0.0f,
        300.5f, -0.01f,  300.5f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        300.5f,  0.01f,  300.5f,  400.0f, 0.0f, 1.0f, 0.0f, 0.0f,

       -300.5f, -0.01f, -300.5f,  0.0f, 400.0f, 0.0f, -1.0f, 0.0f,
        300.5f, -0.01f, -300.5f,  400.0f, 400.0f, 0.0f, -1.0f, 0.0f,
        300.5f, -0.01f,  300.5f,  400.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        300.5f, -0.01f,  300.5f,  400.0f, 0.0f, 0.0f, -1.0f, 0.0f,
       -300.5f, -0.01f,  300.5f,  0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
       -300.5f, -0.01f, -300.5f,  0.0f, 400.0f, 0.0f, -1.0f, 0.0f,

       -300.5f,  0.01f, -300.5f,  0.0f, 400.0f, 0.0f, 1.0f, 0.0f,
        300.5f,  0.01f, -300.5f,  400.0f, 400.0f, 0.0f, 1.0f, 0.0f,
        300.5f,  0.01f,  300.5f,  400.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        300.5f,  0.01f,  300.5f,  400.0f, 0.0f, 0.0f, 1.0f, 0.0f,
       -300.5f,  0.01f,  300.5f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
       -300.5f,  0.01f, -300.5f,  0.0f, 400.0f, 0.0f, 1.0f, 0.0f
};

    /*
    unsigned int indices[] = {
        0, 1, 2,        // first triangle
        0, 2, 3         // second triangle
    };

    */

    unsigned int VAO, VBO;// EBO;
    glGenVertexArrays(1,&VAO);    //cria um VAO
    glGenBuffers(1,&VBO);         //cria um VBO
    //glGenBuffers(1,&EBO);         //cria um EBO (para armazenar os indices)

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);           //no VBO, colocamos os vértices
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);     //no EBO, colocamos os indices

    //positions
    glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE,8*sizeof(float),(void*)0);            //como o VAO interpretará o VBO atualmente "bound" em location 0
    glEnableVertexAttribArray(0);

    //colors
    //glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));        //como o VAO interpretará o VBO atualmente "bound" em location 1 (primeiro atributo passado como parâmetro)
    //glEnableVertexAttribArray(1);

    //texture
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(5*sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    unsigned int VAO2, VBO2;// EBO2;
    glGenVertexArrays(1,&VAO2);    //cria um VAO
    glGenBuffers(1,&VBO2);         //cria um VBO
    //glGenBuffers(1,&EBO2);         //cria um EBO (para armazenar os indices)

    glBindVertexArray(VAO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);           //no VBO, colocamos os vértices
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);     //no EBO, colocamos os indices

    //positions
    glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE,8*sizeof(float),(void*)0);            //como o VAO interpretará o VBO atualmente "bound" em location 0
    glEnableVertexAttribArray(0);

    //colors
    //glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));        //como o VAO interpretará o VBO atualmente "bound" em location 1 (primeiro atributo passado como parâmetro)
    //glEnableVertexAttribArray(1);

    //texture
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(5*sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    unsigned int VAO3, VBO3;// EBO3;
    glGenVertexArrays(1,&VAO3);    //cria um VAO
    glGenBuffers(1,&VBO3);         //cria um VBO
    //glGenBuffers(1,&EBO3);         //cria um EBO (para armazenar os indices)

    glBindVertexArray(VAO3);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO3);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices3), vertices3, GL_STATIC_DRAW);           //no VBO, colocamos os vértices
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);     //no EBO, colocamos os indices

    //positions
    glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE,8*sizeof(float),(void*)0);            //como o VAO interpretará o VBO atualmente "bound" em location 0
    glEnableVertexAttribArray(0);

    //colors
    //glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));        //como o VAO interpretará o VBO atualmente "bound" em location 1 (primeiro atributo passado como parâmetro)
    //glEnableVertexAttribArray(1);

    //texture
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(5*sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    unsigned int VAO4, VBO4;// EBO4;
    glGenVertexArrays(1,&VAO4);    //cria um VAO
    glGenBuffers(1,&VBO4);         //cria um VBO
    //glGenBuffers(1,&EBO4);         //cria um EBO (para armazenar os indices)

    glBindVertexArray(VAO4);
    glBindBuffer(GL_ARRAY_BUFFER, VBO4);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO4);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices4), vertices4, GL_STATIC_DRAW);           //no VBO, colocamos os vértices
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);     //no EBO, colocamos os indices

    //positions
    glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE,8*sizeof(float),(void*)0);            //como o VAO interpretará o VBO atualmente "bound" em location 0
    glEnableVertexAttribArray(0);

    //colors
    //glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));        //como o VAO interpretará o VBO atualmente "bound" em location 1 (primeiro atributo passado como parâmetro)
    //glEnableVertexAttribArray(1);

    //texture
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(5*sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    unsigned int VAO5, VBO5;// EBO;
    glGenVertexArrays(1,&VAO5);    //cria um VAO
    glGenBuffers(1,&VBO5);         //cria um VBO
    //glGenBuffers(1,&EBO);         //cria um EBO (para armazenar os indices)

    glBindVertexArray(VAO5);
    glBindBuffer(GL_ARRAY_BUFFER, VBO5);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices5), vertices5, GL_STATIC_DRAW);           //no VBO, colocamos os vértices
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);     //no EBO, colocamos os indices

    //positions
    glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE,8*sizeof(float),(void*)0);            //como o VAO interpretará o VBO atualmente "bound" em location 0
    glEnableVertexAttribArray(0);

    //colors
    //glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));        //como o VAO interpretará o VBO atualmente "bound" em location 1 (primeiro atributo passado como parâmetro)
    //glEnableVertexAttribArray(1);

    //texture
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(5*sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    ///textures

    unsigned int Texture;
    glGenTextures(1,&Texture);
    glBindTexture(GL_TEXTURE_2D, Texture);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);       //texture wrapping

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);   //texture filtering

    int width, height, nrChannels;
    const std::string filename("..//..//brickWall.jpg");   //C://Users//mjwille//Downloads//Maze-OpenGL 3.0//brickWall.jpg
    unsigned char *data = stbi_load(filename.c_str(),&width, &height, &nrChannels, 0);

    if(!data)
        std::cout << "Failed to load texture!";

    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    // definição da texture do chão   TextureGround

    unsigned int TextureGround;
    glGenTextures(1,&TextureGround);
    glBindTexture(GL_TEXTURE_2D, TextureGround);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);       //texture wrapping

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);   //texture filtering

    int width2, height2, nrChannels2;
    const std::string filename2("..//..//greenGrass.jpg");
    unsigned char *data2 = stbi_load(filename2.c_str(),&width2, &height2, &nrChannels2, 0);

    if(!data2)
        std::cout << "Failed to load texture!";

    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width2,height2,0,GL_RGB,GL_UNSIGNED_BYTE,data2);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data2);

    unsigned int TextureDarkSky;
    glGenTextures(1,&TextureDarkSky);
    glBindTexture(GL_TEXTURE_2D, TextureDarkSky);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);       //texture wrapping

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);   //texture filtering

    int width3, height3, nrChannels3;
    const std::string filename3("..//..//nightSky.jpg");
    unsigned char *data3 = stbi_load(filename3.c_str(),&width3, &height3, &nrChannels3, 0);

    if(!data3)
        std::cout << "Failed to load texture!";

    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width3,height3,0,GL_RGB,GL_UNSIGNED_BYTE,data3);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data3);

    ///textures end

    unsigned int modelLocation = glGetUniformLocation(shaderProgram, "modelShader");
    unsigned int viewLocation = glGetUniformLocation(shaderProgram, "viewShader");
    unsigned int projectionLocation = glGetUniformLocation(shaderProgram, "projectionShader");
    unsigned int id_object_location = glGetUniformLocation(shaderProgram, "id_object");
    unsigned int light_source_location = glGetUniformLocation(shaderProgram, "lightPosition");
    unsigned int camera_position_location = glGetUniformLocation(shaderProgram, "cameraPos");
    unsigned int camera_direction_location = glGetUniformLocation(shaderProgram, "cameraDir");
    unsigned int execution_time_location = glGetUniformLocation(shaderProgram, "executionTime");
    unsigned int isSky_location = glGetUniformLocation(shaderProgram, "isSky");

    //para a matriz model, definimos vários vetores de translação. O número de translações que denenharemos a partir de índices será o número de cubos na tela
    const int SIZE_CUBE_MODELS = 59;
    glm::vec3 differentPositions[] = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 0.0f, 0.0f),
        glm::vec3(3.0f, 0.0f, 0.0f),
        glm::vec3(4.0f, 0.0f, 0.0f),
        glm::vec3(5.0f, 0.0f, 0.0f),
        glm::vec3(6.0f, 0.0f, 0.0f),
        glm::vec3(7.0f, 0.0f, 0.0f),  // primeira parede externa
        glm::vec3(0.0f, 0.0f, -8.0f),
        glm::vec3(1.0f, 0.0f, -8.0f),
        glm::vec3(2.0f, 0.0f, -8.0f),
        glm::vec3(3.0f, 0.0f, -8.0f),
        glm::vec3(4.0f, 0.0f, -8.0f),
        glm::vec3(5.0f, 0.0f, -8.0f),
        glm::vec3(6.0f, 0.0f, -8.0f), // segunda parede externa
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(1.0f, 0.0f, -1.0f),
        glm::vec3(2.0f, 0.0f, -1.0f),
        glm::vec3(3.0f, 0.0f, -1.0f),
        glm::vec3(4.0f, 0.0f, -1.0f),
        glm::vec3(5.0f, 0.0f, -1.0f),
        glm::vec3(6.0f, 0.0f, -1.0f),  // primeira parede interna
        glm::vec3(0.0f, 0.0f, -2.0f),
        glm::vec3(1.0f, 0.0f, -2.0f),
        glm::vec3(2.0f, 0.0f, -2.0f),
        glm::vec3(4.0f, 0.0f, -2.0f),
        glm::vec3(6.0f, 0.0f, -2.0f),
        glm::vec3(7.0f, 0.0f, -2.0f),  // segunda parede interna
        glm::vec3(1.0f, 0.0f, -3.0f),
        glm::vec3(2.0f, 0.0f, -3.0f),
        glm::vec3(3.0f, 0.0f, -3.0f),
        glm::vec3(4.0f, 0.0f, -3.0f),
        glm::vec3(5.0f, 0.0f, -3.0f),
        glm::vec3(6.0f, 0.0f, -3.0f),  // terceira parede interna
        glm::vec3(1.0f, 0.0f, -4.0f),
        glm::vec3(2.0f, 0.0f, -4.0f),
        glm::vec3(3.0f, 0.0f, -4.0f),
        glm::vec3(5.0f, 0.0f, -4.0f),
        glm::vec3(6.0f, 0.0f, -4.0f),
        glm::vec3(7.0f, 0.0f, -4.0f),   // quarta parede interna
        glm::vec3(0.0f, 0.0f, -5.0f),
        glm::vec3(1.0f, 0.0f, -5.0f),
        glm::vec3(3.0f, 0.0f, -5.0f),
        glm::vec3(4.0f, 0.0f, -5.0f),
        glm::vec3(5.0f, 0.0f, -5.0f),
        glm::vec3(6.0f, 0.0f, -5.0f),   // quinta parede interna
        glm::vec3(1.0f, 0.0f, -6.0f),
        glm::vec3(2.0f, 0.0f, -6.0f),
        glm::vec3(3.0f, 0.0f, -6.0f),
        glm::vec3(4.0f, 0.0f, -6.0f),
        glm::vec3(5.0f, 0.0f, -6.0f),
        glm::vec3(6.0f, 0.0f, -6.0f),
        glm::vec3(7.0f, 0.0f, -6.0f),   // sexta parede interna
        glm::vec3(0.0f, 0.0f, -7.0f),
        glm::vec3(1.0f, 0.0f, -7.0f),
        glm::vec3(2.0f, 0.0f, -7.0f),
        glm::vec3(3.0f, 0.0f, -7.0f),
        glm::vec3(5.0f, 0.0f, -7.0f),
        glm::vec3(6.0f, 0.0f, -7.0f),  // sétima parede interna  até aqui 59 vértices

        glm::vec3(-0.5f, 0.0f, -0.0f),  // aqui é o vértice 59
        glm::vec3(-0.5f, 0.0f, -1.0f),
        glm::vec3(-0.5f, 0.0f, -2.0f),
        glm::vec3(-0.5f, 0.0f, -3.0f),
        glm::vec3(-0.5f, 0.0f, -4.0f),
        glm::vec3(-0.5f, 0.0f, -5.0f),
        glm::vec3(-0.5f, 0.0f, -6.0f),
        glm::vec3(-0.5f, 0.0f, -7.0f),
        glm::vec3(-0.5f, 0.0f, -8.0f),
        glm::vec3(-0.5f, 0.0f, -9.0f),  // terceira parede externa
        glm::vec3(7.5f, 0.0f, -0.0f),
        glm::vec3(7.5f, 0.0f, -1.0f),
        glm::vec3(7.5f, 0.0f, -2.0f),
        glm::vec3(7.5f, 0.0f, -3.0f),
        glm::vec3(7.5f, 0.0f, -4.0f),
        glm::vec3(7.5f, 0.0f, -5.0f),
        glm::vec3(7.5f, 0.0f, -6.0f),
        glm::vec3(7.5f, 0.0f, -7.0f),
        glm::vec3(7.5f, 0.0f, -8.0f),  // quarta parede externa
        glm::vec3(4.0f, 0.0f, -1.5f),
        glm::vec3(3.4f, 0.0f, -3.5f ),
        glm::vec3(1.4f, 0.0f, -4.5f),
        glm::vec3(6.0f, 0.0f, -7.5f),
        glm::vec3(6.0f, 0.0f, -4.5f),  // 83 até aqui
        glm::vec3(4.0f, 0.0f, -0.25f),
        glm::vec3(5.0f, 0.0f, -0.75f),
        glm::vec3(0.5f, 0.0f, -5.75f),
        glm::vec3(5.6f, 0.0f, -5.75f),
        glm::vec3(5.0f, 0.0f, -6.75f),
        glm::vec3(5.8f, 0.0f, -6.25f)
    };

    // colission
    struct BBox modelPositions[89];
    glm::vec4 leftPointVertices = glm::vec4(-0.63f, -0.5f,  0.16f, 1.0f);   // leftPointVertices.x
    glm::vec4 rightPointVertices = glm::vec4(0.63f,  0.5f, -0.16f, 1.0f);   //59
    glm::vec4 leftPointVertices2 = glm::vec4(-0.16f, -0.5f,  0.63f, 1.0f);
    glm::vec4 rightPointVertices2 = glm::vec4(0.16f,  0.5f, -0.63f, 1.0f);  // 83
    glm::vec4 leftPointVertices3 = glm::vec4(-0.16f, -0.5f,  0.40f, 1.0f);
    glm::vec4 rightPointVertices3 = glm::vec4(0.16f,  0.5f, -0.40f, 1.0f);  // 89

    glm::vec4 result;

    for(int i=0; i<59; i++) {
        glm::mat4 model;
        model = glm::translate(model, differentPositions[i]);
        result = model * leftPointVertices;
        modelPositions[i].leftPosition_X = result.x;
        modelPositions[i].leftPosition_Z = result.z;

        result = model * rightPointVertices;
        modelPositions[i].rightPosition_X = result.x;
        modelPositions[i].rightPosition_Z = result.z;
    }

    for(int i=59; i<83; i++) {
        glm::mat4 model;
        model = glm::translate(model, differentPositions[i]);
        result = model * leftPointVertices2;
        modelPositions[i].leftPosition_X = result.x;
        modelPositions[i].leftPosition_Z = result.z;

        result = model * rightPointVertices2;
        modelPositions[i].rightPosition_X = result.x;
        modelPositions[i].rightPosition_Z = result.z;
    }

    for(int i=83; i<89; i++) {
        glm::mat4 model;
        model = glm::translate(model, differentPositions[i]);
        result = model * leftPointVertices3;
        modelPositions[i].leftPosition_X = result.x;
        modelPositions[i].leftPosition_Z = result.z;

        result = model * rightPointVertices3;
        modelPositions[i].rightPosition_X = result.x;
        modelPositions[i].rightPosition_Z = result.z;
    }

    // modelPositions estão os valores

    while(!glfwWindowShouldClose(window)) {

        timeCurrentFrame = glfwGetTime();
        deltaTime = timeCurrentFrame - timeLastFrame;     //modificamos o tempo delta, para que computadores com diferente FPS tenham a mesma velocidade
        timeLastFrame = timeCurrentFrame;

        if(cameraPosition.z < -8.579) {
            showResult_callback(1);
            glfwSetWindowShouldClose(window, true);
        }

        if(timeCurrentFrame > timeToComplete && cameraPosition.z >= -8.579) {
            showResult_callback(0);
            glfwSetWindowShouldClose(window, true);
        }

        glUniform1f(execution_time_location, timeCurrentFrame);

        //processa input
        key_callback_close_window(window);
        key_callback_camera_movement(window, modelPositions);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);   // background color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);       //limpamos a cada iteração o buffer de cor final e também do de profundidade

        ///rendering here...
        glBindTexture(GL_TEXTURE_2D, Texture);   // precisa fazer bind da textura antes de desenhar na tela

        glUseProgram(shaderProgram);


        glUniform1i(isSky_location, 0);
        glUniform3fv(light_source_location,1,glm::value_ptr(lightSource));    // coloca a uniform na posição da fonte de luz
        glUniform3fv(camera_position_location,1,glm::value_ptr(cameraPosition));   // coloca a uniform na posição do visualizador (câmera)
        glUniform3fv(camera_direction_location,1,glm::value_ptr(cameraFront));   // coloca a uniform da direção da luz (para o spotlight, isso significa a câmera

        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);                    // elements vem de "Element Buffer Object (EBO)". Queremos desenhar tendo como base os vértices

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(fov), (float)WIDTH/(float)HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

        //float Raio = 20.0f;
        //float camXposition = sin(glfwGetTime()) * Raio;
        //float camZposition = cos(glfwGetTime()) * Raio;
        glm::mat4 view;
        view = glm::lookAt(cameraPosition,    // posição da câmera (representado como ponto origem + este vetor) Quando maior for 'z', mais para trás vai a câmera (regra da mão direita)
                           cameraPosition + cameraFront,    // ponto alvo da câmera (representado como ponto origem + este vetor)
                           cameraUp);         // vetor qualquer indo para cima (de preferência normalizado como esse!)

        // Assim temos o processo Gram-Schmidt (feita por essa função). Primeiro temos o ponto posição da câmera - ponto alvo da câmera dando o vetor z
        // da câmera. Depois, um produto vetorial desse vetor z com o último vetor indo pra cima passado dá o vetor x da câmera. O produto vetorial de z
        // com x, por fim, dá o vetor y da câmera. Essa função lookAt faz tudo isso para nós, e ainda podemos especificar o ponto alvo da câmera.
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));

        glUniform1i(id_object_location,0);    // passa o id_object do muro (ver fragment shader)

        glBindVertexArray(VAO);
        for(int i = 0; i < 59; i++) {
            glm::mat4 model;
            model = glm::translate(model, differentPositions[i]);
            //model = glm::rotate(model, (float)glfwGetTime() * glm::radians(140.0f), glm::vec3(0.5f,1.0f,0.5f));
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

           glDrawArrays(GL_TRIANGLES,0,36);
        }

        glBindVertexArray(0);

        glBindVertexArray(VAO2);
        for(int i=59; i<83; i++) {
            glm::mat4 model;
            model = glm::translate(model, differentPositions[i]);
            //model = glm::rotate(model, (float)glfwGetTime() * glm::radians(140.0f), glm::vec3(0.5f,1.0f,0.5f));
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES,0,36);
        }

        glBindVertexArray(0);

        glBindVertexArray(VAO3);
        for(int i=83; i<89; i++) {
            glm::mat4 model;
            model = glm::translate(model, differentPositions[i]);
            //model = glm::rotate(model, (float)glfwGetTime() * glm::radians(140.0f), glm::vec3(0.5f,1.0f,0.5f));
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES,0,36);
        }

        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_2D, TextureGround);

        glUniform1i(id_object_location,1);    // passa o id_object do muro (ver fragment shader)

        glBindVertexArray(VAO4);
        glm::mat4 model;
        model = glm::translate(model, glm::vec3(0.0f, -0.1f, 0.0f));
        //model = glm::rotate(model, (float)glfwGetTime() * glm::radians(140.0f), glm::vec3(0.5f,1.0f,0.5f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES,0,36);

        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, 0);

        glBindTexture(GL_TEXTURE_2D, TextureDarkSky);
        glUniform1i(isSky_location, 1);

        glBindVertexArray(VAO5);
        glm::mat4 othermodel;
        othermodel = glm::translate(othermodel, glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(othermodel));
        glDrawArrays(GL_TRIANGLES,0,36);

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);


        ///rendering end here...
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1,&VAO);
    glfwTerminate();
    return 0;
}

///////////////////////////////////////IMPLEMENTAÇÃO DE FUNÇÕES////////////////////////////////////////////////////////////////////////////////////////////////////

void key_callback_camera_movement(GLFWwindow *window, struct BBox positions[]) {

    float cameraSpeed = 0.8f * deltaTime;
    glm::vec3 cameraFuturePosition = cameraPosition;
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraFuturePosition += cameraSpeed * cameraFront;
        if(!collision_callback(positions, cameraFuturePosition))
            cameraPosition += cameraSpeed * cameraFront;
    }

    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraFuturePosition -= cameraSpeed * cameraFront;
        if(!collision_callback(positions, cameraFuturePosition))
            cameraPosition -= cameraSpeed * cameraFront;
    }

    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraFuturePosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if(!collision_callback(positions, cameraFuturePosition))
            cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }

    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraFuturePosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if(!collision_callback(positions, cameraFuturePosition))
            cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
}

bool collision_callback(struct BBox positions[], glm::vec3 cameraFuturePosition) {

    for(int i=0; i<89; i++) {
        if(cameraFuturePosition.y >= 1.0f)
            return false;                 // para poder andar por cima
        if((cameraFuturePosition.x >= positions[i].leftPosition_X && cameraFuturePosition.x <= positions[i].rightPosition_X) && (cameraFuturePosition.z <= positions[i].leftPosition_Z && cameraFuturePosition.z >= positions[i].rightPosition_Z)) {
            return true;
        }
    }

    return false;

}

void mouse_callback(GLFWwindow *window, double currentXposition, double currentYposition) {   //x e y representam a posição atual do mouse

    if(firstMouse) {
        lastXposition = currentXposition;
        lastYposition = currentYposition;
        firstMouse = false;
    }

    float xoffset = currentXposition - lastXposition;
    float yoffset = lastYposition - currentYposition;     // calculamos quanto x e y (pitch e yaw) do mouse foram deslocados desde a última vez

    lastXposition = currentXposition;
    lastYposition = currentYposition; // atualizamos os valores das globais que pegam a última posição de x e y para que seja a atual (agora já temos o deslocamento)

    float mouseSensitivity = 0.08;
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;
    yaw += xoffset;
    //pitch += yoffset;    // atualizamos os valores de pitch e yaw

    //agora limitamos ao usuário olhar para cima até 89 graus e -89 graus (quando chegamos em 90 e -90, a visão tende a se inverter e ficar estranha)
    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    // agora, falta apenas atualizar o "cameraFront", que é aquilo que mudamos quando queremos mudar o ângulo de visão da câmera.
    // fazemos isso através da conversão das coordenadas esféricas para cartesianas.
    glm::vec3 newCameraFront;
    newCameraFront.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    newCameraFront.y = sin(glm::radians(pitch));
    newCameraFront.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    cameraFront = glm::normalize(newCameraFront);                     // atualizamos cameraFront!
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    if(fov >= 1.0f && fov <= 45.0f)
        fov -= yoffset;
    if(fov <= 1.0f)
        fov = 1.0f;
    if(fov >= 45.0f)
        fov = 45.0f;
}

void key_callback_close_window(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void showResult_callback(int result) {

    if(result == 1) {
         printf("__     ______  _    _  __          _______ _   _ _ \n");
        printf("\ \   / / __ \| |  | | \ \        / /_   _| \ | | |\n");
        printf(" \ \_/ / |  | | |  | |  \ \  /\  / /  | | |  \| | |\n");
        printf("  \   /| |  | | |  | |   \ \/  \/ /   | | | . ` | |\n");
        printf("   | | | |__| | |__| |    \  /\  /   _| |_| |\  |_|\n");
        printf("   |_|  \____/ \____/      \/  \/   |_____|_| \_(_)\n");
    }

    if(result == 0) {
        printf("__     ______  _    _   _      ____   _____ ______ _ \n");
        printf(" \ \   / / __ \| |  | | | |    / __ \ / ____|  ____| |\n");
        printf("  \ \_/ / |  | | |  | | | |   | |  | | (___ | |__  | |\n");
        printf("  \   /| |  | | |  | | | |   | |  | |\___ \|  __| | |\n");
        printf("   | | | |__| | |__| | | |___| |__| |____) | |____|_|\n");
        printf("   |_|  \____/ \____/  |______\____/|_____/|______(_)\n");
    }
}
