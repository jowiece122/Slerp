#include "GL/freeglut.h"
#include "GLFW/glfw3.h"
#include "GL/glui.h"
#include <stdio.h>
#include <list>
#include <iostream>
#include <math.h>
#define PI 3.14159265

double width = 800;
double height = 800;

struct mypoints {
	double x;
	double y;
	double z;
};
mypoints pointlist[30];
mypoints hermitelist[30];
mypoints deCasteljauList[30][30];
mypoints controlPoint[30];
struct quaternion {
	double w;
	double x;
	double y;
	double z;
};


int pointnumber = 0;
int tangentnumber = 0;
int controlnumber = 0;
float rotationx = 0;
float rotationy = 0;
float rotationz = 0;
float rotationa = 0;
quaternion q = {0,0,0,0};
quaternion rotq = { 0,0,0,0 };
quaternion saveq = { 0,0,0,0 };
double mouseDownx = 0.0;
double mouseDowny = 0.0;
int mouseDown = -1;
double slope = 0;
int foundPosition = -1;
int foundPositionH = -1;
int foundPositionC = -1;
double savex = 0,
savey = 0,
savez = 0,
savex2 = 0,
savey2=0,
savez2=0;
int rstate = 0;

int displayWin;
GLUI* glui;
GLUI_Checkbox* checkbox;
GLUI_Panel* panel;
GLUI_EditText* edittext;
int showPolygon = 0,
showCurve = 0,
showDeCasteljau = 0,
hermite = 0,
tangent = 0,
control = 0,
curve = 0;

float tValue = 0.5,
s = 0.3;

quaternion product(quaternion a, quaternion b) {
	quaternion newq = {
		a.w* b.w - a.x * b.x - a.y * b.y - a.z * b.z,
		a.y * b.z - a.z * b.y + a.x * b.w + a.w * b.x,
		a.z * b.x - a.x * b.z + a.y * b.w + a.w * b.y,
		a.x * b.y - a.y * b.x + a.z * b.w + a.w * b.z

	};
	return newq;
}

void setRotation(quaternion a) {
	double u = 2 * acos(a.w);
	u = u * 180 / PI;
	rotationa = u;
	rotationx = a.x;
	rotationy = a.y;
	rotationz = a.z;
}

quaternion setQ(double w, double x, double y,double z) {
	quaternion newq;
	newq.w = cos(w * PI / 360);
	newq.x = sin(w * PI / 360) * x;
	newq.y = sin(w * PI / 360) * y;
	newq.z = sin(w * PI / 360) * z;
	return newq;
}

void drawPoint(double x, double y,double z) {
	glPointSize(5);
	glBegin(GL_POINTS);
	glColor3f(1.0f, 1.0f, 0.0f);
	glVertex3f(x, y, z);
	glEnd();
}

void drawCPoint(double x, double y, double z) {
	glPointSize(5);
	glBegin(GL_POINTS);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(x, y, z);
	glEnd();
}
void drawSphere() {
	glEnable(GL_LIGHTING);
	//GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_pos[] = { 30, 40, 20, 1 };

	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);  // define the position of the light
	//glLightfv(GL_LIGHT0, GL_AMBIENT, white);  // specify the ambient RGBA intensity of the light

	glEnable(GL_LIGHT0);

	glColor3d(0.7, 0.7, 0);
	glPushMatrix();
	glutSolidSphere(0.99, 50, 50);
	
	glPopMatrix();
}

void drawLong() {
	glLineWidth(2);
	glBegin(GL_LINE_STRIP);
	for (double t = 0; t < 180; t += 1) {
		glVertex3d(1.0 * sin(t * PI / 180), 0, 1.0 * cos(t * PI / 180));
	}
	glEnd();
}
void drawLat() {
	for (int i = -2; i <= 2; i++) {
		double degree = (double)i * 30;
		if (i == 0) {
			glColor3f(0.0, 0.0, 1.0);
		}
		else { glColor3f(1.0, 1.0, 1.0); }
		
		glLineWidth(2);
		glBegin(GL_LINE_STRIP);
		for (double i = 0; i < 11; i++) {
			for (double t = 0; t < 360; t += 1) {
				glVertex3d((1.0 * cos(degree * PI / 180)) * cos(t * PI / 180), (1.0 * cos(degree * PI / 180)) * sin(t * PI / 180), 1.0 * sin(degree * PI / 180));
			}
		}
		glEnd();
	}
}

