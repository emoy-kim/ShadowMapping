#include "Renderer.h"

RendererGL* RendererGL::Renderer = nullptr;
RendererGL::RendererGL() : Window( nullptr ), ClickedPoint( -1, -1 ), MainCamera( nullptr )
{
   Renderer = this;
   MainCamera = new CameraGL();

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
   ObjectShader.setShader(
      "Shaders/VertexShaderForObject.glsl",
      "Shaders/FragmentShaderForObject.glsl"
   );
}

void RendererGL::cleanup(GLFWwindow* window)
{
   glDeleteProgram( ObjectShader.ShaderProgram );
   glDeleteVertexArrays( 1, &GroundObject.ObjVAO );
   glDeleteBuffers( 1, &GroundObject.ObjVBO );
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
   const vec4 light_position(256.0f, 500.0f, 256.0f, 1.0f);
   const vec4 ambient_color(0.3f, 0.3f, 0.3f, 1.0f);
   const vec4 diffuse_color(0.7f, 0.7f, 0.7f, 1.0f);
   const vec4 specular_color(0.9f, 0.9f, 0.9f, 1.0f);
   Lights.addLight( light_position, ambient_color, diffuse_color, specular_color );
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

void RendererGL::drawGroundObject()
{
   glDisable( GL_DEPTH );
   glEnable( GL_BLEND );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

   glUseProgram( ObjectShader.ShaderProgram );

   const mat4 to_world = mat4(1.0f);
   const mat4 model_view_projection = MainCamera->ProjectionMatrix * MainCamera->ViewMatrix * to_world;
   glUniformMatrix4fv( ObjectShader.Location.World, 1, GL_FALSE, &to_world[0][0] );
   glUniformMatrix4fv( ObjectShader.Location.View, 1, GL_FALSE, &MainCamera->ViewMatrix[0][0] );
   glUniformMatrix4fv( ObjectShader.Location.Projection, 1, GL_FALSE, &MainCamera->ProjectionMatrix[0][0] );
   glUniformMatrix4fv( ObjectShader.Location.ModelViewProjection, 1, GL_FALSE, &model_view_projection[0][0] );
   
   GroundObject.transferUniformsToShader( ObjectShader );
   Lights.transferUniformsToShader( ObjectShader );

   glBindTextureUnit( ObjectShader.Location.Texture[0].first, GroundObject.TextureID[0] );
   glBindVertexArray( GroundObject.ObjVAO );
   glDrawArrays( GroundObject.DrawMode, 0, GroundObject.VerticesCount );

   glEnable( GL_DEPTH );
   glDisable( GL_BLEND );
}

void RendererGL::render()
{
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   drawGroundObject();

   glBindVertexArray( 0 );
   glUseProgram( 0 );
}

void RendererGL::play()
{
   if (glfwWindowShouldClose( Window )) initialize();

   setLights();
   setGroundObject();
   //ObjectShader.addUniformLocation( ObjectShader.ShaderProgram, "WhichObject" );

   ObjectShader.setUniformLocations( Lights.TotalLightNum );

   while (!glfwWindowShouldClose( Window )) {
      render();

      glfwSwapBuffers( Window );
      glfwPollEvents();
   }
   glfwDestroyWindow( Window );
}