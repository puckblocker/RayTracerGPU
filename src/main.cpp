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
void imageSaver(std::vector<float> &, int, int, string);
static void menuManagerWindow(enum renderState &, string &, string &);
static void startMenuWindow(enum renderState &, string &, string &);
static void configMenuWindow(string &);
void help();

enum renderState
{
    starting,
    paused,
    rendering,
    picture
};

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
    renderState currentState = renderState::paused;
    string fileName;
    string pictureName = "Render";

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
    window = glfwCreateWindow(640, 640, "Beef Wizard Path Tracer", NULL, NULL); // sets window stats like resolution and full screen, etc
    glfwMakeContextCurrent(window);                                             // Sets window to the context we'll be rendering to

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

    // SAMPLE COUNT FOR PROGRESIVE
    float sampleCount;
    GLuint sampleCountLoc;

    // MENU VARIABLES
    int choice;
    bool inMenu = false;
    std::vector<float> pixelBuffer(resWidth * resHeight * 4);
    Packager::Package packageInfo;
    GLuint cameraID;
    GLuint sphereID;
    GLuint triangleID;
    glCreateBuffers(1, &triangleID);
    GLuint planeID;
    GLuint xFormID;
    GLuint pLightID;
    GLuint dLightID;
    GLuint aLightID;

    // ========================================
    // GUI SETUP
    // ========================================
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    // START MENU
    cout << "\n===RENDERER===\n\n";

    // ========================================
    // WINDOW OPENER
    // ========================================
    while (!glfwWindowShouldClose(window)) // keeps window up until closed by user
    {

        glfwPollEvents(); // keeps event queue from overflowing (events are constantly being made)

        // ----------------------------------------
        // START RENDER (PACKAGER)
        // ----------------------------------------
        if (currentState == renderState::starting)
        {
            sampleCount = 0.0f;

            // ========================================
            // INFO SHIPPER
            // ========================================
            packageInfo = newPackager.packager(fileName);

            // ----------------------------------------
            // CAMERA / VIEWPORT PACKAGER
            // ----------------------------------------
            cameraID;
            glCreateBuffers(1, &cameraID);                                                                      // Create unique memory ID and initiliaze
            glNamedBufferData(cameraID, sizeof(Camera::CompCam), &packageInfo.camera.compCam, GL_DYNAMIC_DRAW); // allocate memory & send to GPU
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraID);                                                   // link memory location to port for GPU access

            // ----------------------------------------
            // SHAPE PACKAGER
            // ----------------------------------------

            // SPHERE PACKAGER
            sphereID;
            glCreateBuffers(1, &sphereID);
            glNamedBufferData(sphereID, packageInfo.spheres.size() * sizeof(Intersect::Sphere), packageInfo.spheres.data(), GL_STATIC_DRAW);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sphereID);

            // TRIANGLE
            triangleID;
            glCreateBuffers(1, &triangleID);
            glNamedBufferData(triangleID, packageInfo.triangles.size() * sizeof(Intersect::Triangle), packageInfo.triangles.data(), GL_STATIC_DRAW);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, triangleID);

            // PLANE
            planeID;
            glCreateBuffers(1, &planeID);
            glNamedBufferData(planeID, packageInfo.planes.size() * sizeof(Intersect::Plane), packageInfo.planes.data(), GL_STATIC_DRAW);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, planeID);

            // TRANSFORMS
            xFormID;
            glCreateBuffers(1, &xFormID);
            glNamedBufferData(xFormID, packageInfo.xForms.size() * sizeof(Intersect::xForm), packageInfo.xForms.data(), GL_STATIC_DRAW);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, xFormID);

            // ----------------------------------------
            // LIGHT PACKAGER
            // ----------------------------------------
            // POINT
            pLightID;
            glCreateBuffers(1, &pLightID);
            glNamedBufferData(pLightID, packageInfo.pointLights.size() * sizeof(Light::pLight), packageInfo.pointLights.data(), GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, pLightID);

            // DIRECTIONAL
            dLightID;
            glCreateBuffers(1, &dLightID);
            glNamedBufferData(dLightID, packageInfo.directionalLights.size() * sizeof(Light::dLight), packageInfo.directionalLights.data(), GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, dLightID);

            // AREA
            aLightID;
            glCreateBuffers(1, &aLightID);
            glNamedBufferData(aLightID, packageInfo.areaLights.size() * sizeof(Light::aLight), packageInfo.areaLights.data(), GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, aLightID);

            sampleCountLoc = glGetUniformLocation(compProg, "sampleCount");

            // SWITCH STATE TO RENDERING
            currentState = renderState::rendering;
        }
        // ----------------------------------------
        // LOAD RENDER (ACTUAL RENDERING)
        // ----------------------------------------
        else if (currentState == renderState::rendering)
        {
            // WAKE UP GPU
            glUseProgram(compProg); // binds compute shader file

            // SEND SAMPLE COUNT
            glUniform1f(sampleCountLoc, sampleCount);

            // EXECUTE GPU WORK GROUPS
            glDispatchCompute(resWidth / 16, resHeight / 16, 1); // work groups of 16 x 16 filling screen res

            // SAMPLE COUNT
            sampleCount += 4;
            std::cout << "Total Samples: " << sampleCount << endl;
        }
        // ----------------------------------------
        // PAUSE RENDER (DEFAULT STATE)
        // ----------------------------------------
        else if (currentState == renderState::paused)
        {
        }

        // ----------------------------------------
        // TAKE PICTURE OF RENDER
        // ----------------------------------------
        else if (currentState == renderState::picture)
        {
            glBindTexture(GL_TEXTURE_2D, texID);
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixelBuffer.data());
            imageSaver(pixelBuffer, resWidth, resHeight, pictureName);

            currentState = renderState::paused;
        }

        // DISPLAY
        glClearColor(.75f, .5f, .75f, 1.0f); // set color that will be used to clear the screen
        glClear(GL_COLOR_BUFFER_BIT);        // clears screen with constant to tell which buffer to clear (color buffer)

        // GUI NEW FRAME
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6); // starting indices of triangles

        menuManagerWindow(currentState, fileName, pictureName);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window); // keeps display updated / swaps buffer
    }

    // TERMINATE
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glDeleteProgram(compProg);
    glfwTerminate(); // terminate window
    return 0;        // terminate program
}