void drawPolygon() {
	mypoints temp;
	for (int i = 0; i<pointnumber;i++){
		if (i==0) {
			temp = { pointlist[i].x, pointlist[i].y, pointlist[i].z};
		}
		else {
			double dproduct = temp.x * pointlist[i].x + temp.y * pointlist[i].y + temp.z * pointlist[i].z;
			double angle = acos(dproduct);
			glColor3f(0.0, 1.0, 0.0);
			glLineWidth(2);
			glBegin(GL_LINE_STRIP);
			for (double t = 0; t <1; t+=0.01) {
				double at = sin((1.0 - t) * angle) / sin(angle);
				double bt = sin(t * angle) / sin(angle);
				double rtx = at * temp.x + bt * pointlist[i].x;
				double rty = at * temp.y + bt * pointlist[i].y;
				double rtz = at * temp.z + bt * pointlist[i].z;
				glVertex3f(rtx, rty, rtz);
			}
			glEnd();
			temp = { pointlist[i].x, pointlist[i].y, pointlist[i].z };
		}
	}

}
void DeCasteljau() {
	//mypoints temp;
	for (int i = 0; i < pointnumber;i++) {
		for (int j = 0; j < pointnumber - i; j++) {
			if (i == 0) {
				deCasteljauList[i][j] = pointlist[j];
			}
			else {
				double tempx = deCasteljauList[i - 1][j].x;
				double tempy = deCasteljauList[i - 1][j].y;
				double tempz = deCasteljauList[i - 1][j].z;
				double cx = deCasteljauList[i - 1][j + 1].x;
				double cy = deCasteljauList[i - 1][j + 1].y;
				double cz = deCasteljauList[i - 1][j + 1].z;
				double dproduct = tempx*cx + tempy*cy + tempz*cz;
				double angle = acos(dproduct);
				double at = sin((1.0 - tValue) * angle) / sin(angle);
				double bt = sin(tValue * angle) / sin(angle);
				double rtx = at * tempx + bt * cx;
				double rty = at * tempy + bt * cy;
				double rtz = at * tempz + bt * cz;
				mypoints newp = { rtx,rty,rtz };
				deCasteljauList[i][j] = newp;
			}
		}
	}
}
void DeCasteljauPolygon() {
	for (int i = 1; i < pointnumber; i++) {
		for (int j = 0; j < pointnumber -1 - i; j++) {
			double tempx = deCasteljauList[i][j].x;
			double tempy = deCasteljauList[i][j].y;
			double tempz = deCasteljauList[i][j].z;
			double cx = deCasteljauList[i][j + 1].x;
			double cy = deCasteljauList[i][j + 1].y;
			double cz = deCasteljauList[i][j + 1].z;
			double dproduct = tempx * cx + tempy * cy + tempz * cz;
			double angle = acos(dproduct);
			glColor3f(1.0, 0.0, 0.0);
			glLineWidth(2);
			glBegin(GL_LINE_STRIP);
			for (double t = 0; t < 1; t += 0.01) {
				double at = sin((1.0 - t) * angle) / sin(angle);
				double bt = sin(t * angle) / sin(angle);
				double rtx = at * tempx + bt * cx;
				double rty = at * tempy + bt * cy;
				double rtz = at * tempz + bt * cz;
				glVertex3f(rtx, rty, rtz);
			}
			glEnd();

		}
	}
}

