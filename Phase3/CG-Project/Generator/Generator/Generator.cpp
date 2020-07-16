// Generator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Generator.h"
int nPatches = 0;

class Point {
public:
    double x = 0;
    double y = 0;
    double z = 0;
    Point() {

    }
    Point(double x1, double y1, double z1) {
        x = x1;
        y = y1;
        z = z1;
    }

    string pointToString() {
        char* buff = (char*)malloc(sizeof(char) * 100);
        snprintf(buff, 100, "%.8f %.8f %.8f", x, y, z);
        return buff;
    }
};

class Triangle {
public:
    tuple <Point, Point, Point> coord;

    Triangle() {
        Point p(0, 0, 0);
        coord = make_tuple(p, p, p);
    }

    Triangle(Point a, Point b, Point c) {
        coord = make_tuple(a, b, c);
    }
};
void trianglesToFile(vector<Triangle> t, string f) {
    ofstream file(f);
    unsigned int i = 0;
    while (i < t.size()) {
        int x = (rand() % 3 + 1), y = (rand() % 3), z = (rand() % 3);
        file << x << " " << y << " " << z << ",";

        file << get<0>(t[i].coord).pointToString() << "," << get<1>(t[i].coord).pointToString() << "," << get<2>(t[i].coord).pointToString() << endl;
        i++;
    }
}

void generatePlaneFile(double x, double z, string f) {
    double auxX = x / 2, auxZ = z / 2;

    //Pontos para Triangulo 1
    Point p1(-auxX, 0, auxZ);
    Point p2(-auxX, 0, -auxZ);
    Point p3(auxX, 0, -auxZ);

    //Pontos para Trinagulo 2
    Point p4(-auxX, 0, auxZ);
    Point p5(auxX, 0, -auxZ);
    Point p6(auxX, 0, auxZ);

    Triangle t1(p1, p2, p3);
    //ordem original(p4,p5,p6) mas p4 = p1 e p5=p3
    Triangle t2(p1, p3, p6);
    Triangle t3(p3, p2, p1);
    //ordem original(p6,p5,p4) mas p4 = p1 e p5=p3
    Triangle t4(p6, p3, p1);

    vector <Triangle> v = { t1,t2,t3,t4 };

    trianglesToFile(v, f);

}

