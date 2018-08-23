#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <iostream>

#include <stack>
#include <map>

#include <aurora/window.h>
#include <aurora/gfx.h>
#include <aurora/input.h>
#include "input.h"
#include "input_keyboard_mouse_win32.h"

#include <aurora/timer.h>

#include "lib/audio/audio_mixer_3d.h"

#include "lib/event.h"

#include <util/imgui_wrapper.h>
#include <util/imgui_console.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3.h>
#include <glfw/glfw3native.h>

#include <asset.h>

#include <common.h>

#include <util/frame_graph.h>

#include "scene_controller.h"

#include "debug_draw.h"

inline void FramebufferResizeCallback(GLFWwindow* win, int width, int height)
{
    Common.frameSize.x = width;
    Common.frameSize.y = height;
}

class GameState
{
public:    
    virtual ~GameState(){}

    static SceneController& GetSceneController() { return sceneController; }

    static void Init()
    {
        if(!glfwInit())
        {
            std::cout << "glfwInit() failed" << std::endl;
            return;
        }
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        
        window = glfwCreateWindow(1280, 720, "qwe", NULL, NULL);
        if(!window)
        {
            glfwTerminate();
            std::cout << "failed to create a window" << std::endl;
            return;
        }
        glfwSetFramebufferSizeCallback(window, &FramebufferResizeCallback);
        Common.frameSize.x = 1280;
        Common.frameSize.y = 720;
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        WGLEXTLoadFunctions();
        GLEXTLoadFunctions();

        LOG("GL_VENDOR    : " << glGetString(GL_VENDOR));
        LOG("GL_RENDERER  : " << glGetString(GL_RENDERER));
        LOG("GL_VERSION   : " << glGetString(GL_VERSION));
        LOG("GLSL_VERSION : " << glGetString(GL_SHADING_LANGUAGE_VERSION));

        glDisable(GL_CULL_FACE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        DebugDraw::Init();

        audioMixer.Init(48000, 16);

        keyboardWin32 = new InputKeyboardMouseWin32(window);
        //mouseWin32 = new InputMouseWin32(window);
        input.AddDevice(keyboardWin32);
        input.Init();

        sceneController.Init();

        deltaTime = 0.0f;

        ImGuiInit();

        Update();
    }
    static ImGuiDbgConsole dbgConsole;
    
    static bool Update()
    {
        bool result;
        timer.Start();

        result = glfwWindowShouldClose(window) == 0;
        glfwPollEvents();

        input.Update();

        FrameStart(Job());
        AudioMix(Job());
        sceneController.Update();

        glDisable(GL_CULL_FACE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

        bool consoleOpen = false;
        bool profOverlay = true;
        //dbgConsole.Draw("Dev console", &consoleOpen);
        ShowProfOverlay(&profOverlay, (int)(1.0f / deltaTime), 1);
        //ShowFpsPlot((int)(1.0f / deltaTime));
        ImGuiDraw();
        glfwSwapBuffers(window);
/*
        if(result)
        {
            Job* job_frameStart = frameGraph.Add(
                &FrameStart, 0, 0, AFFINITY_MAIN_THREAD
            );
            Job* job_audioMix = frameGraph.Add(
                &AudioMix, 0
            );
            Job* job_updateState = frameGraph.Add(
                &UpdateState, 0, job_frameStart
            );
            Job* job_renderState = frameGraph.Add(
                &RenderState, 
                0,
                job_updateState, 
                AFFINITY_MAIN_THREAD
            );
            
            frameGraph.Run();
        }
        */
        deltaTime = timer.End() / 1000000.0f;
        frameCount++;

        return result;
    }
    
    static void Cleanup()
    {
        ImGuiCleanup();

        audioMixer.Cleanup();

        DebugDraw::Cleanup();

        glfwDestroyWindow(window);
        glfwTerminate();
    }
    
    static float DeltaTime() { return deltaTime; }
    static uint64_t FrameCount() { return frameCount; }
    
    static FrameGraph* GetFrameGraph() { return &frameGraph; }
    static AudioMixer3D* GetAudioMixer() { return &audioMixer; }
    static Input* GetInput() { return &input; }
    
private:
    static void FrameStart(Job&)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGuiUpdate(DeltaTime());
    }

    static void AudioMix(Job&)
    {
        AudioMixer3D* mixer = GetAudioMixer();
        mixer->Update();
    }

    static SceneController sceneController;

    static FrameGraph frameGraph;

    static uint64_t frameCount;
    static float deltaTime;
    static Au::Timer timer;

    static GLFWwindow* window;
    static AudioMixer3D audioMixer;

    static InputKeyboardMouseWin32* keyboardWin32;
    static Input input;
};

#endif
