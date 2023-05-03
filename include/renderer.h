/*
 * Author: Emoy Kim
 * E-mail: emoy.kim_AT_gmail.com
 * 
 * This code is a free software; it can be freely used, changed and redistributed.
 * If you use any version of the code, please reference the code.
 * 
 */

#pragma once

#include "base.h"
#include "light.h"
#include "object.h"

class RendererGL
{
public:
   RendererGL(const RendererGL&) = delete;
   RendererGL(const RendererGL&&) = delete;
   RendererGL& operator=(const RendererGL&) = delete;
   RendererGL& operator=(const RendererGL&&) = delete;


   RendererGL();
   ~RendererGL();

   void play();

private:
   inline static RendererGL* Renderer = nullptr;
   GLFWwindow* Window;
   int FrameWidth;
   int FrameHeight;
   GLuint FBO;
   GLuint DepthTextureID;
   glm::ivec2 ClickedPoint;
   float LightTheta;
   std::unique_ptr<CameraGL> MainCamera;
   std::unique_ptr<CameraGL> LightCamera;
   std::unique_ptr<ShaderGL> ObjectShader;
   std::unique_ptr<ShaderGL> ShadowShader;
   std::unique_ptr<ObjectGL> GroundObject;
   std::unique_ptr<ObjectGL> TigerObject;
   std::unique_ptr<ObjectGL> PandaObject;
   std::unique_ptr<LightGL> Lights;

   void registerCallbacks() const;
   void initialize();

   static void printOpenGLInformation();
   static void cleanup(GLFWwindow* window);
   static void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
   static void cursor(GLFWwindow* window, double xpos, double ypos);
   static void mouse(GLFWwindow* window, int button, int action, int mods);
   static void mousewheel(GLFWwindow* window, double xoffset, double yoffset);
   static void reshape(GLFWwindow* window, int width, int height);

   void setLights() const;
   void setGroundObject() const;
   void setTigerObject() const;
   void setPandaObject() const;
   void setDepthFrameBuffer();

   void drawGroundObject(ShaderGL* shader, CameraGL* camera) const;
   void drawTigerObject(ShaderGL* shader, CameraGL* camera) const;
   void drawPandaObject(ShaderGL* shader, CameraGL* camera) const;
   void drawDepthMapFromLightView(int light_index) const;
   void drawShadow(int light_index) const;
   void render() const;
};