void generateBoxFile(double x, double y, double z, long int n, string f) {
    double xn = x / (n + 1), yn = y / (n + 1), zn = z / (n + 1);
    double i, j, k;
    vector<Triangle> triangles;

    //face 1 e 6 (laterais)
    for (j = 0; j < y; j += yn) {
        for (k = 0; k < z; k += zn) {
            //Face 1 Triangulo A
            Point p1(0, j + yn, k + zn);
            Point p2(0, j + yn, k);
            Point p3(0, j, k);

            Triangle f1a(p1, p2, p3);
            triangles.push_back(f1a);

            //Face 1 Triangulo B
//          Point p4(0, j, k);
            Point p5(0, j, k + zn);
            //          Point p6(0, j+yn, k+zn);

                        //ordem original(p4,p5,p6) mas p6=p1 e p4=p3
            Triangle f1b(p3, p5, p1);
            triangles.push_back(f1b);

            //Face 6 Triangulo A
            Point p7(x, j, k);
            Point p8(x, j + yn, k);
            Point p9(x, j + yn, k + zn);

            Triangle f6a(p7, p8, p9);
            triangles.push_back(f6a);

            //Face 6 Triangulo B
//            Point p10(x, j + yn, k + zn);
            Point p11(x, j, k + zn);
            //          Point p12(x, j , k);

                      //ordem original(p10,p11,p12) mas p12=p7 p10=p9
            Triangle f6b(p9, p11, p7);
            triangles.push_back(f6b);

        }
    }

    //face 2 e 4 (cima e baixo)
    for (k = 0; k < z; k += zn) {
        for (i = 0; i < x; i += xn) {
            //Face 2 Triangulo A
            Point p1(i, y, k);
            Point p2(i + xn, y, k + zn);
            Point p3(i + xn, y, k);

            Triangle f2a(p1, p2, p3);
            triangles.push_back(f2a);

            //Face 2 Triangulo B
//            Point p4(i, y, k);
            Point p5(i, y, k + zn);
            //          Point p6(i+xn, y, k+zn);

                      //ordem original(p4,p5,p6) mas p4=p1 e p6=p2
            Triangle f2b(p1, p5, p2);
            triangles.push_back(f2b);

            //Face 4 Triangulo A
            Point p7(i + xn, 0, k + zn);
            Point p8(i, 0, k + zn);
            Point p9(i, 0, k);

            Triangle f4a(p7, p8, p9);
            triangles.push_back(f4a);

            //Face 4 Triangulo B
 //           Point p10(i, 0, k);
            Point p11(i + xn, 0, k);
            //         Point p12(i+xn, 0, k+zn);

                     //ordem original(p10,p11,p12) mas p10=p9 e p12=p7
            Triangle f4b(p9, p11, p7);
            triangles.push_back(f4b);

        }
    }

    //face 5 e 3
    for (j = 0; j < y;j += yn) {
        for (i = 0; i < x;i += xn) {
            //Face 5 Triangulo A
            Point p1(i + xn, j, z);
            Point p2(i + xn, j + yn, z);
            Point p3(i, j + yn, z);

            Triangle f5a(p1, p2, p3);
            triangles.push_back(f5a);

            //Face 5 Triangulo B
            //Point p4(i, j+yn, z);
            Point p5(i, j, z);
            //Point p6(i+xn, j, z);

            //ordem original(p4,p5,p6) mas p4=p3 p6=p1
            Triangle f5b(p3, p5, p1);
            triangles.push_back(f5b);

            //Face 3 Triangulo A
            Point p7(i, j, 0);
            Point p8(i, j + yn, 0);
            Point p9(i + xn, j, 0);

            Triangle f3a(p7, p8, p9);
            triangles.push_back(f3a);

            //Face 3 Triangulo B
       //     Point p10(i, j+yn, 0);
            Point p11(i + xn, j + yn, 0);
            //   Point p12(i+xn, j, 0);

               //ordem original(p10,p11,p12) p10=p8 p12=p9
            Triangle f3b(p8, p11, p9);
            triangles.push_back(f3b);
        }
    }
    trianglesToFile(triangles, f);
}

void generateSphereFile(double radius, int slices, int stacks, string f) {

    double i, j;
    vector<Triangle> triangles;

    double stackSkew = PI / (stacks), sliceSkew = (2 * PI) / slices;
    for (i = 1; i <= stacks;i++) {
        double phi = (PI / 2) - (i * stackSkew);
        for (j = 1; j <= slices;j++) {
            double teta = j * sliceSkew;

            double previousX = radius * cos(phi) * sin(teta);
            double previousZ = radius * cos(teta) * cos(phi);


            Point p1(radius * cos(phi + stackSkew) * sin(teta + sliceSkew), radius * sin(phi + stackSkew), radius * cos(teta + sliceSkew) * cos(phi + stackSkew));
            Point p2(previousX, radius * sin(phi), previousZ);
            Point p3(previousX * cos(sliceSkew) + previousZ * sin(sliceSkew), radius * sin(phi), -previousX * sin(sliceSkew) + previousZ * cos(sliceSkew));

            Triangle t(p1, p2, p3);
            triangles.push_back(t);
            double auxX = radius * cos(phi + stackSkew) * sin(teta + sliceSkew), auxZ = radius * cos(teta + sliceSkew) * cos(phi + stackSkew);

            //Point p4(previousX * cos(sliceSkew) + previousZ * sin(sliceSkew), radius * sin(phi), -previousX * sin(sliceSkew) + previousZ * cos(sliceSkew));
            Point p5(auxX * cos(sliceSkew) + auxZ * sin(sliceSkew), radius * sin(phi + stackSkew), -auxX * sin(sliceSkew) + auxZ * cos(sliceSkew));
            //Point p6(radius * cos(phi + stackSkew) * sin(teta + sliceSkew), radius * sin(phi + stackSkew), radius * cos(teta + sliceSkew) * cos(phi + stackSkew));

            //ordem original(p4,p5,p6) mas p4=p3 e p6=p1
            Triangle t1(p3, p5, p1);
            triangles.push_back(t1);

        }
    }
    trianglesToFile(triangles, f);
}

