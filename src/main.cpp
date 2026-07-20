// ========================================
// PROGRAM: GPU PATH TRACER
// PURPOSE: CREATE A GPU PATH TRACER UTILIZING OPENGL
// AUTHOR: SETH HIRD
// ========================================

#include "config.h"
#include "package_manager.h"
#include <fstream>
#include <sstream>
#include <iostream>

// IMAGE SAVING
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std;

// ========================================
// VERTEX SHADER (Sets Vertices)
// ========================================
const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec2 aPos;\n" // gets inputs for attribute position aPos(bridge to rest of code look to glVertexAttribPointer with 0)
                                 "layout (location = 1) in vec2 aTexCoord;\n"
                                 "out vec2 TexCoord;\n" // output to fragment shader
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n" // sets vertex position
                                 "   TexCoord = aTexCoord;\n"                         // passes texture coord further down pipeline
                                 "}\0";

// ========================================
// FRAGMENT SHADER (Sets color)
// ========================================
const char *fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"              // final color output
                                   "in vec2 TexCoord;\n"                // vertex shader input
                                   "uniform sampler2D screenTexture;\n" // represents the image
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = texture(screenTexture, TexCoord);\n" // looks at color at the coords
                                   "}\n\0";

// FORWARD DECLARATION
void imageSaver(std::vector<float> &, int, int);

