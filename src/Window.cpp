#include "Window.h"
#include "Skeleton.h"
#include "Skin.h"
#include <string>
#include "AnimationClip.h"


#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Window Properties
int Window::width;
int Window::height;
const char* Window::windowTitle = "Skeleton Viewer";

// .skel filename
std::string Window::skelFilename = "test.skel";

// Objects to render - initialize both to nullptr
Skeleton* Window::skeleton = nullptr;
Skin* Window::skin = nullptr;

// Animation - project 3
AnimationClip* Window::animation = nullptr;
float Window::animationTime = 0.0f;
bool Window::playAnimation = true;

//Project 4
Cloth* Window::cloth = nullptr;

// Camera Properties
Camera* Cam;

// Interaction Variables
bool LeftDown, RightDown;
int MouseX, MouseY;

// The shader program id
GLuint Window::shaderProgram;

// Constructors and desctructors
bool Window::initializeProgram() {
    // Create a shader program with a vertex shader and a fragment shader.
    shaderProgram = LoadShaders("shaders/shader.vert", "shaders/shader.frag");

    // Check the shader program.
    if (!shaderProgram) {
        std::cerr << "Failed to initialize shader program" << std::endl;
        return false;
    }

    return true;
}

bool Window::initializeObjects(const char* filename) {
    std::string fname(filename);
    std::string ext = fname.substr(fname.find_last_of('.') + 1);

    if (ext == "skel") {
        skeleton = new Skeleton();
        if (!skeleton->Load(filename)) {
            std::cerr << "Failed to load skeleton file: " << filename << std::endl;
            return false;
        }
        std::cout << "Successfully loaded skeleton: " << filename << std::endl;
    }
    else if (ext == "skin") {
        skin = new Skin();
        if (!skin->Load(filename)) {
            std::cerr << "Failed to load skin file: " << filename << std::endl;
            return false;
        }
        std::cout << "Successfully loaded skin: " << filename << std::endl;
    }
    else if (ext == "anim") {
        animation = new AnimationClip();
        if (!animation->Load(filename)) {
            std::cerr << "Failed to load animation file: " << filename << std::endl;
            return false;
        }
        std::cout << "Successfully loaded animation: " << filename << std::endl;
    }
    else {
        std::cerr << "Unknown file type: " << filename << std::endl;
        return false;
    }

    return true;
}

void Window::cleanUp() {
    // Clean up skeleton if it exists
    if (skeleton) {
        delete skeleton;
        skeleton = nullptr;
    }

    // Clean up skin if it exists
    if (skin) {
        delete skin;
        skin = nullptr;
    }

    // Clean up animation if it exists
    if (animation) {
        delete animation;
        animation = nullptr;
    }

    // Clean up cloth if it exists
    if (cloth) {
        delete cloth;
        cloth = nullptr;
    }

    glDeleteProgram(shaderProgram);
}

// for the Window
GLFWwindow* Window::createWindow(int width, int height) {
    // Initialize GLFW.
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return NULL;
    }

    // 4x antialiasing.
    glfwWindowHint(GLFW_SAMPLES, 16);

    // Create the GLFW window.
    GLFWwindow* window = glfwCreateWindow(width, height, windowTitle, NULL, NULL);

    // Check if the window could not be created.
    if (!window) {
        std::cerr << "Failed to open GLFW window." << std::endl;
        glfwTerminate();
        return NULL;
    }

    // Make the context of the window.
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewInit();

    // Set swap interval to 1.
    glfwSwapInterval(0);

    // set up the camera
    Cam = new Camera();
    Cam->SetAspect(float(width) / float(height));

    // initialize the interaction variables
    LeftDown = RightDown = false;
    MouseX = MouseY = 0;

    // Call the resize callback to make sure things get drawn immediately.
    Window::resizeCallback(window, width, height);

    return window;
}