void torus(float iRadius, float eRadius, float slices, float stacks, string f) {
    float x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4;
    float theta = 0, phi = 0;
    float sliceSkew = 2 * PI / slices;
    float stackSkew = 2 * PI / stacks;
    //Stack-> phi, slice -> teta
    vector<Triangle> triangles;

    for (int i = 0; i < slices; i++) {
        for (int j = 0; j < stacks; j++) {

            x1 = (iRadius + eRadius * cos(phi)) * cos(theta);
            z1 = (iRadius + eRadius * cos(phi)) * sin(theta);
            y1 = eRadius * sin(phi);


            x2 = (iRadius + eRadius * cos(phi)) * cos(theta + sliceSkew);
            z2 = (iRadius + eRadius * cos(phi)) * sin(theta + sliceSkew);
            y2 = eRadius * sin(phi);


            x3 = (iRadius + eRadius * cos(phi + stackSkew)) * cos(theta + sliceSkew);
            z3 = (iRadius + eRadius * cos(phi + stackSkew)) * sin(theta + sliceSkew);
            y3 = eRadius * sin(phi + stackSkew);

            x4 = (iRadius + eRadius * cos(phi + stackSkew)) * cos(theta);
            z4 = (iRadius + eRadius * cos(phi + stackSkew)) * sin(theta);
            y4 = eRadius * sin(phi + stackSkew);


            Point p1(x1, y1, z1);
            Point p2(x2, y2, z2);
            Point p3(x3, y3, z3);
            Point p4(x4, y4, z4);


            Triangle t1(p1, p2, p3);
            Triangle t2(p3, p4, p1);

            triangles.push_back(t1);
            triangles.push_back(t2);
            phi = stackSkew * (j + 1);
        }
        theta = sliceSkew * (i + 1);
    }
    trianglesToFile(triangles, f);

}

void generateConeFile(double radius, double height, double slices, double stacks, string f) {

    vector<Triangle> triangles;

    double alpha = (2 * PI) / slices;
    double beta = height / stacks;
    double altura = -height / 2; //centra o cone no referencial
    double i, j;
    double ang, raio1, raio2, frst, scnd;

    //fazer a circunfer�ncia da base
    for (i = 0; i < slices; i++) {

        ang = alpha * i;

        Point p1(0, altura, 0);
        Point p2(radius * sin(ang + alpha), altura, radius * cos(ang + alpha));
        Point p3(radius * sin(ang), altura, radius * cos(ang));

        Triangle t1(p1, p2, p3);
        triangles.push_back(t1);
    }
    //##############################

    for (i = 0; i < stacks; i++) {
        scnd = altura + (i * beta);
        frst = altura + ((i + 1) * beta);

        raio2 = radius - ((radius / stacks) * i);
        raio1 = radius - ((radius / stacks) * (i + 1));

        for (j = 0; j < slices; j++) {
            height = alpha * j;

            Point p4(raio2 * sin(height), scnd, raio2 * cos(height));
            Point p5(raio1 * sin(height + alpha), frst, raio1 * cos(height + alpha));
            Point p6(raio1 * sin(height), frst, raio1 * cos(height));

            Triangle t2(p4, p5, p6);
            triangles.push_back(t2);

            Point p7(raio2 * sin(height), scnd, raio2 * cos(height));
            Point p8(raio2 * sin(height + alpha), scnd, raio2 * cos(height + alpha));
            Point p9(raio1 * sin(height + alpha), frst, raio1 * cos(height + alpha));

            Triangle t3(p7, p8, p9);
            triangles.push_back(t3);
        }
    }
    trianglesToFile(triangles, f);
}

