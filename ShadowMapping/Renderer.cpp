#include "Renderer.h"

RendererGL* RendererGL::Renderer = nullptr;
RendererGL::RendererGL() : Window( nullptr ), ClickedPoint( -1, -1 ), FBO( 0 ), DepthTextureID( 0 )
{
   Renderer = this;
   MainCamera = make_shared<CameraGL>();
   LightCamera = make_shared<CameraGL>();
   ObjectShader = make_shared<ShaderGL>();
   ShadowShader = make_shared<ShaderGL>();

   initialize();
   printOpenGLInformation();
}

RendererGL::~RendererGL()
{
   glfwTerminate();
}

void RendererGL::printOpenGLInformation() const
{
   cout << "****************************************************************" << endl;
   cout << " - GLFW version supported: " << glfwGetVersionString() << endl;
   cout << " - GLEW version supported: " << glewGetString( GLEW_VERSION ) << endl;
   cout << " - OpenGL renderer: " << glGetString( GL_RENDERER ) << endl;
   cout << " - OpenGL version supported: " << glGetString( GL_VERSION ) << endl;
   cout << " - OpenGL shader version supported: " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << endl  ;
   cout << "****************************************************************" << endl << endl;
}

void RendererGL::initialize()
{
   if (!glfwInit()) {
      cout << "Cannot Initialize OpenGL..." << endl;
      return;
   }
   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 5 );
   glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

   const int width = 1920;
   const int height = 1080;
   Window = glfwCreateWindow( width, height, "Main Camera", nullptr, nullptr );
   glfwMakeContextCurrent( Window );
   glewInit();
   
   registerCallbacks();
   
   glEnable( GL_DEPTH_TEST );
   glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );

   MainCamera->updateWindowSize( width, height );
   LightCamera->updateWindowSize( width, height );
   ObjectShader->setShader(
      "Shaders/VertexShaderForObject.glsl",
      "Shaders/FragmentShaderForObject.glsl"
   );
   ShadowShader->setShader(
      "Shaders/VertexShaderForShadow.glsl",
      "Shaders/FragmentShaderForShadow.glsl"
   );
}

void RendererGL::cleanup(GLFWwindow* window)
{
   if (DepthTextureID != 0) glDeleteTextures( 1, &DepthTextureID );
   if (FBO != 0) glDeleteFramebuffers( 1, &FBO );
   glDeleteVertexArrays( 1, &GroundObject.ObjVAO );
   glDeleteBuffers( 1, &GroundObject.ObjVBO );
   glDeleteVertexArrays( 1, &PandaObject.ObjVAO );
   glDeleteBuffers( 1, &PandaObject.ObjVBO );
   glfwSetWindowShouldClose( window, GLFW_TRUE );
}

void RendererGL::cleanupWrapper(GLFWwindow* window)
{
   Renderer->cleanup( window );
}

void RendererGL::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   if (action != GLFW_PRESS) return;

   switch (key) {
      case GLFW_KEY_UP:
         MainCamera->moveForward();
         break;
      case GLFW_KEY_DOWN:
         MainCamera->moveBackward();
         break;
      case GLFW_KEY_LEFT:
         MainCamera->moveLeft();
         break;
      case GLFW_KEY_RIGHT:
         MainCamera->moveRight();
         break;
      case GLFW_KEY_W:
         MainCamera->moveUp();
         break;
      case GLFW_KEY_S:
         MainCamera->moveDown();
         break;
      case GLFW_KEY_I:
         MainCamera->resetCamera();
         break;
      case GLFW_KEY_L:
         Lights.toggleLightSwitch();
         cout << "Light Turned " << (Lights.isLightOn() ? "On!" : "Off!") << endl;
         break;
      case GLFW_KEY_Q:
      case GLFW_KEY_ESCAPE:
         cleanupWrapper( window );
         break;
      default:
         return;
   }
}

void RendererGL::keyboardWrapper(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   Renderer->keyboard( window, key, scancode, action, mods );
}

void RendererGL::cursor(GLFWwindow* window, double xpos, double ypos)
{
   if (MainCamera->getMovingState()) {
      const auto x = static_cast<float>(round( xpos ));
      const auto y = static_cast<float>(round( ypos ));
      const int dx = static_cast<int>(x - ClickedPoint.x);
      const int dy = static_cast<int>(y - ClickedPoint.y);
      MainCamera->moveForward( -dy );
      MainCamera->rotateAroundWorldY( -dx );

      if (glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_RIGHT ) == GLFW_PRESS) {
         MainCamera->pitch( -dy );
      }

      ClickedPoint.x = x;
      ClickedPoint.y = y;
   }
}

void RendererGL::cursorWrapper(GLFWwindow* window, double xpos, double ypos)
{
   Renderer->cursor( window, xpos, ypos );
}

void RendererGL::mouse(GLFWwindow* window, int button, int action, int mods)
{
   if (button == GLFW_MOUSE_BUTTON_LEFT) {
      const bool moving_state = action == GLFW_PRESS;
      if (moving_state) {
         double x, y;
         glfwGetCursorPos( window, &x, &y );
         ClickedPoint.x = static_cast<float>(round( x ));
         ClickedPoint.y = static_cast<float>(round( y ));
      }
      MainCamera->setMovingState( moving_state );
   }
}