// ========================================
// MAIN CODE
// ========================================
int main()
{
    // VARIABLES
    GLFWwindow *window; //  create window
    Packager newPackager;
    const int resWidth = 640;
    const int resHeight = 640;

    // GLFW ERROR CHECKER
    if (!glfwInit())
    {
        cout << "GLFW failed to start.\n";
        return -1;
    }

    // ========================================
    // DISPLAY SETUP CODE
    // ========================================
    // WINDOW STATS
    window = glfwCreateWindow(640, 640, "My Window", NULL, NULL); // sets window stats like resolution and full screen, etc
    glfwMakeContextCurrent(window);                               // Sets window to the context we'll be rendering to

    // GLAD ERROR CHECKER
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) // tells glad to run through system with the location of all function definitions
    {
        glfwTerminate();
        return -1;
    }

    // Create Reference to Store Vertex Shader / Compile Shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);     // create shader and get reference value
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); // feed vertex shader the source code from before main
    glCompileShader(vertexShader);                              // compile source code

    // Create Reference to Store Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);     // create shader and get reference value
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL); // feed fragment shader the source code from before main
    glCompileShader(fragmentShader);                                // compile source code

    // Wrap Up Shaders Into Shader Program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader); // attach shaders to shader program
    glLinkProgram(shaderProgram);                  // wraps up shader program
    glDeleteShader(vertexShader);                  // delete shaders (they are alreday implemented)
    glDeleteShader(fragmentShader);

    // Setup Quad Geometry (Two Triangles)
    GLfloat quadVertices[] = {
        // Triangle Coords & Texture Coords (Screen Coverage (0-1))
        // Triangle 1
        -1.0f, 1.0f, 0.0f, 1.0f,  // top left
        -1.0f, -1.0f, 0.0f, 0.0f, // bottom left
        1.0f, -1.0f, 1.0f, 0.0f,  // bottom right
        // Triangle 2
        1.0f, -1.0f, 1.0f, 0.0f, // bottom right
        1.0f, 1.0f, 1.0f, 1.0f,  // top right
        -1.0f, 1.0f, 0.0f, 1.0f  // top left
    };

    // CREATE VERTICES BUFFER & VERTICES ARRAY (slow to send things between CPU to GPU, this sends the data in batches)
    GLuint VAO, VBO; // reference to store vertex data (VBO Vertex Buffer Object)
                     // stores pointers to VBOs and tells OpenGL how to interpret them (VAO Vertex Array Object)
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO); // creates buffer object with 1 3D object

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);                                                // binds object making the object the current object (changes to binded object change current object)
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW); // stores vertices in VBO

    // POSITION
    int stride = 4 * sizeof(float);                                     // how many bytes OpenGl needs to skip to get to next vertex in buffer
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void *)0); // tells openGL how to read VBO (void* 0 is because vertices begin at start of array)
    glEnableVertexAttribArray(0);                                       // enables ^
    // TEXTURE COORDS
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void *)(2 * sizeof(float))); // different void to represent offset since vertices are 4 bytes apart
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0); // combines VAO & VBO to prevent errors <
    glBindVertexArray(0);             // <

    // ----------------------------------------
    // SCREEN / TEXTURE PACKAGER
    // ----------------------------------------
    // SETUP TEXTURE
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glActiveTexture(GL_TEXTURE0); // plugs texture into texture0 slot
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Allocate the memory on the GPU (NULL because we haven't uploaded data yet)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resWidth, resHeight, 0, GL_RGB, GL_FLOAT, NULL); // creates a 2D texture imaghe (allocates GPU memory) / stores vertices in the texture

    glBindImageTexture(0, texID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // ========================================
    // COMPUTE SHADER SETUP CODE
    // ========================================
    // FILE READER
    std::ifstream compFile("renderer.comp");
    if (!compFile.is_open())
    {
        std::cout << "Failed to Open Compute Shader File: " << "renderer.comp" << std::endl;
        return 0;
    }

    // CONVERT FILE TO STRING STREAM
    std::stringstream compShaderStream;
    compShaderStream << compFile.rdbuf();
    compFile.close();

    // CONVERT STREAM TO STRING
    string compCode = compShaderStream.str();
    const char *compShaderSourceText = compCode.c_str();

    // PASS FILE TEXT TO GPU
    GLuint compShader = glCreateShader(GL_COMPUTE_SHADER);

    glShaderSource(compShader, 1, &compShaderSourceText, NULL);
    glCompileShader(compShader);

    // CREATE COMPUTE SHADER PROGRAM
    GLuint compProg = glCreateProgram();
    glAttachShader(compProg, compShader); // attaches binary to the program
    glLinkProgram(compProg);              // links the code (such as actually making the ports connect and mean something)
    glDeleteShader(compShader);

    // ========================================
    // INFO SHIPPER
    // ========================================
    Packager::Package packageInfo = newPackager.packager();

    // ----------------------------------------
    // CAMERA / VIEWPORT PACKAGER
    // ----------------------------------------
    GLuint cameraID;
    glCreateBuffers(1, &cameraID);                                                                      // Create unique memory ID and initiliaze
    glNamedBufferData(cameraID, sizeof(Camera::CompCam), &packageInfo.camera.compCam, GL_DYNAMIC_DRAW); // allocate memory & send to GPU
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraID);                                                   // link memory location to port for GPU access

    // ----------------------------------------
    // SHAPE PACKAGER
    // ----------------------------------------

    // SPHERE PACKAGER
    GLuint sphereID;
    glCreateBuffers(1, &sphereID);
    glNamedBufferData(sphereID, packageInfo.spheres.size() * sizeof(Intersect::Sphere), packageInfo.spheres.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sphereID);

    // TRIANGLE
    GLuint triangleID;
    glCreateBuffers(1, &triangleID);
    glNamedBufferData(triangleID, packageInfo.triangles.size() * sizeof(Intersect::Triangle), packageInfo.triangles.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, triangleID);

    // PLANE
    GLuint planeID;
    glCreateBuffers(1, &planeID);
    glNamedBufferData(planeID, packageInfo.planes.size() * sizeof(Intersect::Plane), packageInfo.planes.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, planeID);

    // TRANSFORMS
    GLuint xFormID;
    glCreateBuffers(1, &xFormID);
    glNamedBufferData(xFormID, packageInfo.xForms.size() * sizeof(Intersect::xForm), packageInfo.xForms.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, xFormID);

    // ----------------------------------------
    // LIGHT PACKAGER
    // ----------------------------------------
    // POINT
    GLuint pLightID;
    glCreateBuffers(1, &pLightID);
    glNamedBufferData(pLightID, sizeof(Light::pLight), &packageInfo.pointLight, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 5, pLightID);

    // DIRECTIONAL
    GLuint dLightID;
    glCreateBuffers(1, &dLightID);
    glNamedBufferData(dLightID, sizeof(Light::dLight), &packageInfo.directionalLight, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 6, dLightID);

    // AREA
    GLuint aLightID;
    glCreateBuffers(1, &aLightID);
    glNamedBufferData(aLightID, sizeof(Light::aLight), &packageInfo.areaLight, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 7, aLightID);

    // SAMPLE COUNT FOR PROGRESIVE
    float sampleCount = 0.0f;
    GLuint sampleCountLoc = glGetUniformLocation(compProg, "sampleCount");

    // MENU VARIABLES
    int choice;
    bool inMenu = false;
    std::vector<float> pixelBuffer(resWidth * resHeight * 4);

    // START MENU
    cout << "\n===RENDERER===\n\n"
         << "To Pause Hold ESCAPE\n\n";

    // ========================================
    // WINDOW OPENER
    // ========================================
    while (!glfwWindowShouldClose(window)) // keeps window up until closed by user
    {

        glfwPollEvents(); // keeps event queue from overflowing (events are constantly being made)

        // Pause Handler
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            cout << "\n\n--Program Paused--\n";
            inMenu = true;

            // Pause Loop
            while (inMenu)
            {
                cout << "1. Save Image\n"
                     << "2. Continue\n"
                     << "0. Terminate\n";
                cout << "What would you like to do?\n";

                cin >> choice;

                switch (choice)
                {
                case (0):
                    glfwSetWindowShouldClose(window, true);
                    inMenu = false;
                    break;

                case (1):
                    glBindTexture(GL_TEXTURE_2D, texID);
                    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixelBuffer.data());
                    imageSaver(pixelBuffer, resWidth, resHeight);
                    break;

                case (2):
                    inMenu = false;
                    break;

                default:
                    cout << "Invalid...\n";
                    break;
                }
            }

            inMenu = true;
        }

        // WAKE UP GPU
        glUseProgram(compProg); // binds compute shader file

        // SEND SAMPLE COUNT
        glUniform1f(sampleCountLoc, sampleCount);

        // EXECUTE GPU WORK GROUPS
        glDispatchCompute(resWidth / 16, resHeight / 16, 1); // work groups of 16 x 16 filling screen res

        // MEMORY BARRIER
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); // forces CPU to wait for GPU to finish VRAM texture

        // DISPLAY
        glClearColor(.75f, .5f, .75f, 1.0f); // set color that will be used to clear the screen
        glClear(GL_COLOR_BUFFER_BIT);        // clears screen with constant to tell which buffer to clear (color buffer)

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6); // starting indices of triangles

        glfwSwapBuffers(window); // keeps display updated / swaps buffer

        // SAMPLE COUNT
        sampleCount += 4;
        std::cout << "Total Samples: " << sampleCount << endl;
    }

    // TERMINATE
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glDeleteProgram(compProg);
    glfwTerminate(); // terminate window
    return 0;        // terminate program
}

