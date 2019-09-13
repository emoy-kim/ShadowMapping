#include "Renderer.h"

RendererGL* RendererGL::Renderer = nullptr;
RendererGL::RendererGL() :
   Window( nullptr ), ClickedPoint( -1, -1 ), LightTheta( 0.0f ), FBO( 0 ), DepthTextureID( 0 )
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
   const vec4 light_position(256.0f, 500.0f, 512.0f, 1.0f);
   const vec4 ambient_color(1.0f, 1.0f, 1.0f, 1.0f);
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

void RendererGL::setTigerObject()
{
   ifstream file("Samples/Tiger/tiger.txt");
   int polygon_num;
   file >> polygon_num;

   const int vertex_num = polygon_num * 3;
   vector<vec3> tiger_vertices(vertex_num);
   vector<vec3> tiger_normals(vertex_num);
   vector<vec2> tiger_textures(vertex_num);
   for (int i = 0; i < polygon_num; ++i) {
      int triangle_vertex_num;
      file >> triangle_vertex_num;
      for (int v = 0; v < triangle_vertex_num; ++v) {
         const int index = i * triangle_vertex_num + v;
         file >> tiger_vertices[index].x >> tiger_vertices[index].y >> tiger_vertices[index].z;
         file >> tiger_normals[index].x >> tiger_normals[index].y >> tiger_normals[index].z;
         file >> tiger_textures[index].x >> tiger_textures[index].y;
      }
   }
   file.close();

   TigerObject.setObject( GL_TRIANGLES, tiger_vertices, tiger_normals, tiger_textures, "Samples/Tiger/tiger.jpg" );
   TigerObject.setDiffuseReflectionColor( { 1.0f, 1.0f, 1.0f, 1.0f } );
}

void RendererGL::setPandaObject()
{
   PandaObject.setObject( GL_TRIANGLES, "Samples/Panda/panda.obj", "Samples/Panda/panda.png" );
   PandaObject.setDiffuseReflectionColor( { 1.0f, 1.0f, 1.0f, 1.0f } );
}

void RendererGL::setDepthFrameBuffer()
{
   glCreateTextures( GL_TEXTURE_2D, 1, &DepthTextureID );
   glTextureStorage2D( DepthTextureID, 1, GL_DEPTH_COMPONENT32F, LightCamera->getCameraWidth(), LightCamera->getCameraHeight() );
   glTextureParameteri( DepthTextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
   glTextureParameteri( DepthTextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   glTextureParameteri( DepthTextureID, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
   glTextureParameteri( DepthTextureID, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );

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

void RendererGL::drawTigerObject(ShaderGL* shader, CameraGL* camera)
{
   const mat4 to_world =
      translate( mat4(1.0f), vec3(250.0f, 0.0f, 330.0f) ) *
      rotate( mat4(1.0f), radians( 180.0f ), vec3(0.0f, 1.0f, 0.0f) ) * 
      rotate( mat4(1.0f), radians( -90.0f ), vec3(1.0f, 0.0f, 0.0f) ) *
      scale( mat4(1.0f), vec3( 0.3f, 0.3f, 0.3f ) );
   const mat4 model_view_projection = camera->ProjectionMatrix * camera->ViewMatrix * to_world;
   glUniformMatrix4fv( shader->Location.World, 1, GL_FALSE, &to_world[0][0] );
   glUniformMatrix4fv( shader->Location.View, 1, GL_FALSE, &camera->ViewMatrix[0][0] );
   glUniformMatrix4fv( shader->Location.Projection, 1, GL_FALSE, &camera->ProjectionMatrix[0][0] );
   glUniformMatrix4fv( shader->Location.ModelViewProjection, 1, GL_FALSE, &model_view_projection[0][0] );

   glBindTextureUnit( shader->Location.Texture[0].first, TigerObject.TextureID[0] );
   glBindVertexArray( TigerObject.ObjVAO );
   glDrawArrays( TigerObject.DrawMode, 0, TigerObject.VerticesCount );
}

void RendererGL::drawPandaObject(ShaderGL* shader, CameraGL* camera)
{
   const mat4 to_world = 
      translate( mat4(1.0f), vec3(250.0f, -5.0f, 180.0f) ) *
      scale( mat4(1.0f), vec3( 20.0f, 20.0f, 20.0f ) );
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
   glEnable( GL_POLYGON_OFFSET_FILL );
   glPolygonOffset( 2.0f, 4.0f );

   glBindFramebuffer( GL_FRAMEBUFFER, FBO );
   glClearDepth( 1.0f );
   glClear( GL_DEPTH_BUFFER_BIT );

   glUseProgram( ObjectShader->ShaderProgram );

   LightCamera->updateCameraPosition(
      vec3(Lights.getLightPosition( light_index )),
      vec3(256.0f, 0.0f, 10.0f),
      vec3(0.0f, 1.0f, 0.0f)
   );
   
   drawGroundObject( ObjectShader.get(), LightCamera.get() );
   drawTigerObject( ObjectShader.get(), LightCamera.get() );
   drawPandaObject( ObjectShader.get(), LightCamera.get() );

   glBindFramebuffer( GL_FRAMEBUFFER, 0 );
   glDisable( GL_POLYGON_OFFSET_FILL );
}

void RendererGL::drawShadow(const uint& light_index)
{
   glUseProgram( ShadowShader->ShaderProgram );

   Lights.transferUniformsToShader( ShadowShader.get() );
   glUniform1i( ShadowShader->CustomLocations["LightIndex"], light_index );

   const mat4 model_view_projection = LightCamera->ProjectionMatrix * LightCamera->ViewMatrix;
   glUniformMatrix4fv( ShadowShader->CustomLocations["LightModelViewProjectionMatrix"], 1, GL_FALSE, &model_view_projection[0][0] );
   
   glBindTextureUnit( ShadowShader->Location.Texture[1].first, DepthTextureID );
   drawGroundObject( ShadowShader.get(), MainCamera.get() );
}

void RendererGL::render()
{
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   const float light_x = 1024.0f * cos( LightTheta ) + 256.0f;
   const float light_z = 1024.0f * sin( LightTheta ) + 256.0f;
   Lights.setLightPosition( vec4(light_x, 200.0f, light_z, 1.0f), 0 );

   drawDepthMapFromLightView( 0 );
   drawShadow( 0 );
     
   glUseProgram( ObjectShader->ShaderProgram );
   Lights.transferUniformsToShader( ObjectShader.get() );
   drawTigerObject( ObjectShader.get(), MainCamera.get() );
   drawPandaObject( ObjectShader.get(), MainCamera.get() );
   
   glBindVertexArray( 0 );
   glUseProgram( 0 );
}

void RendererGL::play()
{
   if (glfwWindowShouldClose( Window )) initialize();

   setLights();
   setGroundObject();
   setTigerObject();
   setPandaObject();
   setDepthFrameBuffer();

   ObjectShader->setUniformLocations( Lights.TotalLightNum );
   ShadowShader->setUniformLocations( 1 );
   ShadowShader->addUniformLocation( ShadowShader->ShaderProgram, "LightIndex" );
   ShadowShader->addUniformLocation( ShadowShader->ShaderProgram, "LightModelViewProjectionMatrix" );

   while (!glfwWindowShouldClose( Window )) {
      render();

      LightTheta += 0.01f;
      if (LightTheta >= 360.0f) LightTheta -= 360.0f;

      glfwSwapBuffers( Window );
      glfwPollEvents();
   }
   glfwDestroyWindow( Window );
}