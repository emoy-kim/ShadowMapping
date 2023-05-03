#include "renderer.h"

RendererGL::RendererGL() :
   Window( nullptr ), FrameWidth( 1920 ), FrameHeight( 1080 ), FBO( 0 ), DepthTextureID( 0 ),
   ClickedPoint( -1, -1 ), LightTheta( 0.0f ), MainCamera( std::make_unique<CameraGL>() ),
   LightCamera( std::make_unique<CameraGL>() ), ObjectShader( std::make_unique<ShaderGL>() ),
   ShadowShader( std::make_unique<ShaderGL>() ), GroundObject( std::make_unique<ObjectGL>() ),
   TigerObject( std::make_unique<ObjectGL>() ), PandaObject( std::make_unique<ObjectGL>() ),
   Lights( std::make_unique<LightGL>() )
{
   Renderer = this;

   initialize();
   printOpenGLInformation();
}

RendererGL::~RendererGL()
{
   if (DepthTextureID != 0) glDeleteTextures( 1, &DepthTextureID );
   if (FBO != 0) glDeleteFramebuffers( 1, &FBO );
}

void RendererGL::printOpenGLInformation()
{
   std::cout << "****************************************************************\n";
   std::cout << " - GLFW version supported: " << glfwGetVersionString() << "\n";
   std::cout << " - OpenGL renderer: " << glGetString( GL_RENDERER ) << "\n";
   std::cout << " - OpenGL version supported: " << glGetString( GL_VERSION ) << "\n";
   std::cout << " - OpenGL shader version supported: " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << "\n";
   std::cout << "****************************************************************\n\n";
}

void RendererGL::initialize()
{
   if (!glfwInit()) {
      std::cout << "Cannot Initialize OpenGL...\n";
      return;
   }
   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
   glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

   Window = glfwCreateWindow( FrameWidth, FrameHeight, "Main Camera", nullptr, nullptr );
   glfwMakeContextCurrent( Window );

   if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return;
   }

   registerCallbacks();

   glEnable( GL_DEPTH_TEST );
   glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );

   MainCamera->updateWindowSize( FrameWidth, FrameHeight );
   LightCamera->updateWindowSize( FrameWidth, FrameHeight );

   const std::string shader_directory_path = std::string(CMAKE_SOURCE_DIR) + "/shaders";
   ObjectShader->setShader(
      std::string(shader_directory_path + "/BasicPipeline.vert").c_str(),
      std::string(shader_directory_path + "/BasicPipeline.frag").c_str()
   );
   ShadowShader->setShader(
      std::string(shader_directory_path + "/Shadow.vert").c_str(),
      std::string(shader_directory_path + "/Shadow.frag").c_str()
   );
}

void RendererGL::cleanup(GLFWwindow* window)
{
   glfwSetWindowShouldClose( window, GLFW_TRUE );
}

void RendererGL::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   if (action != GLFW_PRESS) return;

   switch (key) {
      case GLFW_KEY_L:
         Renderer->Lights->toggleLightSwitch();
         std::cout << "Light Turned " << (Renderer->Lights->isLightOn() ? "On!\n" : "Off!\n");
         break;
      case GLFW_KEY_P: {
         const glm::vec3 pos = Renderer->MainCamera->getCameraPosition();
         std::cout << "Camera Position: " << pos.x << ", " << pos.y << ", " << pos.z << "\n";
      } break;
      case GLFW_KEY_Q:
      case GLFW_KEY_ESCAPE:
         cleanup( window );
         break;
      default:
         return;
   }
}

void RendererGL::cursor(GLFWwindow* window, double xpos, double ypos)
{
   if (Renderer->MainCamera->getMovingState()) {
      const auto x = static_cast<int>(round( xpos ));
      const auto y = static_cast<int>(round( ypos ));
      const int dx = x - Renderer->ClickedPoint.x;
      const int dy = y - Renderer->ClickedPoint.y;
      Renderer->MainCamera->moveForward( -dy );
      Renderer->MainCamera->rotateAroundWorldY( -dx );

      if (glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_RIGHT ) == GLFW_PRESS) {
         Renderer->MainCamera->pitch( -dy );
      }

      Renderer->ClickedPoint.x = x;
      Renderer->ClickedPoint.y = y;
   }
}

void RendererGL::mouse(GLFWwindow* window, int button, int action, int mods)
{
   if (button == GLFW_MOUSE_BUTTON_LEFT) {
      const bool moving_state = action == GLFW_PRESS;
      if (moving_state) {
         double x, y;
         glfwGetCursorPos( window, &x, &y );
         Renderer->ClickedPoint.x = static_cast<int>(round( x ));
         Renderer->ClickedPoint.y = static_cast<int>(round( y ));
      }
      Renderer->MainCamera->setMovingState( moving_state );
   }
}

void RendererGL::mousewheel(GLFWwindow* window, double xoffset, double yoffset)
{
   if (yoffset >= 0.0) Renderer->MainCamera->zoomIn();
   else Renderer->MainCamera->zoomOut();
}

