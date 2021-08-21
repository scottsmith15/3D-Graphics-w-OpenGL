// All changes made by Scott Smith CS-330

#include <iostream>             // cout, cerr
#include <cstdlib>              // EXIT_FAILURE
#include <GL/glew.h>            // GLEW library
#include <GLFW/glfw3.h>         // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>      // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Icosphere.h"
#include "sphere/sphere/src/Sphere.h"               // edited for hemisphere 
#include "Cylinder.h"
#include <learnOpengl/camera.h> // Camera class

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "CS-330 Project"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbo;         // Handle for the vertex buffer object
        GLuint vbos[2];     // Handle for the vertex buffer object
        GLuint nIndices;    // Number of indices of the mesh
        GLuint nVertices;   // Number of vertices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Mouse Bungee Body mesh data
    GLMesh gMeshBungeeBody;
    // Mouse Bungee Back mesh data
    GLMesh gMeshBungeeBack;
    // Sphere mesh data
    GLMesh gMeshSphere;
    // Cylinder mesh data
    GLMesh gMeshCylinder;
    // Cube mesh data
    GLMesh gMeshCube;
    // Plane mesh data
    GLMesh gMeshPlane;
    // Hemisphere mesh data
    GLMesh gMeshHemisphere;
    // Mousepad mesh data
    GLMesh gMeshMousepad;

    // Textures
    GLuint gTextureId_desk;
    GLuint gTextureId_blackPlastic;
    GLuint gTextureId_bungeeHighlights;
    GLuint gTextureId_cylinders;
    GLuint gTextureId_redPlastic;
    GLuint gTextureId_mouse;
    GLuint gTextureId_mousepad;

    glm::vec2 gUVScale(5.0f, 5.0f);
    GLint gTexWrapMode = GL_REPEAT;

    // Shader programs
    GLuint gProgramId;
    GLuint gCubeProgramId;
    GLuint gLampProgramId;
    
    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    // Creates a perspective projection
    glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

    // Cube and light color
    glm::vec3 gObjectColor(1.f, 0.2f, 0.0f);
    glm::vec3 gLightColor(1.0f, 1.0f, 1.0f);    // white light
    glm::vec3 gLightColor2(1.0f, 1.0f, 1.0f);   // RGB light 


    // Light position and scale
    glm::vec3 gLightPosition(-3.0f, 2.0f, -4.0f);  // light 1
    glm::vec3 gLightPosition2(5.2f, 2.0f, 4.0f);   // light 2
    glm::vec3 gLightScale(0.3f);

}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateMeshBungeeBody(GLMesh& mesh);
void UCreateMeshBungeeBack(GLMesh& mesh);
void UCreateMeshCube(GLMesh& mesh);
void UCreateMeshSphere(GLMesh& mesh);
void UCreateMeshCylinder(GLMesh& mesh);
void UCreateMeshPlane(GLMesh& mesh);
void UCreateMeshHemisphere(GLMesh& mesh);
void UCreateMeshMousepad(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position;
    layout(location = 2) in vec2 textureCoordinate;

    out vec2 vertexTextureCoordinate;


    //Global variables for the transform matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
        vertexTextureCoordinate = textureCoordinate;
    }
);


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
    in vec2 vertexTextureCoordinate;

    out vec4 fragmentColor;

    uniform sampler2D uTexture;
    uniform sampler2D uTexture2;
    uniform bool multipleTextures;
    uniform bool useScale;
    uniform vec2 uvScale;

    void main()
    {
        if (useScale)
        {
            fragmentColor = texture(uTexture, vertexTextureCoordinate * uvScale);
        }
        else 
        {
            fragmentColor = texture(uTexture, vertexTextureCoordinate);
        }

        if (multipleTextures)
        {
            vec4 extraTexture = texture(uTexture2, vertexTextureCoordinate);
            if (extraTexture.a != 0.0)
                fragmentColor = extraTexture;
        }
    }
);


/* Cube Vertex Shader Source Code*/
const GLchar* cubeVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
    layout(location = 1) in vec3 normal; // VAP position 1 for normals
    layout(location = 2) in vec2 textureCoordinate;

    out vec3 vertexNormal; // For outgoing normals to fragment shader
    out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
    out vec2 vertexTextureCoordinate;

    //Uniform / Global variables for the  transform matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates

        vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

        vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
        vertexTextureCoordinate = textureCoordinate;
    }
);


