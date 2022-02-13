// CS 247 - Scientific Visualization, KAUST
//
// Programming Assignment #2&3
#include "CS247_prog.h"

// cycle clear colors
static void setClearColor(GLFWwindow* window)
{
    glfwMakeContextCurrent(window);
    switch(clearColor)
    {
        case 0:
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            break;
        case 1:
            glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
            break;
        default:
            glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
            break;
    }
}
static void nextClearColor(void)
{
    clearColor = (++clearColor) % 3;
    setClearColor(window2D);
    setClearColor(window3D);
}

// callbacks
// framebuffer to fix viewport
void frameBufferCallback(GLFWwindow* window, int width, int height)
{
    glfwMakeContextCurrent(window);
    // adjust viewport
    if(width > height)
        glViewport(0, 0, height, height);
    else
        glViewport(0, 0, width, width);
}
// key callback to take user inputs for both windows
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                std::cout << "Bye :D" << std::endl;
                glfwSetWindowShouldClose(window, true);
                break;
            case GLFW_KEY_W:
                current_slice[current_axis] = std::min((current_slice[current_axis] + 1), vol_dim[current_axis] - 1);
                fprintf(stderr, "increasing current slice: %i\n", current_slice[current_axis]);
                marchingSquares();
                break;
            case GLFW_KEY_S:
                current_slice[current_axis] = std::max((current_slice[current_axis] - 1), 0);
                fprintf(stderr, "decreasing current slice: %i\n", current_slice[current_axis]);
                marchingSquares();
                break;
            case GLFW_KEY_A:
                current_axis = ((current_axis + 1) % 3);
                fprintf(stderr, "toggling viewing axis to: %i\n", current_axis);
                marchingSquares();
                break;
            case GLFW_KEY_C:
                show_contour = !show_contour;
                fprintf(stderr, "toggle showing contour: %i\n", show_contour);
                marchingSquares();
                break;
            case GLFW_KEY_I:
                current_iso_value = std::min(current_iso_value+0.01f, 1.f);
                fprintf(stderr, "increasing current iso value to: %f\n", current_iso_value);
                marchingSquares();
                marchingCubes();
                break;
            case GLFW_KEY_K:
                current_iso_value = std::max(current_iso_value-0.01f, 0.f);
                fprintf(stderr, "decreasing current iso value to: %f\n", current_iso_value);
                marchingSquares();
                marchingCubes();
                break;
            case GLFW_KEY_O:
                current_iso_value = std::min(current_iso_value+0.1f, 1.f);
                fprintf(stderr, "increasing current iso value to: %f\n", current_iso_value);
                marchingSquares();
                marchingCubes();
                break;
            case GLFW_KEY_L:
                current_iso_value = std::max(current_iso_value-0.1f, 0.f);
                fprintf(stderr, "decreasing current iso value to: %f\n", current_iso_value);
                marchingSquares();
                marchingCubes();
                break;
            case GLFW_KEY_1:
                loadData("../data/lobster.dat");
                break;
            case GLFW_KEY_2:
                loadData("../data/skewed_head.dat");
                break;
            case GLFW_KEY_B:
                nextClearColor();
                break;
            case GLFW_KEY_HOME:
                // return to initial state/view
                fXDiff    = 0;
                fYDiff    = 0;
                fZDiff    = 0;
                xLastIncr = 0;
                yLastIncr = 0;
                fScale    = 0.75;
                break;
            default:
                fprintf(stderr, "\nKeyboard commands:\n\n"
                                "b - Toggle among background clear colors\n"
                                "w - Increase current slice\n"
                                "s - Decrease current slice\n"
                                "a - Toggle viewing axis\n"
                                "c - Toggle contour\n"
                                "i - Increase iso-value by 0.01\n"
                                "o - Increase iso-value by 0.1\n"
                                "k - Decrease iso-value by 0.01\n"
                                "l - Decrease iso-value by 0.1\n"
                                "1 - Load lobster dataset\n"
                                "2 - Load head dataset\n");
                break;
        }
    }
}
// key callback to enable interaction in 3D window
static void cursor3DCallback(GLFWwindow* window, double x, double y)
{
    // to enable continuous rotation
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
            xLast = -1;
            yLast = -1;
    }

    // rotation/scaling when left button is pressed
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if ((xLast != -1) || (yLast != -1)) {
            xLastIncr = (x - xLast);
            yLastIncr = (y - yLast);
            if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
                if (xLast != -1) { //zoom
                    fScale = std::max(fScale + yLastIncr * SCALE_FACTOR, 0.f);
                    fZDiff += xLastIncr * ROTATE_FACTOR;
                }
            } else {
                if (xLast != -1) { // rotate
                    fXDiff += xLastIncr * ROTATE_FACTOR;
                    fYDiff += yLastIncr * ROTATE_FACTOR;
                }
            }
        }
        xLast = x;
        yLast = y;
    }
    xLastIncr = 0;
    yLastIncr = 0;
}
// glfw error callback
static void errorCallback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

