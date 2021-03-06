#include <GL/glut.h>
#include <bevgrafmath2017.h>
#include <iostream>

GLint winWidth = 800, winHeight = 600;

vec3 b[60];  //control points
vec3 q[301]; //surface points

int currentCP = 0;
int maxCP = 15; // maximum controlpoints
float controlPointMoveDist = 1.0;



void knot(int n, int c, int x[]) {
	int nplusc, nplus2, i;

	nplusc = n + c;
	nplus2 = n + 2;

	x[1] = 0;
	for (i = 2; i <= nplusc; i++) {
		if ((i > c) && (i < nplus2))
			x[i] = x[i - 1] + 1;
		else
			x[i] = x[i - 1];
	}
}

void basis(int c, float t, int npts, int x[], float n[]) {
	int nplusc;
	int i, k;
	float d, e;
	float temp[36];

	nplusc = npts + c;

	/*		printf("knot vector is \n");
			for (i = 1; i <= nplusc; i++){
				printf(" %d %d \n", i,x[i]);
			}
			printf("t is %f \n", t);
	*/

	/* calculate the first order basis functions n[i][1]	*/

	for (i = 1; i <= nplusc - 1; i++) {
		if ((t >= x[i]) && (t < x[i + 1]))
			temp[i] = 1;
		else
			temp[i] = 0;
	}

	/* calculate the higher order basis functions */

	for (k = 2; k <= c; k++) {
		for (i = 1; i <= nplusc - k; i++) {
			if (temp[i] != 0)    /* if the lower order basis function is zero skip the calculation */
				d = ((t - x[i])*temp[i]) / (x[i + k - 1] - x[i]);
			else
				d = 0;

			if (temp[i + 1] != 0)     /* if the lower order basis function is zero skip the calculation */
				e = ((x[i + k] - t)*temp[i + 1]) / (x[i + k] - x[i + 1]);
			else
				e = 0;

			temp[i] = d + e;
		}
	}

	if (t == (float)x[nplusc]) {		/*    pick up last point	*/
		temp[npts] = 1;
	}

	/* put in n array	*/

	for (i = 1; i <= npts; i++) {
		n[i] = temp[i];
	}
}

//Simple bspline for understanding
void bspline(int npts, int k, int p1, vec3 b[], vec3 p[]) {
	int i, icount, jcount;
	int i1;
	int x[30];		/* allows for 20 data points with basis function of order 5 */
	int nplusc;

	float step;
	float t;
	float nbasis[20];
	float tempx;
	float tempy;
	float tempz;


	nplusc = npts + k;

	/*  zero and redimension the knot vector and the basis array */
	for (i = 0; i <= npts; i++) {
		nbasis[i] = 0.;
	}

	for (i = 0; i <= nplusc; i++) {
		x[i] = 0;
	}

	/* generate the uniform open knot vector */
	knot(npts, k, x);

	/*
		printf("The knot vector is ");
		for (i = 1; i <= nplusc; i++){
			printf(" %d ", x[i]);
		}
		printf("\n");
	*/

	/*    calculate the points on the bspline curve */
	icount = 0;
	t = 0;
	step = ((float)x[nplusc]) / ((float)(p1 - 1));

	for (i1 = 1; i1 <= p1; i1++) {

		if ((float)x[nplusc] - t < 5e-6) {
			t = (float)x[nplusc];
		}

		basis(k, t, npts, x, nbasis);      /* generate the basis function for this value of t */
		/*
		printf("t = %f \n", t);
		printf("nbasis = ");
		for (i = 1; i <= npts; i++) {
			printf("%f  ", nbasis[i]);
		}
		printf("\n");
		*/
		p[icount].x = 0;
		p[icount].y = 0;
		p[icount].z = 0;
		for (i = 0; i < npts; i++) { /* Do local matrix multiplication */
			tempx = nbasis[i + 1] * b[i].x;
			p[icount].x = p[icount].x + tempx;
			tempy = nbasis[i + 1] * b[i].y;
			p[icount].y = p[icount].y + tempy;
			tempz = nbasis[i + 1] * b[i].z;
			p[icount].z = p[icount].z + tempz;
			/*
			printf("nbasis,b.x,b.y,b.z,nbasis*b,p.x,p.y,p.z = %f; %f %f %f; %f %f %f; %f %f %f\n", nbasis[i + 1], b[i].x, b[i].y, b[i].z, tempx, tempy, tempz, p[icount].x, p[icount].y, p[icount].z);
			*/
		}
		icount++;
		t = t + step;
	}
}