void Window::resizeCallback(GLFWwindow* window, int width, int height) {
    Window::width = width;
    Window::height = height;
    // Set the viewport size.
    glViewport(0, 0, width, height);

    Cam->SetAspect(float(width) / float(height));
}

// update and draw functions
void Window::idleCallback() {
    // Update camera
    Cam->Update();

    // Update animation time if playing (NEW for Project 3)
    if (animation && playAnimation) {
        static double lastTime = glfwGetTime();
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        animationTime += deltaTime;

        // Loop animation, cannot show extrapolation in/out
        
        float duration = animation->GetDuration();
        if (duration > 0.0f) {
            while (animationTime > animation->GetEndTime()) {
                animationTime -= duration;
            }
            while (animationTime < animation->GetStartTime()) {
                animationTime += duration;
            }
        }
        
    }

    // Apply animation to skeleton if both exist (NEW for Project 3)
    if (animation && skeleton) {
        animation->Evaluate(animationTime, skeleton);
    }

    // Update skeleton if it exists (computes world matrices for all joints)
    if (skeleton) {
        skeleton->Update();
    }

    // Update skin - pass skeleton pointer (or nullptr if no skeleton)
    // If skeleton exists: performs skinning using joint world matrices
    // If skeleton is null: just copies bind pose to current pose
    if (skin) {
        skin->Update(skeleton);
    }

    // Project 4
    if (cloth) {
        static double lastClothTime = glfwGetTime();
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastClothTime);
        lastClothTime = currentTime;

        // Fix frame rate
        deltaTime = glm::min(deltaTime, 1.0f / 600.0f); 

        for (int i = 0; i < 10; i++) {
            cloth->Update(deltaTime);
        }

        
    }


}

void Window::displayCallback(GLFWwindow* window) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (skeleton) {
        skeleton->Draw(Cam->GetViewProjectMtx(), shaderProgram);
    }
        
    if (skin)
        skin->Draw(Cam->GetViewProjectMtx(), shaderProgram);

    // Project 4 
    if (cloth) {
        cloth->Draw(Cam->GetViewProjectMtx(), shaderProgram);
    }

    // ImGui frame setup
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // DOF Controls window (only if skeleton exists)
    if (skeleton) {
        ImGui::Begin("DOF Controls");
        skeleton->DrawDOFGui();
        ImGui::End();
    }

    // Cloth Controls window (only if cloth exists)
    if (cloth) {
        ImGui::Begin("Cloth Controls");

        // Wind controls
        glm::vec3 wind = cloth->getWind();
        float windSpeed[3] = { wind.x, wind.y, wind.z };
        if (ImGui::SliderFloat3("Wind Velocity", windSpeed, -20.0f, 20.0f)) {
            cloth->setWind(glm::vec3(windSpeed[0], windSpeed[1], windSpeed[2]));
        }

        // Ground height
        float ground = cloth->getGround();
        if (ImGui::SliderFloat("Ground Height", &ground, -5.0f, 5.0f)) {
            cloth->setGround(ground);
        }

        ImGui::Separator();
        ImGui::Text("Fixed Particles Control");

        // Translation controls 
        static float translateX = 0.0f;
        static float translateY = 0.0f;
        static float translateZ = 0.0f;
        static float prevX = 0.0f, prevY = 0.0f, prevZ = 0.0f;

        if (ImGui::SliderFloat("Translate X", &translateX, -5.0f, 5.0f)) {
            float deltaX = translateX - prevX;
            cloth->TranslateFixedRow(glm::vec3(deltaX, 0.0f, 0.0f));
            prevX = translateX;
        }

        if (ImGui::SliderFloat("Translate Y", &translateY, -5.0f, 5.0f)) {
            float deltaY = translateY - prevY;
            cloth->TranslateFixedRow(glm::vec3(0.0f, deltaY, 0.0f));
            prevY = translateY;
        }

        if (ImGui::SliderFloat("Translate Z", &translateZ, -5.0f, 5.0f)) {
            float deltaZ = translateZ - prevZ;
            cloth->TranslateFixedRow(glm::vec3(0.0f, 0.0f, deltaZ));
            prevZ = translateZ;
        }
        ImGui::Separator();

        // Rotation control
        static float rotationX = 0.0f;
        static float rotationY = 0.0f;
        static float rotationZ = 0.0f;
        static float prevRotationX = 0.0f;
        static float prevRotationY = 0.0f;
        static float prevRotationZ = 0.0f;

        if (ImGui::SliderFloat("Rotate X", &rotationX, -180.0f, 180.0f)) {
            float deltaAngle = rotationX - prevRotationX;
            cloth->RotateFixedRow(deltaAngle, glm::vec3(1, 0, 0));
            prevRotationX = rotationX;
        }

        if (ImGui::SliderFloat("Rotate Y", &rotationY, -180.0f, 180.0f)) {
            float deltaAngle = rotationY - prevRotationY;
            cloth->RotateFixedRow(deltaAngle, glm::vec3(0, 1, 0));
            prevRotationY = rotationY;
        }

        if (ImGui::SliderFloat("Rotate Z", &rotationZ, -180.0f, 180.0f)) {
            float deltaAngle = rotationZ - prevRotationZ;
            cloth->RotateFixedRow(deltaAngle, glm::vec3(0, 0, 1));
            prevRotationZ = rotationZ;
        }

        ImGui::Separator();


        // Release button
        if (ImGui::Button("Release All Fixed Particles")) {
            cloth->ReleaseAllFixed();
        }

        // Reset button 
        if (ImGui::Button("Reset")) {
            delete cloth;               
            cloth = new Cloth();

            rotationX = rotationY = rotationZ = 0.0f;
            prevRotationX = prevRotationY = prevRotationZ = 0.0f;

            translateX = translateY = translateZ = 0.0f;
            prevX = prevY = prevZ = 0.0f;
        }

        ImGui::End();
    }

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Swap buffers and poll events
    glfwSwapBuffers(window);
    glfwPollEvents();
}

