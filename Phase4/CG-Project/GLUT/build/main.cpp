#include<fstream>
#include<sstream>
#include <stdlib.h>
#include <time.h>
#include "../../Generator/Generator/tinyxml2/tinyxml2.cpp"
#include <unordered_map>
#include <vector>
#include <iostream>
#include <IL/il.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include "GL/glew.h"
#include "GL/glut.h"
#endif



// XML library. Helps reading XML files.
using namespace tinyxml2;

// Important paths for getting xml or simple models.
std::string pathGen = "../../../Generator/Debug/";
std::string pathXML = "../../../Generator/Debug/Files.xml";

// Color changing and step of the drawing variables.
bool color = false;
unsigned int steps = -1;
unsigned int stepRange = 1;


#define XMLDOC "Files.xml"


float alfa = 0.0f, beta = 0.5f, radius = 100.0f;
float camX, camY, camZ;

// Control variables for the movement of the screen with the keyboard keys.
float translate[3] = { 0,0,0 };
float rotate[4] = { 0,0,0,0 };

// Catmull variable.
int tecellation = 500;

// VBO buffer.
GLuint vertices_gl, normals, textures;

float previousY[3] = { 0,1,0 };




class Light {
public:
	char *type;
	float pos[4];
	float spotDir[3];
	float attenuation = 1;
	float angle;

	//POINT LIGHT
	Light(char *t, float x, float y, float z, float att) {
		type = t;
		pos[0] = x;
		pos[1] = y;
		pos[2] = z;
		pos[3] = 1.0;
		attenuation = att;
	}

	//SPOTLIGHT LIGHT
	Light(char* t, float posx, float posy, float posz, float ang, float att , float dirx, float diry, float dirz) {
		type = t;
		pos[0] = posx;
		pos[1] = posy;
		pos[2] = posz;
		pos[3] = 1.0;
		angle = isValid(ang)?ang:0;
		attenuation = att;
		spotDir[0] = dirx;
		spotDir[1] = diry;
		spotDir[2] = dirz;
	}

	//DIRECCIONAL LIGHT
	Light(char* t, float posx, float posy, float posz) {
		type = t;
		pos[0] = posx;
		pos[1] = posy;
		pos[2] = posz;
		pos[3] = 0.0;
	}

	bool isValid(float ang) {
		return ((ang >= 0 && ang <= 90) || ang == 180) ? true : false;
	}
};


// Light global variables.
std::vector<Light> lights;

int loadTexture(std::string s) {

	unsigned int t, tw, th;
	unsigned char* texData;
	unsigned int texID;

	ilInit();
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	ilGenImages(1, &t);
	ilBindImage(t);
	ilLoadImage((ILstring)s.c_str());
	
	tw = ilGetInteger(IL_IMAGE_WIDTH);
	th = ilGetInteger(IL_IMAGE_HEIGHT);
	ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	texData = ilGetData();

	glGenTextures(1, &texID);

	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
	//glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	

	return texID;
}

void buildRotMatrix(float* x, float* y, float* z, float* m) {

	m[0] = x[0]; m[1] = x[1]; m[2] = x[2]; m[3] = 0;
	m[4] = y[0]; m[5] = y[1]; m[6] = y[2]; m[7] = 0;
	m[8] = z[0]; m[9] = z[1]; m[10] = z[2]; m[11] = 0;
	m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
}


void cross(float* a, float* b, float* res) {

	res[0] = a[1] * b[2] - a[2] * b[1];
	res[1] = a[2] * b[0] - a[0] * b[2];
	res[2] = a[0] * b[1] - a[1] * b[0];
}


void normalize(float* a) {

	float l = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	a[0] = a[0] / l;
	a[1] = a[1] / l;
	a[2] = a[2] / l;
}

void multMatrixVector(float* m, float* v, float* res) {

	for (int j = 0; j < 4; ++j) {
		res[j] = 0;
		for (int k = 0; k < 4; ++k) {
			res[j] += v[k] * m[j * 4 + k];
		}
	}

}

