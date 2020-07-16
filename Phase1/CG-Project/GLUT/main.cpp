#include<fstream>
#include<sstream>
#include <stdlib.h>
#include <time.h>
#include "../Generator/Generator/tinyxml2/tinyxml2.cpp"
#include <unordered_map>
#include <vector>
#include <iostream>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include "glut/GL/glut.h"
#endif

using namespace tinyxml2;


std::string pathGen = "../../Generator/Debug/";
char *pathXML = "../../Generator/Debug/Files.xml";
bool color = false;
unsigned int steps = -1;
unsigned int stepRange = 1;


#define XMLDOC "Files.xml"


float translate[3] = { 0,0,0 };
float rotate[4] = { 0,0,0,0 };


void removeChar(std::string& str, char character)
{
	size_t pos;
	while ((pos = str.find(character)) != std::string::npos)
		str[pos] = ' ';
}

std::vector<std::vector<float>> fileToVector(std::string file) {

	std::vector<std::vector<float>> ret;

	std::ifstream fd(file);

	if (fd.fail()) {
		printf("Model File does not exist.");
		exit(-1);
	}

	float x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12;

	for (std::string line; getline(fd, line);) {

		removeChar(line, ',');

		std::istringstream data(line);
		
		data >> x1 >> x2 >> x3 >> x4 >> x5 >> x6 >> x7 >> x8 >> x9 >> x10 >> x11 >> x12;

		std::vector<float> t = { x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12 };
		
		ret.push_back(t);

	}

	return ret;

}

class Models{
public:
	std::unordered_map<std::string, std::vector<std::vector<float>>> data;
	std::vector<std::string> files;
	int pos;
	int len;

	Models() {
		pos = 0;
		len = 0;
	}
	
public:
	void addFile(std::string file) {
		data[file] = fileToVector(pathGen + file);
		files.push_back(file);
		len++;
	}

	void stepDrawGL(std::string key,unsigned int steps, bool oneColor = true) {
		if (data.find(key) != data.end()) {
			int i = 0;
			for (std::vector<std::vector<float>>::iterator it = data[key].begin(); it != data[key].end() && i < steps; it++, i++) {
				if (oneColor == false) {
					glColor3f(it->at(0), it->at(1), it->at(2));
				}

				for (int i = 3; i < 12; i += 3) {
					glVertex3f(it->at(i), it->at(i + 1), it->at(i + 2));
				}

			}

			steps = steps >= data[key].size() ? data[key].size() : steps;

		}
		else {
			std::cout << "File with the name " << key << "does not exist.";
		}
	}

	void drawGL(std::string key, bool oneColor = true){
		if (data.find(key) != data.end()) {
			for (std::vector<std::vector<float>>::iterator it = data[key].begin(); it != data[key].end(); it++) {
				if (oneColor == false) {
					glColor3f(it->at(0), it->at(1), it->at(2));
				}

				for (int i = 3; i < 12; i+=3) {
					glVertex3f(it->at(i), it->at(i + 1),it->at( i + 2 ));
				}
				
			}

		}
		else {
			std::cout << "File with the name " << key << "does not exist.";
		}

	}

	void printData() {

		std::unordered_map<std::string, std::vector<std::vector<float>>>::iterator itr;

		for (itr = data.begin(); itr != data.end(); itr++)
		{
			// itr works as a pointer to pair<string, double> 
			// type itr->first stores the key part  and 
			// itr->second stroes the value part 
			std::cout << itr->first << "  " << std::endl;
		}
	}
};

Models* models = new Models();


void fileToGL(std::string file, bool oneColor = true) {

	std::ifstream fd(file);

	if (fd.fail()) {
		printf("File does not exist.");
		exit(-1);
	}

	float x, y, z;
	int i;

	for (std::string line; getline(fd, line);) {

		removeChar(line, ',');

		std::istringstream data(line);
		
		data >> x >> y >> z;
		if (oneColor == false) {
			glColor3f(x, y, z);
		}

		for (i = 0; i < 3; i++) {
			data >> x >> y >> z;
			glVertex3f(x, y, z);

		}
	}
}