void drawCurve() {
	glColor3f(0.0, 1.0, 1.0);
	glLineWidth(2);
	glBegin(GL_LINE_STRIP);
	double x[30];
	double y[30];
	double z[30];
		for (double t = 0; t <= 1.0; t += 0.01) {
			for (int i = 0; i < pointnumber; i++) {
				for (int j = 0; j < pointnumber - 1 - i; j++) {
					if (i == 0) {
						double tempx = pointlist[j].x;
						double tempy = pointlist[j].y;
						double tempz = pointlist[j].z;
						double cx = pointlist[j + 1].x;
						double cy = pointlist[j + 1].y;
						double cz = pointlist[j + 1].z;
						double dproduct = tempx * cx + tempy * cy + tempz * cz;
						double angle = acos(dproduct);
						double at = sin((1.0 - t) * angle) / sin(angle);
						double bt = sin(t * angle) / sin(angle);
						x[j] = at * tempx + bt * cx;
						y[j] = at * tempy + bt * cy;
						z[j] = at * tempz + bt * cz;
					}
					else {
						double tempx = x[j];
						double tempy = y[j];
						double tempz = z[j];
						double cx = x[j + 1];
						double cy = y[j + 1];
						double cz = z[j + 1];
						double dproduct = tempx * cx + tempy * cy + tempz * cz;
						double angle = acos(dproduct);
						double at = sin((1.0 - t) * angle) / sin(angle);
						double bt = sin(t * angle) / sin(angle);
						x[j] = at * tempx + bt * cx;
						y[j] = at * tempy + bt * cy;
						z[j] = at * tempz + bt * cz;
					}
				}
			}
			glVertex3f(x[0], y[0], z[0]);
		}
	glEnd();
}

void cutLeft(int) {
	DeCasteljau();
	for (int i = 0; i < pointnumber; i++) {
		pointlist[i] = deCasteljauList[i][0];
	}
	glPushMatrix();
	glRotatef(rotationa, rotationx, rotationy, rotationz);
	drawCurve();
	glPopMatrix();
	glutSetWindow(displayWin);
	glutPostRedisplay();
}

void cutRight(int) {
	DeCasteljau();
	for (int i = 0; i < pointnumber; i++) {
		pointlist[i] = deCasteljauList[pointnumber-1-i][i];
	}
	glPushMatrix();
	glRotatef(rotationa, rotationx, rotationy, rotationz);
	drawCurve();
	glPopMatrix();
	glutSetWindow(displayWin);
	glutPostRedisplay();
}

void extendLeft(int) {
	DeCasteljau();
	for (int i = 0; i < pointnumber; i++) {
		pointlist[i] = deCasteljauList[i][0];
	}
	tValue = 0.5;
	glPushMatrix();
	glRotatef(rotationa, rotationx, rotationy, rotationz);
	drawCurve();
	glPopMatrix();
	glutSetWindow(displayWin);
	glutPostRedisplay();
}

void extendRight(int) {
	DeCasteljau();
	for (int i = 0; i < pointnumber; i++) {
		pointlist[i] = deCasteljauList[pointnumber - 1 - i][i];
	}
	tValue = 0.5;
	glPushMatrix();
	glRotatef(rotationa, rotationx, rotationy, rotationz);
	drawCurve();
	glPopMatrix();
	glutSetWindow(displayWin);
	glutPostRedisplay();
}

void drawTangent() {
	for (int i = 0; i < tangentnumber;i++) {
		double dproduct = pointlist[i].x * hermitelist[i].x + pointlist[i].y * hermitelist[i].y + pointlist[i].z * hermitelist[i].z;
		double angle = acos(dproduct);
		glColor3f(1.0, 0.0, 1.0);
		glLineWidth(2);
		glBegin(GL_LINE_STRIP);
		for (double t = 0; t < 1; t += 0.01) {
			double at = sin((1.0 - t) * angle) / sin(angle);
			double bt = sin(t * angle) / sin(angle);
			double rtx = at * pointlist[i].x + bt * hermitelist[i].x;
			double rty = at * pointlist[i].y + bt * hermitelist[i].y;
			double rtz = at * pointlist[i].z + bt * hermitelist[i].z;
			glVertex3f(rtx, rty, rtz);
		}
		glEnd();
	}
}

void drawControl() {
	for (int i = 0; i < controlnumber; i++) {
		drawCPoint(controlPoint[i].x, controlPoint[i].y, controlPoint[i].z);
	}
	mypoints temp;
	for (int i = 0; i < controlnumber; i++) {
		if (i == 0) {
			temp = { controlPoint[i].x, controlPoint[i].y, controlPoint[i].z };
		}
		else {
			double dproduct = temp.x * controlPoint[i].x + temp.y * controlPoint[i].y + temp.z * controlPoint[i].z;
			double angle = acos(dproduct);
			glColor3f(0.0, 1.0, 0.0);
			glLineWidth(2);
			glBegin(GL_LINE_STRIP);
			for (double t = 0; t < 1; t += 0.01) {
				double at = sin((1.0 - t) * angle) / sin(angle);
				double bt = sin(t * angle) / sin(angle);
				double rtx = at * temp.x + bt * controlPoint[i].x;
				double rty = at * temp.y + bt * controlPoint[i].y;
				double rtz = at * temp.z + bt * controlPoint[i].z;
				glVertex3f(rtx, rty, rtz);
			}
			glEnd();
			temp = { controlPoint[i].x, controlPoint[i].y,controlPoint[i].z };
		}
	}
}