void createXML() {
    XMLDocument doc;
    XMLElement* Scene = doc.NewElement("scene");
    doc.LinkEndChild(Scene);
    doc.SaveFile(XMLDOC);

}

bool checkRepeatedFiles(const char* file) {
    XMLDocument doc;
    doc.LoadFile("Files.xml");
    XMLElement* root = doc.RootElement();

    const char* atr;

    for (XMLElement* it = root->FirstChildElement(); it != NULL; it = it->NextSiblingElement()) {
        if (strcmp(file, it->Attribute("file")) == 0) {
            return true;
        }

    }

    return false;
}

void updateXML(const char* file) {
    XMLDocument doc;
    if (doc.LoadFile("Files.xml") == XML_SUCCESS) {
        if (checkRepeatedFiles(file)) {
            printf("A model with that filename already exists. Please choose another one.\n");
            exit(0);
        }
        XMLElement* root = doc.RootElement();
        XMLElement* model = doc.NewElement("model");
        model->SetAttribute("file", file);
        root->LinkEndChild(model);
        doc.SaveFile(XMLDOC);
    }
    else {
        createXML();
        updateXML(file);
    }
}

void multVectorMatrix(float** m, float* v, float* res) {

    for (int j = 0; j < 4; ++j) {
        res[j] = 0;
        for (int k = 0; k < 4; ++k) {
            res[j] += v[k] * m[k][j];
        }
    }
}

float* bezier(float t, float* p0, float* p1, float* p2, float* p3) {
    float* points[4] = { p0,p1,p2,p3 };
    float vectorT[4] = { pow((1 - t),3), 3 * t * pow((1 - t),2), 3 * (1 - t) * pow(t,2), pow(t,3) };

    float* res = (float*)malloc(sizeof(float*) * 10);
    multVectorMatrix(points, vectorT, res);
    return res;
}

float* bezierPatch(float u, float v, float** allPoints, int* index) {
    float* controlPoints[4];
    for (int i = 0;i < 4;i++) {
        int idd = index[4 * i];

        float* p0 = allPoints[index[4 * i]];
        float* p1 = allPoints[index[4 * i + 1]];
        float* p2 = allPoints[index[4 * i + 2]];
        float* p3 = allPoints[index[4 * i + 3]];

        float* point = bezier(u, p0, p1, p2, p3);

        controlPoints[i] = point;
    }
    return bezier(v, controlPoints[0], controlPoints[1], controlPoints[2], controlPoints[3]);
}

void removeChar(std::string& str, char character)
{
    size_t pos;
    while ((pos = str.find(character)) != std::string::npos)
        str[pos] = ' ';
}

void writeResultPoints(int tecellationLevel, float** allPoints, int** index, string f) {
    vector<Triangle> triangles;
    float step = 1.0 / tecellationLevel;
    float u, u1, v, v1;


    for (int i = 0; i < nPatches; i++) {

        for (int j = 0; j < tecellationLevel; j++) {
            for (int k = 0; k < tecellationLevel; k++) {
                u = step * j;
                u1 = step * (j + 1);
                v = step * k;
                v1 = step * (k + 1);

                float* aux = bezierPatch(u, v, allPoints, index[i]);
                float* aux1 = bezierPatch(u, v1, allPoints, index[i]);
                float* aux2 = bezierPatch(u1, v, allPoints, index[i]);
                float* aux3 = bezierPatch(u1, v1, allPoints, index[i]);

                Point pA(aux[0], aux[1], aux[2]);
                Point pB(aux1[0], aux1[1], aux1[2]);
                Point pC(aux2[0], aux2[1], aux2[2]);
                Point pD(aux3[0], aux3[1], aux3[2]);

                Triangle t1(pC, pA, pB);
                Triangle t2(pB, pD, pC);
                triangles.push_back(t1);
                triangles.push_back(t2);
            }
        }
    }
    trianglesToFile(triangles, f);

}


