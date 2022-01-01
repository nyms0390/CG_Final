#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <iostream>

#include <GLFW/glfw3.h>
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#undef GLAD_GL_IMPLEMENTATION
#include <glm/glm.hpp>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#include <stb_image.h>
#undef STB_IMAGE_IMPLEMENTATION
#include "graphics.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Unnamed namespace for global variables
namespace {
    // Cameras
    graphics::camera::Camera* currentCamera = nullptr;
    // Control variables
    bool isWindowSizeChanged = false;
    bool isLightChanged = true;
    bool updatePhongParameters = true;
    bool updateFogParameters = true;
    bool updateOutlineParameters = true;
    bool updatePixelizeParameters = true;
    bool updatePosterizeParameters = true;
    bool mouseBinded = false;
    int currentLight = 1;
    int currentShader = 1;
    int alignSize = 256;
    float ambient = 0.1;
    float ks = 0.75;
    float kd = 0.75;
    float fogB = 0.03;

    constexpr int LIGHT_COUNT = 3;
    constexpr int CAMERA_COUNT = 1;
    constexpr int MESH_COUNT = 3;
    constexpr int SHADER_PROGRAM_COUNT = 5;
}  // namespace

int uboAlign(int i) { return ((i + 1 * (alignSize - 1)) / alignSize) * alignSize; }

void keyCallback(GLFWwindow* window, int key, int, int action, int) {
    // There are three actions: press, release, hold
    if (action != GLFW_PRESS) return;
    // Press ESC to close the window.
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    } else if (key == GLFW_KEY_F9) {
        // Disable / enable mouse cursor.
        if (mouseBinded)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        mouseBinded = !mouseBinded;
    }
    
    switch (key) {
    // TODO: Detect key-events, to:
    //       1. switch among directional light, point light, and spot light, or
    //       2. switch between phong shader and gouraurd shader
    // Hint: use currentLight, isLightChanged, currentShader
    // Note: 1 key for 1 variable change
        default: break;
    }
}

void resizeCallback(GLFWwindow* window, int width, int height) {
    OpenGLContext::framebufferResizeCallback(window, width, height);
    assert(currentCamera != nullptr);
    currentCamera->updateProjection(OpenGLContext::getAspectRatio());
    isWindowSizeChanged = true;
}

void renderMainPanel();
void renderGUI();