// Save Image
void imageSaver(std::vector<float> &pixelBuffer, int width, int height, string pictureName)
{
    cout << "\nSaving Image...\n";
    pictureName = pictureName + ".png";

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

    stbi_write_png(pictureName.c_str(), width, height, 3, byteData, width * 3);

    cout << "\nImage Saved...\n";

    // Clean up
    delete[] byteData;

    return;
}

static void menuManagerWindow(renderState &currentState, string &fileName, string &pictureName)
{
    ImGui::Begin("Path Tracer Config Menu");

    if (ImGui::CollapsingHeader("Help"))
    {
        help();
    }

    if (ImGui::CollapsingHeader("Render"))
    {
        startMenuWindow(currentState, pictureName, fileName);
    }

    if (ImGui::CollapsingHeader("Scene Creation"))
    {
        configMenuWindow(fileName);
    }

    ImGui::End();
}

static void startMenuWindow(renderState &currentState, string &pictureName, string &fileName)
{
    ImGui::SeparatorText("Scene");

    static char tempFile[32] = "";
    ImGui::InputTextWithHint("##SceneFileInput", "enter file name", tempFile, IM_COUNTOF(tempFile));
    if (ImGui::Button("Use Selected Scene"))
    {
        ImGui::SameLine();
        ImGui::Text("Saved Selected File!");
        fileName = tempFile;
        fileName += ".txt";
    }

    ImGui::SeparatorText("Controls");
    if (ImGui::Button("Start"))
    {
        currentState = renderState::starting;
    }

    if (ImGui::Button("Continue"))
    {
        currentState = renderState::rendering;
    }

    ImGui::SameLine();
    if (ImGui::Button("Pause"))
    {
        currentState = renderState::paused;
    }

    ImGui::SeparatorText("Picture");
    static char tempPic[32] = "";
    ImGui::InputTextWithHint("##PictureInput", "enter picture name", tempPic, IM_COUNTOF(tempPic));
    if (ImGui::Button("Take Screenshot"))
    {
        ImGui::SameLine();
        ImGui::Text("Saved Picture!");
        pictureName = tempPic;
        currentState = renderState::picture;
    }
}