void getCatmullRomPoint(float t, float* p0, float* p1, float* p2, float* p3, float* pos, float* deriv) {
	int i;
	// catmull-rom matrix
	float m[4][4] = { {-0.5f,  1.5f, -1.5f,  0.5f},
						{ 1.0f, -2.5f,  2.0f, -0.5f},
						{-0.5f,  0.0f,  0.5f,  0.0f},
						{ 0.0f,  1.0f,  0.0f,  0.0f} };


	for (i = 0; i < 3; i++) {
		float vectorA[4] = { 0,0,0,0 };
		float vectorP[4] = { p0[i],p1[i],p2[i],p3[i] };
		multMatrixVector(*m, vectorP, vectorA);

		float vectorT[4] = { pow(t,3),pow(t,2),pow(t,1),1 };
		float vectordT[4] = { 3 * pow(t,2),2 * t,1,0 };

		pos[i] = (vectorT[0] * vectorA[0]) + (vectorT[1] * vectorA[1]) + (vectorT[2] * vectorA[2]) + (vectorT[3] * vectorA[3]);
		deriv[i] = (vectordT[0] * vectorA[0]) + (vectordT[1] * vectorA[1]) + (vectordT[2] * vectorA[2]) + (vectordT[3] * vectorA[3]);

	}

}


// given  global t, returns the point in the curve
void getGlobalCatmullRomPoint(float gt, float* pos, float* deriv, std::vector<std::vector<float>> p) {
	int POINT_COUNT = p.size();
	float t = gt * POINT_COUNT; // this is the real global t
	int index = floor(t);  // which segment
	t = t - index; // where within  the segment

	// indices store the points
	int indices[4];
	indices[0] = (index + POINT_COUNT - 1) % POINT_COUNT;
	indices[1] = (indices[0] + 1) % POINT_COUNT;
	indices[2] = (indices[1] + 1) % POINT_COUNT;
	indices[3] = (indices[2] + 1) % POINT_COUNT;

	float* p0 = p[indices[0]].data();
	float* p1 = p[indices[1]].data();
	float* p2 = p[indices[2]].data();
	float* p3 = p[indices[3]].data();

	getCatmullRomPoint(t, p0, p1, p2, p3, pos, deriv);
}


void renderCatmullRomCurve(std::vector< std::vector< float >> basePoints) {

	// draw curve using line segments with GL_LINE_LOOP
	float res[3];
	float deriv[3];

	//gera a linha a partir dos pontos gerados acima
	glBegin(GL_LINE_LOOP);
	
	for (int i = 0; i < tecellation; i++) {
		getGlobalCatmullRomPoint(i / float(tecellation), res, deriv,basePoints);
		//glVertex3f(res[0], res[1], res[2]);
	}

	glEnd();
}


void removeChar(std::string& str, char character)
{
	size_t pos;
	while ((pos = str.find(character)) != std::string::npos)
		str[pos] = ' ';
}

std::tuple< std::vector<std::vector<float>>, std::vector<std::vector<float>>> fileToVector(std::string file) {

	std::vector<std::vector<float>> ret;

	std::ifstream fd(file);

	if (fd.fail()) {
		printf("Model File does not exist [%s].", file);
		exit(-1);
	}

	float x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12;
	float n1, n2, n3, n4, n5, n6, n7, n8, n9;
	float t1, t2, t3, t4, t5, t6, t7, t8, t9;

	std::vector< float > vbo;
	std::vector< float > normals;
	std::vector< float > texture;


	for (std::string line; getline(fd, line);) {
		
		removeChar(line, ',');
		removeChar(line, '|');

		std::istringstream data(line);
		
		data >> x1 >> x2 >> x3 >> x4 >> x5 >> x6 >> x7 >> x8 >> x9 >> x10 >> x11 >> x12; // Points.
		data >> n1 >> n2 >> n3 >> n4 >> n5 >> n6 >> n7 >> n8 >> n9; // Normals.
		data >> t1 >> t2 >> t3 >> t4 >> t5 >> t6 >> t7 >> t8 >> t9; // Texture coords.

		std::vector<float> t = { x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12 };

		vbo.insert(vbo.end(), { x4,x5,x6,x7,x8,x9,x10,x11,x12 });
		normals.insert(normals.end(), { n1,n2,n3,n4,n5,n6,n7,n8,n9 });


		ret.push_back(t);


		if (t1 <= 1) {
			//printf("%f %f %f %f %f %f\n", t1, t2, t4, t5, t7, t8);
			texture.insert(texture.end(), { t1, t2, t4, t5, t7, t8});
			
		}
		
		
		

	}

	//printf("-----\n");
	//std::reverse(texture.begin(), texture.end());


	std::vector<std::vector<float>> aux = { vbo, normals, texture};

	

	return std::make_tuple(ret, aux);

}