// data
// loading data from file
void loadData(char* filename)
{
    fprintf(stderr, "loading data %s\n", filename);

    FILE* fp;
    // open file, read only, binary mode
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open file %s for reading.\n", filename);
        return;
    }

    // read volume dimension
    fread(&vol_dim[0], sizeof(unsigned short), 1, fp);
    fread(&vol_dim[1], sizeof(unsigned short), 1, fp);
    fread(&vol_dim[2], sizeof(unsigned short), 1, fp);

    fprintf(stderr, "volume dimensions: x: %i, y: %i, z:%i \n", vol_dim[0], vol_dim[1], vol_dim[2]);

    if (data_array != NULL) {
        delete[] data_array;
    }

    data_array = new unsigned short[vol_dim[0] * vol_dim[1] * vol_dim[2]]; //for intensity volume

    for(int z = 0; z < vol_dim[2]; z++) {
        for(int y = 0; y < vol_dim[1]; y++) {
            for(int x = 0; x < vol_dim[0]; x++) {

                fread(&data_array[x + (y * vol_dim[0]) + (z * vol_dim[0] * vol_dim[1])], sizeof(unsigned short), 1, fp);
                data_array[x + (y * vol_dim[0]) + (z * vol_dim[0] * vol_dim[1])] = data_array[x + (y * vol_dim[0]) + (z * vol_dim[0] * vol_dim[1])] << 4;
            }
        }
    }

    fclose(fp);

    current_slice[0] = vol_dim[0] / 2;
    current_slice[1] = vol_dim[1] / 2;
    current_slice[2] = vol_dim[2] / 2;

    downloadVolumeAsTexture();

    data_loaded = true;

    marchingSquares();
    marchingCubes();
}
// load volume texture
void downloadVolumeAsTexture()
{
    // texture is only needed in 2D window, so we switch to 2D context
    GLFWwindow* currentWindow = glfwGetCurrentContext();
    glfwMakeContextCurrent(window2D);

    fprintf(stderr, "downloading volume to 3D texture\n");

    glEnable(GL_TEXTURE_3D);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_3D, texture);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_INTENSITY16, vol_dim[0], vol_dim[1], vol_dim[2], 0, GL_LUMINANCE, GL_UNSIGNED_SHORT, data_array);

    // switch back to current window
    glfwMakeContextCurrent(currentWindow);
}