int main() {
    // Initialize OpenGL context, details are wrapped in class.
    OpenGLContext::createContext(43, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = OpenGLContext::getWindow();
    glfwSetWindowTitle(window, "Final");
    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, resizeCallback);
#ifndef NDEBUG
    OpenGLContext::printSystemInfo();
    // This is useful if you want to debug your OpenGL API calls.
    OpenGLContext::enableDebugCallback();
#endif
    // Initialize ImGUI
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    if (OpenGLContext::getOpenGLVersion() >= 41) {
        ImGui_ImplOpenGL3_Init("#version 410 core");
    } else {
        ImGui_ImplOpenGL3_Init(nullptr);
    }
    // Initialize shader
    std::vector<graphics::shader::ShaderProgram> shaderPrograms(SHADER_PROGRAM_COUNT);
    std::string filenames[SHADER_PROGRAM_COUNT] = {"phong", "fog", "outline", "pixelize", "posterize"};
    for (int i = 0; i < SHADER_PROGRAM_COUNT; ++i) {
        graphics::shader::VertexShader vs;
        graphics::shader::FragmentShader fs;
        vs.fromFile("../assets/shader/" + filenames[i] + ".vert");
        fs.fromFile("../assets/shader/" + filenames[i] + ".frag");
        shaderPrograms[i].attach(&vs, &fs);
        shaderPrograms[i].link();
        shaderPrograms[i].detach(&vs, &fs);
        shaderPrograms[i].use();
        
        shaderPrograms[i].uniformBlockBinding("model", 0);
        shaderPrograms[i].uniformBlockBinding("camera", 1);
        shaderPrograms[i].uniformBlockBinding("light", 2);

        shaderPrograms[i].setUniform("diffuseTexture", 0);
        shaderPrograms[i].setUniform("shadowMap", 1);
        shaderPrograms[i].setUniform("diffuseCubeTexture", 2);
        shaderPrograms[i].setUniform("isCube", 0);
    }
    graphics::buffer::UniformBuffer meshUBO, cameraUBO, lightUBO;
    // Calculate UBO alignment size
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignSize);
    constexpr int perMeshSize = 2 * sizeof(glm::mat4);
    constexpr int perCameraSize = sizeof(glm::mat4) + sizeof(glm::vec4);
    constexpr int perLightSize = sizeof(glm::mat4) + 2 * sizeof(glm::vec4);
    int perMeshOffset = uboAlign(perMeshSize);
    int perCameraOffset = uboAlign(perCameraSize);
    int perLightOffset = uboAlign(perLightSize);
    meshUBO.allocate(MESH_COUNT * perMeshOffset, GL_DYNAMIC_DRAW);
    cameraUBO.allocate(CAMERA_COUNT * perCameraOffset, GL_DYNAMIC_DRAW);
    lightUBO.allocate(LIGHT_COUNT * perLightOffset, GL_DYNAMIC_DRAW);
    // Default to first data
    meshUBO.bindUniformBlockIndex(0, 0, perMeshSize);
    cameraUBO.bindUniformBlockIndex(1, 0, perCameraSize);
    lightUBO.bindUniformBlockIndex(2, 0, perLightSize);
    // Get texture information
    int maxTextureSize = 1024;
    // Uncomment the following line if your GPU is very poor
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    maxTextureSize = std::min(maxTextureSize, 4096);
    // Camera
    std::vector<graphics::camera::CameraPTR> cameras;
    cameras.emplace_back(graphics::camera::QuaternionCamera::make_unique(glm::vec3(0, 0, 15)));
    assert(cameras.size() == CAMERA_COUNT);
    for (int i = 0; i < CAMERA_COUNT; ++i) {
        int offset = i * perCameraOffset;
        cameras[i]->initialize(OpenGLContext::getAspectRatio());
        cameraUBO.load(offset, sizeof(glm::mat4), cameras[i]->getViewProjectionMatrixPTR());
        cameraUBO.load(offset + sizeof(glm::mat4), sizeof(glm::vec4), cameras[i]->getPositionPTR());
    }
    currentCamera = cameras[0].get();
    // Lights
    glm::vec2 cutoff = glm::vec2(cos(glm::radians(5.0f)), glm::cos(glm::radians(22.0f)));
    std::vector<graphics::light::LightPTR> lights;
    lights.emplace_back(graphics::light::DirectionalLight::make_unique(glm::vec3(8, 6, 6)));
    lights.emplace_back(graphics::light::PointLight::make_unique(glm::vec3(8, 6, 6)));
    lights.emplace_back(graphics::light::Spotlight::make_unique(currentCamera->getFront(), cutoff));
    assert(lights.size() == LIGHT_COUNT);
    // TODO: Bind light object's buffer
    // Hint: look what we did when binding other UBO
    for (int i = 0; i < LIGHT_COUNT; ++i) {
        int offset = i * perLightOffset;
        lightUBO.load(offset, sizeof(glm::mat4), lights[i]->getLightSpaceMatrixPTR());
        lightUBO.load(offset + sizeof(glm::mat4), sizeof(glm::vec4), lights[i]->getLightVectorPTR());
        lightUBO.load(offset + sizeof(glm::mat4) + sizeof(glm::vec4), sizeof(glm::vec4), lights[i]->getLightCoefficientsPTR());
    }
    // Texture
    graphics::texture::ShadowMap shadow(maxTextureSize);
    graphics::texture::Texture2D colorOrange, wood;
    graphics::texture::TextureCubeMap dice;
    colorOrange.fromColor(glm::vec4(1, 0.5, 0, 1));
    std::string path_texture = "/Users/zhangyang/Documents/Assignments/CG/Final/assets/texture/";
    wood.fromFile(path_texture + "wood.jpg");
    dice.fromFile(path_texture + "posx.jpg", path_texture + "negx.jpg", path_texture + "posy.jpg", path_texture + "negy.jpg", path_texture + "posz.jpg", path_texture + "negz.jpg");
    // Meshes
    std::vector<graphics::shape::ShapePTR> meshes;
    std::vector<graphics::texture::Texture*> diffuseTextures;
    {
        std::vector<GLfloat> vertexData;
        std::vector<GLuint> indexData;
        graphics::shape::Plane::generateVertices(vertexData, indexData, 40, 20, 20, false);
        auto sphere = graphics::shape::Sphere::make_unique();
        auto cube = graphics::shape::Cube::make_unique();
        auto ground = graphics::shape::Plane::make_unique(vertexData, indexData);

        glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(3, 0, 4));
        model = glm::scale(model, glm::vec3(2));
        model = glm::rotate(model, glm::half_pi<float>(), glm::vec3(1, 0, 0));
        sphere->setModelMatrix(model);

        model = glm::translate(glm::mat4(1), glm::vec3(-3, -1, 0));
        model = glm::scale(model, glm::vec3(2));
        cube->setModelMatrix(model);

        model = glm::translate(glm::mat4(1), glm::vec3(0, -3, 0));
        ground->setModelMatrix(model);

        meshes.emplace_back(std::move(ground));
        diffuseTextures.emplace_back(&wood);
        meshes.emplace_back(std::move(sphere));
        diffuseTextures.emplace_back(&colorOrange);
        meshes.emplace_back(std::move(cube));
        diffuseTextures.emplace_back(&wood);
    }
    assert(meshes.size() == MESH_COUNT);
    assert(diffuseTextures.size() == MESH_COUNT);
    for (int i = 0; i < MESH_COUNT; ++i) {
        int offset = i * perMeshOffset;
        meshUBO.load(offset, sizeof(glm::mat4), meshes[i]->getModelMatrixPTR());
        meshUBO.load(offset + sizeof(glm::mat4), sizeof(glm::mat4), meshes[i]->getNormalMatrixPTR());
    }
    shadow.bind(1);
    dice.bind(2);
    // Main rendering loop
    while (!glfwWindowShouldClose(window)) {
        // Polling events.
        glfwPollEvents();
        // Update camera's uniforms if camera moves.
        bool isCameraMove = mouseBinded ? currentCamera->move(window) : false;
        if (isCameraMove || isWindowSizeChanged) {
            isWindowSizeChanged = false;
            cameraUBO.load(0, sizeof(glm::mat4), currentCamera->getViewProjectionMatrixPTR());
            cameraUBO.load(sizeof(glm::mat4), sizeof(glm::vec4), currentCamera->getPositionPTR());
            if (lights[currentLight]->getType() == graphics::light::LightType::Spot) {
                lights[currentLight]->update(currentCamera->getViewMatrix());
                int offset = currentLight * perLightOffset;
                glm::vec4 front = currentCamera->getFront();
                lightUBO.load(offset, sizeof(glm::mat4), lights[currentLight]->getLightSpaceMatrixPTR());
                lightUBO.load(offset + sizeof(glm::mat4), sizeof(glm::vec4), glm::value_ptr(front));
            }
        }
        if (isLightChanged) {
            int offset = currentLight * perLightOffset;
            if (lights[currentLight]->getType() == graphics::light::LightType::Spot) {
                lights[currentLight]->update(currentCamera->getViewMatrix());
                glm::vec4 front = currentCamera->getFront();
                lightUBO.load(offset, sizeof(glm::mat4), lights[currentLight]->getLightSpaceMatrixPTR());
                lightUBO.load(offset + sizeof(glm::mat4), sizeof(glm::vec4), glm::value_ptr(front));
            }
            isLightChanged = false;
        }
        shaderPrograms[currentShader].use();
        if (updatePhongParameters) {
            shaderPrograms[currentShader].setUniform("ambient", ambient);
            shaderPrograms[currentShader].setUniform("kd", kd);
            shaderPrograms[currentShader].setUniform("ks", ks);
            updatePhongParameters = false;
        }
        if (updateFogParameters) {
            shaderPrograms[currentShader].setUniform("b", fogB);
            updateFogParameters = false;
        }
        if (updateOutlineParameters) {}
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Render all objects
        for (int i = 0; i < MESH_COUNT; ++i) {
            // Change uniform if it is a cube (We want to use cubemap texture)
            if (meshes[i]->getType() == graphics::shape::ShapeType::Cube) {
                shaderPrograms[currentShader].setUniform("isCube", 1);
            } else {
                shaderPrograms[currentShader].setUniform("isCube", 0);
            }
            // Bind current object's model matrix
            meshUBO.bindUniformBlockIndex(0, i * perMeshOffset, perMeshSize);
            // Bind current object's texture
            diffuseTextures[i]->bind(0);
            // Render current object
            meshes[i]->draw();
        }
        // Render GUI
        renderGUI();
#ifdef __APPLE__
        // Some platform need explicit glFlush
        glFlush();
#endif
        glfwSwapBuffers(window);
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    return 0;
}