// Save Image
void imageSaver(std::vector<float> &pixelBuffer, int width, int height)
{
    cout << "\nSaving Image...\n";

    // Image Buffer
    unsigned char *byteData = new unsigned char[width * height * 3];

    // Loop Through, Convert Float to Bytes
    for (int i = 0; i < width * height; i++)
    {
        // INCOMING VECTOR (RGBA)
        int inIndex = i * 4;
        // OUTGOING VECTOR (RGB)
        int outIndex = i * 3;
        float r = glm::clamp(pixelBuffer[inIndex], 0.0f, 1.0f);
        float g = glm::clamp(pixelBuffer[inIndex + 1], 0.0f, 1.0f);
        float b = glm::clamp(pixelBuffer[inIndex + 2], 0.0f, 1.0f);

        byteData[outIndex] = static_cast<unsigned char>(r * 255.0f);
        byteData[outIndex + 1] = static_cast<unsigned char>(g * 255.0f);
        byteData[outIndex + 2] = static_cast<unsigned char>(b * 255.0f);
    }

    // Flip Image
    stbi_flip_vertically_on_write(true);

    stbi_write_png("Render.png", width, height, 3, byteData, width * 3);

    cout << "\nImage Saved...\n";

    // Clean up
    delete[] byteData;

    return;
}