void bsplsurf(vec3 b[], int k, int l, int npts, int mpts, int p1, int p2, vec3 q[]) {
	int i, j, j1, jbas;
	int icount;
	int uinc, winc;
	int nplusc, mplusc;
	int x[30], y[30];
	int temp;

	float nbasis[30], mbasis[30];
	float pbasis;
	float u, w;
	float stepu, stepw;

	nplusc = npts + k;
	mplusc = mpts + l;

	for (i = 1; i <= nplusc; i++) {
		x[i] = 0;
	}
	for (i = 1; i <= mplusc; i++) {
		y[i] = 0;
	}
	for (i = 1; i <= npts; i++) {
		nbasis[i] = 0.;
	}
	for (i = 1; i <= mpts; i++) {
		mbasis[i] = 0.;
	}

	for (i = 0; i < p1*p2; i++) {
		q[i].x = 0;
		q[i].y = 0;
		q[i].z = 0;
	}

	//generate the open uniform knot vectors

	knot(npts, k, x);       //  calculate u knot vector
	knot(mpts, l, y);       //  calculate w knot vector

	icount = 0;

	//calculate the points on the surface

	stepu = (float)x[nplusc] / (float)(p1 - 1);
	stepw = (float)y[mplusc] / (float)(p2 - 1);
	u = 0.;
	for (uinc = 1; uinc <= p1; uinc++) {
		//printf("u = %f \n",u);
		if ((float)x[nplusc] - u < 5e-6) {
			u = (float)x[nplusc];
		}
		basis(k, u, npts, x, nbasis);    //basis function for this value of u
		w = 0.;
		for (winc = 1; winc <= p2; winc++) {
			//printf("w = %f \n",w);
			if ((float)y[mplusc] - w < 5e-6) {
				w = (float)y[mplusc];
			}
			basis(l, w, mpts, y, mbasis);    //basis function for this value of w 
			for (i = 0; i < npts; i++) {
				if (nbasis[i + 1] != 0.) {
					jbas = mpts * i;
					for (j = 0; j < mpts; j++) {
						if (mbasis[j + 1] != 0.) {
							j1 = jbas + j;
							pbasis = nbasis[i + 1] * mbasis[j + 1];
							q[icount].x = q[icount].x + b[j1].x * pbasis;  //calculate surface point
							q[icount].y = q[icount].y + b[j1].y * pbasis;
							q[icount].z = q[icount].z + b[j1].z * pbasis;
							//printf("j1,i,j = %d %d %d \n",j1,i,j);
						}
					}

				}
			}
			icount = icount++;
			w = w + stepw;
		}
		u = u + stepu;
	}
}

void setControlPoints() {

	b[0].x = -15.;
	b[0].y = 0.;
	b[0].z = 15.;
	b[1].x = -15.;
	b[1].y = 5.;
	b[1].z = 5.;
	b[2].x = -15.;
	b[2].y = 5.;
	b[2].z = -5.;
	b[3].x = -15.;
	b[3].y = 0.;
	b[3].z = -15.;

	b[4].x = -5.;
	b[4].y = 5.;
	b[4].z = 15.;
	b[5].x = -5.;
	b[5].y = 10.;
	b[5].z = 5.;
	b[6].x = -5.;
	b[6].y = 10.;
	b[6].z = -5.;
	b[7].x = -5.;
	b[7].y = 5.;
	b[7].z = -15.;

	b[8].x = 5.;
	b[8].y = 5.;
	b[8].z = 15.;
	b[9].x = 5.;
	b[9].y = 10.;
	b[9].z = 5.;
	b[10].x = 5.;
	b[10].y = 10.;
	b[10].z = -5.;
	b[11].x = 5.;
	b[11].y = 0.;
	b[11].z = -15.;

	b[12].x = 15.;
	b[12].y = 0.;
	b[12].z = 15.;
	b[13].x = 15.;
	b[13].y = 5.;
	b[13].z = 5.;
	b[14].x = 15.;
	b[14].y = 5.;
	b[14].z = -5.;
	b[15].x = 15.;
	b[15].y = 0.;
	b[15].z = -15.;
}


