/*
 * Author: Emoy Kim
 * E-mail: emoy.kim_AT_gmail.com
 * 
 * This code is a free software; it can be freely used, changed and redistributed.
 * If you use any version of the code, please reference the code.
 * 
 */

#pragma once

#include "_Common.h"

#include "Camera.h"
#include "Object.h"

class RendererGL
{
   static RendererGL* Renderer;
   GLFWwindow* Window;

   vec2 ClickedPoint;
   float LightTheta;

   shared_ptr<CameraGL> MainCamera;
   shared_ptr<CameraGL> LightCamera;
   shared_ptr<ShaderGL> ObjectShader;
   shared_ptr<ShaderGL> ShadowShader;
   
   LightGL Lights;
   ObjectGL GroundObject, TigerObject, PandaObject;

   GLuint FBO;
   GLuint DepthTextureID;

   void registerCallbacks() const;
   void initialize();
   void printOpenGLInformation() const;

   void cleanup(GLFWwindow* window);
   void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
   void cursor(GLFWwindow* window, double xpos, double ypos);
   void mouse(GLFWwindow* window, int button, int action, int mods);
   void mousewheel(GLFWwindow* window, double xoffset, double yoffset);
   void reshape(GLFWwindow* window, int width, int height);
   static void cleanupWrapper(GLFWwindow* window);
   static void keyboardWrapper(GLFWwindow* window, int key, int scancode, int action, int mods);
   static void cursorWrapper(GLFWwindow* window, double xpos, double ypos);
   static void mouseWrapper(GLFWwindow* window, int button, int action, int mods);
   static void mousewheelWrapper(GLFWwindow* window, double xoffset, double yoffset);
   static void reshapeWrapper(GLFWwindow* window, int width, int height);

   void setLights();
   void setGroundObject();
   void setTigerObject();
   void setPandaObject();
   void setDepthFrameBuffer();

   void drawGroundObject(ShaderGL* shader, CameraGL* camera);
   void drawTigerObject(ShaderGL* shader, CameraGL* camera);
   void drawPandaObject(ShaderGL* shader, CameraGL* camera);
   void drawDepthMapFromLightView(const uint& light_index);
   void drawShadow(const uint& light_index);
   void render();


public:
   RendererGL(const RendererGL&) = delete;
   RendererGL(const RendererGL&&) = delete;
   RendererGL& operator=(const RendererGL&) = delete;
   RendererGL& operator=(const RendererGL&&) = delete;


   RendererGL();
   ~RendererGL();

   void play();
};