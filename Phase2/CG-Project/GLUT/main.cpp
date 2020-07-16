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


std::string pathGen = "../../../Generator/Debug/";
std::string pathXML = "../../../Generator/Debug/Files.xml";
bool color = false;
unsigned int steps = -1;
unsigned int stepRange = 1;


#define XMLDOC "Files.xml"

float alfa = 0.0f, beta = 0.5f, radius = 100.0f;
float camX, camY, camZ;

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
class Model {
public:
	std::vector < std::vector<float>> points;
	std::vector < float > translation;
	std::vector < float > rotation;
	std::vector < float > scale;

	Model() {
		translation = { 0,0,0 };
		rotation = { 0,0,0,0 };
		scale = { 0,0,0 };
	}
	void setTranslation(float x, float y, float z) {
		translation[0] = x;
		translation[1] = y;
		translation[2] = z;

	}
	void setRotation(float angle, float x, float y, float z) {
		rotation[0] = angle;
		rotation[1] = x;
		rotation[2] = y;
		rotation[3] = z;
	}
	void setScale(float x, float y, float z) {
		scale[0] = x;
		scale[1] = y;
		scale[2] = z;

	}

};
class Models {
public:
	std::unordered_map<std::string, std::vector<Model*>> data;
	std::vector<std::string> files;
	int pos;
	int len;

	Models() {
		pos = 0;
		len = 0;
	}

public:
	//get points from file and add to hashmap
	void addFile(std::string file, std::vector<float> trans, std::vector<float> rot, std::vector<float> sca) {
		Model* m = new Model();
		m->setTranslation(trans[0], trans[1], trans[2]);
		m->setRotation(rot[0],rot[1],rot[2],rot[3]);
		m->setScale(sca[0], sca[1], sca[2]);
		m->points = fileToVector(pathGen + file);
		if (data.find(file) != data.end()) {
			data[file].push_back(m);
		}
		else {
			auto vect = new std::vector<Model*>();
			vect->push_back(m);
			data[file] = (*vect);
		}
		files.push_back(file);
		len++;
	}
	/*
	void addFile(std::string file) {
		Model* m = new Model();
		m->points = fileToVector(pathGen + file);
		data[file] = m;
		files.push_back(file);
		len++;
	}
	*/
	void stepDrawGL(std::string key, unsigned int steps, bool oneColor = true, bool debug = false) {
		if (data.find(key) != data.end()) {
			int i = 0;
			int j = 0;
			
			for (auto aux : data[key]) {
				i = 0;
				if (debug) {
					std::cout << std::endl << key << std::endl;
					std::cout << "Numero de pontos: " << aux->points.size() << std::endl;
					std::cout << "Tranlation data: (" << aux->translation[0] << ", " << aux->translation[1] << ", " << aux->translation[2] << ")" << std::endl;
					std::cout << "Rotation data: (" << aux->rotation[0] << ", " << aux->rotation[1] << ", " << aux->rotation[2] << ", " << aux->rotation[3] << ")" << std::endl;
					std::cout << "Scale data: (" << aux->scale[0] << ", " << aux->scale[1] << ", " << aux->scale[2] << ")" << std::endl;
				}
				
				glPushMatrix();

				glTranslatef(aux->translation[0], aux->translation[1], aux->translation[2]);
				glRotatef(aux->rotation[0], aux->rotation[1], aux->rotation[2], aux->rotation[3]);
				glScalef(aux->scale[0], aux->scale[1], aux->scale[2]);
				glBegin(GL_TRIANGLES);
				for (std::vector<std::vector<float>>::iterator it = aux->points.begin(); it != aux->points.end() && i < steps; it++, i++) {
					j++;
					if (oneColor == false) {
						glColor3f(it->at(0), it->at(1), it->at(2));
					}

					for (int i = 3; i < 12; i += 3) {
						glVertex3f(it->at(i), it->at(i + 1), it->at(i + 2));
					}

				}
				if(debug) std::cout << "Pontos processados: " << j << "; Pontos processados esperados: " << aux->points.size() * data[key].size() << " (" << (float)j / (float)(aux->points.size() * data[key].size()) * 100 << "%)" << std::endl;
				glEnd();
				glPopMatrix();
				steps = steps >= aux->points.size() ? aux->points.size() : steps;

			}


		}
		else {
			std::cout << "File with the name " << key << "does not exist.";
		}
		
	}