void init() {

	setControlPoints();
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0, 0.0, winWidth, winHeight);
	glOrtho(-20, 20, -20, 20, 0.1, 100.0);
	glShadeModel(GL_FLAT);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_POINT_SMOOTH);
	gluLookAt(0, 0, -60, 0, 0, 0, 0, 1, 0);
	glPointSize(3.0);
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int i;
	int npts, mpts;
	int k, l;
	int p1, p2;



	//test data control points
	npts = 4;
	mpts = 4;
	k = 4;
	l = 3;

	p1 = 5;
	p2 = 5;

	/*
	for (i = 1; i <= 3 * npts; i++) {
		b[i] = 0.;
	}*/

	for (i = 1; i <= 3 * p1*p2; i++) {
		q[i] = 0.;
	}


	bsplsurf(b, k, l, npts, mpts, p1, p2, q);

	glColor3d(0.0, 1.0, 0.0);
	glBegin(GL_POINTS);
	for (i = 0; i < npts*mpts; i = i++) {
		if (i == currentCP)
			glColor3d(0.0, 0.0, 1.0);
		else
			glColor3d(0.0, 1.0, 0.0);
		glVertex3d(b[i].x, b[i].y, b[i].z);
	}
	glEnd();

	glColor3d(1.0, 0.0, 0.0);
	glBegin(GL_POINTS);
	for (i = 0; i < p1*p2; i = i++) {
		glVertex3d(q[i].x, q[i].y, q[i].z);
	}
	glEnd();

	//glFlush();
	glutSwapBuffers();
}

