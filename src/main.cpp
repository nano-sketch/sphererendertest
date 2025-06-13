#define _USE_MATH_DEFINES
#define IMGUI_DEFINE_MATH_OPERATORS
#include<cmath>
#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include"backends/imgui.h"
#include"backends/imgui_impl_glfw.h"
#include"backends/imgui_impl_opengl3.h"
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<iostream>
#include<vector>
typedef int ImGuiDockNodeFlags;typedef int ImGuiConfigFlags;typedef int ImGuiWindowFlags;
#define ImGuiConfigFlags_NavEnableKeyboard (1<<0)
#define ImGuiConfigFlags_NavEnableGamepad (1<<1)
#define ImGuiConfigFlags_DockingEnable (1<<10)
#define ImGuiConfigFlags_ViewportsEnable (1<<11)
#define ImGuiDockNodeFlags_None 0
#define ImGuiWindowFlags_NoDocking (1<<13)
#define ImGuiWindowFlags_NoBackground (1<<14)
#define ImGuiWindowFlags_NoTitleBar (1<<15)
#define ImGuiWindowFlags_NoCollapse (1<<16)
#define ImGuiWindowFlags_NoMove (1<<17)
#define ImGuiWindowFlags_NoBringToFrontOnFocus (1<<18)
#define ImGuiWindowFlags_NoResize (1<<19)
namespace ImGui{
void SetNextWindowViewport(ImGuiID viewport_id){}
ImGuiID DockSpace(ImGuiID id,const ImVec2& size,ImGuiDockNodeFlags flags){return 0;}
void UpdatePlatformWindows(){}
void RenderPlatformWindowsDefault(){}
ImGuiID DockBuilderSplitNode(ImGuiID node_id,ImGuiDir split_dir,float size_ratio,ImGuiID* out_id_at_dir,ImGuiID* out_id_dir){
if(out_id_at_dir)*out_id_at_dir=0;if(out_id_dir)*out_id_dir=0;return node_id;}
void DockBuilderDockWindow(const char* window_name,ImGuiID node_id){}
void DockBuilderFinish(ImGuiID node_id){}
enum ImGuiDir{ImGuiDir_None=-1,ImGuiDir_Left=0,ImGuiDir_Right=1,ImGuiDir_Up=2,ImGuiDir_Down=3};}
const char* vertexShaderSource=R"glsl(
#version 330 core
layout(location=0)in vec3 aPos;layout(location=1)in vec3 aNormal;out vec3 Normal;out vec3 FragPos;
uniform mat4 model;uniform mat4 view;uniform mat4 projection;
void main(){FragPos=vec3(model*vec4(aPos,1.0));Normal=mat3(transpose(inverse(model)))*aNormal;
gl_Position=projection*view*vec4(FragPos,1.0);}
)glsl";
const char* fragmentShaderSource=R"glsl(
#version 330 core
out vec4 FragColor;in vec3 Normal;in vec3 FragPos;
uniform vec3 lightPos;uniform vec3 viewPos;uniform vec3 lightColor;uniform vec3 objectColor;
void main(){float ambientStrength=0.1;vec3 ambient=ambientStrength*lightColor;
vec3 norm=normalize(Normal);vec3 lightDir=normalize(lightPos-FragPos);
float diff=max(dot(norm,lightDir),0.0);vec3 diffuse=diff*lightColor;
float specularStrength=0.5;vec3 viewDir=normalize(viewPos-FragPos);
vec3 reflectDir=reflect(-lightDir,norm);float spec=pow(max(dot(viewDir,reflectDir),0.0),32);
vec3 specular=specularStrength*spec*lightColor;
vec3 result=(ambient+diffuse+specular)*objectColor;FragColor=vec4(result,1.0);}
)glsl";
class Sphere{
private:
unsigned int VAO,VBO;std::vector<float> vertices;int segments;
void generateSphere(float radius,int segments){
vertices.clear();this->segments=segments;
for(int lat=0;lat<=segments;++lat){float theta=lat*M_PI/segments;float sinTheta=std::sin(theta);float cosTheta=std::cos(theta);
for(int lon=0;lon<=segments;++lon){float phi=lon*2*M_PI/segments;float sinPhi=std::sin(phi);float cosPhi=std::cos(phi);
float x=radius*sinTheta*cosPhi;float y=radius*sinTheta*sinPhi;float z=radius*cosTheta;
vertices.push_back(x);vertices.push_back(y);vertices.push_back(z);vertices.push_back(x/radius);vertices.push_back(y/radius);vertices.push_back(z/radius);}}}
public:
Sphere(float radius=1.0f,int segments=50){generateSphere(radius,segments);setupBuffers();}
void setupBuffers(){
glGenVertexArrays(1,&VAO);glGenBuffers(1,&VBO);glBindVertexArray(VAO);glBindBuffer(GL_ARRAY_BUFFER,VBO);
glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(float),vertices.data(),GL_STATIC_DRAW);
glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);glEnableVertexAttribArray(0);
glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(3*sizeof(float)));glEnableVertexAttribArray(1);
glBindBuffer(GL_ARRAY_BUFFER,0);glBindVertexArray(0);}
void draw(){glBindVertexArray(VAO);glDrawArrays(GL_TRIANGLE_STRIP,0,(segments+1)*(segments+1)*2);glBindVertexArray(0);}
~Sphere(){glDeleteVertexArrays(1,&VAO);glDeleteBuffers(1,&VBO);}};
unsigned int compileShader(unsigned int type,const char* source){
unsigned int shader=glCreateShader(type);glShaderSource(shader,1,&source,nullptr);glCompileShader(shader);
int success;char infoLog[512];glGetShaderiv(shader,GL_COMPILE_STATUS,&success);
if(!success){glGetShaderInfoLog(shader,sizeof(infoLog),nullptr,infoLog);std::cerr<<"compiling shader failed: "<<infoLog<<std::endl;return 0;}
return shader;}
int windowWidth=800;int windowHeight=600;
void framebuffer_size_callback(GLFWwindow* window,int width,int height){
windowWidth=width;windowHeight=height;glViewport(0,0,width,height);}
bool showSkeleton=false;bool fullColour=true;bool showObject=true;glm::vec3 objectColor=glm::vec3(0.5f,0.5f,1.0f);
float cameraDistance=5.0f;float cameraYaw=0.0f;float cameraPitch=0.0f;bool isDraggingLeft=false;bool isDraggingRight=false;double lastMouseX=0.0;double lastMouseY=0.0;
void mouseButtonCallback(GLFWwindow* window,int button,int action,int mods){
ImGuiIO& io=ImGui::GetIO();if(io.WantCaptureMouse)return;
if(button==GLFW_MOUSE_BUTTON_LEFT){if(action==GLFW_PRESS){isDraggingLeft=true;glfwGetCursorPos(window,&lastMouseX,&lastMouseY);}
else if(action==GLFW_RELEASE){isDraggingLeft=false;}}
else if(button==GLFW_MOUSE_BUTTON_RIGHT){if(action==GLFW_PRESS){isDraggingRight=true;glfwGetCursorPos(window,&lastMouseX,&lastMouseY);}
else if(action==GLFW_RELEASE){isDraggingRight=false;}}}
void cursorPosCallback(GLFWwindow* window,double xpos,double ypos){
ImGuiIO& io=ImGui::GetIO();if(io.WantCaptureMouse)return;
if(isDraggingLeft){float deltaX=static_cast<float>(xpos-lastMouseX);float deltaY=static_cast<float>(ypos-lastMouseY);
cameraYaw+=deltaX*0.5f;cameraPitch+=deltaY*0.5f;cameraPitch=std::max(-89.0f,std::min(89.0f,cameraPitch));lastMouseX=xpos;lastMouseY=ypos;}
else if(isDraggingRight){float deltaX=static_cast<float>(xpos-lastMouseX);float deltaY=static_cast<float>(ypos-lastMouseY);
objectColor.x=std::max(0.0f,std::min(1.0f,objectColor.x+deltaX*0.01f));objectColor.y=std::max(0.0f,std::min(1.0f,objectColor.y+deltaY*0.01f));lastMouseX=xpos;lastMouseY=ypos;}}
void generateNoiseMesh(std::vector<float>& vertices,int resolution){
vertices.clear();for(int x=0;x<resolution;++x){for(int y=0;y<resolution;++y){
float noiseX=static_cast<float>(rand())/RAND_MAX*2.0f-1.0f;float noiseY=static_cast<float>(rand())/RAND_MAX*2.0f-1.0f;float noiseZ=static_cast<float>(rand())/RAND_MAX*2.0f-1.0f;
vertices.push_back(noiseX);vertices.push_back(noiseY);vertices.push_back(noiseZ);}}}
int main(){
if(!glfwInit()){std::cerr<<"Failed to initialize GLFW"<<std::endl;return -1;}
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);glfwWindowHint(GLFW_RESIZABLE,GLFW_TRUE);
GLFWwindow* window=glfwCreateWindow(windowWidth,windowHeight,"sphere render",nullptr,nullptr);
if(!window){std::cerr<<"failed to make a window"<<std::endl;glfwTerminate();return -1;}
glfwSetFramebufferSizeCallback(window,framebuffer_size_callback);glfwMakeContextCurrent(window);glewExperimental=GL_TRUE;
if(glewInit()!=GLEW_OK){std::cerr<<"failed to initialize glew"<<std::endl;return -1;}
IMGUI_CHECKVERSION();ImGui::CreateContext();ImGuiIO& io=ImGui::GetIO();(void)io;
io.ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard;io.ConfigFlags|=ImGuiConfigFlags_NavEnableGamepad;
ImGui_ImplGlfw_InitForOpenGL(window,true);ImGui_ImplOpenGL3_Init("#version 330");ImGui::StyleColorsDark();
ImGuiStyle& style=ImGui::GetStyle();if(io.ConfigFlags&ImGuiConfigFlags_ViewportsEnable){style.WindowRounding=0.0f;style.Colors[ImGuiCol_WindowBg].w=1.0f;}
unsigned int vertexShader=compileShader(GL_VERTEX_SHADER,vertexShaderSource);unsigned int fragmentShader=compileShader(GL_FRAGMENT_SHADER,fragmentShaderSource);
unsigned int shaderProgram=glCreateProgram();glAttachShader(shaderProgram,vertexShader);glAttachShader(shaderProgram,fragmentShader);glLinkProgram(shaderProgram);
int success;char infoLog[512];glGetProgramiv(shaderProgram,GL_LINK_STATUS,&success);
if(!success){glGetProgramInfoLog(shaderProgram,sizeof(infoLog),nullptr,infoLog);std::cerr<<"shader linking failed: "<<infoLog<<std::endl;return -1;}
glfwSetMouseButtonCallback(window,mouseButtonCallback);glfwSetCursorPosCallback(window,cursorPosCallback);
glfwSetScrollCallback(window,[](GLFWwindow* window,double xoffset,double yoffset){
ImGuiIO& io=ImGui::GetIO();if(io.WantCaptureMouse)return;
cameraDistance-=static_cast<float>(yoffset)*0.5f;cameraDistance=std::max(1.0f,std::min(10.0f,cameraDistance));});
Sphere sphere(1.0f,50);while(!glfwWindowShouldClose(window)){glfwPollEvents();
ImGui_ImplOpenGL3_NewFrame();ImGui_ImplGlfw_NewFrame();ImGui::NewFrame();
const ImGuiViewport* viewport=ImGui::GetMainViewport();
ImGui::SetNextWindowSize(ImVec2(250,viewport->WorkSize.y));ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x,viewport->WorkPos.y));
ImGui::Begin("Sphere Controls",nullptr,ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoDecoration);
ImGui::Text("Sphere Renderer");if(ImGui::Button("Skeleton")){showSkeleton=true;fullColour=false;}ImGui::SameLine();
if(ImGui::Button("Full Colour")){showSkeleton=false;fullColour=true;}ImGui::Checkbox("Show Object",&showObject);
if(ImGui::Button("Generate Noise Mesh")){std::vector<float> noiseVertices;generateNoiseMesh(noiseVertices,50);}
if(fullColour){ImGui::ColorEdit3("Object Color",(float*)&objectColor);}
ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",1000.0f/ImGui::GetIO().Framerate,ImGui::GetIO().Framerate);ImGui::End();
ImGui::Render();glClearColor(0.2f,0.3f,0.3f,1.0f);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
if(showObject){glUseProgram(shaderProgram);float time=glfwGetTime();
glm::mat4 projection=glm::perspective(glm::radians(45.0f),static_cast<float>(windowWidth)/static_cast<float>(windowHeight),0.1f,100.0f);
glm::mat4 view=glm::mat4(1.0f);view=glm::rotate(view,glm::radians(cameraPitch),glm::vec3(1.0f,0.0f,0.0f));
view=glm::rotate(view,glm::radians(cameraYaw),glm::vec3(0.0f,1.0f,0.0f));view=glm::translate(view,glm::vec3(0.0f,0.0f,-cameraDistance));
glm::mat4 model=glm::mat4(1.0f);model=glm::rotate(model,time*glm::radians(50.0f),glm::vec3(0.0f,1.0f,0.0f));
int modelLoc=glGetUniformLocation(shaderProgram,"model");int viewLoc=glGetUniformLocation(shaderProgram,"view");
int projectionLoc=glGetUniformLocation(shaderProgram,"projection");
glUniformMatrix4fv(modelLoc,1,GL_FALSE,glm::value_ptr(model));glUniformMatrix4fv(viewLoc,1,GL_FALSE,glm::value_ptr(view));
glUniformMatrix4fv(projectionLoc,1,GL_FALSE,glm::value_ptr(projection));
int lightPosLoc=glGetUniformLocation(shaderProgram,"lightPos");int viewPosLoc=glGetUniformLocation(shaderProgram,"viewPos");
int lightColorLoc=glGetUniformLocation(shaderProgram,"lightColor");int objectColorLoc=glGetUniformLocation(shaderProgram,"objectColor");
glUniform3f(lightPosLoc,1.2f,1.0f,2.0f);glUniform3f(viewPosLoc,0.0f,0.0f,5.0f);glUniform3f(lightColorLoc,1.0f,1.0f,1.0f);
if(showSkeleton){glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);glUniform3f(objectColorLoc,1.0f,1.0f,1.0f);}
else if(fullColour){glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);glUniform3f(objectColorLoc,objectColor.x,objectColor.y,objectColor.z);}
sphere.draw();glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);}
ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
if(ImGui::GetIO().ConfigFlags&ImGuiConfigFlags_ViewportsEnable){ImGui::UpdatePlatformWindows();ImGui::RenderPlatformWindowsDefault();glfwMakeContextCurrent(window);}
glfwSwapBuffers(window);}
glDeleteProgram(shaderProgram);ImGui_ImplOpenGL3_Shutdown();ImGui_ImplGlfw_Shutdown();ImGui::DestroyContext();
glfwDestroyWindow(window);glfwTerminate();return 0;}