	void drawGL(std::string key, bool oneColor = true) {
		if (data.find(key) != data.end()) {

			for (auto aux : data[key]) {

				glPushMatrix();
				glTranslatef(aux->translation[0], aux->translation[1], aux->translation[2]);
				glRotatef(aux->rotation[0], aux->rotation[1], aux->rotation[2], aux->rotation[3]);
				glScalef(aux->scale[0], aux->scale[1], aux->scale[2]);
				glBegin(GL_TRIANGLES);

				for (std::vector<std::vector<float>>::iterator it = aux->points.begin(); it != aux->points.end(); it++) {
					if (oneColor == false) {
						glColor3f(it->at(0), it->at(1), it->at(2));
					}

					for (int i = 3; i < 12; i += 3) {
						glVertex3f(it->at(i), it->at(i + 1), it->at(i + 2));
					}

				}

				glPopMatrix();
				glEnd();

			}

		}
		else {
			std::cout << "File with the name " << key << "does not exist.";
		}
		
	}

	void printData() {

		std::unordered_map<std::string, std::vector<Model*>>::iterator itr;

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


// without hierarchical scenes
void readFileSM() {
	XMLDocument doc;
	if (doc.LoadFile(pathXML.c_str()) == XML_SUCCESS) {
		XMLElement* root = doc.RootElement();
		for (XMLElement* child = root->FirstChildElement(); child != NULL; child = child->NextSiblingElement()) {
			std::string name = child->Attribute("file");
			std::vector<float> translation = { 0,0,0 };
			std::vector<float> rotation = { 0,0,0,0 };
			std::vector<float> scale = { 1,1,1 };
			models->addFile(name, translation, rotation, scale);
		}
	}
	else {
		printf("Nenhum modelo gerado de momento! Gere em %s\n", pathXML);
		exit(0);
	}

}

class TransformationState{
public:
	std::vector < float > translation;
	std::vector < float > rotation;
	std::vector < float > scale;

	TransformationState() {
		translation = { 0,0,0 };
		rotation = { 0,0,0,0 };
		scale = { 1,1,1 };
	}
	
	TransformationState(std::vector<float> x, std::vector<float> y, std::vector<float> z) {
		translation = { x[0],x[1],x[2] };
		rotation = { y[0],y[1],y[2],y[3]};
		scale = { z[0], z[1],z[2]};
	}

	void translate(float x, float y, float z) {
		translation[0] = x;
		translation[1] = y;
		translation[2] = z;
	}

	void rotate(float x, float y, float z, float w) {
		rotation[0] = x;
		rotation[1] = y;
		rotation[2] = z;
		rotation[3] = w;
	}

	void setScale(float x, float y, float z) {
		scale[0] = x;
		scale[1] = y;
		scale[2] = z;
	}	

	TransformationState clone() {
		return TransformationState(translation, rotation, scale);
	}

};

void auxReadFile(XMLElement * elem, TransformationState ts) {
	for (XMLElement* child = elem->FirstChildElement(); child != NULL; child = child->NextSiblingElement()) {
		
		if (strcmp(child->Name(), "translate") == 0) {
			ts.translate(child->FindAttribute("X") ? atof(child->FindAttribute("X")->Value()) + ts.translation[0] : ts.translation[0], child->FindAttribute("Y") ? atof(child->FindAttribute("Y")->Value()) + ts.translation[1] : ts.translation[1], child->FindAttribute("Z") ? atof(child->FindAttribute("Z")->Value()) + ts.translation[2] : ts.translation[2]);
			
		}
		else if (strcmp(child->Name(), "rotate") == 0) {
			ts.rotate(child->FindAttribute("angle") ? atof(child->FindAttribute("angle")->Value()) + ts.rotation[0] : ts.rotation[0], child->FindAttribute("axisX") ? atof(child->FindAttribute("axisX")->Value()) + ts.rotation[1] : ts.rotation[1], child->FindAttribute("axisY") ? atof(child->FindAttribute("axisY")->Value()) + ts.rotation[2] : ts.rotation[2], child->FindAttribute("axisZ") ? atof(child->FindAttribute("axisZ")->Value()) + ts.rotation[3] : ts.rotation[3]);
		}
		else if (strcmp(child->Name(), "scale") == 0) {
			ts.setScale(child->FindAttribute("X") ? atof(child->FindAttribute("X")->Value())  : ts.scale[0], child->FindAttribute("Y") ? atof(child->FindAttribute("Y")->Value()) : ts.scale[1], child->FindAttribute("Z") ? atof(child->FindAttribute("Z")->Value()) : ts.scale[2]);
		}
		else if (strcmp(child->Name(), "models") == 0) {
			for (XMLElement* childModels = child->FirstChildElement(); childModels != NULL; childModels = childModels->NextSiblingElement()) {
				std::string filename = childModels->Attribute("file");
				models->addFile(filename, ts.translation, ts.rotation, ts.scale);
				
			}
		}
		else if (strcmp(child->Name(), "group") == 0) {
			auxReadFile(child, ts.clone());
	
		}

		else {
			std::cout << "What command is this? -> " << child->Name() << std::endl;
			exit(0);
		}
	}
}

void readXMLFile() {
	XMLDocument doc;
	if (doc.LoadFile(pathXML.c_str()) == XML_SUCCESS) {
		XMLElement* root = doc.RootElement();
		for (XMLElement* child = root->FirstChildElement(); child != NULL; child = child->NextSiblingElement()) {
			auxReadFile(child, TransformationState());
		}
	}
	else {
		printf("Nenhum modelo gerado de momento ou ficheiro mal formatado! Gere em %s\n", pathXML);
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
	gluLookAt(camX, camY, camZ,
		0.0, 0.0, 0.0,
		0.0f, 1.0f, 0.0f);

	glBegin(GL_LINES);
	// X axis in red
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(-200.0f, 0.0f, 0.0f);
	glVertex3f(200.0f, 0.0f, 0.0f);
	// Y Axis in Green
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, -200.0f, 0.0f);
	glVertex3f(0.0f, 200.0f, 0.0f);
	// Z Axis in Blue
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, -200.0f);
	glVertex3f(0.0f, 0.0f, 200.0f);
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
	
	
	//glBegin(GL_TRIANGLES);
	models->stepDrawGL(models->files[models->pos], steps ,color, false);
	glPushMatrix();
	glTranslatef(90,0,0);
	glRotatef(-26.73, 0, 0, 1);
	glRotatef(90,1,0,0);
	glutSolidTorus(1, 3.8, 2, 40);
	glPopMatrix();

	//glEnd();

	// End of frame
	glutSwapBuffers();
}

void spherical2Cartesian() {

	camX = radius * cos(beta) * sin(alfa);
	camY = radius * sin(beta);
	camZ = radius * cos(beta) * cos(alfa);
}

// write function to process keyboard events
void processSpecialKeys(int key, int xx, int yy) {

	switch (key) {

	case GLUT_KEY_RIGHT:
		alfa -= 0.1; break;

	case GLUT_KEY_LEFT:
		alfa += 0.1; break;

	case GLUT_KEY_UP:
		beta += 0.1f;
		if (beta > 1.5f)
			beta = 1.5f;
		break;

	case GLUT_KEY_DOWN:
		beta -= 0.1f;
		if (beta < -1.5f)
			beta = -1.5f;
		break;

	case GLUT_KEY_PAGE_DOWN: radius -= 1.0f;
		if (radius < 1.0f)
			radius = 1.0f;
		break;

	case GLUT_KEY_PAGE_UP: radius += 1.0f; break;
	}
	spherical2Cartesian();
	glutPostRedisplay();

}



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

