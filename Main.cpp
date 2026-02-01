#define RAYGUI_IMPLEMENTATION
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <map>
#include <algorithm>
#include <glm/gtx/rotate_vector.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

int main() {

    // First thing you do in your main function is to initialize GLFW. This will help you with UI and functionality 
    glfwInit();

    unsigned int width = 1000;
    unsigned int height = 700;

    // We need to tell glfw what version we're using by giving it "hints" A We're using ver. 3.3 so the major will be 3 and the minor will be 3.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    //This is telling GLFW what set of OpenGL methods we want to use. THe core profile is basically all the modern mathods we need.
    // For example, core profile forces us to use VBO's, VAOs, Shaders, etc. because it's the more modern way of doing things. 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    //Widdth, Height, Window Name, Full Screen?, Multiple Windows with the same context?
    GLFWwindow* window = glfwCreateWindow(width, height, "Engine YURR", NULL, NULL);

    // Is the window function doesn't load, it'll terminate 
    if (!window) {
        glfwTerminate();
        return -1;
    }

    //Putting GLFW in the context of the window 
    glfwMakeContextCurrent(window);

    //Setting up GLAD - GLAD is an openGL function loader. It loads all OpenGL function Pointers on start-up 
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // set viewport
    glViewport(0, 0, width, height);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");


    // These are the verticies for our icosphere
    std::vector<glm::vec3> verts = {
      {0.000f,  0.000f,  1.000f},
      {0.894f,  0.000f,  0.447f},
      {0.276f,  0.851f,  0.447f},
      {-0.724f,  0.526f,  0.447f},
      {-0.724f, -0.526f,  0.447f},
      {0.276f, -0.851f,  0.447f},
      {0.724f,  0.526f, -0.447f},
      {-0.276f,  0.851f, -0.447f},
      {-0.894f,  0.000f, -0.447f},
      {-0.276f, -0.851f, -0.447f},
      {0.724f, -0.526f, -0.447f},
      {0.000f,  0.000f, -1.000f}
    };

    //This is the data that we will use to call the verticies. It gives us an easy way to send vert data to wherever it needs to go. 
    std::vector<unsigned int> idx = {
        0, 1, 2,
        0, 2, 3,
        0, 3, 4,
        0, 4, 5,
        0, 5, 1,
        1, 6, 2,
        2, 7, 3,
        3, 8, 4,
        4, 9, 5,
        5, 10, 1,
        1, 10, 6,
        2, 6, 7,
        3, 7, 8,
        4, 8, 9,
        5, 9, 10,
        6, 11, 7,
        7, 11, 8,
        8, 11, 9,
        9, 11, 10,
        10, 11, 6
    };

    // Subdivide triangles to make the sphere higher-poly
    int subdivisions = 3; // Increase this to make it more high poly


    //we run this once per subdevision. 
    //NewIndx holds the new triangle indices, which will change depending on how many subdevisions you have
    //stores calculated midpoints so we donthave duplicates. Kinda like an EBO
    for (int s = 0; s < subdivisions; ++s) {
        std::vector<unsigned int> newIdx;
        std::map<std::pair<unsigned int, unsigned int>, unsigned int> midpointCache;

        //Lambda function to get midpoint
        // A lambda is a function that has no name, can only contain one expression, and automatically returns the result of that expression.

           // getMidPoint(point a, point b), calculates the midpoint of the edge connecting point a and b
        auto getMidpoint = [&](unsigned int a, unsigned int b) -> unsigned int {
            //ensures the edge is stored consistantly reguardless of vertex order
            auto edge = std::minmax(a, b);
            //prevents creating the same midpoint multiple times to save memeory 
            if (midpointCache.find(edge) != midpointCache.end())
                return midpointCache[edge];

            // This is where we calculate the midpoint and normalize it to lie on the sphere
            //Adds the positions of the two vertices, divides by 2 and gets the geometric midpoint.
            //normalize moves the midpoint onto the unit sphere. Without this, the vertices would create a “bulged” shape rather than a sphere
            //This creates a new vertex on the sphere
            glm::vec3 midpoint = glm::normalize((verts[a] + verts[b]) / 2.0f);

            verts.push_back(midpoint);
            unsigned int newIndex = verts.size() - 1;
            midpointCache[edge] = newIndex;
            return newIndex;
            };

        //This returns the index of this vertex so you can form the 4 new triangles.
        //Loops through every triangle in mesh and calculates the midpoint of each edge
        //a = midpoint of v1-v2
        //b = midpoint of v2-v3
        //c = midpoint of v3-v4
        for (size_t i = 0; i < idx.size(); i += 3) {
            unsigned int v1 = idx[i];
            unsigned int v2 = idx[i + 1];
            unsigned int v3 = idx[i + 2];

            unsigned int a = getMidpoint(v1, v2);
            unsigned int b = getMidpoint(v2, v3);
            unsigned int c = getMidpoint(v3, v1);

            // Add 4 new triangles
            //Each original triangle becomes 4 smaller triangles:
            //This is what gradually smooths out the sphere with each subdivision.
            // So each time we subdivide, it creates 4 triangles for every triangle that's on the sphere
            //You're basically 4x'ing the triangles every subdevision
            newIdx.insert(newIdx.end(), { v1, a, c });
            newIdx.insert(newIdx.end(), { v2, b, a });
            newIdx.insert(newIdx.end(), { v3, c, b });
            newIdx.insert(newIdx.end(), { a, b, c });
        }

        // replace old indices with new ones
        idx = newIdx;
    }

    //Creating normals for Directional Lighting 
    // Flatten verts into a float array for OpenGL
    std::vector<float> flatData;
    //For loop to normalize each vert
    for (size_t i = 0; i < verts.size(); ++i) {
        glm::vec3 v = verts[i];
        glm::vec3 n = glm::normalize(v); // normal
        //push_back appends the array so that if our subdevisions or amount of verticies change, it will account for it.
        flatData.push_back(v.x); flatData.push_back(v.y); flatData.push_back(v.z);
        flatData.push_back(n.x); flatData.push_back(n.y); flatData.push_back(n.z);
    }




    //We Need to use VBO's VAO's and EBO's in modern openGL. 
    // A VBO is basically like a backpack that's used to hold all the vertex and color data 
    // A VAO is a vertex array object, this tells the GPU how to use the data we give it from the VBO
    //EBO is there to handle reused verticies to save memory. So if we have 2 verticies in the same place, it'll handle it as 1.
    unsigned int VBO, VAO, EBO;

    //Creates 1 Vertex Array Object and stores its ID in VAO.
    glGenVertexArrays(1, &VAO);
    // Same as VAO but also stores your vertex data
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);


    // This here makes the VAO you just created the current active one 
    glBindVertexArray(VAO);

    //makes the VBO you just created the current active one 
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    //This uploads your vertex data (positions, colors, normals, etc.) from CPU memory (RAM) into GPU memory (VRAM).
    //Target Type, Size in bytes of the data, Pointer to the data in CPU memory, Usage hint
    //There are 3 types of usage hints: 
    //GL_STATIC_DRAW:  Good for static meshes and unmoving meshes
    //GL_DYNAMIC_DRAW: Good for geometry that moves more often, like animations 
    //GL_STREAM_DRAW:  Good for simulations
    glBufferData(GL_ARRAY_BUFFER, flatData.size() * sizeof(float), flatData.data(), GL_STATIC_DRAW);

    //Binds your EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    //same here, just uploads index data to GPU
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);

    // If you look back at our verticies, each vert has 3 floats for position and 3 floats for color. That's 6 floats total.

    //index,size,type,normalized,stride,pointer.

    // 0 --                 This connects to layout(location = 0) (loaction =  1 would be color) in our shader.
    // 3 --                 each vertex position has 3 floats (x,y,z) or (r,g,b)
    // GL_FLOAT --          Data is float 
    // GL_FALSE --          don't normalize 
    // 6 * sizeof(float) -- there are 6 floats per vertex total (color / position) so opengl knows where the next vertex starts 
    //(void*)0 --           The position data starts at the beginning of the vertex 

    //Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);



    //Shaders:

    const char* vertexShaderSource = R"(
                #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aNormal;

        uniform mat4 MVP;
        uniform mat4 model;

        out vec3 FragPos;   // Position in world space
        out vec3 Normal;    // Normal in world space

        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal; 
            gl_Position = MVP * vec4(aPos, 1.0);
        }

        )";

    const char* fragmentShaderSource = R"(
     #version 330 core
        in vec3 FragPos;
        in vec3 Normal;

        out vec4 FragColor;

        uniform vec3 lightDir; // Directional light direction
        uniform vec3 lightColor;
        uniform vec3 objectColor;

        void main() {
            // Normalize input
            vec3 norm = normalize(Normal);
            vec3 light = normalize(-lightDir); // lightDir points TO the light

            // Diffuse lighting (Lambert)
            float diff = max(dot(norm, light), 0.0);
            vec3 diffuse = diff * lightColor;

            vec3 result = (diffuse) * objectColor;
            FragColor = vec4(result, 1.0);
        }

        )";


    //VERTEX SHADER
    //Creates a vertex shader object in GPU memory.
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    //Loads the shader code
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    //Compiles the shader from GLSL into GPU-understandable code.
    glCompileShader(vertexShader);


    //FRAGMENT SHADER
    //same for vertex but with pixels
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    //creates a shader program object
    unsigned int shaderProgram = glCreateProgram();
    //attaches your vertex and fragment shaders to the program.
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    //links them together so they can run as a single pipeline.
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);





    //While statment that keeps the window open until we close it. Basically a giant state machine. 
    //This is our physics process. Everything we want to run, will have to be called in here.
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    float myValue = 0.0f; // The value to be controlled by the slider

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Initializing shader program
        glUseProgram(shaderProgram);

        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();


        // Note: Projection + View + Model = MVP
        // MVP = combined transformation sent to the GPU


        // Field of view (FOV) in vertical degrees, converted to radians
        // Aspect ratio
        // Near Clip plane
        //Far Clip plane
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1000.0f / 700.0f, 0.1f, 100.0f);

        //This sets up the camera’s position and orientation.
        //creates a translation matrix.
        //moves the world backward 3 units so the camera can see the object.
        //Fun fact: you dont move the camera, you move the world around the camera.
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -7.0f));

        //This represents your object’s transformation: position, rotation, scale.
        //is the identity matrix → object is at origin, no rotation, no scaling.
        glm::mat4 model = glm::mat4(1.0f);
        //MVP = Model × View × Projection
        //This combines object space → world space → camera space → screen space.
        //Order matters
        glm::mat4 mvp = projection * view * model;
        //finds the location of the MVP uniform in your shader.
        //sends the 4x4 matrix to the GPU.

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP"), 1, GL_FALSE, glm::value_ptr(mvp));


        //Slider
     


        // Persistent color storage
        static float pickedColor[3] = { 0.0f, 0.0f, 1.0f }; // Start blue

        // ImGui window
        ImGui::SetNextWindowSize(ImVec2(250, 250));
        ImGui::Begin("Object Color");
        ImGui::ColorPicker3("Pick", pickedColor);
        ImGui::End();

        // Upload color to shader
        glm::vec3 objectColor(pickedColor[0], pickedColor[1], pickedColor[2]);
        glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(objectColor));




        glm::vec3 lightDirection = glm::normalize(glm::vec3(1.0f, 1.0f, 0.0f));

        //color of light

        static float pickedLightColor[3] = { 0.0f, 0.0f, 0.0f }; 
        

        ImGui::SetNextWindowSize(ImVec2(250, 250));
        ImGui::Begin("Light Color");
        ImGui::ColorPicker3("Pick", pickedLightColor);
        ImGui::End();

        glm::vec3 lightColor = glm::vec3(pickedLightColor[0], pickedLightColor[1], pickedLightColor[2]);
        // color of sphere


        //tells  the shader which direction the light is coming from
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightDir"), 1, glm::value_ptr(lightDirection));
        //This lets your shader know how to position, rotate, and scale your object in the world.
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, idx.size(), GL_UNSIGNED_INT, 0);


        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();



    //Frees up memory and terminates window when it's shut down. 
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();

    return 0;


}