void findControl() {
	controlnumber = 0;
	for (int i = 0; i < pointnumber; i++) {
		double dproduct = pointlist[i].x * hermitelist[i].x + pointlist[i].y * hermitelist[i].y + pointlist[i].z * hermitelist[i].z;
		double angle = acos(dproduct);
		if (i == 0) {
			controlPoint[i] = pointlist[i];
			controlnumber++;
			double t = 1;
			double at = sin((1.0 - t) * angle) / sin(angle);
			double bt = sin(t * angle) / sin(angle);
			double rtx = at * pointlist[i].x + bt * hermitelist[i].x;
			double rty = at * pointlist[i].y + bt * hermitelist[i].y;
			double rtz = at * pointlist[i].z + bt * hermitelist[i].z;
			mypoints newp = { rtx,rty,rtz };
			controlPoint[i+1] = newp;
			//drawCPoint(controlPoint[controlnumber].x, controlPoint[controlnumber].y, controlPoint[controlnumber].z);
			controlnumber++;
		}
		else if (i == pointnumber-1) {
			double t = -1;
			double at = sin((1.0 - t) * angle) / sin(angle);
			double bt = sin(t * angle) / sin(angle);
			double rtx = at * pointlist[i].x + bt * hermitelist[i].x;
			double rty = at * pointlist[i].y + bt * hermitelist[i].y;
			double rtz = at * pointlist[i].z + bt * hermitelist[i].z;
			mypoints newp = { rtx,rty,rtz };
			controlPoint[3*i-1] = newp;
			//drawCPoint(controlPoint[controlnumber].x, controlPoint[controlnumber].y, controlPoint[controlnumber].z);
			controlnumber++;
			controlPoint[3*i] = pointlist[i];
			controlnumber++;
		}
		else {
			double t = -1;
			double at = sin((1.0 - t) * angle) / sin(angle);
			double bt = sin(t * angle) / sin(angle);
			double rtx = at * pointlist[i].x + bt * hermitelist[i].x;
			double rty = at * pointlist[i].y + bt * hermitelist[i].y;
			double rtz = at * pointlist[i].z + bt * hermitelist[i].z;
			mypoints newp1 = { rtx,rty,rtz };
			controlPoint[3*i-1] = newp1;
			//drawCPoint(controlPoint[controlnumber].x, controlPoint[controlnumber].y, controlPoint[controlnumber].z);
			controlnumber++;
			controlPoint[3*i] = pointlist[i];
			controlnumber++;
			t = 1;
			at = sin((1.0 - t) * angle) / sin(angle);
			bt = sin(t * angle) / sin(angle);
			rtx = at * pointlist[i].x + bt * hermitelist[i].x;
			rty = at * pointlist[i].y + bt * hermitelist[i].y;
			rtz = at * pointlist[i].z + bt * hermitelist[i].z;
			mypoints newp2 = { rtx,rty,rtz };
			controlPoint[3*i+1] = newp2;
			//drawCPoint(controlPoint[controlnumber].x, controlPoint[controlnumber].y, controlPoint[controlnumber].z);
			controlnumber++;
		}
		
	}
	//printf("controlnumber:%d\n", controlnumber);
	
}