void keyUp(unsigned char key, int x, int y) {
	switch (key) {
	case 'w':
		glRotatef(1, 1.0, 0.0, 0.0);
		break;
	case 's':
		glRotatef(-1, 1.0, 0.0, 0.0);
		break;
	case 'a':
		glRotatef(1, 0.0, 1.0, 0.0);
		break;
	case 'd':
		glRotatef(-1, 0.0, 1.0, 0.0);
		break;
	case 'q':
		glRotatef(-1, 0.0, 0.0, 1.0);
		break;
	case 'e':
		glRotatef(1, 0.0, 0.0, 1.0);
		break;
	case 'h':
		break;
		// selecting the control point in focus
	case '-':
		if (currentCP > 0)
			currentCP -= 1;
		printf("Current control point: %d\n", currentCP);
		break;
	case '+':
		if (currentCP < maxCP)
			currentCP += 1;
		printf("Current control point: %d\n", currentCP);
		break;
	case '0':
		b[currentCP].x -= controlPointMoveDist;
		printf("Current: %d.x = %g \n", currentCP, b[currentCP].x);
		break;
	case '1':
		b[currentCP].x += controlPointMoveDist;
		printf("Current: %d.x = %g \n", currentCP, b[currentCP].x);
		break;
	case '2':
		b[currentCP].y -= controlPointMoveDist;
		printf("Current: %d.y = %g \n", currentCP, b[currentCP].y);
		break;
	case '3':
		b[currentCP].y += controlPointMoveDist;
		printf("Current: %d.y = %g \n", currentCP, b[currentCP].y);
		break;
	case '4':
		b[currentCP].z -= controlPointMoveDist;
		printf("Current: %d.z = %g \n", currentCP, b[currentCP].z);
		break;
	case '5':
		b[currentCP].z += controlPointMoveDist;
		printf("Current: %d.z = %g \n", currentCP, b[currentCP].z);
		break;

		//NOT REALLY A GOOD WAY	
		/*
		case '-':
				glScalef(0.5, 0.5, 1);
			break;
		case '+':
				glScalef(2, 2, 1);
			break;
			*/
	default:
		break;
	}
	glutPostRedisplay();
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);										// Initialize GLUT.
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);   // Set display mode.
	glutInitWindowPosition(winWidth / 2, winHeight / 2);		// Set top-left display-window position.
	glutInitWindowSize(winWidth, winHeight);					// Set display-window width and height.
	glutCreateWindow("T Spline");								// Create display window.

	init();														// Execute initialization procedure.
	glutDisplayFunc(display);									// Send graphics to display window.
	glutKeyboardFunc(keyUp);
	glutMainLoop();

	/*int i;
	int npts, k, p1;
	vec3 b[30];  // allows for up to 30  control vertices
	vec3 p[100]; // allows for up to 100 points on curve
	npts = 4;  // control points
	k = 2;     // second order, change to 4 to get fourth order
	p1 = 11;   // eleven points on curve
	for (i = 1; i <= 3 * npts; i++) {
		b[i] = 0.;
	}
	for (i = 1; i <= 3 * p1; i++) {
		p[i] = 0.;
	}
	//test data control points
	b[0].x = 1;
	b[0].y = 1;
	b[0].z = 1;
	b[1].x = 2;
	b[1].y = 3;
	b[1].z = 1;
	b[2].x = 4;
	b[2].y = 3;
	b[2].z = 1;
	b[3].x = 3;
	b[3].y = 1;
	b[3].z = 1;
	bspline(npts, k, p1, b, p);
	printf("\nPolygon points\n\n");
	for (i = 0; i < npts; i = i++) {
		printf(" %f %f %f \n", b[i].x, b[i].y, b[i].z);
	}
	printf("\nCurve points\n\n");
	for (i = 0; i < p1; i = i++) {
		printf(" %f %f %f \n", p[i].x, p[i].y, p[i].z);
	}*/

	/*int i;
	int npts, mpts;
	int k, l;
	int p1, p2;
	vec3 b[60];  //control points
	vec3 q[301]; //surface points
	//test data control points
	npts = 4;
	mpts = 4;
	k = 4;
	l = 3;
	p1 = 5;
	p2 = 5;
	for (i = 1; i <= 3 * npts; i++) {
		b[i] = 0.;
	}
	for (i = 1; i <= 3 * p1*p2; i++) {
		q[i] = 0.;
	}
	b[0].x = -15.;
	b[0].y = 0.;
	b[0].z = 15.;
	b[1].x = -15.;
	b[1].y = 5.;
	b[1].z = 5.;
	b[2].x = -15.;
	b[2].y = 5.;
	b[2].z = -5.;
	b[3].x = -15.;
	b[3].y = 0.;
	b[3].z = -15.;
	b[4].x = -5.;
	b[4].y = 5.;
	b[4].z = 15.;
	b[5].x = -5.;
	b[5].y = 10.;
	b[5].z = 5.;
	b[6].x = -5.;
	b[6].y = 10.;
	b[6].z = -5.;
	b[7].x = -5.;
	b[7].y = 5.;
	b[7].z = -15.;
	b[8].x = 5.;
	b[8].y = 5.;
	b[8].z = 15.;
	b[9].x = 5.;
	b[9].y = 10.;
	b[9].z = 5.;
	b[10].x = 5.;
	b[10].y = 10.;
	b[10].z = -5.;
	b[11].x = 5.;
	b[11].y = 0.;
	b[11].z = -15.;
	b[12].x = 15.;
	b[12].y = 0.;
	b[12].z = 15.;
	b[13].x = 15.;
	b[13].y = 5.;
	b[13].z = 5.;
	b[14].x = 15.;
	b[14].y = 5.;
	b[14].z = -5.;
	b[15].x = 15.;
	b[15].y = 0.;
	b[15].z = -15.;
	bsplsurf(b, k, l, npts, mpts, p1, p2, q);
	printf("\nPolygon points\n\n");
	for (i = 0; i < npts*mpts; i = i++) {
		printf(" %f %f %f \n", b[i].x, b[i].y, b[i].z);
	}
	printf("\nSurface points\n\n");
	for (i = 0; i < p1*p2; i = i++) {
		printf(" %f %f %f \n", q[i].x, q[i].y, q[i].z);
	}
	*/


	return 0;
}