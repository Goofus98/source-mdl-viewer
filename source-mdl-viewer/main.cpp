// Your First C++ Program

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm\glm.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "shader.h"
#include "MDLParser.h"

int width{ 1920 }, height{ 1080 };
//Load the mdl vvd and vtx files.
const std::string src_mdl{"./mdls/coco_player.mdl"};
const std::string src_vvd{"./mdls/coco_player.vvd"};
const std::string src_vtx{"./mdls/coco_player.dx80.vtx"};

size_t numVerts{};

//Setup number of buffers and atribs
const size_t numVAOs{1};
const size_t numVBOs{2};


glm::vec3 origin(0.0f, 0.0f, 0.0f);
glm::vec3 cameraLoc(0.0f, 30.0f, 150.4f); //Default Camera location

//Allocate variables used in draw() function, so that they won’t need to be allocated during rendering
GLuint mvLoc, projLoc;

float aspect;
//Create Matrices for percpective view model and model-view
glm::mat4 pMat, vMat, mMat, mvMat;
size_t BufferSize{};
GLuint renderingProgram;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
const float camspeed = 25.0f; //Default camera speed when moving

glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float lastX = width / 2, lastY = height / 2;
float yaw{}, pitch{};

inline std::vector<uint8_t> read_vector_from_disk(std::string file_path)
{
    std::ifstream instream(file_path, std::ios::in | std::ios::binary);
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
    return data;
}


void setupVertices(void) {
    std::vector<uint8_t> vvd = read_vector_from_disk(src_vvd);
    std::vector<uint8_t> mdl = read_vector_from_disk(src_mdl);
    std::vector<uint8_t> vtx = read_vector_from_disk(src_vtx);
    MDL model = MDL(
        mdl.data(), sizeof(mdl[0]) * mdl.size(),
        vvd.data(), sizeof(vvd[0]) * vvd.size(),
        vtx.data(), sizeof(vtx[0]) * vtx.size()
    );
    
    VVD vv = VVD(vvd.data(), sizeof(vvd[0]) * vvd.size(), model.GetChecksum());

    numVerts = vv.GetNumVertices();
    std::vector<glm::vec3> vertices{};

    for (size_t i = 0; i < numVerts; i++)
    {
        const VVDStructs::Vertex* vertex = vv.GetVertex(i);
        auto pos = vertex->pos;

        vertices.push_back(glm::vec3(pos.x, pos.z, pos.y));


        ++BufferSize;
    }
    glGenVertexArrays(1, vao);
    glBindVertexArray(vao[0]);
    glGenBuffers(numVBOs, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, BufferSize * sizeof(vertices[0]), &vertices[0], GL_STATIC_DRAW);

}

void init(GLFWwindow* window) {

    shader mainShader{ "vShader.glsl", "fShader.glsl" };
    renderingProgram = mainShader.getProg();


    setupVertices();
}

void draw(GLFWwindow* window, double currentTime) {
    glClear(GL_DEPTH_BUFFER_BIT);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(renderingProgram);
    mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
    projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");
    glfwGetFramebufferSize(window, &width, &height);
    aspect = (float)width / (float)height;
    pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);

    deltaTime = currentTime - lastFrame;
    lastFrame = currentTime;


    vMat = glm::lookAt(cameraLoc, cameraLoc + cameraFront, cameraUp);

    mMat = glm::translate(glm::mat4(1.0f), origin);


    mvMat = vMat * mMat;

    //WASD Camera controls
    float cameraSpeed = camspeed * deltaTime; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraLoc += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraLoc -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraLoc -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraLoc += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;


    // copy perspective and MV matrices to corresponding uniform variables
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    
    // adjust OpenGL settings and draw model
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDrawArrays(GL_POINTS, 0, BufferSize);
}

//Create Camera Controls
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;


    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}
int main() {
    if (!glfwInit()) { exit(EXIT_FAILURE); }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(width, height, "MDL Viewer", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //Disable cursor
    glfwSetCursorPosCallback(window, mouse_callback); //Setup the callback func for mouse camera controls

    if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
    glfwSwapInterval(1);

    init(window);
    while (!glfwWindowShouldClose(window)) {
        draw(window, glfwGetTime());
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);

}