void hermiteCurve() {
	for (int k = 0; k < controlnumber-1; k = k + 3) {
		glColor3f(0.0, 1.0, 1.0);
		glLineWidth(2);
		glBegin(GL_LINE_STRIP);
		double x[30];
		double y[30];
		double z[30];
		for (double t = 0; t <= 1.0; t += 0.01) {
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 3 - i; j++) {
					if (i == 0) {
						double tempx = controlPoint[k+j].x;
						double tempy = controlPoint[k + j].y;
						double tempz = controlPoint[k + j].z;
						double cx = controlPoint[k+j + 1].x;
						double cy = controlPoint[k+j + 1].y;
						double cz = controlPoint[k+j + 1].z;
						double dproduct = tempx * cx + tempy * cy + tempz * cz;
						double angle = acos(dproduct);
						double at = sin((1.0 - t) * angle) / sin(angle);
						double bt = sin(t * angle) / sin(angle);
						x[j] = at * tempx + bt * cx;
						y[j] = at * tempy + bt * cy;
						z[j] = at * tempz + bt * cz;
					}
					else {
						double tempx = x[j];
						double tempy = y[j];
						double tempz = z[j];
						double cx = x[j + 1];
						double cy = y[j + 1];
						double cz = z[j + 1];
						double dproduct = tempx * cx + tempy * cy + tempz * cz;
						double angle = acos(dproduct);
						double at = sin((1.0 - t) * angle) / sin(angle);
						double bt = sin(t * angle) / sin(angle);
						x[j] = at * tempx + bt * cx;
						y[j] = at * tempy + bt * cy;
						z[j] = at * tempz + bt * cz;
					}
				}
			}
			glVertex3f(x[0], y[0], z[0]);
		}
		glEnd();
	}
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.3, 1.3, -1.3, 1.3, -1.3, 1.3);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	drawSphere();
	
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	for (int i = 0; i < 12; i++) {
		glPushMatrix();
		//glRotatef(20, 0, 1, 0);
		glRotatef(rotationa, rotationx, rotationy, rotationz);
		glRotatef(i*30, 0, 0, 1);
		if (i == 0) {
			glColor3f(0.2, 0.7, 0.2);
		}
		else {
			glColor3f(1.0, 1.0, 1.0);
		}
		drawLong();
		glPopMatrix();
	}
	glPushMatrix();
	//glRotatef(20, 0, 1, 0);
	glRotatef(rotationa, rotationx, rotationy, rotationz);
	drawLat();
	glPopMatrix();
	glPushMatrix();
	//glRotatef(20, 0, 1, 0);
	glRotatef(rotationa, rotationx, rotationy, rotationz);
	glPointSize(10);
	glBegin(GL_POINTS);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0,0.0,1.02);
	glEnd();
	glPopMatrix();
	if (mouseDown == 2) {
		glPushMatrix();
		//glRotatef(20, 0, 1,0);
		glRotatef(rotationa, rotationx, rotationy, rotationz);
		glColor3f(1.0f, 1.0f, 0.0f);
		// top left
		glBegin(GL_LINE_STRIP);
		for (double t = -5; t < 5; t += 1) {
			glVertex3d(t, slope * t, 0);
		}
		glEnd();
		glPopMatrix();
	}
	
	else {
		for (int i = 0; i < pointnumber; i++) {
		glPushMatrix();
		//glRotatef(20, 0, 1, 0);
		glRotatef(rotationa, rotationx, rotationy, rotationz);
		drawPoint(pointlist[i].x, pointlist[i].y, pointlist[i].z);
		glPopMatrix();
		//printf("draw\n");
	}
	}
	if (showPolygon == 1) {
		glPushMatrix();
		//glRotatef(20, 0, 1,0);
		glRotatef(rotationa, rotationx, rotationy, rotationz);
		drawPolygon();
		glPopMatrix();
	}
	if (showDeCasteljau == 1) {
		DeCasteljau();
		for (int i = 1; i < pointnumber;i++) {
			for (int j = 0; j < pointnumber-i;j++){
			glPushMatrix();
			//glRotatef(20, 0, 1, 0);
			glRotatef(rotationa, rotationx, rotationy, rotationz);
			drawPoint(deCasteljauList[i][j].x, deCasteljauList[i][j].y, deCasteljauList[i][j].z);
			//printf("i:%d j:%d x:%f y:%f z:%f  ", i, j,deCasteljauList[i][j].x, deCasteljauList[i][j].y, deCasteljauList[i][j].z);
			glPopMatrix();
			}
		}
		glPushMatrix();
		glRotatef(rotationa, rotationx, rotationy, rotationz);
		DeCasteljauPolygon();
		glPopMatrix();
	}
	if (showCurve == 1) {
		glPushMatrix();
		glRotatef(rotationa, rotationx, rotationy, rotationz);
		drawCurve();
		glPopMatrix();
	}
	if (hermite == 1) {
		if (control == 1) {
			if (tangentnumber != pointnumber) {
				printf("Not enough tangents are defined!\n");
			}
			else {
				glPushMatrix();
				glRotatef(rotationa, rotationx, rotationy, rotationz);
				findControl();
				drawControl();
				glPopMatrix();
			}
		}
		if (tangent == 1) {
			glPushMatrix();
			glRotatef(rotationa, rotationx, rotationy, rotationz);
			drawTangent();
			glPopMatrix();
		}
		if (curve == 1) {
			if (tangentnumber != pointnumber) {
				printf("Not enough tangents are defined!\n");
			}
			else {
				glPushMatrix();
				glRotatef(rotationa, rotationx, rotationy, rotationz);
				findControl();
				hermiteCurve();
				glPopMatrix();
			}
		}
		
	}
	glFlush();
	glutSwapBuffers();
	
}

