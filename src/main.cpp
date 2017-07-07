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

void key_callback_camera_movement(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xposition, double yposition);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void key_callback_close_window(GLFWwindow *window);

glm::vec3 cameraPosition = glm::vec3(0.0f,0.0f,3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f,0.0f,-1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f,1.0f,0.0f);
float deltaTime = 0.0f;
float timeLastFrame = 0.0f;
float timeCurrentFrame = 0.0f;

// posições default do cursor do mouse são a metade de cada tamanho da tela
float lastXposition = WIDTH/2.0;
float lastYposition = HEIGHT/2.0;
float pitch = 0.0f;
float yaw = -90.0f;
bool firstMouse = true;
float fov = 45.0f;       // fov = field of view

const char *vertexShaderSource = "#version 330 core \n"
"layout (location = 0) in vec3 position; \n"
//"layout (location = 1) in vec3 inColor; \n"
"layout (location = 1) in vec2 inTex; \n"
//"out vec3 finalColor; \n"
"out vec2 finalTex; \n"
"\n"
"uniform mat4 modelShader; \n"
"uniform mat4 viewShader; \n"
"uniform mat4 projectionShader; \n"
"void main(){ \n"
"   gl_Position = projectionShader * viewShader * modelShader * vec4(position.x, position.y, position.z, 1.0); \n"
//"   finalColor = inColor; \n"
"   finalTex = inTex; \n"
"}\0";

const char *fragmentShaderSource = "#version 330 core\n"
//"in vec3 finalColor; \n"
"in vec2 finalTex;   \n"
"out vec4 FragColor; \n"
"                    \n"
"uniform sampler2D TEX; \n"
"void main(){ \n"
"    FragColor = texture(TEX, finalTex); \n" //* vec4(finalColor.x+0.5, finalColor.y+0.5, finalColor.z+0.5, 1.0f); \n"
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
    key_callback_camera_movement(window);
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

    //traseira
       -0.5f, -0.5f, -0.01f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.01f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.01f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.01f,  1.0f, 1.0f,
       -0.5f,  0.5f, -0.01f,  0.0f, 1.0f,
       -0.5f, -0.5f, -0.01f,  0.0f, 0.0f,

       -0.5f, -0.5f,  0.01f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.01f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.01f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.01f,  1.0f, 1.0f,
       -0.5f,  0.5f,  0.01f,  0.0f, 1.0f,
       -0.5f, -0.5f,  0.01f,  0.0f, 0.0f,

       -0.5f,  0.5f,  0.01f,  1.0f, 0.0f,
       -0.5f,  0.5f, -0.01f,  1.0f, 1.0f,
       -0.5f, -0.5f, -0.01f,  0.0f, 1.0f,
       -0.5f, -0.5f, -0.01f,  0.0f, 1.0f,
       -0.5f, -0.5f,  0.01f,  0.0f, 0.0f,
       -0.5f,  0.5f,  0.01f,  1.0f, 0.0f,

        0.5f,  0.5f,  0.01f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.01f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.01f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.01f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.01f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.01f,  1.0f, 0.0f,

       -0.5f, -0.5f, -0.01f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.01f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.01f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.01f,  1.0f, 0.0f,
       -0.5f, -0.5f,  0.01f,  0.0f, 0.0f,
       -0.5f, -0.5f, -0.01f,  0.0f, 1.0f,

       -0.5f,  0.5f, -0.01f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.01f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.01f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.01f,  1.0f, 0.0f,
       -0.5f,  0.5f,  0.01f,  0.0f, 0.0f,
       -0.5f,  0.5f, -0.01f,  0.0f, 1.0f
};