class Model {
public:
	std::vector < std::vector<float>> points; // Vertex drawing.
	std::vector < float > vbo_ready; // VBO's
	std::vector < float > normals;
	std::vector < float > texture;
	GLuint textureID = 0;
	std::vector<float> rgbDif;
	std::vector<float> rgbSpe;
	std::vector<float> rgbEme;
	std::vector<float> rgbAmb;
	std::vector< std::vector < std::vector<float>>> translation; // Translations.
	std::vector < float > rotation;
	float angleRot;
	std::vector < int > color;
	std::vector<float> time = {1,1,1,1,1};
	std::vector<float> elapsedTime = {0,0,0,0};
	int deepLevel;
	std::vector < float > scale;
	float rotationTime = 1;
	float angle = 0;

	Model() {
		rotation = { 0,0,0,0 };
		scale = { 0,0,0 };
		color = { 255,255,255 };
	}

	void setRotation( float x, float y, float z) {
		
		rotation[0] = x;
		rotation[1] = y;
		rotation[2] = z;
	}
	void setScale(float x, float y, float z) {
		scale[0] = x;
		scale[1] = y;
		scale[2] = z;

	}

	void setColor(int x, int y, int z) {
		color[0] = x;
		color[1] = y;
		color[2] = z;
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
	void addFile(std::string file, std::vector<std::vector<std::vector<float>>> trans, std::vector<float> rot, std::vector<float> sca, std::vector<float> timeTS, std::vector< int > colorTS, int deep, float rotTime, std::string tName, std::vector<float> rgbDifv, std::vector<float> rgbSpev, std::vector<float> rgbEmev, std::vector<float> rgbAmbv, float angle) {
		Model* m = new Model();
		m->translation = trans;
		m->setRotation(rot[0],rot[1],rot[2]);
		m->setScale(sca[0], sca[1], sca[2]);
		m->time = timeTS;

		m->angleRot = angle;

		m->rgbDif = rgbDifv;
		m->rgbSpe = rgbSpev;
		m->rgbEme = rgbEmev;
		m->rgbDif = rgbAmbv;
		

		if (tName.size() > 0) {
			std::string pathTex = "../../textures/";
			m->textureID = loadTexture(pathTex + tName);
			
		}
		

		auto dataTuple = fileToVector(pathGen + file);

		m->points = std::get<0>(dataTuple);
		m->vbo_ready = std::get<1>(dataTuple)[0];
		m->normals = std::get<1>(dataTuple)[1];
		m->texture = std::get<1>(dataTuple)[2];

		m->setColor(colorTS[0], colorTS[1], colorTS[2]);
		m->deepLevel = deep;
		m->rotationTime = rotTime;
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


	void drawVBO( unsigned int steps, bool oneColor = true, bool debug = false) {
		int i = 0;
		GLuint imgTexture;

		float eme[4] = { 1,1,1,1 };
		float defaultt[4] = { 0,0,0,1 };

		for (auto itr = data.begin(); itr != data.end(); itr++, i++) {
			std::string key = itr->first;
			for (auto aux : data[key]) {

				glPushMatrix();
				
				
				glColor3f(aux->color[0] / 100.0, aux->color[1] / 100.0, aux->color[2] / 100.0);

				float pos[3]; float deriv[3];
				for (int i = 0; i <= aux->deepLevel; i++) {
					if (aux->time[i] > 0) {
						
						//glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, eme);
						//renderCatmullRomCurve(aux->translation[i]);
						//glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, defaultt);

						getGlobalCatmullRomPoint(aux->elapsedTime[i] + (1.0 / aux->time[i]), pos, deriv, aux->translation[i]);
						glTranslatef(pos[0], pos[1], pos[2]);
						aux->elapsedTime[i] += (1.0 / aux->time[i]);

					}else {
						glTranslatef(aux->translation[aux->deepLevel][0][0], aux->translation[aux->deepLevel][0][1], aux->translation[aux->deepLevel][0][2]);
					}
					
				}
				
				if (aux->rotationTime != 0) {
					glRotatef(aux->angle, aux->rotation[0], aux->rotation[1], aux->rotation[2]);
					aux->angle += 360 / aux->rotationTime;
				} else {
					glRotatef(aux->angleRot, aux->rotation[0], aux->rotation[1], aux->rotation[2]);
				}
				
				glScalef(aux->scale[0], aux->scale[1], aux->scale[2]);
				
				int size = aux->vbo_ready.size();
				
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, aux->rgbDif.data());
				
				glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, aux->rgbSpe.data());
				
				glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, aux->rgbEme.data());
				
				//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, aux->rgbAmb.data());

				glBindBuffer(GL_ARRAY_BUFFER, vertices_gl);
				glBufferData(GL_ARRAY_BUFFER, sizeof(float) * size, aux->vbo_ready.data(), GL_STATIC_DRAW);
				glVertexPointer(3, GL_FLOAT, 0, 0);

				if (aux->normals.size() > 0) {
					glBindBuffer(GL_ARRAY_BUFFER, normals);
					glBufferData(GL_ARRAY_BUFFER, sizeof(float) * size, aux->normals.data(), GL_STATIC_DRAW);
					glNormalPointer(GL_FLOAT, 0, 0);
				}
				
				if (aux->texture.size() > 0 && aux->textureID > 0) {
					
					glBindBuffer(GL_ARRAY_BUFFER, textures);
					glBufferData(GL_ARRAY_BUFFER, sizeof(float) * aux->texture.size(), aux->texture.data(), GL_STATIC_DRAW);
					glTexCoordPointer(2, GL_FLOAT, 0, 0);

					glBindTexture(GL_TEXTURE_2D, aux->textureID);

					
				}
				
				
				glDrawArrays(GL_TRIANGLES, 0, (GLuint)(size / 3));

				if (aux->texture.size() > 0 && aux->textureID > 0) {
					glBindTexture(GL_TEXTURE_2D, 0);
				}


				//float mat_emi[] = { 0.0, 0.0, 0.0, 1.0 };
				//glMaterialfv(GL_FRONT, GL_EMISSION, mat_emi);

				
				
				glPopMatrix();

				
			}
			
			
		
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
			//glColor3f(x, y, z);
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
			std::vector<std::vector< std::vector< float >>> translation;
			std::vector<float> rotation = { 0,0,0,0 };
			std::vector<float> scale = { 1,1,1 };
			models->addFile(name, translation, rotation, scale, { 1,1,1,1 }, { 255,255,255 }, 0, 1, "", { 0.8, 0.8, 0.8, 1.0 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0.2, 0.2, 0.2, 1.0 }, 0);
		}
	}
	else {
		printf("Nenhum modelo gerado de momento! Gere em %s\n", pathXML);
		exit(0);
	}

}