void motion(int x, int y) {
	float xi = (float)(x - width / 2) / (float)(width / 2) * 1.3;
	float yi = (float)(height / 2 - y) / (float)(height / 2) * 1.3;
	float zi = sqrt(1 - pow(xi, 2) - pow(yi, 2));
	//rotate with current rotation
	quaternion p = { 0,xi,yi,zi };
	quaternion rq = { -q.w,q.x,q.y,q.z };
	quaternion rp = product(rq, p);
	quaternion rs = { -q.w,-q.x,-q.y,-q.z };
	quaternion newp = product(rp, rs);
	double xd = newp.x;
	double yd = newp.y;
	double zd = newp.z;
	if (rstate == 0) {
		xd = xi;
		yd = yi;
		zd = zi;
	}
	if (mouseDown == 1 && (foundPosition != -1||foundPositionH!=-1||foundPositionC!=-1)) {
		/* Drag the Point interactively*/
		if (foundPosition != -1) {
			pointlist[foundPosition].x = xd;
			pointlist[foundPosition].y = yd;
			pointlist[foundPosition].z = zd;
		}
		else if (foundPositionH != -1) {
			hermitelist[foundPositionH].x = xd;
			hermitelist[foundPositionH].y = yd;
			hermitelist[foundPositionH].z = zd;
		}
		else if (foundPositionC != -1) {
			hermitelist[foundPositionC].x = savex2-xd+savex;
			hermitelist[foundPositionC].y = savey2-yd+savey;
			hermitelist[foundPositionC].z = savez2-zd+savez;
		}
		display();
	}
	else if (mouseDown == 2) {
		double xdiff = xi - mouseDownx;
		//printf("xdiff:%f\n", xdiff);
		if (rstate == 0) {
			rotationa = xdiff*100;
			rotationx = rotq.x;
			rotationy = rotq.y;
			rotationz = rotq.z;
			//printf("Q:%f,%f,%f,%f\n\n", q.w, q.x, q.y, q.z);
			display();
		}
		else {
			rotationa = xdiff * 100;
			rotq = setQ(rotationa, rotq.x, rotq.y, rotq.z);
			quaternion rp = product(rotq, q);
			quaternion rs = { rotq.w,-rotq.x,-rotq.y,-rotq.z };
			quaternion newq = product(rp, rs);
			rotationx = newq.x;
			rotationy = newq.y;
			rotationz = newq.z;
			display();
		}
	}
}