void RendererGL::mouseWrapper(GLFWwindow* window, int button, int action, int mods)
{
   Renderer->mouse( window, button, action, mods );
}

void RendererGL::mousewheel(GLFWwindow* window, double xoffset, double yoffset)
{
   if (yoffset >= 0.0) MainCamera->zoomIn();
   else MainCamera->zoomOut();
}

void RendererGL::mousewheelWrapper(GLFWwindow* window, double xoffset, double yoffset)
{
   Renderer->mousewheel( window, xoffset, yoffset );
}

void RendererGL::reshape(GLFWwindow* window, int width, int height)
{
   MainCamera->updateWindowSize( width, height );
   glViewport( 0, 0, width, height );
}

void RendererGL::reshapeWrapper(GLFWwindow* window, int width, int height)
{
   Renderer->reshape( window, width, height );
}

void RendererGL::registerCallbacks() const
{
   glfwSetWindowCloseCallback( Window, cleanupWrapper );
   glfwSetKeyCallback( Window, keyboardWrapper );
   glfwSetCursorPosCallback( Window, cursorWrapper );
   glfwSetMouseButtonCallback( Window, mouseWrapper );
   glfwSetScrollCallback( Window, mousewheelWrapper );
   glfwSetFramebufferSizeCallback( Window, reshapeWrapper );
}

void RendererGL::setLights()
{  
   const vec4 light_position(256.0f, 500.0f, 200.0f, 1.0f);
   const vec4 ambient_color(0.3f, 0.3f, 0.3f, 1.0f);
   const vec4 diffuse_color(0.7f, 0.7f, 0.7f, 1.0f);
   const vec4 specular_color(0.9f, 0.9f, 0.9f, 1.0f);
   const vec3 spotlight_direction(-1.0f, -1.0f, -1.5f);
   Lights.addLight( light_position, ambient_color, diffuse_color, specular_color, spotlight_direction );
}

void RendererGL::setGroundObject()
{
   const float size = 512.0f;
   vector<vec3> ground_vertices;
   ground_vertices.emplace_back( 0.0f, 0.0f, 0.0f );
   ground_vertices.emplace_back( size, 0.0f, 0.0f );
   ground_vertices.emplace_back( size, 0.0f, size );
   
   ground_vertices.emplace_back( 0.0f, 0.0f, 0.0f );
   ground_vertices.emplace_back( size, 0.0f, size );
   ground_vertices.emplace_back( 0.0f, 0.0f, size );


   vector<vec3> ground_normals;
   ground_normals.emplace_back( 0.0f, 1.0f, 0.0f );
   ground_normals.emplace_back( 0.0f, 1.0f, 0.0f );
   ground_normals.emplace_back( 0.0f, 1.0f, 0.0f );
   
   ground_normals.emplace_back( 0.0f, 1.0f, 0.0f );
   ground_normals.emplace_back( 0.0f, 1.0f, 0.0f );
   ground_normals.emplace_back( 0.0f, 1.0f, 0.0f );

   vector<vec2> ground_textures;
   ground_textures.emplace_back( 0.0f, 0.0f );
   ground_textures.emplace_back( 1.0f, 0.0f );
   ground_textures.emplace_back( 1.0f, 1.0f );
   
   ground_textures.emplace_back( 0.0f, 0.0f );
   ground_textures.emplace_back( 1.0f, 1.0f );
   ground_textures.emplace_back( 0.0f, 1.0f );

      
   GroundObject.setObject( GL_TRIANGLES, ground_vertices, ground_normals, ground_textures, "Samples/sand.jpg" );
   GroundObject.setDiffuseReflectionColor( { 1.0f, 1.0f, 1.0f, 1.0f } );
}

void RendererGL::setPandaObject()
{
   PandaObject.setObject( GL_TRIANGLES, "Samples/Panda/panda.obj", "Samples/Panda/panda.png" );
   PandaObject.setDiffuseReflectionColor( { 1.0f, 1.0f, 1.0f, 1.0f } );
}