float vertices2[] = {

    //traseira
       -0.01f, -0.5f, -0.5f,  0.0f, 0.0f,
        0.01f, -0.5f, -0.5f,  1.0f, 0.0f,
        0.01f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.01f,  0.5f, -0.5f,  1.0f, 1.0f,
       -0.01f,  0.5f, -0.5f,  0.0f, 1.0f,
       -0.01f, -0.5f, -0.5f,  0.0f, 0.0f,

       -0.01f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.01f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.01f,  0.5f,  0.5f,  1.0f, 1.0f,
        0.01f,  0.5f,  0.5f,  1.0f, 1.0f,
       -0.01f,  0.5f,  0.5f,  0.0f, 1.0f,
       -0.01f, -0.5f,  0.5f,  0.0f, 0.0f,

       -0.01f,  0.5f,  0.5f,  1.0f, 0.0f,
       -0.01f,  0.5f, -0.5f,  1.0f, 1.0f,
       -0.01f, -0.5f, -0.5f,  0.0f, 1.0f,
       -0.01f, -0.5f, -0.5f,  0.0f, 1.0f,
       -0.01f, -0.5f,  0.5f,  0.0f, 0.0f,
       -0.01f,  0.5f,  0.5f,  1.0f, 0.0f,

        0.01f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.01f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.01f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.01f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.01f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.01f,  0.5f,  0.5f,  1.0f, 0.0f,

       -0.01f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.01f, -0.5f, -0.5f,  1.0f, 1.0f,
        0.01f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.01f, -0.5f,  0.5f,  1.0f, 0.0f,
       -0.01f, -0.5f,  0.5f,  0.0f, 0.0f,
       -0.01f, -0.5f, -0.5f,  0.0f, 1.0f,

       -0.01f,  0.5f, -0.5f,  0.0f, 1.0f,
        0.01f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.01f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.01f,  0.5f,  0.5f,  1.0f, 0.0f,
       -0.01f,  0.5f,  0.5f,  0.0f, 0.0f,
       -0.01f,  0.5f, -0.5f,  0.0f, 1.0f
};