void RendererGL::reshape(GLFWwindow* window, int width, int height)
{
   Renderer->MainCamera->updateWindowSize( width, height );
   glViewport( 0, 0, width, height );
}

void RendererGL::registerCallbacks() const
{
   glfwSetWindowCloseCallback( Window, cleanup );
   glfwSetKeyCallback( Window, keyboard );
   glfwSetCursorPosCallback( Window, cursor );
   glfwSetMouseButtonCallback( Window, mouse );
   glfwSetScrollCallback( Window, mousewheel );
   glfwSetFramebufferSizeCallback( Window, reshape );
}

void RendererGL::setLights() const
{
   const glm::vec4 light_position(256.0f, 500.0f, 512.0f, 1.0f);
   const glm::vec4 ambient_color(1.0f, 1.0f, 1.0f, 1.0f);
   const glm::vec4 diffuse_color(0.7f, 0.7f, 0.7f, 1.0f);
   const glm::vec4 specular_color(0.9f, 0.9f, 0.9f, 1.0f);
   Lights->addLight( light_position, ambient_color, diffuse_color, specular_color );
}

void RendererGL::setGroundObject() const
{
   const float size = 512.0f;
   std::vector<glm::vec3> ground_vertices;
   ground_vertices.emplace_back( 0.0f, 0.0f, 0.0f );
   ground_vertices.emplace_back( size, 0.0f, 0.0f );
   ground_vertices.emplace_back( size, 0.0f, size );

   ground_vertices.emplace_back( 0.0f, 0.0f, 0.0f );
   ground_vertices.emplace_back( size, 0.0f, size );
   ground_vertices.emplace_back( 0.0f, 0.0f, size );


   std::vector<glm::vec3> ground_normals;
   ground_normals.emplace_back( 0.0f, 1.0f, 0.0f );
   ground_normals.emplace_back( 0.0f, 1.0f, 0.0f );
   ground_normals.emplace_back( 0.0f, 1.0f, 0.0f );

   ground_normals.emplace_back( 0.0f, 1.0f, 0.0f );
   ground_normals.emplace_back( 0.0f, 1.0f, 0.0f );
   ground_normals.emplace_back( 0.0f, 1.0f, 0.0f );

   std::vector<glm::vec2> ground_textures;
   ground_textures.emplace_back( 0.0f, 0.0f );
   ground_textures.emplace_back( 1.0f, 0.0f );
   ground_textures.emplace_back( 1.0f, 1.0f );

   ground_textures.emplace_back( 0.0f, 0.0f );
   ground_textures.emplace_back( 1.0f, 1.0f );
   ground_textures.emplace_back( 0.0f, 1.0f );

   const std::string sample_directory_path = std::string(CMAKE_SOURCE_DIR) + "/samples";
   GroundObject->setObject(
      GL_TRIANGLES,
      ground_vertices,
      ground_normals,
      ground_textures,
      std::string(sample_directory_path + "/sand.jpg")
   );
   GroundObject->setDiffuseReflectionColor( { 1.0f, 1.0f, 1.0f, 1.0f } );
}

void RendererGL::setTigerObject() const
{
   const std::string sample_directory_path = std::string(CMAKE_SOURCE_DIR) + "/samples";
   std::ifstream file(std::string(sample_directory_path + "/Tiger/tiger.txt"));
   int polygon_num;
   file >> polygon_num;

   const int vertex_num = polygon_num * 3;
   std::vector<glm::vec3> tiger_vertices(vertex_num);
   std::vector<glm::vec3> tiger_normals(vertex_num);
   std::vector<glm::vec2> tiger_textures(vertex_num);
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

   TigerObject->setObject(
      GL_TRIANGLES,
      tiger_vertices,
      tiger_normals,
      tiger_textures,
      std::string(sample_directory_path + "/Tiger/tiger.jpg")
   );
   TigerObject->setDiffuseReflectionColor( { 1.0f, 1.0f, 1.0f, 1.0f } );
}

void RendererGL::setPandaObject() const
{
   const std::string sample_directory_path = std::string(CMAKE_SOURCE_DIR) + "/samples";
   PandaObject->setObject(
      GL_TRIANGLES,
      std::string(sample_directory_path + "/Panda/panda.obj"),
      std::string(sample_directory_path + "/Panda/panda.png")
   );
   PandaObject->setDiffuseReflectionColor( { 1.0f, 1.0f, 1.0f, 1.0f } );
}

