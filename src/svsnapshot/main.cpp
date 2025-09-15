
//=============================================================
//  This file is part of the SymVox (Symmetry Voxelization) software
//  Copyright (C) 2016 by CRS4 Visual Computing Group, Pula, Italy
//
//  For more information, visit the CRS4 Visual Computing Group 
//  web pages at http://vic.crs4.it
//
//  This file may be used under the terms of the GNU General Public
//  License as published by the Free Software Foundation and appearing
//  in the file LICENSE included in the packaging of this file.
//
//  CRS4 reserves all rights not expressly granted herein.
//  
//  This file is provided AS IS with NO WARRANTY OF ANY KIND, 
//  INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS 
//  FOR A PARTICULAR PURPOSE.
//=============================================================


#include <fstream>
#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <symvox/encoded_octree.hpp>
#include <symvox/encoded_svdag.hpp>
#include <symvox/encoded_ussvdag.hpp>
#include <symvox/encoded_ssvdag.hpp>
#include "../svviewer/camera.hpp"
#include "../svviewer/renderer_monitor.hpp"
#include "../svviewer/octree_dda_renderer.hpp"

#define UPDATE_INFO_TIME 200.0 // ms
#define SCREEN_WIDTH	1280
#define SCREEN_HEIGHT	720

bool finish = false;
bool printCamera = false;
float frameTime = 0;
std::string filename = "";
static char filenameInput[128] = "";
static char filenameInput2[128] = "";

static char recordingInput[128] = "";
static char loadRecordingInput[128] = "";

OctreeDDARenderer * renderer;
EncodedOctree* encoded_octree;
RendererMonitor::FrameStats frameStats;
GLFWwindow * window;
Camera * cam;

bool loadFile(std::string inputFile) {
	filename = sl::pathname_base(inputFile);
	std::string ext = sl::pathname_extension(inputFile);
	bool incorrectFile = false;

	if (encoded_octree != nullptr)
        delete encoded_octree;

	if (ext == "svdag" || ext == "SVDAG") {
		encoded_octree = new EncodedSVDAG();
		if (!encoded_octree->load(inputFile)) incorrectFile = true;
	}
	else if (ext == "ussvdag" || ext == "USSVDAG") {
		encoded_octree = new EncodedUSSVDAG();
		if (!encoded_octree->load(inputFile)) incorrectFile = true;
	}
	else if (ext == "ssvdag" || ext == "SSVDAG"
		  || ext == "esvdag" || ext == "ESVDAG") {
		encoded_octree = new EncodedSSVDAG();
		if (!encoded_octree->load(inputFile)) incorrectFile = true;
	}
	else
		incorrectFile = true;
	return incorrectFile;
}

int main(int argc, char ** argv)
{
	// Arguments parsing
	if (argc < 17) {
		printf("Usage: svsnapshot model.[svdag | ussvdag | ssvdag] [position x y z] [target x y z] [up x y z] [fovy] [z_near] [z_far] [light_pos x y z]\n");
		exit(1);
	}

    float p_x = atof(argv[2]);
    float p_y = atof(argv[3]);
    float p_z = atof(argv[4]);

    float t_x = atof(argv[5]);
    float t_y = atof(argv[6]);
    float t_z = atof(argv[7]);

    float u_x = atof(argv[8]);
    float u_y = atof(argv[9]);
    float u_z = atof(argv[10]);

    float fovy = atof(argv[11]);
    float z_near = atof(argv[12]);
    float z_far = atof(argv[13]);

    float l_x = atof(argv[14]);
    float l_y = atof(argv[15]);
    float l_z = atof(argv[16]);


	std::string inputFile(argv[1]);

	bool incorrectFile = loadFile(inputFile);

	if (incorrectFile) {
		printf("* ERROR: Unsupported octree '%s'\n", inputFile.c_str());
		printf("         This viewer support '.svdag', '.ussvdag' and '.ssvdag' "
			"files, built with the SymVox tool 'svbuilder'.\n");
		return 1;
	}

	renderer = new OctreeDDARenderer(encoded_octree);
	renderer->setScreenResolution(SCREEN_WIDTH, SCREEN_HEIGHT);

	// Initialise GLFW --------------------------------
	if (!glfwInit()) {
		std::cerr << "ERROR: Failed to initialize GLFW" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Voxelator Viewer | Loading...", NULL, NULL);

	if (!window) {
		std::cerr << "ERROR: Can't open the window" << std::endl;
		glfwTerminate();
		return -1;
	}

	cam = new Camera(window);
	sl::aabox3f sceneBBox = renderer->getSceneBBox();
	cam->setInitCamera(sl::point3f(p_x, p_y, p_z),
		sl::point3f(t_x, t_y, t_z),
		(u_y > 0.0f ? Camera::Y_UP : Camera::Z_UP),
		fovy,
		z_near,
		z_far);

	cam->setWalkFactor(sceneBBox.diagonal().two_norm() * 0.001f);

	// Make the window's context current
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		printf("Failed to initialize GLEW\n");
		return -1;
	}

	glGetError();

	renderer->setCamera(cam);
	renderer->init();
	renderer->selectRenderMode(OctreeDDARenderer::VIEWER);
    renderer->setViewerRenderMode(3);
    renderer->toggleRandomColors();
	renderer->clearState();
	renderer->resetState();
	renderer->toggleRenderingStats();
    renderer->setLightPos(sl::point3f(l_x, l_y, l_z));

	glClearColor(0.3f, 0.5, 0.7f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);
    renderer->draw();
    renderer->saveLastFrameBMP("output.bmp");

    glfwTerminate();

	return 0;
}