void mouse(int button, int state, int x, int y) {
	int mode = glutGetModifiers();
	//get the points
	float xi = (float)(x - width / 2) / (float)(width / 2) * 1.3;
	float yi = (float)(height / 2 - y) / (float)(height / 2) * 1.3;
	float zi = sqrt(1 - pow(xi, 2) - pow(yi, 2));
	//rotate with current rotation
	quaternion p = { 0,xi,yi,zi };
	quaternion rq = { -q.w,q.x,q.y,q.z };
	quaternion rp = product(rq, p);
	quaternion rs = { -q.w,-q.x,-q.y,-q.z };
	quaternion newp = product(rp, rs);
	double xd = newp.x;
	double yd = newp.y;
	double zd = newp.z;
	if (rstate == 0) {
		xd = xi;
		yd = yi;
		zd = zi;
	}
	if (mode == GLUT_ACTIVE_SHIFT) {
		/* define a new point */
		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
			if (pointnumber <= 30) {
				//double zd = sqrt(1 - pow(xd, 2) - pow(yd, 2));
				mypoints newpoint = { xd,yd,zd};
				if (tangent == 1) {
					hermitelist[tangentnumber] = newpoint;
					tangentnumber++;
				}
				else {
					pointlist[pointnumber] = newpoint;
					pointnumber++;
				}
				display();
				//printf("pointnumber:%d\n", pointnumber);
			}
			else {
				printf("max point number 30!\n");
			}
		}
	}
	else if (mode == GLUT_ACTIVE_CTRL) {
		mouseDown = 1;
		/* drags the point that you clicked */
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
				for (int i = 0; i < pointnumber; i++) {
					if (pointnumber > 0) {
						if ((xd <= (pointlist[i].x + 0.05)) && (xd >= (pointlist[i].x - 0.05)) && (yd <= (pointlist[i].y + 0.05)) && (yd >= (pointlist[i].y - 0.05))) {
							foundPosition = i;
							break;
						}
					}
				}
				if (foundPosition == -1) {
					for (int i = 0; i < tangentnumber; i++) {
						if (tangentnumber > 0) {
							if ((xd <= (hermitelist[i].x + 0.05)) && (xd >= (hermitelist[i].x - 0.05)) && (yd <= (hermitelist[i].y + 0.05)) && (yd >= (hermitelist[i].y - 0.05))) {
								foundPositionH = i;
								break;
							}
						}
					}
				}
				if (foundPosition == -1 && foundPositionH == -1) {
					for (int i = 2; i < controlnumber; i=i+3) {
						if (controlnumber> 2) {
							if ((xd <= (controlPoint[i].x + 0.05)) && (xd >= (controlPoint[i].x - 0.05)) && (yd <= (controlPoint[i].y + 0.05)) && (yd >= (controlPoint[i].y - 0.05))) {
								foundPositionC = (i+1)/3;
								savex = xd;
								savey = yd;
								savez = zd;
								savex2 = hermitelist[foundPositionC].x;
								savey2 = hermitelist[foundPositionC].y;
								savez2 = hermitelist[foundPositionC].z;
								break;
							}
						}
					}
				}
		}if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
			foundPosition = -1;
			foundPositionC = -1;
			foundPositionH = -1;
		}
	}
	else if (mode == (GLUT_ACTIVE_CTRL | GLUT_ACTIVE_SHIFT)) {
		//printf("both\n");
		/* delete the target point*/
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
			for (int i = 0; i < pointnumber;i++) {
				if (pointnumber > 0) {
					if ((xd <= (pointlist[i].x + 0.05)) && (xd >= (pointlist[i].x - 0.05)) && (yd <= (pointlist[i].y + 0.05)) && (yd >= (pointlist[i].y - 0.05))) {
						foundPosition = i;
						pointnumber--;
						break;
						
					}
				}
			}
			for (int j = foundPosition; j < pointnumber;j++) {
				pointlist[j] = pointlist[j + 1];
			}
		}
	}
	else {
		/* rotate the sphere */
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
			mouseDown = 2;
			double temp = -(xd / yd);
			double newrotationx = sqrt(1 / (1 + pow(temp, 2)));
			double newrotationy = sqrt(1 - pow(newrotationx, 2));
			double newrotationz = 0;
			quaternion p = { 0,newrotationx,newrotationy,newrotationz };
			quaternion rq = { -q.w,q.x,q.y,q.z };
			quaternion rp = product(rq, p);
			quaternion rs = { -q.w,-q.x,-q.y,-q.z };
			quaternion newp = product(rp, rs);
			
			mouseDownx = xi;
			mouseDowny = yi;
			rotq.x = newp.x;
			rotq.y = newp.y;
			rotq.z = newp.z;
			if (rstate == 0) {
				rotq.x = newrotationx;
				rotq.y = newrotationy;
				rotq.z = newrotationz;
			}
			slope = rotq.y / rotq.x;
			saveq.w = q.w;
			saveq.x = q.x;
			saveq.y = q.y;
			saveq.z = q.z;
		}
		else {
			//printf("rotationa:%f,rotationx:%f,rotationy:%f,rotationz:%f\n", rotationa, rotationx, rotationy, rotationz);
			q = setQ(rotationa, rotationx, rotationy, rotationz);
			mouseDown = -1;
			rstate = 1;
			//printf("state:%d\n", state);
		}
	}
	
}