/* Cube Fragment Shader Source Code*/
const GLchar* cubeFragmentShaderSource = GLSL(440,

    in vec3 vertexNormal; // For incoming normals
    in vec3 vertexFragmentPos; // For incoming fragment position
    in vec2 vertexTextureCoordinate;

    out vec4 fragmentColor; // For outgoing cube color to the GPU

    // Uniform / Global variables for object color, light color, light position, and camera/view position
    uniform vec3 objectColor;
    uniform vec3 lightColor;
    uniform vec3 lightColor2;
    uniform vec3 lightPos;
    uniform vec3 lightPos2;
    uniform vec3 viewPosition;
    uniform sampler2D uTexture; // Useful when working with multiple textures
    uniform vec2 uvScale;

    vec3 generatePhong(vec3 lightColor, vec3 lightPos)
    {
        //Calculate Ambient lighting*/
        float ambientStrength = 0.1f; // Set ambient or global lighting strength
        vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

        //Calculate Diffuse lighting*/
        vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
        vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
        float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
        vec3 diffuse = impact * lightColor; // Generate diffuse light color

        //Calculate Specular lighting*/
        float specularIntensity = 0.8f;  // Set specular light strength
        float highlightSize = 16.0f; // Set specular highlight size
        vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
        vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
        //Calculate specular component
        float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
        vec3 specular = specularIntensity * specularComponent * lightColor;

        // Texture holds the color to be used for all three components
        vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

        // Calculate phong result
        vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;
        return phong;
    }

    void main()
    {
        /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/
        vec3 phong_result = vec3(0.0f);
        phong_result += generatePhong(lightColor, lightPos);
        // RGB light
        phong_result += (generatePhong(lightColor2, lightPos2) * vec3(0.2f));

        fragmentColor = vec4(phong_result, 1.0); // Send lighting results to GPU

    }
);


/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

                //Uniform / Global variables for the  transform matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
    }
);


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

    uniform bool color;
    uniform vec3 lightColor2;

    void main()
    {    
        if (color) 
        {
            fragmentColor = vec4(lightColor2, 1.0f); // Set color to RGB   
        }
        else 
        {
            fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
        }
    }
);


// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}


int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    UCreateMeshBungeeBody(gMeshBungeeBody); // Calls the function to create the Vertex Buffer Object
    UCreateMeshBungeeBack(gMeshBungeeBack);
    UCreateMeshSphere(gMeshSphere);
    UCreateMeshCylinder(gMeshCylinder);
    UCreateMeshCube(gMeshCube);
    UCreateMeshPlane(gMeshPlane);
    UCreateMeshHemisphere(gMeshHemisphere);
    UCreateMeshMousepad(gMeshMousepad);

    // Create the shader programs
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    if (!UCreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gCubeProgramId))
        return EXIT_FAILURE;

    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
        return EXIT_FAILURE;

    // Load texture
    const char* texFilename = "desk_texture.jpg";
    if (!UCreateTexture(texFilename, gTextureId_desk))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    // Load texture
    texFilename = "black_plastic.jpg";
    if (!UCreateTexture(texFilename, gTextureId_blackPlastic))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    // Load texture
    texFilename = "bungee_highlights.png";
    if (!UCreateTexture(texFilename, gTextureId_bungeeHighlights))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    // Load texture
    texFilename = "red_plastic.jpg";
    if (!UCreateTexture(texFilename, gTextureId_redPlastic))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    // Load texture
    texFilename = "cylinders.jpg";
    if (!UCreateTexture(texFilename, gTextureId_cylinders))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    // Load texture
    texFilename = "mouse.jpg";
    if (!UCreateTexture(texFilename, gTextureId_mouse))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    // Load texture
    texFilename = "mousepad.jpg";
    if (!UCreateTexture(texFilename, gTextureId_mousepad))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    
    // We set the textures as texture unit 0 & 1
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture2"), 1);

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gMeshBungeeBody);
    UDestroyMesh(gMeshBungeeBack);
    UDestroyMesh(gMeshSphere);
    UDestroyMesh(gMeshCylinder);
    UDestroyMesh(gMeshCube);
    UDestroyMesh(gMeshPlane);
    UDestroyMesh(gMeshHemisphere);
    UDestroyMesh(gMeshMousepad);

    // Release textures
    UDestroyTexture(gTextureId_desk);
    UDestroyTexture(gTextureId_blackPlastic);
    UDestroyTexture(gTextureId_bungeeHighlights);
    UDestroyTexture(gTextureId_redPlastic);
    UDestroyTexture(gTextureId_cylinders);
    UDestroyTexture(gTextureId_mouse);
    UDestroyTexture(gTextureId_mousepad);


    // Release shader programs
    UDestroyShaderProgram(gProgramId);
    UDestroyShaderProgram(gCubeProgramId);
    UDestroyShaderProgram(gLampProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;
    
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    // change perspective/ortho view                                  (changes perspective/ortho ONLY when key is held down)
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 0.1f, 100.0f); 
    else
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}