// initializations
// init application
bool initApplication(int argc, char **argv)
{

    std::string version((const char *)glGetString(GL_VERSION));
    std::stringstream stream(version);
    unsigned major, minor;
    char dot;

    stream >> major >> dot >> minor;

    assert(dot == '.');
    if (major > 3 || (major == 2 && minor >= 0)) {
        std::cout << "OpenGL Version " << major << "." << minor << std::endl;
    } else {
        std::cout << "The minimum required OpenGL version is not supported on this machine. Supported is only " << major << "." << minor << std::endl;
        return false;
    }

    return true;
}
// set up the 2D scene
void setup2D() {
    // compile & link shader
    // slice shader
    sliceProgram.compileShader("../shaders/slice2D.vs");
    sliceProgram.compileShader("../shaders/slice2D.fs");
    sliceProgram.link();

    // make slice
    // see: vboquad.h and vboquad.cpp
    slice.init();

    // TODO: iso contour VAO and VBO
    // HINT: we only need the 2D positions

}
// set up the 3D scene
void setup3D() {
    // compile & link shader
    volumeProgram.compileShader("../shaders/volume3D.vs");
    volumeProgram.compileShader("../shaders/volume3D.fs");
    volumeProgram.link();

    // view & projection matrices
    view = glm::lookAt(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    projection = glm::ortho(-1.f, 1.f, -1.f, 1.f, -10.f, 10.f);

    // make bounding box
    // see: vbocube.h and vbocube.cpp
    boundingBox.init();

    // TODO: iso surface VAO and VBO
    // HINT: we need the 3D positions and normals!
}


// rendering
// render the 2D frame
void render2D() {
    // draw the slice
    glBindTexture(GL_TEXTURE_3D, texture);
    sliceProgram.use();

    // adjust aspect ratio
    first_axis = current_axis == 0;
    second_axis = current_axis == 2 ? 1 : 2;
    if(vol_dim[first_axis] > vol_dim[second_axis]) {
        aspect_2d_x = 1.f;
        aspect_2d_y = (float) vol_dim[second_axis] / vol_dim[first_axis];
    }
    else {
        aspect_2d_x = (float) vol_dim[first_axis] / vol_dim[second_axis];
        aspect_2d_y = 1.f;
    }
    model = mat4(1);
    model = glm::scale(model, glm::vec3(aspect_2d_x, aspect_2d_y, 1));

    // permutation
    texCoordTransform = glm::mat4(0);
    texCoordTransform[3][3] = 1;
    if(current_axis==2) {
        /*
         * 1    0    0
         * 0    1    0
         * 0    0    1
         */
        texCoordTransform[0][0] = 1;
        texCoordTransform[1][1] = 1;
        texCoordTransform[2][2] = 1;
    } else if(current_axis==1) {
        /*
         * 1    0    0
         * 0    0    1
         * 0    1    0
         */
        texCoordTransform[0][0] = 1;
        texCoordTransform[2][1] = 1;
        texCoordTransform[1][2] = 1;
    } else {
        /*
         * 0    1    0
         * 0    0    1
         * 1    0    0
         */
        texCoordTransform[2][0] = 1;
        texCoordTransform[0][1] = 1;
        texCoordTransform[1][2] = 1;
    }
    texCoordTransform = glm::translate(texCoordTransform, glm::vec3(0, 0, (float)current_slice[current_axis] / vol_dim[current_axis]));

    sliceProgram.setUniform("vertexColor", glm::vec4(0));
    sliceProgram.setUniform("texCoordTransform", texCoordTransform);
    sliceProgram.setUniform("model", model);

    slice.render();

    // overlay iso contour
    if(show_contour){
        drawContour();
    }
}
// render the 3D frame
void render3D() {
    volumeProgram.use();

    // set up model matrix
    model = glm::mat4(1);

    // adjust aspect ratio
    if (data_loaded) {
        aspect_3d_z = (vol_dim[2] / (float)vol_dim[0]);
        aspect_3d_y = (vol_dim[1] / (float)vol_dim[0]);
    }

    // user interaction
    model = glm::rotate(model, glm::radians(fYDiff), vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(fXDiff), vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(fZDiff), vec3(0, 0, 1));
    model = glm::scale(model, glm::vec3(fScale, fScale * aspect_3d_y, fScale * aspect_3d_z));


    volumeProgram.setUniform("model", model);
    volumeProgram.setUniform("view", view);
    volumeProgram.setUniform("projection", projection);

    // bounding box color
    volumeProgram.setUniform("vertexColor", glm::vec3(0.8));

    // render bounding box
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    boundingBox.render();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // overlay iso surface
    if (data_loaded) {
        drawSurface();
    }
}

// entry point
int main(int argc, char** argv)
{
    // initialize variables
    data_array = nullptr;

    current_slice[0] = 0;
    current_slice[1] = 0;
    current_slice[2] = 0;
    current_axis = 2;

    aspect_3d_z = 1.f;
    aspect_3d_y = 1.f;
    aspect_2d_x = 1.f;
    aspect_2d_y = 1.f;

    fXDiff = 0;
    fYDiff = 0;
    fZDiff = 0;
    xLastIncr = 0;
    yLastIncr = 0;

    fScale = .75f;
    xLast = -1;
    yLast = -1;

    data_loaded = false;

    clearColor = 0;

    show_contour = true;
    current_iso_value = 0.5f;

    // set glfw error callback
    glfwSetErrorCallback(errorCallback);

    // init glfw
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    // init glfw windows
    // 2D window for visualizing iso contour
    window2D = glfwCreateWindow(gWindowWidth, gWindowHeight, "CS247 - Scientific Visualization - Marching Squares", nullptr, nullptr);
    if (!window2D)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwSetWindowPos(window2D, 100, 100);
    glfwMakeContextCurrent(window2D);
    gladLoadGL();

    // 3D window for visualizing iso surface
    window3D = glfwCreateWindow(gWindowWidth, gWindowHeight, "CS247 - Scientific Visualization - Marching Cubes", nullptr, nullptr);
    if (!window3D)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwSetWindowPos(window3D, 200 + gWindowWidth, 100);
    glfwMakeContextCurrent(window3D);
    gladLoadGL();


    // init our application
    if (!initApplication(argc, argv)) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // set GLFW callback functions
    glfwSetKeyCallback(window2D, keyCallback);
    glfwSetKeyCallback(window3D, keyCallback);
    glfwSetFramebufferSizeCallback(window2D, frameBufferCallback);
    glfwSetFramebufferSizeCallback(window3D, frameBufferCallback);
    glfwSetCursorPosCallback(window3D, cursor3DCallback);

    // print menu
    keyCallback(window2D, GLFW_KEY_BACKSLASH, 0, GLFW_PRESS, 0);

    // set up the 2D scene
    glfwMakeContextCurrent(window2D);
    setup2D();

    // set up the 3D scene
    glfwMakeContextCurrent(window3D);
    setup3D();

    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    // main loop
    while (!glfwWindowShouldClose(window2D) && !glfwWindowShouldClose(window3D))
    {
        /** 2D **/
        // work on 2D context
        glfwMakeContextCurrent(window2D);
        // clear 2D frame buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // render the 2D frame
        render2D();
        // swap 2D front and back buffers
        glfwSwapBuffers(window2D);

        /** 3D **/
        // work on 3D context
        glfwMakeContextCurrent(window3D);
        // clear 3D frame buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // render the 3D frame
        render3D();
        // swap 3D front and back buffers
        glfwSwapBuffers(window3D);

        // poll and process input events (keyboard, mouse, window, ...)
        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}

// TODO: define any useful functions you might need.
//  e.g., indexing, linear interpolation, central difference..etc

// 2D screen
void marchingSquares()
{
    // TODO: Extract 2D surface using marching squares algorithm


    // update VBO
    glfwMakeContextCurrent(window2D);
    // TODO: update VBO
    // HINT: you can use: glBufferData(GL_ARRAY_BUFFER, SIZE * sizeof(float), DATA, GL_STREAM_DRAW);

}
void drawContour()
{
    glBindTexture(GL_TEXTURE_3D, 0);
    sliceProgram.use();
    sliceProgram.setUniform("vertexColor", glm::vec4(0.99608, 0.36471 , 0.14902, 1.f));
    // TODO: add drawing code for your 2D contours

}

// 3D screen
void marchingCubes()
{
    // TODO: Extract 3D surface using marching cubes algorithm


    // TODO: update VBO

}
void drawSurface()
{
    volumeProgram.setUniform("vertexColor", glm::vec3(0.2f, 0.5f, 0.7f));
    volumeProgram.setUniform("model", model);

    // TODO: draw your 3D surface.

}