void readXMLFile() {
	XMLDocument doc;
	if (doc.LoadFile(pathXML) == XML_SUCCESS) {
		XMLElement* root = doc.RootElement();
		for (XMLElement* child = root->FirstChildElement(); child != NULL; child = child->NextSiblingElement()) {
			std::string name = child->Attribute("file");
			models->addFile(name);
		}
	}
	else {
		printf("Nenhum modelo gerado de momento! Gere em %s\n", pathXML);
		exit(0);
	}

}

void changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window with zero width).
	if(h == 0)
		h = 1;

	// compute window's aspect ratio 
	float ratio = w * 1.0 / h;

	// Set the projection matrix as current
	glMatrixMode(GL_PROJECTION);
	// Load Identity Matrix
	glLoadIdentity();
	
	// Set the viewport to be the entire window
    glViewport(0, 0, w, h);

	// Set perspective
	gluPerspective(45.0f ,ratio, 1.0f ,1000.0f);

	// return to the model view matrix mode
	glMatrixMode(GL_MODELVIEW);
}


void renderScene(void) {

	// clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set the camera
	glLoadIdentity();
	gluLookAt(5.0, 5.0, 5.0,
		0.0, 0.0, 0.0,
		0.0f, 1.0f, 0.0f);

	glBegin(GL_LINES);
	// X axis in red
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(-100.0f, 0.0f, 0.0f);
	glVertex3f(100.0f, 0.0f, 0.0f);
	// Y Axis in Green
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, -100.0f, 0.0f);
	glVertex3f(0.0f, 100.0f, 0.0f);
	// Z Axis in Blue
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, -100.0f);
	glVertex3f(0.0f, 0.0f, 100.0f);
	glEnd();


	// put the geometric transformations here
	glTranslatef(translate[0],translate[1],translate[2]);
	glRotatef(rotate[0],rotate[1],rotate[2],rotate[3]);

	// put drawing instructions here
	//glBegin(GL_TRIANGLES);
	//glColor3f(0.0f, 1.0f, 0.0f);

	/*
	glPushMatrix();
	glutWireSphere(1, 2, 2);
	glPopMatrix();
	*/
	

	glBegin(GL_TRIANGLES);
	models->drawGL(models->files[models->pos], color);
	glEnd();

	// End of frame
	glutSwapBuffers();
}



// write function to process keyboard events




void function(unsigned char key, int x, int y) { 
	if (key == 'A' || key == 'a') {
		translate[0] += -.5;
		glutPostRedisplay();
	}
	else if (key == 'S' || key == 's') {
		translate[1] += -.5;
		glutPostRedisplay();
	}
	else if (key == 'd' || key == 'D') {
		translate[0] += .5;
		glutPostRedisplay();
	}
	else if (key == 'w' || key == 'W') {
		translate[1] += .5;
		glutPostRedisplay();
	}
	else if (key == 'R' || key == 'r') {
		rotate[0] += 5;
		rotate[2] = 1.0;
		glutPostRedisplay();
	}

	else if (key == 't' || key == 'T') {
		rotate[0] += 5;
		rotate[1] = 1.0;
		glutPostRedisplay();
	}
	else if (key == 'N' || key == 'n') {
		models->pos = ((models->pos)+1) % models->len;
		glutPostRedisplay();

	}
	else if (key == 'C' || key == 'c') {
		color = !color;
		glutPostRedisplay();
	}
	else if (key == 'k' || key == 'K') {
		steps+=stepRange;
		glutPostRedisplay();
	}
	else if (key == 'j' || key == 'J') {
		steps-=stepRange;
		glutPostRedisplay();
	}
	else if (key == 'i' || key == 'I') {
		stepRange = stepRange >= 20 ? 20 : stepRange + 1;
		glutPostRedisplay();
	}
	else if (key == 'u' || key == 'U') {
		stepRange = stepRange <= 1 ? 1 : stepRange-1;
		glutPostRedisplay();
	}
}

int main(int argc, char **argv) {
// Read XML structered models.
	readXMLFile();
// init GLUT and the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(800,800);
	glutCreateWindow("CG@DI-UM");
		
// Required callback registry 
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);

	
// put here the registration of the keyboard callbacks
	glutKeyboardFunc(function);


//  OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
// enter GLUT's main cycle
	glutMainLoop();
	
	return 1;
}