// helper to reset the camera
void Window::resetCamera() {
    Cam->Reset();
    Cam->SetAspect(float(Window::width) / float(Window::height));
}

// callbacks - for Interaction
void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    /*
     * TODO: Modify below to add your key callbacks.
     */

     // Check for a key press.
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            // Close the window. This causes the program to also terminate.
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;

        case GLFW_KEY_R:
            resetCamera();
            break;

        default:
            break;
        }
    }
}

void Window::mouse_callback(GLFWwindow* window, int button, int action, int mods) {
    // Ignore mouse clicks when ImGui is active
    if (ImGui::GetIO().WantCaptureMouse)
        return;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        LeftDown = (action == GLFW_PRESS);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        RightDown = (action == GLFW_PRESS);
    }
}

void Window::cursor_callback(GLFWwindow* window, double currX, double currY) {
    if (ImGui::GetIO().WantCaptureMouse)
        return;

    int maxDelta = 100;
    int dx = glm::clamp((int)currX - MouseX, -maxDelta, maxDelta);
    int dy = glm::clamp(-((int)currY - MouseY), -maxDelta, maxDelta);

    MouseX = (int)currX;
    MouseY = (int)currY;

    // Move camera
    // NOTE: this should really be part of Camera::Update()
    if (LeftDown) {
        const float rate = 1.0f;
        Cam->SetAzimuth(Cam->GetAzimuth() + dx * rate);
        Cam->SetIncline(glm::clamp(Cam->GetIncline() - dy * rate, -90.0f, 90.0f));
    }
    if (RightDown) {
        const float rate = 0.005f;
        float dist = glm::clamp(Cam->GetDistance() * (1.0f - dx * rate), 0.01f, 1000.0f);
        Cam->SetDistance(dist);
    }
}