#include "Window.h"
#include "core.h"
#include <string>  
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Cloth.h"

void error_callback(int error, const char* description) {
    // Print error.
    std::cerr << description << std::endl;
}

void setup_callbacks(GLFWwindow* window) {
    // Set the error callback.
    glfwSetErrorCallback(error_callback);
    // Set the window resize callback.
    glfwSetWindowSizeCallback(window, Window::resizeCallback);
    // Set the key callback.
    glfwSetKeyCallback(window, Window::keyCallback);
    // Set the mouse and cursor callbacks
    glfwSetMouseButtonCallback(window, Window::mouse_callback);
    glfwSetCursorPosCallback(window, Window::cursor_callback);
}

void setup_opengl_settings() {
    // Enable depth buffering.
    glEnable(GL_DEPTH_TEST);
    // Related to shaders and z value comparisons for the depth buffer.
    glDepthFunc(GL_LEQUAL);
    // Set polygon drawing mode to fill front and back of each polygon.
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // Set clear color to black.
    glClearColor(0.0, 0.0, 0.0, 0.0);
}

void print_versions() {
    // Get info of GPU and supported OpenGL version.
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL version supported: " << glGetString(GL_VERSION)
        << std::endl;
    // If the shading language symbol is defined.
#ifdef GL_SHADING_LANGUAGE_VERSION
    std::cout << "Supported GLSL version is: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
#endif
}

int main(int argc, char** argv) {
    // Initialize file pointers to null - will be set based on command line args
    const char* skelFilename = nullptr;
    const char* skinFilename = nullptr;
    const char* animFilename = nullptr;  // NEW for Project 3
    bool clothSimMode = false; //Project 4
    bool ikMode = false; //Project 5

    // Parse command line arguments to determine which files to load
    if (argc == 1) {
        // No arguments provided - default to just skeleton (Project 1 behavior)
        skelFilename = "test.skel";
        std::cout << "No arguments provided, using default: " << skelFilename << std::endl;
    }
    else {
        // Parse all command line arguments
        for (int i = 1; i < argc; i++) {
            std::string fname(argv[i]);

            if (fname == "ClothSim") {
                clothSimMode = true;
                std::cout << "Initializing Cloth Simulation" << std::endl;
                continue;  // Skip extension check
            }

            if (fname == "IK") {
                ikMode = true;
                continue;
            }

            std::string ext = fname.substr(fname.find_last_of('.') + 1);

            if (ext == "skel") {
                skelFilename = argv[i];
                std::cout << "Found skeleton file: " << skelFilename << std::endl;
            }
            else if (ext == "skin") {
                skinFilename = argv[i];
                std::cout << "Found skin file: " << skinFilename << std::endl;
            }
            else if (ext == "anim") {
                animFilename = argv[i];
                std::cout << "Found animation file: " << animFilename << std::endl;
            }
            else {
                std::cerr << "No valid file passed as argument " << argv[i] << std::endl;
            }
        }
    }

    // Create the GLFW window
    GLFWwindow* window = Window::createWindow(1920, 1080);
    if (!window) exit(EXIT_FAILURE);

    // Print OpenGL and GLSL versions
    print_versions();
    setup_callbacks(window);
    setup_opengl_settings();

    // ---------- ImGui Initialization ----------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Initialize shader program
    if (!Window::initializeProgram()) exit(EXIT_FAILURE);

    // Load skeleton if a .skel file was provided
    if (skelFilename) {
        Window::skeleton = new Skeleton();
        if (!Window::skeleton->Load(skelFilename)) {
            std::cerr << "Failed to load skeleton file: " << skelFilename << std::endl;
            exit(EXIT_FAILURE);
        }
        std::cout << "Successfully loaded skeleton: " << skelFilename << std::endl;
    }

    // Load skin if a .skin file was provided
    if (skinFilename) {
        Window::skin = new Skin();
        if (!Window::skin->Load(skinFilename)) {
            std::cerr << "Failed to load skin file: " << skinFilename << std::endl;
            exit(EXIT_FAILURE);
        }
        std::cout << "Successfully loaded skin: " << skinFilename << std::endl;
    }

    // Load animation if a .anim file was provided (NEW for Project 3)
    if (animFilename) {
        Window::animation = new AnimationClip();
        if (!Window::animation->Load(animFilename)) {
            std::cerr << "Failed to load animation file: " << animFilename << std::endl;
            exit(EXIT_FAILURE);
        }
        std::cout << "Successfully loaded animation: " << animFilename << std::endl;
        Window::animationTime = Window::animation->GetStartTime();
    }

    if (clothSimMode) {
        Window::cloth = new Cloth();
        std::cout << "Cloth simulation initialized" << std::endl;
    }

    if (ikMode) {
        Window::ikChain = new IKChain();
        const float reach = Window::ikChain->GetChainLength();
        Window::ikGoal = glm::vec3(reach * 0.4f, reach * 0.6f, reach * 0.3f);
        Window::ikMode = true;
        std::cout << "IK demo ready";
    }

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        // Display callback renders the scene (skeleton and/or skin)
        Window::displayCallback(window);

        // Idle callback updates the skeleton and performs skinning
        Window::idleCallback();
    }

    // Cleanup resources before exit
    Window::cleanUp();

    // ---------- ImGui Shutdown ----------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}