int readBezier(ifstream file, char* fDest, int tecellationLevel) {
    int x = 0;

    string line;
    if (file.is_open()) {
        getline(file, line);
        nPatches = atoi(line.c_str());
        int** index = (int**)malloc(sizeof(int*) * nPatches);

        for (int efe = 0; efe < nPatches; efe++) {
            index[efe] = (int*)malloc(sizeof(int) * 16);
        }

        for (x = 0; x < nPatches;x++)
        {
            getline(file, line);
            removeChar(line, ',');

            std::istringstream data(line.c_str());
            int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
            data >> a >> b >> c >> d >> e >> f >> g >> h >> i >> j >> k >> l >> m >> n >> o >> p;

            vector<int>* aux = new vector<int>();
            *aux = { a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p };

            index[x] = aux->data();

        }
        /*        for (int i = 0; i < nPatches; i++) {
            printf("I: %d -> %f %f %f\n", i, allPoints[i][0], allPoints[i][1], allPoints[i][2]);
        }*/


        getline(file, line);
        int nPoints = atoi(line.c_str());
        float** allPoints = (float**)malloc(sizeof(float*) * nPoints);

        for (int efe = 0; efe < nPoints; efe++) {
            allPoints[efe] = (float*)malloc(sizeof(float) * 3);
        }


        for (x = 0; x < nPoints; x++)
        {
            getline(file, line);
            removeChar(line, ',');

            std::istringstream data(line.c_str());
            float a, b, c;
            data >> a >> b >> c;
            vector<float>* aux = new vector<float>();
            *aux = { a,b,c };

            allPoints[x] = aux->data();
        }

        writeResultPoints(tecellationLevel, allPoints, index, fDest);
        return 0;
    }
    else {
        return -1;
    }
}


int main(int argc, char* argv[]) {

    if (strcmp(argv[1], "plane") == 0 && argc == 5) {
        generatePlaneFile(atof(argv[2]), atof(argv[3]), argv[4]);
        updateXML(argv[4]);
    }
    else if (strcmp(argv[1], "box") == 0 && argc == 6) {
        //float x, float y, float z, float n, string f
        generateBoxFile(atof(argv[2]), atof(argv[3]), atoi(argv[4]), 0, argv[5]);
        updateXML(argv[5]);
    }
    else if (strcmp(argv[1], "box") == 0 && argc == 7) {
        //float x, float y, float z, float n, string f
        generateBoxFile(atof(argv[2]), atof(argv[3]), atoi(argv[4]), atoi(argv[5]), argv[6]);
        updateXML(argv[6]);
    }
    else if (strcmp(argv[1], "sphere") == 0 && argc == 6) {
        generateSphereFile(atof(argv[2]), atoi(argv[3]), atoi(argv[4]), argv[5]);
        updateXML(argv[5]);
    }
    else if (strcmp(argv[1], "cone") == 0 && argc == 7) {
        generateConeFile(atof(argv[2]), atof(argv[3]), atof(argv[4]), atof(argv[5]), argv[6]);
        updateXML(argv[6]);

    }
    else if (strcmp(argv[1], "bezier") == 0 && argc == 5) {
        //bezier file tecellation destFile.txt

        int res = readBezier(ifstream(argv[2]), argv[4], atoi(argv[3]));
        if (res == -1) printf("Erro");
        else {
            updateXML(argv[4]);
        }
    }
    else if (strcmp(argv[1], "torus") == 0 && argc == 7) {
        torus(atof(argv[2]), atof(argv[3]), atof(argv[4]), atof(argv[5]), argv[6]);
    }
    else {
        printf("%s\n", "Can't Generate that. Please see the README for more details.");
    }

}

