#pragma once

#include "Camera.h"
//#include "Cube.h"
#include "Skeleton.h"
#include "Shader.h"
#include "core.h"
#include "Skin.h"
#include "AnimationClip.h"
#include "Cloth.h"

class Window {
public:
    // Window Properties
    static int width;
    static int height;
    static const char* windowTitle;

    static std::string skelFilename;


    // Objects to render
    //static Cube* cube;
    static Skeleton* skeleton;
    static Skin* skin;

    // Animation - project 3
    static AnimationClip* animation;
    static float animationTime;
    static bool playAnimation;

    //Project 4
    static Cloth* cloth;

    // Shader Program
    static GLuint shaderProgram;

    // Act as Constructors and desctructors
    static bool initializeProgram();
    static bool initializeObjects(const char* filename = "test.skel");
    static void cleanUp();

    // for the Window
    static GLFWwindow* createWindow(int width, int height);
    static void resizeCallback(GLFWwindow* window, int width, int height);

    // update and draw functions
    static void idleCallback();
    static void displayCallback(GLFWwindow*);

    // helper to reset the camera
    static void resetCamera();

    // callbacks - for interaction
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_callback(GLFWwindow* window, int button, int action, int mods);
    static void cursor_callback(GLFWwindow* window, double currX, double currY);
};