class TransformationState{
public:
	std::vector<std::vector < std::vector<float> >> translation;
	std::vector< int > color = {1,1,1};
	std::vector < float > rotation;
	std::vector < float > scale;
	int deepLevel = -1;
	float angle = 0;
	std::vector<float> time;
	float rotationTime = 0;

	TransformationState() {
		rotation = { 0,0,0,0 };
		scale = { 1,1,1 };
		time = {1,1,1,1};
		color = { 255, 255, 255 };
		
	}
	
	TransformationState(int dl, std::vector<std::vector<std::vector<float>>> x, std::vector<float> y, std::vector<float> z, std::vector<float> time1, std::vector<int> color1, float rotationT, float angleT) {
		translation = x;
		rotation = { y[0],y[1],y[2]};
		scale = { z[0], z[1],z[2]};
		time = time1;
		angle = angleT;
		color = { color1[0],color1[1],color1[2] };
		deepLevel = dl;
		rotationTime = rotationT;
	}

	void translate(int i, std::vector<std::vector<float>> trans) {
		auto clone(trans);
		
		translation.push_back(clone);
		
	}

	void rotate(float y, float z, float w) {
		rotation[0] = y;
		rotation[1] = z;
		rotation[2] = w;
	}

	void setScale(float x, float y, float z) {
		scale[0] = x;
		scale[1] = y;
		scale[2] = z;
	}	