	if (argc < 4) {
		std::cout << "\nWrong number of arguments provided!\n\n > ./Engine.exe <MODE> <PATH_TO_XML_AND_MODELS_DIR> <XML_FILE_NAME>\n\n Mode: 'x' (XML with Groups) and 's' (Simple Model(s) XML)\n PATH: XML and Model(s) directory path\n\n";
		exit(1);
	}
	else {
		try {


			if (strcmp(argv[1], "x") == 0) {
				pathGen = argv[2];
				pathXML = pathGen + argv[3];
				readXMLFile();
			} 
			else if (strcmp(argv[1], "s") == 0) {
				pathGen = argv[2];
				pathXML = pathGen + argv[3];
				readFileSM();
			}
			else {
				std::cout << "Invalid mode provided. MODES: 'x' or 's' not found!\n";
				exit(1);
			}
		}
		catch (...) {
			std::cout << "Something went wrong.\n";
		}
	}

// Read XML structered models.
// init GLUT and the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(800,800);
	glutCreateWindow("CG@DI-UM");
		
// Required callback registry 
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	glutSpecialFunc(processSpecialKeys);

	
// put here the registration of the keyboard callbacks
	glutKeyboardFunc(function);


//  OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	spherical2Cartesian();

// enter GLUT's main cycle
	glutMainLoop();
	
	return 1;
}