void RendererGL::setDepthFrameBuffer()
{
   glBindFramebuffer( GL_FRAMEBUFFER, FBO );
   glClearBufferfi( GL_DEPTH, 0, 1.0f, 0 );
   glBindFramebuffer( GL_FRAMEBUFFER, 0 );

   glCreateTextures( GL_TEXTURE_2D, 1, &DepthTextureID );
   glTextureStorage2D( DepthTextureID, 1, GL_DEPTH_COMPONENT32F, MainCamera->getCameraWidth(), MainCamera->getCameraHeight() );
   glTextureParameteri( DepthTextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
   glTextureParameteri( DepthTextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

   glCreateFramebuffers( 1, &FBO );
   glNamedFramebufferTexture2DEXT( FBO, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthTextureID, 0 );
}

void RendererGL::drawGroundObject(ShaderGL* shader, CameraGL* camera)
{
   const mat4 to_world = mat4(1.0f);
   const mat4 model_view_projection = camera->ProjectionMatrix * camera->ViewMatrix * to_world;
   glUniformMatrix4fv( shader->Location.World, 1, GL_FALSE, &to_world[0][0] );
   glUniformMatrix4fv( shader->Location.View, 1, GL_FALSE, &camera->ViewMatrix[0][0] );
   glUniformMatrix4fv( shader->Location.Projection, 1, GL_FALSE, &camera->ProjectionMatrix[0][0] );
   glUniformMatrix4fv( shader->Location.ModelViewProjection, 1, GL_FALSE, &model_view_projection[0][0] );
   
   GroundObject.transferUniformsToShader( shader );

   glBindTextureUnit( shader->Location.Texture[0].first, GroundObject.TextureID[0] );
   glBindVertexArray( GroundObject.ObjVAO );
   glDrawArrays( GroundObject.DrawMode, 0, GroundObject.VerticesCount );
}

void RendererGL::drawPandaObject(ShaderGL* shader, CameraGL* camera)
{
   const mat4 to_world = translate( mat4(1.0f), vec3(250.0f, -5.0f, 150.0f) ) *
      scale( mat4(1.0f), vec3( 50.0f, 50.0f, 50.0f ) );
   const mat4 model_view_projection = camera->ProjectionMatrix * camera->ViewMatrix * to_world;
   glUniformMatrix4fv( shader->Location.World, 1, GL_FALSE, &to_world[0][0] );
   glUniformMatrix4fv( shader->Location.View, 1, GL_FALSE, &camera->ViewMatrix[0][0] );
   glUniformMatrix4fv( shader->Location.Projection, 1, GL_FALSE, &camera->ProjectionMatrix[0][0] );
   glUniformMatrix4fv( shader->Location.ModelViewProjection, 1, GL_FALSE, &model_view_projection[0][0] );
   
   PandaObject.transferUniformsToShader( shader );

   glBindTextureUnit( shader->Location.Texture[0].first, PandaObject.TextureID[0] );
   glBindVertexArray( PandaObject.ObjVAO );
   glDrawArrays( PandaObject.DrawMode, 0, PandaObject.VerticesCount );
}

void RendererGL::drawDepthMapFromLightView(const uint& light_index)
{
   glEnable( GL_DEPTH );
   glDisable( GL_BLEND );

   glBindFramebuffer( GL_FRAMEBUFFER, FBO );
   glClearBufferfi( GL_DEPTH, 0, 1.0f, 0 );

   glUseProgram( ObjectShader->ShaderProgram );

   const vec3 light_position = vec3(Lights.getLightPosition( light_index ));
   LightCamera->updateCameraPosition(
      light_position,
      light_position + Lights.getSpotlightDirection( light_index ),
      vec3(0.0f, 1.0f, 0.0f)
   );
   
   drawGroundObject( ObjectShader.get(), LightCamera.get() );
   drawPandaObject( ObjectShader.get(), LightCamera.get() );

   glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void RendererGL::drawShadow(const uint& light_index)
{
   glDisable( GL_DEPTH );
   glEnable( GL_BLEND );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

   glUseProgram( ShadowShader->ShaderProgram );

   Lights.transferUniformsToShader( ShadowShader.get() );
   glUniform1i( ShadowShader->CustomLocations["LightIndex"], light_index );

   const mat4 model_view_projection = LightCamera->ProjectionMatrix * LightCamera->ViewMatrix;
   glUniformMatrix4fv( ShadowShader->CustomLocations["LightModelViewProjectionMatrix"], 1, GL_FALSE, &model_view_projection[0][0] );

   glBindTextureUnit( ShadowShader->Location.Texture[1].first, DepthTextureID );
   drawGroundObject( ShadowShader.get(), MainCamera.get() );

   glEnable( GL_DEPTH );
   glDisable( GL_BLEND );
}

void RendererGL::render()
{
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   drawDepthMapFromLightView( 0 );
   drawShadow( 0 );
     
   glUseProgram( ObjectShader->ShaderProgram );
   Lights.transferUniformsToShader( ObjectShader.get() );
   drawPandaObject( ObjectShader.get(), MainCamera.get() );

   glBindVertexArray( 0 );
   glUseProgram( 0 );
}

void RendererGL::play()
{
   if (glfwWindowShouldClose( Window )) initialize();

   setLights();
   setGroundObject();
   setPandaObject();
   setDepthFrameBuffer();

   ObjectShader->setUniformLocations( Lights.TotalLightNum );
   ShadowShader->setUniformLocations( 1 );
   ShadowShader->addUniformLocation( ShadowShader->ShaderProgram, "LightIndex" );
   ShadowShader->addUniformLocation( ShadowShader->ShaderProgram, "LightModelViewProjectionMatrix" );

   while (!glfwWindowShouldClose( Window )) {
      render();

      glfwSwapBuffers( Window );
      glfwPollEvents();
   }
   glfwDestroyWindow( Window );
}