void RendererGL::setDepthFrameBuffer()
{
   glCreateTextures( GL_TEXTURE_2D, 1, &DepthTextureID );
   glTextureStorage2D( DepthTextureID, 1, GL_DEPTH_COMPONENT32F, LightCamera->getWidth(), LightCamera->getHeight() );
   glTextureParameteri( DepthTextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
   glTextureParameteri( DepthTextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   glTextureParameteri( DepthTextureID, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
   glTextureParameteri( DepthTextureID, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );

   glCreateFramebuffers( 1, &FBO );
   glNamedFramebufferTexture( FBO, GL_DEPTH_ATTACHMENT, DepthTextureID, 0 );
}

void RendererGL::drawGroundObject(ShaderGL* shader, CameraGL* camera) const
{
   shader->transferBasicTransformationUniforms( glm::mat4(1.0f), camera, true );
   GroundObject->transferUniformsToShader( shader );

   glBindTextureUnit( 0, GroundObject->getTextureID( 0 ) );
   glBindVertexArray( GroundObject->getVAO() );
   glDrawArrays( GroundObject->getDrawMode(), 0, GroundObject->getVertexNum() );
}

void RendererGL::drawTigerObject(ShaderGL* shader, CameraGL* camera) const
{
   const glm::mat4 to_world =
      translate( glm::mat4(1.0f), glm::vec3(250.0f, 0.0f, 330.0f) ) *
      rotate( glm::mat4(1.0f), glm::radians( 180.0f ), glm::vec3(0.0f, 1.0f, 0.0f) ) *
      rotate( glm::mat4(1.0f), glm::radians( -90.0f ), glm::vec3(1.0f, 0.0f, 0.0f) ) *
      scale( glm::mat4(1.0f), glm::vec3( 0.3f, 0.3f, 0.3f ) );
   shader->transferBasicTransformationUniforms( to_world, camera, true );

   glBindTextureUnit( 0, TigerObject->getTextureID( 0 ) );
   glBindVertexArray( TigerObject->getVAO() );
   glDrawArrays( TigerObject->getDrawMode(), 0, TigerObject->getVertexNum() );
}

void RendererGL::drawPandaObject(ShaderGL* shader, CameraGL* camera) const
{
   const glm::mat4 to_world =
      translate(glm::mat4(1.0f), glm::vec3(250.0f, -5.0f, 180.0f) ) *
      scale(glm::mat4(1.0f), glm::vec3( 20.0f, 20.0f, 20.0f ) );
   shader->transferBasicTransformationUniforms( to_world, camera, true );

   PandaObject->transferUniformsToShader( shader );

   glBindTextureUnit( 0, PandaObject->getTextureID( 0 ) );
   glBindVertexArray( PandaObject->getVAO() );
   glDrawArrays( PandaObject->getDrawMode(), 0, PandaObject->getVertexNum() );
}

void RendererGL::drawDepthMapFromLightView(int light_index) const
{
   glBindFramebuffer( GL_FRAMEBUFFER, FBO );
   glClearDepth( 1.0f );
   glClear( GL_DEPTH_BUFFER_BIT );

   glUseProgram( ObjectShader->getShaderProgram() );

   LightCamera->updateCameraPosition(
      glm::vec3(Lights->getLightPosition( light_index )),
      glm::vec3(256.0f, 0.0f, 10.0f),
      glm::vec3(0.0f, 1.0f, 0.0f)
   );

   drawTigerObject( ObjectShader.get(), LightCamera.get() );
   drawPandaObject( ObjectShader.get(), LightCamera.get() );
   drawGroundObject( ObjectShader.get(), LightCamera.get() );

   glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void RendererGL::drawShadow(int light_index) const
{
   glUseProgram( ShadowShader->getShaderProgram() );

   Lights->transferUniformsToShader( ShadowShader.get() );
   glUniform1i( ShadowShader->getLocation( "LightIndex" ), light_index );

   const glm::mat4& model_view_projection = LightCamera->getProjectionMatrix() * LightCamera->getViewMatrix();
   glUniformMatrix4fv( ShadowShader->getLocation( "LightModelViewProjectionMatrix" ), 1, GL_FALSE, &model_view_projection[0][0] );

   glBindTextureUnit( 1, DepthTextureID );
   drawTigerObject( ShadowShader.get(), MainCamera.get() );
   drawPandaObject( ShadowShader.get(), MainCamera.get() );
   drawGroundObject( ShadowShader.get(), MainCamera.get() );
}

void RendererGL::render() const
{
   glClear( OPENGL_COLOR_BUFFER_BIT | OPENGL_DEPTH_BUFFER_BIT );

   const float light_x = 1024.0f * cosf( LightTheta ) + 256.0f;
   const float light_z = 1024.0f * sinf( LightTheta ) + 256.0f;
   Lights->setLightPosition( glm::vec4(light_x, 200.0f, light_z, 1.0f), 0 );

   drawDepthMapFromLightView( 0 );
   drawShadow( 0 );

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

   ObjectShader->setBasicUniformLocations();
   ShadowShader->setUniformLocations( 1 );
   ShadowShader->addUniformLocation( "LightIndex" );
   ShadowShader->addUniformLocation( "LightModelViewProjectionMatrix" );

   while (!glfwWindowShouldClose( Window )) {
      render();

      LightTheta += 0.01f;
      if (LightTheta >= 360.0f) LightTheta -= 360.0f;

      glfwSwapBuffers( Window );
      glfwPollEvents();
   }
   glfwDestroyWindow( Window );
}