void showMe(int select) {
		glutSetWindow(displayWin);
		glutPostRedisplay();
}

void Clear(int) {
	//pointlist;
	pointnumber = 0;
	tangentnumber = 0;
	controlnumber = 0;
	rotationx = 0;
	rotationy = 0;
	rotationz = 0;
	rotationa = 0;
	mouseDown = -1;
	mouseDownx = 0;
	mouseDowny = 0;
	slope = 0;
	rstate = 0;
	
	showPolygon = 0;
	showCurve = 0;
	showDeCasteljau = 0;
	hermite = 0;
	tangent = 0;
	control = 0;
	curve = 0;

	glutSetWindow(displayWin);
	glutPostRedisplay();
}

void Exit(int) {
	exit(0);
}

void buildGlui(int displayWin) {
	GLUI_Master.set_glutDisplayFunc(display);
	GLUI_Master.set_glutMouseFunc(mouse);
	glui = GLUI_Master.create_glui("controls",0L,900,0);

	glui->add_separator();
	glui->add_statictext("Basic SLERP Bezier");
	glui->add_separator();
	GLUI_Panel* panel1 = new GLUI_Panel(glui, "Show", GLUI_PANEL_EMBOSSED);
	new GLUI_Checkbox(panel1, "Ctrl Polygon", &showPolygon, 1, showMe);
	//new GLUI_Checkbox(panel1, "De Casteljau", &showDeCasteljau, 2, showMe); 
	new GLUI_Checkbox(panel1, "Curve", &showCurve, 3, showMe);
	//new GLUI_Checkbox(panel1, "Extend Left", &extendLeft, 6, showMe);
	//new GLUI_Checkbox(panel1, "Extend Right", &extendRight, 7, showMe);
	
	GLUI_Panel* panel9 = new GLUI_Panel(glui, "Hermite", GLUI_PANEL_EMBOSSED);
	new GLUI_Checkbox(panel9, "Hermite Mode", &hermite, 10, showMe);
	new GLUI_Checkbox(panel9, "Define Tangent", &tangent, 11, showMe);
	new GLUI_Checkbox(panel9, "Draw Control", &control, 12, showMe);
	new GLUI_Checkbox(panel9, "Draw Hermite Curve", &curve, 13, showMe);
	
	GLUI_Panel* panel5 = new GLUI_Panel(glui, "Display Parameters", GLUI_PANEL_EMBOSSED);
	edittext = new GLUI_EditText(panel5, "t: ", &tValue, 10, showMe);
	//new GLUI_EditText(panel5, "s: ", &s, 11, showMe);
	//new GLUI_EditText(panel5, "Lift Cruves: ", &liftCurves, 11, showMe);
	//new GLUI_EditText(panel5, "Ticks: ", &ticks, 12, showMe);
	GLUI_Panel* panel3 = new GLUI_Panel(glui, "  ", GLUI_PANEL_NONE);
	new GLUI_Button(panel3, "Cut Left", 4, cutLeft);

	GLUI_Panel* panel6 = new GLUI_Panel(glui, "  ", GLUI_PANEL_NONE);
	new GLUI_Button(panel6, "Cut Right", 5, cutRight);
	
	GLUI_Panel* panel7 = new GLUI_Panel(glui, "  ", GLUI_PANEL_NONE);
	new GLUI_Button(panel7, "Extend Right", 6, extendLeft);

	GLUI_Panel* panel8 = new GLUI_Panel(glui, "  ", GLUI_PANEL_NONE);
	new GLUI_Button(panel8, "Extend Left", 7, extendRight);

	GLUI_Panel* panel2 = new GLUI_Panel(glui, "  ", GLUI_PANEL_NONE);
	new GLUI_Button(panel2, "Clear", 8, Clear);

	GLUI_Panel* panel4 = new GLUI_Panel(glui, "  ", GLUI_PANEL_NONE);
	new GLUI_Button(panel4, "EXIT", 9, Exit);

	

}

void init() {
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
}

void reshape(int w, int h) {
	glViewport(0, 0, w, h);
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	buildGlui(displayWin);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(0, 0);
	displayWin=glutCreateWindow("Moving on a sphere");
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	
	glutMainLoop();
	return 0;
}