static void configMenuWindow(string &fileName)
{
    // SCENE SETTINGS VARIABLES
    static stringstream sceneContent;
    static char sceneName[32] = "NewScene";
    static float camPos[3] = {0.0, 0.0, 2.0};
    static float camBasis[3] = {0.0, 1.0, 0.0};
    static float camGaze[3] = {0.0, 0.0, -1.0};
    static float camLength = 1.0;
    static float viewportStats[2] = {2.0, 2.0};
    static float lensStats[2] = {0.0, 1.0};

    // SPHERE VARIABLES
    static float spherePos[3] = {0.0, 0.0, 0.0};
    static float sphereRadius = 0.5;
    static float sphereColor[3] = {0.5, 0.5, 0.5};
    static float sphereMat1[3] = {0.0, 0.0, 0.0};
    static float sphereMat2[3] = {0.0, 0.0, 0.0};
    static float sphereAnim = 0.0;

    // TRIANGLE VARIABLES
    static float triP0[3] = {0.0, 0.0, 0.0};
    static float triP1[3] = {0.0, 0.0, 0.0};
    static float triP2[3] = {0.0, 0.0, 0.0};
    static float triColor[3] = {0.5, 0.5, 0.5};
    static float triMat1[3] = {0.0, 0.0, 0.0};
    static float triMat2[3] = {0.0, 0.0, 0.0};
    static float triAnim = 0.0;

    // PLANE VARIABLES
    static float planePos[3] = {0.0, 0.0, 0.0};
    static float planeNorm[3] = {0.0, 0.0, 1.0};
    static float planeColor[3] = {0.5, 0.5, 0.5};
    static float planeMat1[3] = {0.0, 0.0, 0.0};
    static float planeMat2[3] = {0.0, 0.0, 0.0};
    static float planeAnim = 0.0;

    // POINT LIGHT VARIABLES

    // AREA LIGHT VARIABLES

    // DIRECTIONAL LIGHT VARIABLES

    if (ImGui::TreeNode("Scene Settings"))
    {
        // SCENE NAME
        ImGui::InputTextWithHint("Scene Name", "enter text here", sceneName, IM_COUNTOF(sceneName));

        // CAMERA SETTINGS
        ImGui::SeparatorText("Camera");
        ImGui::InputFloat3("Camera Position (X,Y,Z)", camPos);
        ImGui::InputFloat3("Camera Basis (X,Y,Z)", camBasis);
        ImGui::InputFloat3("Camera Gaze (X,Y,Z)", camGaze);
        ImGui::InputFloat("Camera Length", &camLength);

        // VIEWPORT SETTINGS
        ImGui::SeparatorText("Viewport");
        ImGui::InputFloat2("Viewport (Height, Width)", viewportStats);

        // DEPTH OF FIELD SETTINGS
        ImGui::SeparatorText("Depth Of Field");
        ImGui::InputFloat2("Lens (Diameter, Focus Distance)", lensStats);

        if (ImGui::Button("Add Scene Settings"))
        {
            sceneContent << "Camera" << endl;
            sceneContent << camPos[0] << " " << camPos[1] << " " << camPos[2] << endl;
            sceneContent << camBasis[0] << " " << camBasis[1] << " " << camBasis[2] << endl;
            sceneContent << camGaze[0] << " " << camGaze[1] << " " << camGaze[2] << endl;
            sceneContent << camLength << endl;

            sceneContent << "Viewport " << endl;
            sceneContent << viewportStats[0] << " " << viewportStats[1] << endl;

            sceneContent << "Lens " << endl;
            sceneContent << lensStats[0] << " " << lensStats[1] << endl;

            cout << "Added scene settings to sstream.\n";
        }

        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Shapes"))
    {
        ImGui::SeparatorText("Spheres");

        ImGui::InputFloat3("Sphere Position (X,Y,Z)", spherePos);
        ImGui::InputFloat("Sphere Radius", &sphereRadius);
        ImGui::InputFloat3("Sphere Color (R,G,B)", sphereColor);
        ImGui::InputFloat3("Sphere Material (metallic, roughness , ior)", sphereMat1);
        ImGui::InputFloat3("Sphere Material (emissive, layer depth, layer ior)", sphereMat2);

        if (ImGui::Button("Add Sphere"))
        {
            // WRITE TO STRING STREAM
            sceneContent << "Sphere\n";
            sceneContent << spherePos[0] << " " << spherePos[1] << " " << spherePos[2] << endl;
            sceneContent << sphereRadius << endl;
            sceneContent << sphereColor[0] << " " << sphereColor[1] << " " << sphereColor[2] << endl;
            sceneContent << sphereMat1[0] << " " << sphereMat1[1] << " " << sphereMat1[2] << endl;
            sceneContent << sphereMat2[0] << " " << sphereMat2[1] << " " << sphereMat2[2] << endl;
            sceneContent << sphereAnim << endl;

            cout << "Added sphere to sstream.\n";
        }

        ImGui::SeparatorText("Triangles");

        ImGui::InputFloat3("Triangle Point 1 Position (X,Y,Z)", triP0);
        ImGui::InputFloat3("Triangle Point 2 Position (X,Y,Z)", triP1);
        ImGui::InputFloat3("Triangle Point 3 Position (X,Y,Z)", triP2);
        ImGui::InputFloat3("Triangle Color (R,G,B)", triColor);
        ImGui::InputFloat3("Triangle Material (metallic, roughness , ior)", triMat1);
        ImGui::InputFloat3("Triangle Material (emissive, layer depth, layer ior)", triMat2);

        if (ImGui::Button("Add Triangle"))
        {
            // WRITE TO STRING STREAM
            sceneContent << "Triangle\n";
            sceneContent << triP0[0] << " " << triP0[1] << " " << triP0[2] << endl;
            sceneContent << triP1[0] << " " << triP1[1] << " " << triP1[2] << endl;
            sceneContent << triP2[0] << " " << triP2[1] << " " << triP2[2] << endl;
            sceneContent << triColor[0] << " " << triColor[1] << " " << triColor[2] << endl;
            sceneContent << triMat1[0] << " " << triMat1[1] << " " << triMat1[2] << endl;
            sceneContent << triMat2[0] << " " << triMat2[1] << " " << triMat2[2] << endl;

            cout << "Added triangle to sstream.\n";
        }

        ImGui::SeparatorText("Planes");

        ImGui::InputFloat3("Plane Position (X,Y,Z)", planePos);
        ImGui::InputFloat3("Plane Normal (X,Y,Z)", planeNorm);
        ImGui::InputFloat3("Plane Color (R,G,B)", planeColor);
        ImGui::InputFloat3("Plane Material (metallic, roughness , ior)", planeMat1);
        ImGui::InputFloat3("Plane Material (emissive, layer depth, layer ior)", planeMat2);

        if (ImGui::Button("Add Plane"))
        {
            // WRITE TO STRING STREAM
            sceneContent << "Plane\n";
            sceneContent << planePos[0] << " " << planePos[1] << " " << planePos[2] << endl;
            sceneContent << planeNorm[0] << " " << planeNorm[1] << " " << planeNorm[2] << endl;
            sceneContent << planeColor[0] << " " << planeColor[1] << " " << planeColor[2] << endl;
            sceneContent << planeMat1[0] << " " << planeMat1[1] << " " << planeMat1[2] << endl;
            sceneContent << planeMat2[0] << " " << planeMat2[1] << " " << planeMat2[2] << endl;

            cout << "Added plane to sstream.\n";
        }

        ImGui::SeparatorText("Transforms");
        if (ImGui::Button("Take Screenshot"))
        {
        }

        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Lighting"))
    {
        ImGui::SeparatorText("Point Lights");
        ImGui::SeparatorText("Area Lights");
        ImGui::SeparatorText("Directional Lights");
        ImGui::TreePop();
    }

    // EXPORT CUSTOM SCENE TO FILE
    if (ImGui::Button("Save Custom Scene"))
    {
        // OPEN/CREATE FILE
        string customFileName = string(sceneName) + ".txt";
        ofstream outFile(customFileName);

        if (outFile.is_open())
        {
            // INPUT SCENE CONTENTS INTO FILE
            outFile << sceneContent.str();

            // RESET SCENE SSTREAM CONTENTS
            sceneContent.str("");
            sceneContent.clear();
        }
    }
}

void help()
{
    ImGui::SeparatorText("START INSTRUCTIONS");
    ImGui::BulletText("To begin, enter your scene's file name, click 'Use Selected Scene', and select start render from the menu. (If you want to use a premade file, drag that file into the cendtral folder)");
    ImGui::BulletText("Optionally you can create your own scene, to do so check out 'Scene Creation'");
}