float vertices3[] = {

    //traseira
       -0.1f, -0.5f, -0.25f,  0.0f, 0.0f,
        0.1f, -0.5f, -0.25f,  1.0f, 0.0f,
        0.1f,  0.5f, -0.25f,  1.0f, 1.0f,
        0.1f,  0.5f, -0.25f,  1.0f, 1.0f,
       -0.1f,  0.5f, -0.25f,  0.0f, 1.0f,
       -0.1f, -0.5f, -0.25f,  0.0f, 0.0f,

       -0.1f, -0.5f,  0.25f,  0.0f, 0.0f,
        0.1f, -0.5f,  0.25f,  1.0f, 0.0f,
        0.1f,  0.5f,  0.25f,  1.0f, 1.0f,
        0.1f,  0.5f,  0.25f,  1.0f, 1.0f,
       -0.1f,  0.5f,  0.25f,  0.0f, 1.0f,
       -0.1f, -0.5f,  0.25f,  0.0f, 0.0f,

       -0.1f,  0.5f,  0.25f,  1.0f, 0.0f,
       -0.1f,  0.5f, -0.25f,  1.0f, 1.0f,
       -0.1f, -0.5f, -0.25f,  0.0f, 1.0f,
       -0.1f, -0.5f, -0.25f,  0.0f, 1.0f,
       -0.1f, -0.5f,  0.25f,  0.0f, 0.0f,
       -0.1f,  0.5f,  0.25f,  1.0f, 0.0f,

        0.1f,  0.5f,  0.25f,  1.0f, 0.0f,
        0.1f,  0.5f, -0.25f,  1.0f, 1.0f,
        0.1f, -0.5f, -0.25f,  0.0f, 1.0f,
        0.1f, -0.5f, -0.25f,  0.0f, 1.0f,
        0.1f, -0.5f,  0.25f,  0.0f, 0.0f,
        0.1f,  0.5f,  0.25f,  1.0f, 0.0f,

       -0.1f, -0.5f, -0.25f,  0.0f, 1.0f,
        0.1f, -0.5f, -0.25f,  1.0f, 1.0f,
        0.1f, -0.5f,  0.25f,  1.0f, 0.0f,
        0.1f, -0.5f,  0.25f,  1.0f, 0.0f,
       -0.1f, -0.5f,  0.25f,  0.0f, 0.0f,
       -0.1f, -0.5f, -0.25f,  0.0f, 1.0f,

       -0.1f,  0.5f, -0.25f,  0.0f, 1.0f,
        0.1f,  0.5f, -0.25f,  1.0f, 1.0f,
        0.1f,  0.5f,  0.25f,  1.0f, 0.0f,
        0.1f,  0.5f,  0.25f,  1.0f, 0.0f,
       -0.1f,  0.5f,  0.25f,  0.0f, 0.0f,
       -0.1f,  0.5f, -0.25f,  0.0f, 1.0f
};

 float vertices4[] = {

    //traseira
       -40.5f, -0.01f, -40.5f,  0.0f, 0.0f,
        40.5f, -0.01f, -40.5f,  1.0f, 0.0f,
        40.5f,  0.01f, -40.5f,  1.0f, 1.0f,
        40.5f,  0.01f, -40.5f,  1.0f, 1.0f,
       -40.5f,  0.01f, -40.5f,  0.0f, 1.0f,
       -40.5f, -0.01f, -40.5f,  0.0f, 0.0f,

       -40.5f, -0.01f,  40.5f,  0.0f, 0.0f,
        40.5f, -0.01f,  40.5f,  1.0f, 0.0f,
        40.5f,  0.01f,  40.5f,  1.0f, 1.0f,
        40.5f,  0.01f,  40.5f,  1.0f, 1.0f,
       -40.5f,  0.01f,  40.5f,  0.0f, 1.0f,
       -40.5f, -0.01f,  40.5f,  0.0f, 0.0f,

       -40.5f,  0.01f,  40.5f,  1.0f, 0.0f,
       -40.5f,  0.01f, -40.5f,  1.0f, 1.0f,
       -40.5f, -0.01f, -40.5f,  0.0f, 1.0f,
       -40.5f, -0.01f, -40.5f,  0.0f, 1.0f,
       -40.5f, -0.01f,  40.5f,  0.0f, 0.0f,
       -40.5f,  0.0f,  40.5f,  1.0f, 0.0f,

        40.5f,  0.01f,  40.5f,  1.0f, 0.0f,
        40.5f,  0.01f, -40.5f,  1.0f, 1.0f,
        40.5f, -0.01f, -40.5f,  0.0f, 1.0f,
        40.5f, -0.01f, -40.5f,  0.0f, 1.0f,
        40.5f, -0.01f,  40.5f,  0.0f, 0.0f,
        40.5f,  0.01f,  40.5f,  1.0f, 0.0f,

       -40.5f, -0.01f, -40.5f,  0.0f, 1.0f,
        40.5f, -0.01f, -40.5f,  1.0f, 1.0f,
        40.5f, -0.01f,  40.5f,  1.0f, 0.0f,
        40.5f, -0.01f,  40.5f,  1.0f, 0.0f,
       -40.5f, -0.01f,  40.5f,  0.0f, 0.0f,
       -40.5f, -0.01f, -40.5f,  0.0f, 1.0f,

       -40.5f,  0.01f, -40.5f,  0.0f, 1.0f,
        40.5f,  0.01f, -40.5f,  1.0f, 1.0f,
        40.5f,  0.01f,  40.5f,  1.0f, 0.0f,
        40.5f,  0.01f,  40.5f,  1.0f, 0.0f,
       -40.5f,  0.01f,  40.5f,  0.0f, 0.0f,
       -40.5f,  0.01f, -40.5f,  0.0f, 1.0f
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
    glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE,5*sizeof(float),(void*)0);            //como o VAO interpretará o VBO atualmente "bound" em location 0
    glEnableVertexAttribArray(0);

    //colors
    //glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));        //como o VAO interpretará o VBO atualmente "bound" em location 1 (primeiro atributo passado como parâmetro)
    //glEnableVertexAttribArray(1);

    //texture
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

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
    glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE,5*sizeof(float),(void*)0);            //como o VAO interpretará o VBO atualmente "bound" em location 0
    glEnableVertexAttribArray(0);

    //colors
    //glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));        //como o VAO interpretará o VBO atualmente "bound" em location 1 (primeiro atributo passado como parâmetro)
    //glEnableVertexAttribArray(1);

    //texture
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

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
    glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE,5*sizeof(float),(void*)0);            //como o VAO interpretará o VBO atualmente "bound" em location 0
    glEnableVertexAttribArray(0);

    //colors
    //glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));        //como o VAO interpretará o VBO atualmente "bound" em location 1 (primeiro atributo passado como parâmetro)
    //glEnableVertexAttribArray(1);

    //texture
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

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
    glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE,5*sizeof(float),(void*)0);            //como o VAO interpretará o VBO atualmente "bound" em location 0
    glEnableVertexAttribArray(0);

    //colors
    //glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));        //como o VAO interpretará o VBO atualmente "bound" em location 1 (primeiro atributo passado como parâmetro)
    //glEnableVertexAttribArray(1);

    //texture
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

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
    const std::string filename("C://Users//paulo//Desktop//Trabalho Final - Backup 4//brickWall.jpg");
    unsigned char *data = stbi_load(filename.c_str(),&width, &height, &nrChannels, 0);

    if(!data)
        std::cout << "Failed to load texture!";

    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    // definição da texture do chão   TextureGround

    ///textures end

    unsigned int modelLocation = glGetUniformLocation(shaderProgram, "modelShader");
    unsigned int viewLocation = glGetUniformLocation(shaderProgram, "viewShader");
    unsigned int projectionLocation = glGetUniformLocation(shaderProgram, "projectionShader");

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
        glm::vec3(6.0f, 0.0f, -4.5f),  // 80 até aqui
        glm::vec3(4.0f, 0.0f, -0.25f),
        glm::vec3(5.0f, 0.0f, -0.75f),
        glm::vec3(0.5f, 0.0f, -5.75f),
        glm::vec3(5.6f, 0.0f, -5.75f)
    };

    while(!glfwWindowShouldClose(window)) {

        timeCurrentFrame = glfwGetTime();
        deltaTime = timeCurrentFrame - timeLastFrame;     //modificamos o tempo delta, para que computadores com diferente FPS tenham a mesma velocidade
        timeLastFrame = timeCurrentFrame;

        //processa input
        key_callback_close_window(window);
        key_callback_camera_movement(window);

        glClearColor(0.0f, 0.8f, 0.8f, 1.0f);   // background color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);       //limpamos a cada iteração o buffer de cor final e também do de profundidade

        ///rendering here...
        glBindTexture(GL_TEXTURE_2D, Texture);   // precisa fazer bind da textura antes de desenhar na tela

        glUseProgram(shaderProgram);

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
        for(int i=59; i<80; i++) {
            glm::mat4 model;
            model = glm::translate(model, differentPositions[i]);
            //model = glm::rotate(model, (float)glfwGetTime() * glm::radians(140.0f), glm::vec3(0.5f,1.0f,0.5f));
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES,0,36);
        }

        glBindVertexArray(0);

        glBindVertexArray(VAO3);
        for(int i=80; i<84; i++) {
            glm::mat4 model;
            model = glm::translate(model, differentPositions[i]);
            //model = glm::rotate(model, (float)glfwGetTime() * glm::radians(140.0f), glm::vec3(0.5f,1.0f,0.5f));
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES,0,36);
        }

        glBindVertexArray(0);

        glBindVertexArray(VAO4);
        glm::mat4 model;
        model = glm::translate(model, glm::vec3(0.0f, -0.1f, 0.0f));
        //model = glm::rotate(model, (float)glfwGetTime() * glm::radians(140.0f), glm::vec3(0.5f,1.0f,0.5f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES,0,36);

        glBindVertexArray(VAO4);

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

void key_callback_camera_movement(GLFWwindow *window) {

    float cameraSpeed = 3.5f * deltaTime;
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPosition += cameraSpeed * cameraFront;
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPosition -= cameraSpeed * cameraFront;
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
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
    pitch += yoffset;    // atualizamos os valores de pitch e yaw

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