	TransformationState clone() {
		auto clone(translation);
		std::vector<int> clonedColor(color);
		
		return TransformationState(deepLevel,clone, rotation, scale,time, clonedColor, rotationTime, angle);
	}
	
};


void auxReadFile(XMLElement * elem, TransformationState ts) {
	for (XMLElement* child = elem->FirstChildElement(); child != NULL; child = child->NextSiblingElement()) {
		
		if (strcmp(child->Name(), "colour") == 0) {
			ts.color[0] = child->FindAttribute("R") ? atoi(child->FindAttribute("R")->Value()) : ts.color[0];
			ts.color[1] = child->FindAttribute("G") ? atoi(child->FindAttribute("G")->Value()) : ts.color[1];
			ts.color[2] = child->FindAttribute("B") ? atoi(child->FindAttribute("B")->Value()) : ts.color[2];
			
		}
		else if (strcmp(child->Name(), "translate") == 0) {
			if (child->FindAttribute("time")) {
				
				ts.time[ts.deepLevel + 1] = atof(child->FindAttribute("time")->Value()) > 0 ? atof(child->FindAttribute("time")->Value()) : 1000;
				std::vector<std::vector<float>> tstate;
				std::vector<float> points;
					for (auto tag = child->FirstChildElement(); tag != NULL; tag = tag->NextSiblingElement()) {
						
						 points = { tag->FindAttribute("X") ? (float)atof(tag->FindAttribute("X")->Value()) : 0, tag->FindAttribute("Y") ? (float)atof(tag->FindAttribute("Y")->Value()) : 0, tag->FindAttribute("Z") ? (float)atof(tag->FindAttribute("Z")->Value()) : 0 };
						
						tstate.push_back(std::vector<float>(points));
						
					}					

					
					ts.translate(ts.deepLevel, tstate);
					ts.deepLevel++;
			}	
			else {
				std::cout << "Attribute time missing from translate tag." << std::endl;
				exit(-1);
			}
		}
		else if (strcmp(child->Name(), "rotate") == 0) {

			if (child->FindAttribute("time")) {
				ts.rotationTime = atof(child->FindAttribute("time")->Value());
				ts.rotate(child->FindAttribute("axisX") ? atof(child->FindAttribute("axisX")->Value()) : 0, child->FindAttribute("axisY") ? atof(child->FindAttribute("axisY")->Value()) : 0, child->FindAttribute("axisZ") ? atof(child->FindAttribute("axisZ")->Value()) : 0);
				ts.angle = child->FindAttribute("ang") ? atof(child->FindAttribute("ang")->Value()) : 0;

			}
			else {
				std::cout << "Attribute time missing from translate tag." << std::endl;
				exit(-1);
			}
			
		}
		else if (strcmp(child->Name(), "scale") == 0) {
			ts.setScale(child->FindAttribute("X") ? atof(child->FindAttribute("X")->Value())  : ts.scale[0], child->FindAttribute("Y") ? atof(child->FindAttribute("Y")->Value()) : ts.scale[1], child->FindAttribute("Z") ? atof(child->FindAttribute("Z")->Value()) : ts.scale[2]);
		}
		else if (strcmp(child->Name(), "models") == 0) {
			for (XMLElement* childModels = child->FirstChildElement(); childModels != NULL; childModels = childModels->NextSiblingElement()) {
				std::string filename = childModels->Attribute("file");
				std::string textureName = "";
				std::vector<float> rgbDif = { 0.8, 0.8, 0.8, 1.0 };
				std::vector<float> rgbEme = { 0, 0, 0, 1 };
				std::vector<float> rgbSpe = { 0, 0, 0, 1 };
				std::vector<float> rgbAmb = { 0.2, 0.2, 0.2, 1.0 };
				
				
				if (childModels->FindAttribute("texture")) {
					
					textureName = childModels->Attribute("texture");
				}

				if (childModels->FindAttribute("emR") && childModels->FindAttribute("emG") && childModels->FindAttribute("emB")) {

					rgbEme = { (float)atof(childModels->Attribute("emR")),  (float)atof(childModels->Attribute("emG")) , (float)atof(childModels->Attribute("emB")) , 1};

				}

				if (childModels->FindAttribute("difR") && childModels->FindAttribute("difG") && childModels->FindAttribute("difB")) {

					rgbDif = { (float)atof(childModels->Attribute("difR")),  (float)atof(childModels->Attribute("difG")) , (float)atof(childModels->Attribute("difB")) , 1};

				}
				if (childModels->FindAttribute("speR") && childModels->FindAttribute("speG") && childModels->FindAttribute("speB")) {

					rgbSpe = { (float)atof(childModels->Attribute("speR")),  (float)atof(childModels->Attribute("speG")) , (float)atof(childModels->Attribute("speB")), 1 };

				}
				if (childModels->FindAttribute("ambR") && childModels->FindAttribute("ambG") && childModels->FindAttribute("ambB")) {

					rgbAmb = { (float)atof(childModels->Attribute("ambR")),  (float)atof(childModels->Attribute("ambG")) , (float)atof(childModels->Attribute("ambB")), 1.0 };

				}

				models->addFile(filename, ts.translation, ts.rotation, ts.scale,ts.time, ts.color,ts.deepLevel, ts.rotationTime, textureName, rgbDif, rgbSpe, rgbEme, rgbAmb, ts.angle);
				
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
			
			if (strcmp(child->Name(), "lights") == 0) {

				for (auto light = child->FirstChildElement(); light != NULL; light = light->NextSiblingElement()) {
					if (light->FindAttribute("type")) {
						//POINT LIGHT
						if ((strcmp(light->Attribute("type"),"POINT") == 0) && light->FindAttribute("posX") && light->FindAttribute("posY") && light->FindAttribute("posZ") && light->FindAttribute("att")) {
							Light pointLight ("POINT",(float)atof(light->FindAttribute("posX")->Value()), (float)atof(light->FindAttribute("posY")->Value()), (float)atof(light->FindAttribute("posZ")->Value()), (float)atof(light->FindAttribute("att")->Value()));

							lights.push_back(pointLight);

						}
						//SPOTLIGHT LIGHT
						else if ((strcmp(light->Attribute("type"), "SPOT") == 0) && light->FindAttribute("posX") && light->FindAttribute("posY") && light->FindAttribute("posZ") && light->FindAttribute("ang") && light->FindAttribute("att") && light->FindAttribute("dirX") && light->FindAttribute("dirY") && light->FindAttribute("dirZ")) {
							Light spotLight("SPOT", (float)atof(light->FindAttribute("posX")->Value()), (float)atof(light->FindAttribute("posY")->Value()), (float)atof(light->FindAttribute("posZ")->Value()), (float)atof(light->FindAttribute("ang")->Value()),(float)atof(light->FindAttribute("att")->Value()), (float)atof(light->FindAttribute("dirX")->Value()), (float)atof(light->FindAttribute("dirY")->Value()), (float)atof(light->FindAttribute("dirZ")->Value()));

							lights.push_back(spotLight);

						}
						//DIRECTIONAL LIGHT
						else if ((strcmp(light->Attribute("type"), "DIRECTIONAL") == 0) && light->FindAttribute("posX") && light->FindAttribute("posY") && light->FindAttribute("posZ")) {
							Light dirLight("DIRECTIONAL", (float)atof(light->FindAttribute("posX")->Value()), (float)atof(light->FindAttribute("posY")->Value()), (float)atof(light->FindAttribute("posZ")->Value()));

							lights.push_back(dirLight);

						}
						else {
							std::cout << "Coordinates on the light tags are not right!\n";
							exit(-1);
						}
						
					}
					else {
						std::cout << "One light source does not have a type!\n";
						exit(-1);
					}
					
					
				}
			}
			else {
				auxReadFile(child, TransformationState());
			}
			
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

void loadLights() {
	int i = 0;
	
	//glMaterialf(GL_FRONT, GL_SHININESS, 50);

	GLfloat amb[3] = { 0.001, 0.001, 0.001 };
	GLfloat diff[4] = { 1, 1, 1, 1.0 };

	for (auto light : lights) {

		if (i == 8) {
			std::cout << "Reached max number of lights!\n";
			exit(-1);
		}
		glEnable(GL_LIGHT0 + i);
		glLightfv(GL_LIGHT0 + i, GL_AMBIENT, amb); // luz ambiente

		if(strcmp(light.type,"POINT") == 0 ){
				
			//printf("POS: %f %f %f", light.pos[0], light.pos[1], light.pos[2], light.pos[3]);
			glLightfv(GL_LIGHT0 + i, GL_POSITION, light.pos); // posição da luz
			glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, light.attenuation);
		}
		else if (strcmp(light.type, "DIRECTIONAL") == 0) {
			glLightfv(GL_LIGHT0 + i, GL_POSITION, light.pos);

		}
		else if (strcmp(light.type, "SPOT") == 0) {
			glLightfv(GL_LIGHT0, GL_POSITION, light.pos);
			glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light.spotDir);
			glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, light.angle);       // [0,90] ou 180
		}
		else {

		}
		i++;

	}
}


void renderScene(void) {

	// clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// set the camera
	glLoadIdentity();

	gluLookAt(camX, camY, camZ,
		0.0, 0.0, 0.0,
		0.0f, 1.0f, 0.0f);

	//glBegin(GL_LINES);
	// X axis in red
	//glColor3f(1.0f, 0.0f, 0.0f);
	//glVertex3f(-200.0f, 0.0f, 0.0f);
	//glVertex3f(200.0f, 0.0f, 0.0f);
	// Y Axis in Green
	//glColor3f(0.0f, 1.0f, 0.0f);
	//glVertex3f(0.0f, -200.0f, 0.0f);
	//glVertex3f(0.0f, 200.0f, 0.0f);
	// Z Axis in Blue
	//glColor3f(0.0f, 0.0f, 1.0f);
	//glVertex3f(0.0f, 0.0f, -200.0f);
	//glVertex3f(0.0f, 0.0f, 200.0f);
	//glEnd();


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
	loadLights();
	models->drawVBO(steps, color, false);

	glPushMatrix();


	glPopMatrix();

	/*
	glPushMatrix();
	glTranslatef(90,0,0);
	glRotatef(-26.73, 0, 0, 1);
	glRotatef(90,1,0,0);
	glutSolidTorus(1, 3.8, 2, 40);
	glPopMatrix();
	*/
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
	else if (key == 'l' || key == 'L') {
		glPolygonMode(GL_FRONT, GL_LINE);
		glutPostRedisplay();
	}
	else if (key == 'p' || key == 'P') {
		glPolygonMode(GL_FRONT, GL_POINT);
		glutPostRedisplay();
	}
	else if (key == 'f' || key == 'F') {
		glPolygonMode(GL_FRONT, GL_FILL);
		glutPostRedisplay();
	}
}

int main(int argc, char **argv) {

	

// Read XML structered models.

// init GLUT and the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(800,800);
	glutCreateWindow("CG@DI-UM");

	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);

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

	
		
// Required callback registry 
	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);
	glutReshapeFunc(changeSize);
	glutSpecialFunc(processSpecialKeys);

	#ifndef __APPLE__
		glewInit();
	#endif

// put here the registration of the keyboard callbacks
	glutKeyboardFunc(function);
	glGenBuffers(1, &vertices_gl);
	glGenBuffers(1, &normals);
	glGenBuffers(1, &textures);
	

//  OpenGL settings
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	spherical2Cartesian();
	glEnableClientState(GL_NORMAL_ARRAY);
	
	

// enter GLUT's main cycle
	glutMainLoop();
	
	return 1;
}