// Function called to render a frame
void URender()
{
    // Simple mockup for RGB lighting 
    const float angularVelocity = glm::radians(90.0f);   
    
    glm::vec4 newPosition = glm::rotate(angularVelocity * gDeltaTime, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(gLightColor2, 1.0f);
    gLightColor2.r = newPosition.r;
    gLightColor2.g = newPosition.g;
    gLightColor2.b = newPosition.b; 
    
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set the shader to be used
    glUseProgram(gCubeProgramId);

    // Redo model matrix per object
    // 1. Scales the object
    glm::mat4 scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
    // 2. Rotates shape by 15 degrees in the x axis
    glm::mat4 rotation = glm::rotate(glm::radians(0.0f), glm::vec3(1.0, 1.0f, 1.0f));
    // 3. Place object at the origin
    glm::mat4 translation = glm::translate(glm::vec3(0.0f, 5.0f, 0.0f));
    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = translation * rotation * scale;

    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gCubeProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gCubeProgramId, "view");
    GLint projLoc = glGetUniformLocation(gCubeProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
    GLint objectColorLoc = glGetUniformLocation(gCubeProgramId, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(gCubeProgramId, "lightColor");
    GLint lightColor2Loc = glGetUniformLocation(gCubeProgramId, "lightColor2");
    GLint lightPositionLoc = glGetUniformLocation(gCubeProgramId, "lightPos");
    GLint lightPositionLoc2 = glGetUniformLocation(gCubeProgramId, "lightPos2");
    GLint viewPositionLoc = glGetUniformLocation(gCubeProgramId, "viewPosition");

    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightColor2Loc, gLightColor2.r, gLightColor2.g, gLightColor2.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    glUniform3f(lightPositionLoc2, gLightPosition2.x, gLightPosition2.y, gLightPosition2.z);
    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    GLint UVScaleLoc = glGetUniformLocation(gCubeProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMeshPlane.vao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId_desk);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMeshPlane.nVertices);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    // LAMP: draw lamp
    //----------------
    glUseProgram(gLampProgramId);

    // light 1
    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gLightPosition) * glm::scale(gLightScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");
    lightColor2Loc = glGetUniformLocation(gLampProgramId, "lightColor2");

    // use RGB for this lamp? 
    GLuint colorLoc = glGetUniformLocation(gLampProgramId, "color");
    glUniform1i(colorLoc, false);

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(lightColor2Loc, gLightColor2.r, gLightColor2.g, gLightColor2.b);

    glBindVertexArray(gMeshSphere.vao);

    glDrawElements(GL_TRIANGLES,                    // primitive type
        gMeshSphere.nIndices,                       // # of indices
        GL_UNSIGNED_INT,                            // data type
        (void*)0);                                  // offset to indices

    // light 2
    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gLightPosition2) * glm::scale(gLightScale);

    // use RGB for this lamp? 
    glUniform1i(colorLoc, true);

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));    

    glDrawElements(GL_TRIANGLES,                    // primitive type
        gMeshSphere.nIndices,                       // # of indices
        GL_UNSIGNED_INT,                            // data type
        (void*)0);                                  // offset to indices

    glBindVertexArray(0);

    // 1. Scales the object 
    scale = glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));
    // 2. Rotates shape
    rotation = glm::rotate(glm::radians(0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    // 3. Place object 
    translation = glm::translate(glm::vec3(2.0f, 0.0f, 0.0f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;

    // Set the shader to be used
    glUseProgram(gProgramId);

    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");
    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");

    // use tiling for texture?
    GLuint useScaleLoc = glGetUniformLocation(gProgramId, "useScale");
    glUniform1i(useScaleLoc, false);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // use multiple textures for this object? 
    GLuint multipleTexturesLoc = glGetUniformLocation(gProgramId, "multipleTextures");
    glUniform1i(multipleTexturesLoc, false);

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMeshBungeeBody.vao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId_blackPlastic);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMeshBungeeBody.nVertices);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    // use multiple textures for this object?
    glUniform1i(multipleTexturesLoc, true);

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMeshBungeeBack.vao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId_blackPlastic);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gTextureId_bungeeHighlights);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMeshBungeeBack.nVertices);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    // use multiple textures for this object?
    glUniform1i(multipleTexturesLoc, false);

    // Redo model matrix per object
    // 1. Scales the object 
    scale = glm::scale(glm::vec3(0.3f, 0.3f, 0.3f));
    // 2. Rotates shape
    rotation = glm::rotate(glm::radians(-30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    // 3. Place object 
    translation = glm::translate(glm::vec3(2.1f, 0.75f, -1.0f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(gMeshCylinder.vao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId_cylinders);

    glDrawElements(GL_TRIANGLES,                    // primitive type
        gMeshCylinder.nIndices,                     // # of indices
        GL_UNSIGNED_INT,                            // data type
        (void*)0);                                  // offset to indices

    glBindVertexArray(0);

    // Redo model matrix per object
    // 1. Scales the object 
    scale = glm::scale(glm::vec3(0.3f, 0.3f, 0.3f));
    // 2. Rotates shape
    rotation = glm::rotate(glm::radians(-30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    // 3. Place object 
    translation = glm::translate(glm::vec3(1.9f, 0.75f, -1.0f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(gMeshCylinder.vao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId_cylinders);

    glDrawElements(GL_TRIANGLES,                    // primitive type
        gMeshCylinder.nIndices,                     // # of indices
        GL_UNSIGNED_INT,                            // data type
        (void*)0);                                  // offset to indices

    glBindVertexArray(0);

    // Redo model matrix per object
    // 1. Scales the object 
    scale = glm::scale(glm::vec3(0.25f, 0.25f, 0.25f));
    // 2. Rotates shape
    rotation = glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    // 3. Place object 
    translation = glm::translate(glm::vec3(1.75f, 0.85f, -0.5f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMeshCube.vao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId_redPlastic);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMeshCube.nVertices);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    // Redo model matrix per object
    // 1. Scales the object 
    scale = glm::scale(glm::vec3(0.8f, 0.6f, 1.2f));
    // 2. Rotates shape
    rotation = glm::rotate(glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    // 3. Place object 
    translation = glm::translate(glm::vec3(3.5f, 0.0f, 2.0f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMeshHemisphere.vao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId_mouse);

    glDrawElements(GL_TRIANGLES,                    // primitive type
        gMeshHemisphere.nIndices,                   // # of indices
        GL_UNSIGNED_INT,                            // data type
        (void*)0);                                  // offset to indices

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    // Redo model matrix per object
    // 1. Scales the object
    scale = glm::scale(glm::vec3(5.8f, 0.05f, 2.8f));
    // 2. Rotates shape by 15 degrees in the x axis
    rotation = glm::rotate(glm::radians(0.0f), glm::vec3(1.0, 1.0f, 1.0f));
    // 3. Place object at the origin
    translation = glm::translate(glm::vec3(-1.8f, 0.0f, -1.2f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMeshMousepad.vao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId_mousepad);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMeshMousepad.nVertices);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    // Redo model matrix per object
    // 1. Scales the object 
    scale = glm::scale(glm::vec3(0.3f, 0.3f, 2.0f));
    // 2. Rotates shape
    rotation = glm::rotate(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    // 3. Place object 
    translation = glm::translate(glm::vec3(3.5f, 0.0f, -2.0f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    // use tiling for texture?
    glUniform1i(useScaleLoc, true);

    glBindVertexArray(gMeshCylinder.vao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId_cylinders);

    glDrawElements(GL_TRIANGLES,                    // primitive type
        gMeshCylinder.nIndices,                     // # of indices
        GL_UNSIGNED_INT,                            // data type
        (void*)0);                                  // offset to indices

    glBindVertexArray(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


// Create bungee body
void UCreateMeshBungeeBody(GLMesh& mesh)
{
    // Position and Texture Coordinates
    GLfloat verts[] = {
        // Vertex Positions    // Texture Coordinates
         1.5f,  0.0f,  0.0f,   1.0f,  1.0f, // Base      right Vertex 0
        -1.5f,  0.0f,  0.0f,   0.0f,  1.0f, // Base      left  Vertex 1
        -1.0f,  0.0f, -4.0f,   0.25f, 0.0f, // Base deep left  Vertex 3
         1.5f,  0.0f,  0.0f,   1.0f,  1.0f, // Base      right Vertex 0
         1.0f,  0.0f, -4.0f,   0.75f, 0.0f, // Base deep right Vertex 2
        -1.0f,  0.0f, -4.0f,   0.25f, 0.0f, // Base deep left  Vertex 3
         1.5f,  0.0f,  0.0f,   0.0f,  0.0f, // Base      right Vertex 0
         1.0f,  0.0f, -4.0f,   1.0f,  0.0f, // Base deep right Vertex 2
         0.50f, 2.0f, -2.5f,   0.5f,  1.0f, // Top             Vertex 4
         1.5f,  0.0f,  0.0f,   1.0f,  0.0f, // Base      right Vertex 0
        -1.5f,  0.0f,  0.0f,   0.0f,  0.0f, // Base      left  Vertex 1
        -0.50f, 2.0f, -2.5f,   0.25f, 1.0f, // Top             Vertex 5
         1.5f,  0.0f,  0.0f,   1.0f,  0.0f, // Base      right Vertex 0
         0.50f, 2.0f, -2.5f,   0.75f, 1.0f, // Top             Vertex 4
        -0.50f, 2.0f, -2.5f,   0.25f, 1.0f, // Top             Vertex 5
        -1.5f,  0.0f,  0.0f,   1.0f,  0.0f, // Base      left  Vertex 1
        -1.0f,  0.0f, -4.0f,   0.0f,  0.0f, // Base deep left  Vertex 3
        -0.50f, 2.0f, -2.5f,   0.5f,  1.0f, // Top             Vertex 5
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(2);
}


// Create bungee back
void UCreateMeshBungeeBack(GLMesh& mesh)
{
    // Position and Texture Coordinates
    GLfloat verts[] = {
        // Vertex Positions    // Texture Coordinates
         1.0f,  0.0f, -4.0f,   0.0f,  0.0f, // Base deep right Vertex 2
        -1.0f,  0.0f, -4.0f,   1.0f,  0.0f, // Base deep left  Vertex 3
        -0.50f, 2.0f, -2.5f,   0.75f, 1.0f, // Top             Vertex 5
         1.0f,  0.0f, -4.0f,   0.0f,  0.0f, // Base deep right Vertex 2
         0.50f, 2.0f, -2.5f,   0.25f, 1.0f, // Top             Vertex 4
        -0.50f, 2.0f, -2.5f,   0.75f, 1.0f  // Top             Vertex 5
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(2);
}


// Create sphere   
void UCreateMeshSphere(GLMesh& mesh)
{

    // create icosphere with radius=0.5, subdivision=5 and smooth shading=true
    Icosphere sphere(0.5f, 5, true);

    // can change parameters later
    //sphere.setRadius(2.0f);
    //sphere.setSubdivision(6);
    //sphere.setSmooth(false);

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // copy interleaved vertex data (V/N/T) to VBO
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]);           // for vertex data
    glBufferData(GL_ARRAY_BUFFER,                          // target
        sphere.getInterleavedVertexSize(),           // data size, # of bytes
        sphere.getInterleavedVertices(),                   // ptr to vertex data
        GL_STATIC_DRAW);                                   // usage

    // store number of indices to draw later
    mesh.nIndices = sphere.getIndexCount();

    // copy index data to VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);   // for index data
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,                  // target
        sphere.getIndexSize(),                             // data size, # of bytes
        sphere.getIndices(),                               // ptr to index data
        GL_STATIC_DRAW);                                   // usage


    // set attrib arrays with stride and offset
    int stride = sphere.getInterleavedStride();            // should be 32 bytes
    glVertexAttribPointer(0, 3, GL_FLOAT, false, stride, (void*)0);
    // activate attrib array
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(2, 2, GL_FLOAT, false, stride, (void*)(sizeof(float) * 6));
    // activate attrib array
    glEnableVertexAttribArray(2);

    //glVertexAttribPointer(2, 3, GL_FLOAT, false, stride, (void*)(sizeof(float) * 3));
    //glEnableVertexAttribArray(2);

}


// Create cylinder
void UCreateMeshCylinder(GLMesh& mesh)
{
    // create a cylinder with base radius, top radius,
    // height, sectors, stacks, smooth=true
    Cylinder cylinder(0.1, 0.1, 3, 36, 8, true);

    // can change parameters later
    //cylinder.setBaseRadius(1.5f);
    //cylinder.setTopRadius(2.5f);
    //cylinder.setHeight(3.5f);
    //cylinder.setSectorCount(36);
    //cylinder.setStackCount(8);
    //cylinder.setSmooth(false);

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // copy interleaved vertex data (V/N/T) to VBO
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]);           // for vertex data
    glBufferData(GL_ARRAY_BUFFER,                          // target
        cylinder.getInterleavedVertexSize(),               // data size, # of bytes
        cylinder.getInterleavedVertices(),                 // ptr to vertex data
        GL_STATIC_DRAW);                                   // usage

    // store number of indices to draw later
    mesh.nIndices = cylinder.getIndexCount();

    // copy index data to VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);   // for index data
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,                  // target
        cylinder.getIndexSize(),                           // data size, # of bytes
        cylinder.getIndices(),                             // ptr to index data
        GL_STATIC_DRAW);                                   // usage


    // set attrib arrays with stride and offset
    int stride = cylinder.getInterleavedStride();          // should be 32 bytes
    glVertexAttribPointer(0, 3, GL_FLOAT, false, stride, (void*)0);
    // activate attrib array
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(2, 2, GL_FLOAT, false, stride, (void*)(sizeof(float) * 6));
    // activate attrib array
    glEnableVertexAttribArray(2);

    //glVertexAttribPointer(2, 3, GL_FLOAT, false, stride, (void*)(sizeof(float) * 3));
    //glEnableVertexAttribArray(2);
}


// Create cube
void UCreateMeshCube(GLMesh& mesh)
{
    // Position and Texture Coordinates
    GLfloat verts[] = {
        //Positions          //Texture Coordinates
        -0.5f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  1.0f,  0.0f,  1.0f, 1.0f,
        -0.5f,  1.0f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.0f,  0.0f,  0.0f, 0.0f,

        -0.5f,  0.0f,  2.0f,  0.0f, 0.0f,
         0.5f,  0.0f,  2.0f,  1.0f, 0.0f,
         0.5f,  1.0f,  2.0f,  1.0f, 1.0f,
         0.5f,  1.0f,  2.0f,  1.0f, 1.0f,
        -0.5f,  1.0f,  2.0f,  0.0f, 1.0f,
        -0.5f,  0.0f,  2.0f,  0.0f, 0.0f,

        -0.5f,  1.0f,  2.0f,  1.0f, 0.0f,
        -0.5f,  1.0f,  0.0f,  1.0f, 1.0f,
        -0.5f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.0f,  2.0f,  0.0f, 0.0f,
        -0.5f,  1.0f,  2.0f,  1.0f, 0.0f,

         0.5f,  1.0f,  2.0f,  1.0f, 0.0f,
         0.5f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.0f,  2.0f,  0.0f, 0.0f,
         0.5f,  1.0f,  2.0f,  1.0f, 0.0f,

        -0.5f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.0f,  2.0f,  1.0f, 0.0f,
         0.5f,  0.0f,  2.0f,  1.0f, 0.0f,
        -0.5f,  0.0f,  2.0f,  0.0f, 0.0f,
        -0.5f,  0.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  1.0f,  2.0f,  1.0f, 0.0f,
         0.5f,  1.0f,  2.0f,  1.0f, 0.0f,
        -0.5f,  1.0f,  2.0f,  0.0f, 0.0f,
        -0.5f,  1.0f,  0.0f,  0.0f, 1.0f

    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(2);

}


// Create mousepad
void UCreateMeshMousepad(GLMesh& mesh)
{
    // Position and Texture Coordinates
    GLfloat verts[] = {
        //Positions          //Texture Coordinates
        -0.5f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  1.0f,  0.0f,  0.0f, 1.0f,
        -0.5f,  1.0f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.0f,  2.0f,  0.0f, 1.0f,
         0.5f,  0.0f,  2.0f,  0.0f, 1.0f,
         0.5f,  1.0f,  2.0f,  0.0f, 1.0f,
         0.5f,  1.0f,  2.0f,  0.0f, 1.0f,
        -0.5f,  1.0f,  2.0f,  0.0f, 1.0f,
        -0.5f,  0.0f,  2.0f,  0.0f, 1.0f,

        -0.5f,  1.0f,  2.0f,  0.0f, 1.0f,
        -0.5f,  1.0f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.0f,  2.0f,  0.0f, 1.0f,
        -0.5f,  1.0f,  2.0f,  0.0f, 1.0f,

         0.5f,  1.0f,  2.0f,  0.0f, 1.0f,
         0.5f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.0f,  2.0f,  0.0f, 1.0f,
         0.5f,  1.0f,  2.0f,  0.0f, 1.0f,

        -0.5f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.0f,  2.0f,  1.0f, 0.0f,
         0.5f,  0.0f,  2.0f,  1.0f, 0.0f,
        -0.5f,  0.0f,  2.0f,  0.0f, 0.0f,
        -0.5f,  0.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  1.0f,  2.0f,  1.0f, 0.0f,
         0.5f,  1.0f,  2.0f,  1.0f, 0.0f,
        -0.5f,  1.0f,  2.0f,  0.0f, 0.0f,
        -0.5f,  1.0f,  0.0f,  0.0f, 1.0f

    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(2);

}


// Create the plane
void UCreateMeshPlane(GLMesh& mesh)
{
    // Position, Normals, Texture Coordinates
    GLfloat verts[] = {
        // Vertex Positions     // Normals          // Texture Coordinates
        -5.0f, -5.0f, -5.0f,   0.0f, 1.0f, 0.0f,    0.0f, 1.0f,  //   Vertex 0
         5.0f, -5.0f, -5.0f,   0.0f, 1.0f, 0.0f,    1.0f, 1.0f,  //   Vertex 1
         5.0f, -5.0f,  5.0f,   0.0f, 1.0f, 0.0f,    1.0f, 0.0f,  //   Vertex 2
         5.0f, -5.0f,  5.0f,   0.0f, 1.0f, 0.0f,    1.0f, 0.0f,  //   Vertex 2
        -5.0f, -5.0f,  5.0f,   0.0f, 1.0f, 0.0f,    0.0f, 0.0f,  //   Vertex 3
        -5.0f, -5.0f, -5.0f,   0.0f, 1.0f, 0.0f,    0.0f, 1.0f   //   Vertex 0
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

}


// Create the hemisphere
void UCreateMeshHemisphere(GLMesh& mesh) 
{
    // create a sphere with radius=1, sectors=36, stacks=18, smooth=true (default)
    Sphere sphere(1.0f, 36, 18);

    // can change parameters later
    //sphere.setRadius(2.0f);
    //sphere.setSectorCount(72);
    //sphere.setStackCount(24);
    //sphere.setSmooth(false);

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // copy interleaved vertex data (V/N/T) to VBO
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]);           // for vertex data
    glBufferData(GL_ARRAY_BUFFER,                          // target
        sphere.getInterleavedVertexSize(),           // data size, # of bytes
        sphere.getInterleavedVertices(),                   // ptr to vertex data
        GL_STATIC_DRAW);                                   // usage

    // store number of indices to draw later
    mesh.nIndices = sphere.getIndexCount();

    // copy index data to VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);   // for index data
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,                  // target
        sphere.getIndexSize(),                             // data size, # of bytes
        sphere.getIndices(),                               // ptr to index data
        GL_STATIC_DRAW);                                   // usage


    // set attrib arrays with stride and offset
    int stride = sphere.getInterleavedStride();            // should be 32 bytes
    glVertexAttribPointer(0, 3, GL_FLOAT, false, stride, (void*)0);
    // activate attrib array
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(2, 2, GL_FLOAT, false, stride, (void*)(sizeof(float) * 6));
    // activate attrib array
    glEnableVertexAttribArray(2);

    //glVertexAttribPointer(2, 3, GL_FLOAT, false, stride, (void*)(sizeof(float) * 3));
    //glEnableVertexAttribArray(2);
}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(2, mesh.vbos);
}


/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}


void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}
