#define _USE_MATH_DEFINES
#define IMGUI_DEFINE_MATH_OPERATORS
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "backends/imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

typedef int ImGuiDockNodeFlags, ImGuiConfigFlags, ImGuiWindowFlags;
#define ImGuiConfigFlags_NavEnableKeyboard (1 << 0)
#define ImGuiConfigFlags_NavEnableGamepad (1 << 1)
#define ImGuiConfigFlags_DockingEnable (1 << 10)
#define ImGuiConfigFlags_ViewportsEnable (1 << 11)
#define ImGuiDockNodeFlags_None 0
#define ImGuiWindowFlags_NoDocking (1 << 13)
#define ImGuiWindowFlags_NoBackground (1 << 14)
#define ImGuiWindowFlags_NoTitleBar (1 << 15)
#define ImGuiWindowFlags_NoCollapse (1 << 16)
#define ImGuiWindowFlags_NoMove (1 << 17)
#define ImGuiWindowFlags_NoBringToFrontOnFocus (1 << 18)
#define ImGuiWindowFlags_NoResize (1 << 19)

namespace ImGui {
void SetNextWindowViewport(ImGuiID) {}
ImGuiID DockSpace(ImGuiID, const ImVec2&, ImGuiDockNodeFlags) { return 0; }
void UpdatePlatformWindows() {}
void RenderPlatformWindowsDefault() {}
ImGuiID DockBuilderSplitNode(ImGuiID id, ImGuiDir, float, ImGuiID* a, ImGuiID* b) {
if (a) *a = 0; if (b) *b = 0; return id;
}
void DockBuilderDockWindow(const char*, ImGuiID) {}
void DockBuilderFinish(ImGuiID) {}
enum ImGuiDir { ImGuiDir_None = -1, ImGuiDir_Left, ImGuiDir_Right, ImGuiDir_Up, ImGuiDir_Down };
}

const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
out vec3 Normal; out vec3 FragPos;
uniform mat4 model, view, projection;
void main() {
FragPos = vec3(model * vec4(aPos, 1.0));
Normal = mat3(transpose(inverse(model))) * aNormal;
gl_Position = projection * view * vec4(FragPos, 1.0);
}
)glsl";

const char* fragmentShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;
in vec3 Normal, FragPos;
uniform vec3 lightPos, viewPos, lightColor, objectColor;
void main() {
float ambientStrength = 0.1;
vec3 ambient = ambientStrength * lightColor;
vec3 norm = normalize(Normal);
vec3 lightDir = normalize(lightPos - FragPos);
float diff = max(dot(norm, lightDir), 0.0);
vec3 diffuse = diff * lightColor;
float specularStrength = 0.5;
vec3 viewDir = normalize(viewPos - FragPos);
vec3 reflectDir = reflect(-lightDir, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
vec3 specular = specularStrength * spec * lightColor;
vec3 result = (ambient + diffuse + specular) * objectColor;
FragColor = vec4(result, 1.0);
}
)glsl";

class Sphere {
unsigned int VAO, VBO;
std::vector<float> vertices;
int segments;
void generateSphere(float r, int s) {
vertices.clear(); segments = s;
for (int i = 0; i <= s; ++i) {
float t = i * M_PI / s, st = std::sin(t), ct = std::cos(t);
for (int j = 0; j <= s; ++j) {
float p = j * 2 * M_PI / s, sp = std::sin(p), cp = std::cos(p);
float x = r * st * cp, y = r * st * sp, z = r * ct;
vertices.insert(vertices.end(), {x,y,z,x/r,y/r,z/r});
}}}
void setupBuffers() {
glGenVertexArrays(1, &VAO);
glGenBuffers(1, &VBO);
glBindVertexArray(VAO);
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
glEnableVertexAttribArray(1);
glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindVertexArray(0);
}
public:
Sphere(float r=1.0f, int s=50) { generateSphere(r, s); setupBuffers(); }
void draw() {
glBindVertexArray(VAO);
glDrawArrays(GL_TRIANGLE_STRIP, 0, (segments + 1) * (segments + 1) * 2);
glBindVertexArray(0);
}
~Sphere() { glDeleteVertexArrays(1, &VAO); glDeleteBuffers(1, &VBO); }
};

unsigned int compileShader(unsigned int type, const char* src) {
unsigned int shader = glCreateShader(type);
glShaderSource(shader, 1, &src, nullptr);
glCompileShader(shader);
int success; char log[512];
glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
if (!success) {
glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
std::cerr << "shader error: " << log << std::endl;
return 0;
}
return shader;
}

int windowWidth = 800, windowHeight = 600;
void framebuffer_size_callback(GLFWwindow*, int w, int h) {
windowWidth = w; windowHeight = h; glViewport(0, 0, w, h);
}

bool showSkeleton = false, fullColour = true, showObject = true;
glm::vec3 objectColor(0.5f, 0.5f, 1.0f);
float cameraDistance = 5.0f, cameraYaw = 0.0f, cameraPitch = 0.0f;
bool isDraggingLeft = false, isDraggingRight = false;
double lastMouseX = 0.0, lastMouseY = 0.0;

void mouseButtonCallback(GLFWwindow* w, int b, int a, int) {
if (ImGui::GetIO().WantCaptureMouse) return;
if (b == GLFW_MOUSE_BUTTON_LEFT) {
if (a == GLFW_PRESS) { isDraggingLeft = true; glfwGetCursorPos(w, &lastMouseX, &lastMouseY); }
else if (a == GLFW_RELEASE) isDraggingLeft = false;
}
else if (b == GLFW_MOUSE_BUTTON_RIGHT) {
if (a == GLFW_PRESS) { isDraggingRight = true; glfwGetCursorPos(w, &lastMouseX, &lastMouseY); }
else if (a == GLFW_RELEASE) isDraggingRight = false;
}}

void cursorPosCallback(GLFWwindow*, double xpos, double ypos) {
if (ImGui::GetIO().WantCaptureMouse) return;
if (isDraggingLeft) {
float dx = xpos - lastMouseX, dy = ypos - lastMouseY;
cameraYaw += dx * 0.3f; cameraPitch += dy * 0.3f;
cameraPitch = std::max(-89.0f, std::min(89.0f, cameraPitch));
lastMouseX = xpos; lastMouseY = ypos;
}
else if (isDraggingRight) {
float dx = xpos - lastMouseX, dy = ypos - lastMouseY;
objectColor.x = std::max(0.0f, std::min(1.0f, objectColor.x + dx * 0.005f));
objectColor.y = std::max(0.0f, std::min(1.0f, objectColor.y + dy * 0.005f));
lastMouseX = xpos; lastMouseY = ypos;
}}

void generateNoiseMesh(std::vector<float>& vertices, int res) {
vertices.clear();
for (int x = 0; x < res; ++x) {
for (int y = 0; y < res; ++y) {
float nx = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
float ny = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
float nz = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
vertices.insert(vertices.end(), {nx, ny, nz});
}}}