void renderMainPanel() {
    ImGui::SetNextWindowSize(ImVec2(400.0f, 275.0f), ImGuiCond_Once);
    ImGui::SetNextWindowCollapsed(0, ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(50.0f, 50.0f), ImGuiCond_Once);
    ImGui::SetNextWindowBgAlpha(0.2f);
    if (ImGui::Begin("Configs")) {
        updatePhongParameters |= ImGui::RadioButton("Phong", &currentShader, 0);
        ImGui::SameLine();
        updateFogParameters |= ImGui::RadioButton("Fog", &currentShader, 1);
        ImGui::SameLine();
        updateOutlineParameters |= ImGui::RadioButton("Outline", &currentShader, 2);
        ImGui::SameLine();
        updatePixelizeParameters |= ImGui::RadioButton("Pixelize", &currentShader, 3);
        ImGui::SameLine();
        updatePosterizeParameters |= ImGui::RadioButton("Posterize", &currentShader, 4);
        
        updatePhongParameters |= (updateFogParameters || updateOutlineParameters || updatePixelizeParameters || updatePosterizeParameters);
        
        ImGui::Text("---------------------- Phong ----------------------");
        updatePhongParameters |= ImGui::SliderFloat("ambient", &ambient, 0, 1, "%.2f");
        updatePhongParameters |= ImGui::SliderFloat("kd", &kd, 0, 1, "%.2f");
        updatePhongParameters |= ImGui::SliderFloat("ks", &ks, 0, 1, "%.2f");
        if (currentShader == 1) {
            ImGui::Text("----------------------- Fog -----------------------");
            updateFogParameters |= ImGui::SliderFloat("Fog Intensity", &fogB, 0, 1, "%.2f");
        }
        if (currentShader == 2) {
            ImGui::Text("--------------------- Outline ---------------------");
        }
//        if (ImGui::Button("Show normal map")) {
//            //normalMapButton = !normalMapButton;
//        }
//        ImGui::SameLine();
//        if (ImGui::Button("Show height map")) {
//            //heightMapButton = !heightMapButton;
//        }
//        if (normalMapButton) {
//            ImGui::SetNextWindowSize(ImVec2(271.0f, 291.0f), ImGuiCond_Once);
//            ImGui::SetNextWindowCollapsed(0, ImGuiCond_Once);
//            ImGui::SetNextWindowPos(ImVec2(460.0f, 50.0f), ImGuiCond_Once);
//            ImGui::SetNextWindowBgAlpha(1.0f);
//            if (ImGui::Begin("Normal Map")) {
//                auto tex = reinterpret_cast<ImTextureID>(static_cast<std::uintptr_t>(normalmap->getHandle()));
//                auto wsize = ImGui::GetWindowSize();
//                wsize.x -= 15;
//                wsize.y -= 35;
//                ImGui::Image(tex, wsize, ImVec2(0, 1), ImVec2(1, 0));
//            }
//            ImGui::End();
//        }
//        if (heightMapButton) {
//            ImGui::SetNextWindowSize(ImVec2(271.0f, 291.0f), ImGuiCond_Once);
//            ImGui::SetNextWindowCollapsed(0, ImGuiCond_Once);
//            ImGui::SetNextWindowPos(ImVec2(740.0f, 50.0f), ImGuiCond_Once);
//            ImGui::SetNextWindowBgAlpha(1.0f);
//            if (ImGui::Begin("Height Map")) {
//                auto tex = reinterpret_cast<ImTextureID>(static_cast<std::uintptr_t>(heightmap->getHandle()));
//                auto wsize = ImGui::GetWindowSize();
//                wsize.x -= 15;
//                wsize.y -= 35;
//                ImGui::Image(tex, wsize, ImVec2(0, 1), ImVec2(1, 0));
//            }
//            ImGui::End();
//        }
//        ImGui::Text("----------------------- Bonus -----------------------");
//        updateMapping |= ImGui::Checkbox("Displacement", &useDisplacement);
//        ImGui::SameLine();
//        updateMapping |= ImGui::Checkbox("Parallax", &useParallax);
//        ImGui::Text("----------------------- Other -----------------------");
//        ImGui::Text("Current framerate: %.0f", ImGui::GetIO().Framerate);
    }
    ImGui::End();
}

void renderGUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    renderMainPanel();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
