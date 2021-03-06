// ConsoleApplication1.cpp: 定义控制台应用程序的入口点。
//

// 2020_0205_Semi_Stagger_Grid_2D_NSE_DBC_P_Part.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <time.h>
#include "string.h"
#include <iostream>
#include <math.h>
#include <fstream>

using namespace std;

#define dt 0.001
#define N_t 1
#define PI 3.1415926536

#define DA   2.0
#define DALPHA PI
#define DBETA  2.0

#define rho     1.0
#define KINVIS  1.0 //kinetic viscosity 
#define Re      1.0 / KINVIS 

void Set_Grid_And_Constant_Equ_Params(
	double *aw, double *ae, double *an, double *as, double *ap,
	double *igrid_x, double *igrid_y, int GridX, int GridY,
	double dx_h, double dy_h) {

	for (int iy = 1; iy < (GridY - 1); iy++)
	{
		for (int ix = 0; ix < GridX; ix++)
		{
			int i = ix + iy * GridX;

			if ((ix == 0) || (ix == (GridX - 1))) igrid_x[i] = 0.5;
			else igrid_x[i] = 1.0;

			if ((iy == 1) || (iy == (GridY - 2))) igrid_y[i] = 0.5;
			else igrid_y[i] = 1.0;

			double dx = igrid_x[i] * dx_h, dy = igrid_y[i] * dy_h, dV = dx * dy;

			aw[i] = igrid_y[i], ae[i] = aw[i], an[i] = igrid_x[i], as[i] = an[i];

			if (ix == 0) aw[i] = 0;
			if (ix == (GridX - 1)) ae[i] = 0;
			if (iy == 1) an[i] = 0;
			if (iy == (GridY - 2)) as[i] = 0;

			ap[i] = aw[i] + ae[i] + an[i] + as[i];
		}
	}
}

void Set_Grid_And_Constant_Equ_Params_P(
	double *aw, double *ae, double *an, double *as, double *ap,
	double *igrid_x, double *igrid_y, int GridX, int GridY,
	double dx_h, double dy_h) {

	for (int iy = 2; iy < (GridY - 2); iy++)
	{
		for (int ix = 1; ix < (GridX - 1); ix++)
		{
			int i = ix + iy * GridX;

			aw[i] = 1, ae[i] = 1, an[i] = 1, as[i] = 1;

			if (ix == 1) aw[i] = 0;
			if (ix == (GridX - 2)) ae[i] = 0;
			if (iy == 2) an[i] = 0;
			if (iy == (GridY - 3)) as[i] = 0;

			ap[i] = aw[i] + ae[i] + an[i] + as[i];
		}
	}

}

void Real_Load_Central(
	double *u_real, double *v_real, double *p_real,
	int GridX_Central, int GridY_Central,
	double dx_h, double dy_h, int iter_start_step) {

	int N = GridX_Central - 1;

	for (int iy = 1; iy < (GridY_Central - 1); iy++)
	{
		for (int ix = 0; ix < GridX_Central; ix++)
		{
			int i = ix + iy * GridX_Central;

			double y = (iy - (N / 2) - 1) * dy_h, x = ix * dx_h;

			double t = iter_start_step * dt;

			u_real[i] = DA * cos(PI*y)*sin(DALPHA*x)*sin(DBETA*t);
			v_real[i] = -(DA*DALPHA / PI)*sin(PI*y)*cos(DALPHA*x)*sin(DBETA*t);
			p_real[i] = DA * sin(PI*y)*sin(DALPHA*x)*cos(DBETA*t);

		}
	}
}


void Boundary_Load_on_Central(
	double *u_c, double *v_c, double *u_real,
	double *v_real, int GridX_Central, int GridY_Central) {

	for (int iy = 1; iy < (GridY_Central - 1); iy++)
	{
		int i = iy * GridX_Central;
		int i_2 = iy * GridX_Central + (GridX_Central - 1);

		u_c[i] = u_real[i];
		v_c[i] = v_real[i];

		u_c[i_2] = u_real[i_2];
		v_c[i_2] = v_real[i_2];

	}

	for (int ix = 0; ix < GridX_Central; ix++)
	{
		int i = ix + GridX_Central;
		int i_2 = ix + GridX_Central * (GridY_Central - 2);

		u_c[i] = u_real[i];
		v_c[i] = v_real[i];

		u_c[i_2] = u_real[i_2];
		v_c[i_2] = v_real[i_2];
	}
}

void Init_U(
	double *u_n, double *u_c,
	int GridX_U, int GridY_U, int GridX_Central, int GridY_Central,
	double dx_h, double dy_h, int i_t) {

	int N = GridX_U - 1;

	for (int ix = 0; ix < GridX_U; ix++)
	{
		for (int iy = 2; iy < (GridY_U - 2); iy++)
		{
			int i = ix + iy * GridX_U;

			double y = (iy - (N / 2) - 1 - 1 + 0.5) * dy_h, x = ix * dx_h;

			double t = i_t * dt;

			u_n[i] = DA * cos(PI*y)*sin(DALPHA*x)*sin(DBETA*t);
		}

		int i_u_0 = ix + 1 * GridX_U, i_u_1 = ix + (GridY_U - 2)* GridX_U;
		int i_c_0 = ix + 1 * GridX_Central, i_c_1 = ix + (GridY_Central - 2)* GridX_Central;

		u_n[i_u_0] = 2 * u_c[i_c_0] - u_n[i_u_0 + GridX_U];
		u_n[i_u_1] = 2 * u_c[i_c_1] - u_n[i_u_1 - GridX_U];
	}
}


void Init_V(
	double *v_n, double *v_c,
	int GridX_V, int GridY_V, int GridX_Central, int GridY_Central,
	double dx_h, double dy_h, int i_t) {

	int N = GridX_V - 1 - 1;

	for (int iy = 1; iy < (GridY_V - 1); iy++)
	{
		for (int ix = 1; ix < (GridX_V - 1); ix++)
		{
			int i = ix + iy * GridX_V;

			double y = (iy - (N / 2) - 1) * dy_h, x = (ix - 1 + 0.5) * dx_h;

			double t = i_t * dt;

			v_n[i] = -(DA*DALPHA / PI)*sin(PI*y)*cos(DALPHA*x)*sin(DBETA*t);
		}

		int i_v_0 = iy * GridX_V, i_v_1 = (GridX_V - 1) + iy * GridX_V;
		int i_c_0 = iy * GridX_Central, i_c_1 = (GridX_Central - 1) + iy * GridX_Central;

		v_n[i_v_0] = 2 * v_c[i_c_0] - v_n[i_v_0 + 1];
		v_n[i_v_1] = 2 * v_c[i_c_1] - v_n[i_v_1 - 1];
	}
}

void Set_DBC_UV_for_PBC_without_interp(
	double *u, double *v,
	int GridX_U, int GridY_U, int GridX_V, int GridY_V,
	double dx_h, double dy_h, int i_t) {

	int N = GridX_U - 1;

	for (int iy = 2; iy < (GridY_U - 2); iy++)
	{
		int i_0 = iy * GridX_U;
		int i_1 = GridX_U - 1 + iy * GridX_U;

		double y = (iy - (N / 2) - 1 - 1 + 0.5) * dy_h;
		double x_0 = 0, x_1 = (GridX_U - 1) * dx_h;

		double t = i_t * dt;

		u[i_0] = DA * cos(PI*y)*sin(DALPHA*x_0)*sin(DBETA*t);
		u[i_1] = DA * cos(PI*y)*sin(DALPHA*x_1)*sin(DBETA*t);
	}

	for (int ix = 1; ix < (GridX_V - 1); ix++)
	{

		int i_0 = ix + 1 * GridX_V;
		int i_1 = ix + (GridX_V - 1) * GridX_V;

		double y_0 = (1 - (N / 2) - 1) * dy_h, y_1 = (GridX_V - 1 - (N / 2) - 1) * dy_h;

		double x = (ix - 1 + 0.5) * dx_h;

		double t = i_t * dt;

		v[i_0] = -(DA*DALPHA / PI)*sin(PI*y_0)*cos(DALPHA*x)*sin(DBETA*t);
		v[i_1] = -(DA*DALPHA / PI)*sin(PI*y_1)*cos(DALPHA*x)*sin(DBETA*t);
	}

}

void Get_Source_U(
	double *F_u, int GridX_U, int GridY_U, double dx_h, double dy_h, int i_t) {

	int N = GridX_U - 1;

	for (int iy = 2; iy <= (GridY_U - 3); iy++)
	{
		for (int ix = 0; ix <= (GridX_U - 1); ix++)
		{
			int i = ix + iy * GridX_U;

			double y = (iy - (N / 2) - 1 - 1 + 0.5) * dy_h, x = ix * dx_h;

			double t = i_t * dt;

			F_u[i] = (DA*DBETA*cos(PI*y)*sin(DALPHA*x)*cos(DBETA*t)
				+ DA * DA*DALPHA*sin(DALPHA*x)*cos(DALPHA*x)*sin(DBETA*t)*sin(DBETA*t))
				+ DA * DALPHA*sin(PI*y)*cos(DALPHA*x)*cos(DBETA*t)
				+ KINVIS * (DA*(PI*PI + DALPHA * DALPHA)*cos(PI*y)*sin(DALPHA*x)*sin(DBETA*t));
		}
	}

}

void Get_Source_V(
	double *F_v, int GridX_V, int GridY_V, double dx_h, double dy_h, int i_t) {

	int N = GridX_V - 1 - 1;

	for (int iy = 1; iy < (GridY_V - 1); iy++)
	{
		for (int ix = 1; ix < (GridX_V - 1); ix++)
		{
			int i = ix + iy * GridX_V;

			double y = (iy - (N / 2) - 1) * dy_h, x = (ix - 1 + 0.5) * dx_h;

			double t = i_t * dt;

			F_v[i] = ((-DA * DALPHA*DBETA / PI)*sin(PI*y)*cos(DALPHA*x)*cos(DBETA*t)
				+ (DA*DA*DALPHA*DALPHA / PI)*cos(PI*y)*sin(PI*y)*sin(DBETA*t)*sin(DBETA*t))
				+ (PI*DA)*cos(PI*y)*sin(DALPHA*x)*cos(DBETA*t)
				- KINVIS * ((DA*DALPHA*(DALPHA*DALPHA + PI * PI) / PI)*sin(PI*y)*cos(DALPHA*x)*sin(DBETA*t));
		}
	}

}

void Gu_Update(double *G_u, double *F_u, double *u_n, double *v_n, int GridX_U, int GridY_U, int GridX_V, double dx_h, double dy_h) {

	double Cu = 0, S = 0;
	double Fw = 0, Fe = 0, Fn = 0, Fs = 0;
	double aw = 0, ae = 0, an = 0, as = 0, ap = 0;

	double inv_dV = 1.0 / (dx_h * dy_h);
	bool alpha_w = 0, alpha_e = 0, alpha_n = 0, alpha_s = 0;

	for (int iy = 2; iy <= (GridY_U - 3); iy++)
	{
		for (int ix = 0; ix <= (GridX_U - 1); ix++)
		{
			int i = ix + iy * GridX_U;
			int i_v = ix + iy * GridX_V;

			Fw = dy_h * (u_n[i] + u_n[i - 1]) * 0.5;
			Fe = dy_h * (u_n[i] + u_n[i + 1]) * 0.5;
			Fn = dx_h * (v_n[i_v - GridX_V] + v_n[i_v - GridX_V + 1]) * 0.5;
			Fs = dx_h * (v_n[i_v] + v_n[i_v + 1]) * 0.5;

			alpha_w = 0, alpha_e = 0, alpha_n = 0, alpha_s = 0;
			if (Fw > 0) alpha_w = 1;
			if (Fe > 0) alpha_e = 1;
			if (Fn > 0) alpha_n = 1;
			if (Fs > 0) alpha_s = 1;

			aw = alpha_w * Fw, ae = -(1 - alpha_e) * Fe;
			an = alpha_n * Fn, as = -(1 - alpha_s) * Fs;

			//add the Fi limits //边界没有可以用的点
			if (ix == 0) aw = 0, Fw = 0;
			if (ix == (GridX_U - 1)) ae = 0, Fe = 0;

			ap = aw + ae + an + as + (Fe - Fw) + (Fs - Fn);

			S = 0;
			//边界可用点大于两个
			if ((ix > 1) && (ix < (GridX_U - 2)))
				S = alpha_w * Fw * (3 * u_n[i] - 2 * u_n[i - 1] - u_n[i - 2])
				+ alpha_e * Fe * (u_n[i - 1] + 2 * u_n[i] - 3 * u_n[i + 1])
				+ (1 - alpha_w) * Fw * (3 * u_n[i - 1] - 2 * u_n[i] - u_n[i + 1])
				+ (1 - alpha_e) * Fe * (2 * u_n[i + 1] + u_n[i + 2] - 3 * u_n[i]);

			if ((iy > 2) && (iy < (GridY_U - 3)))
				S += alpha_n * Fn * (3 * u_n[i] - 2 * u_n[i - GridX_U] - u_n[i - 2 * GridX_U])
				+ alpha_s * Fs * (u_n[i - GridX_U] + 2 * u_n[i] - 3 * u_n[i + GridX_U])
				+ (1 - alpha_n) * Fn * (3 * u_n[i - GridX_U] - 2 * u_n[i] - u_n[i + GridX_U])
				+ (1 - alpha_s) * Fs * (2 * u_n[i + GridX_U] + u_n[i + 2 * GridX_U] - 3 * u_n[i]);

			Cu = ap * u_n[i] - aw * u_n[i - 1] - ae * u_n[i + 1] - an * u_n[i - GridX_U] - as * u_n[i + GridX_U] - S * 0.125;

			//G_u[i] = F_u[i] + u_n[i] / dt - Cu * inv_dV;
			G_u[i] = F_u[i];
		}

	}

}

void Gv_Update(double *G_v, double *F_v, double *u_n, double *v_n, int GridX_U, int GridX_V, int GridY_V, double dx_h, double dy_h)
{

	double Cv = 0, S = 0;
	double Fw = 0, Fe = 0, Fn = 0, Fs = 0;
	double aw = 0, ae = 0, an = 0, as = 0, ap = 0;

	double inv_dV = 1.0 / (dx_h * dy_h);
	bool alpha_w = 0, alpha_e = 0, alpha_n = 0, alpha_s = 0;

	for (int iy = 1; iy < (GridY_V - 1); iy++)
	{
		for (int ix = 1; ix < (GridX_V - 1); ix++)
		{
			int i = ix + iy * GridX_V;
			int i_u = ix + iy * GridX_U;

			Fw = dy_h * (u_n[i_u - 1] + u_n[i_u - 1 + GridX_U]) * 0.5;
			Fe = dy_h * (u_n[i_u] + u_n[i_u + GridX_U]) * 0.5;
			Fn = dx_h * (v_n[i] + v_n[i - GridX_V]) * 0.5;
			Fs = dx_h * (v_n[i] + v_n[i + GridX_V]) * 0.5;

			alpha_w = 0, alpha_e = 0, alpha_n = 0, alpha_s = 0;
			if (Fw > 0) alpha_w = 1;
			if (Fe > 0) alpha_e = 1;
			if (Fn > 0) alpha_n = 1;
			if (Fs > 0) alpha_s = 1;

			aw = alpha_w * Fw, ae = -(1 - alpha_e) * Fe;
			an = alpha_n * Fn, as = -(1 - alpha_s) * Fs;

			//add the Fi limits
			if (iy == 1) an = 0, Fn = 0;
			if (iy == (GridY_V - 2)) as = 0, Fs = 0;

			ap = aw + ae + an + as + (Fe - Fw) + (Fs - Fn);

			S = 0;
			if ((ix > 1) && (ix < (GridX_V - 2)))
				S = alpha_w * Fw * (3 * v_n[i] - 2 * v_n[i - 1] - v_n[i - 2])
				+ alpha_e * Fe * (v_n[i - 1] + 2 * v_n[i] - 3 * v_n[i + 1])
				+ (1 - alpha_w) * Fw * (3 * v_n[i - 1] - 2 * v_n[i] - v_n[i + 1])
				+ (1 - alpha_e) * Fe * (2 * v_n[i + 1] + v_n[i + 2] - 3 * v_n[i]);

			if ((iy > 2) && (iy < (GridY_V - 3)))
				S += alpha_n * Fn * (3 * v_n[i] - 2 * v_n[i - GridX_V] - v_n[i - 2 * GridX_V])
				+ alpha_s * Fs * (v_n[i - GridX_V] + 2 * v_n[i] - 3 * v_n[i + GridX_V])
				+ (1 - alpha_n) * Fn * (3 * v_n[i - GridX_V] - 2 * v_n[i] - v_n[i + GridX_V])
				+ (1 - alpha_s) * Fs * (2 * v_n[i + GridX_V] + v_n[i + 2 * GridX_V] - 3 * v_n[i]);

			Cv = ap * v_n[i] - aw * v_n[i - 1] - ae * v_n[i + 1] - an * v_n[i - GridX_V] - as * v_n[i + GridX_V] - S * 0.125;

			//G_v[i] = F_v[i] + v_n[i] / dt - Cv * inv_dV;
			G_v[i] = F_v[i];
		}
	}
}

void Get_G_real_without_F(
	double *G_u_real, double *G_v_real, double *u_real, double *v_real,
	int GridX_Central, int GridY_Central, double dx_h, double dy_h, int i_t) {

	int N = GridX_Central - 1;

	for (int iy = 1; iy < (GridY_Central - 1); iy++)
	{
		for (int ix = 0; ix < GridX_Central; ix++)
		{
			int i = ix + iy * GridX_Central;

			double y = (iy - (N / 2) - 1) * dy_h, x = ix * dx_h;

			double t = i_t * dt;

			double l_u_C = 4 * PI* sin(2 * t) * sin(PI * x) * sin(2 * t) * cos(PI * x);
			double l_v_C = 4 * PI* sin(2 * t) * sin(PI * y) * sin(2 * t) * cos(PI * y);

			G_u_real[i] = u_real[i] - l_u_C;
			G_v_real[i] = v_real[i] - l_v_C;
		}
	}
}

void Display_errors(
	double *u_n, double *v_n, double *u_real, double *v_real,
	int GridX_Central, int GridY_Central, int GridX_U, int GridX_V) {

	for (int iy = 1; iy < (GridY_Central - 1); iy++)
	{
		for (int ix = 0; ix < GridX_Central; ix++)
		{
			int i_c = ix + iy * GridX_Central;
			int i_u = ix + iy * GridX_U;
			int i_v = ix + iy * GridX_V;

			double u_cal = 0.5 * (u_n[i_u] + u_n[i_u + GridX_U]);
			double v_cal = 0.5 * (v_n[i_v] + v_n[i_v + 1]);

			// cout << u_cal <<" ?= "<<u_real[i_c] <<"    ";
			cout << v_cal - v_real[i_c] << "    ";
		}

		cout << endl;
	}

}

void Set_DBC_UV_for_PBC(double *u, double *v, double *u_c, double *v_c,
	int GridX_U, int GridY_U, int GridX_V, int GridY_V, int GridX_Central, int GridY_Central) {

	for (int ix = 1; ix < (GridX_V - 1); ix++)
	{
		int i_v_0 = ix + 1 * GridX_V, i_c_0 = ix + GridX_Central;
		int i_v_1 = ix + (GridY_V - 2) * GridX_V, i_c_1 = ix + (GridX_Central - 2) * GridX_Central;

		v[i_v_0] = 0.5 * (v_c[i_c_0] + v_c[i_c_0 - 1]);
		v[i_v_1] = 0.5 * (v_c[i_c_1] + v_c[i_c_1 - 1]);
	}

	for (int iy = 2; iy <= (GridX_U - 3); iy++)
	{
		int i_u_0 = 0 + iy * GridX_U;
		int i_u_1 = (GridX_U - 1) + iy * GridX_U;

		int i_c_0 = iy * GridX_Central;
		int i_c_1 = (GridX_Central - 1) + iy * GridX_Central;

		u[i_u_0] = 0.5 * (u_c[i_c_0] + u_c[i_c_0 - GridX_Central]);
		u[i_u_1] = 0.5 * (u_c[i_c_1] + u_c[i_c_1 - GridX_Central]);
	}
}

void Set_Us_for_PBC(double *u_s, double *v_s, double *u_n, double *v_n,
	int GridX_U, int GridY_U, int GridX_V, int GridY_V) {

	for (int iy = 2; iy < (GridY_U - 2); iy++)
	{
		for (int ix = 0; ix < 3; ix++)
		{
			int i_u_0 = ix + iy * GridX_U;
			int i_v_0 = ix + iy * GridX_V;

			int i_u_1 = ix + (GridX_U - 3) + iy * GridX_U;
			int i_v_1 = ix + (GridX_U - 3) + iy * GridX_V;
			u_s[i_u_0] = 0.25 * (v_n[i_v_0] + v_n[i_v_0 + 1] + v_n[i_v_0 - GridX_V] + v_n[i_v_0 - GridX_V + 1]);
			u_s[i_u_1] = 0.25 * (v_n[i_v_1] + v_n[i_v_1 + 1] + v_n[i_v_1 - GridX_V] + v_n[i_v_1 - GridX_V + 1]);

			//U网格上的V
		}
	}

	for (int ix = 1; ix < (GridX_V - 1); ix++)
	{
		for (int iy = 1; iy < 4; iy++)
		{
			int i_v_0 = ix + iy * GridX_V;
			int i_u_0 = ix + iy * GridX_U;

			int i_v_1 = ix + (GridY_V - 5 + iy) * GridX_V;
			int i_u_1 = ix + (GridY_U - 5 + iy) * GridX_U;

			v_s[i_v_0] = 0.25 * (u_n[i_u_0] + u_n[i_u_0 - 1] + u_n[i_u_0 + GridX_U] + u_n[i_u_0 + GridX_U - 1]);
			v_s[i_v_1] = 0.25 * (u_n[i_u_1] + u_n[i_u_1 - 1] + u_n[i_u_1 + GridX_U] + u_n[i_u_1 + GridX_U - 1]);

			//V网格上插值的U
		}
	}
}

void RB_GS_solver(double *p, double *p_0, double *p_s, double *p_b, double *a_w, double *a_e, double *a_n, double *a_s, double *a_p, int Nx, int Ny, double dx_h, double dy_h)
{
	double Dx = dx_h, Dy = dy_h;

	double dif = 0;
	int cir_max = 10000;
	double relax = 1.0;

	for (int cir = 0; (cir < cir_max); cir++)
	{
		for (int iy = 2; iy < (Ny - 2); iy++)
		{
			for (int ix = 1; ix < (Nx - 1); ix++)
			{
				int i = ix + iy * Nx;

				double aw = a_w[i], ae = a_e[i], an = a_n[i], as = a_s[i], ap = a_p[i];

				if (ap == 0)
					cout << "ix = " << ix << "iy ==" << iy << endl;

				if ((i % 2) == 0)
				{
					double temp = (1.0 / ap) * (aw * p_0[i - 1] + ae * p_0[i + 1] + an * p_0[i - Nx] + as * p_0[i + Nx] + p_b[i]);
					p_s[i] = relax * temp + (1 - relax) * p_0[i];
				}

				else
					p_s[i] = p_0[i];
			}
		}

		for (int iy = 2; iy < (Ny - 2); iy++)
		{
			for (int ix = 1; ix < (Nx - 1); ix++)
			{
				int i = ix + iy * Nx;

				double aw = a_w[i], ae = a_e[i], an = a_n[i], as = a_s[i], ap = a_p[i];

				if (ap == 0)
					cout << "ix = " << ix << "iy ==" << iy << endl;

				if ((i % 2) != 0)
				{
					double temp = (1.0 / ap) * (aw * p_s[i - 1] + ae * p_s[i + 1] + an * p_s[i - Nx] + as * p_s[i + Nx] + p_b[i]);
					p[i] = relax * temp + (1 - relax) * p_s[i];
				}
				else
					p[i] = p_s[i];

			}
		}

		double error = 0;

		for (int iy = 2; iy < (Ny - 2); iy++)
		{
			for (int ix = 1; ix < (Nx - 1); ix++)
			{
				int i = ix + iy * Nx;

				error += abs(p[i] - p_0[i]);
			}
		}

		error /= ((Nx - 2) * (Ny - 2));

		if ((cir % 1) == 0)
			cout << "    cir =" << cir << "  iter_error = " << error << endl;

		if (error < 1e-8)
			break;

		if (error > 10)
			break;

		for (int iy = 2; iy < (Ny - 2); iy++)
		{
			for (int ix = 1; ix < (Nx - 1); ix++)
			{
				int i = ix + iy * Nx;

				p_0[i] = p[i];
			}
		}

	}
}

void NSE_P_RHS(double *p_b, double *G_u, double *G_v, double *u_s, double *v_s,
	double *u_n, double *v_n, double *pu_b, double *pv_b, double *u, double *v, double dx_h, double dy_h,
	int GridX_P, int GridY_P, int GridX_U, int GridY_U, int GridX_V, int GridY_V,
	double gamma_0_invdt, int i_t) {

	double u_dxy = 0, v_dxy = 0, u_dyy = 0, v_dxx = 0;
	int i_0, i_1, i_p_0, i_p_1;

	//on P Grid
	for (int iy = 2; iy < (GridY_P - 2); iy++)
	{
		for (int ix = 1; ix < (GridX_P - 1); ix++)
		{
			int i_p = ix + iy * GridX_P;
			int i_u = ix + iy * GridX_U;
			int i_v = ix + iy * GridX_V;

			p_b[i_p] = dy_h * (G_u[i_u] - G_u[i_u - 1]) + dx_h * (G_v[i_v] - G_v[i_v - GridX_V]);

			if (ix == 1) p_b[i_p] = -p_b[i_p] - (G_u[i_u - 1] - gamma_0_invdt * u[i_u - 1]);
			if (ix == (GridX_P - 2)) p_b[i_p] = -p_b[i_p] + (G_u[i_u] - gamma_0_invdt * u[i_u]);
			if (iy == 2) p_b[i_p] = -p_b[i_p] - (G_v[i_v - GridX_V] - gamma_0_invdt * v[i_u - GridX_V]);
			if (iy == (GridY_P - 3)) p_b[i_p] = -p_b[i_p] + (G_v[i_v] - gamma_0_invdt * v[i_v]);
		}
	}


	/*
	//SET P_B

	for (int iy = 2; iy < (GridY_P - 2); iy++)
	{
	for (int ix = 1; ix < (GridX_P - 1); ix++)
	{
	int i = ix + iy * GridX_P;

	double y = (iy - (N / 2) - 1 - 1 + 0.5) * dy_h, x = (ix - 1 + 0.5) * dx_h;

	double t = i_t * dt;

	double p_b = 4 * PI * PI * sin(PI * x) * sin(PI * y) * cos(2 * t)* dx_h * dy_h;

	cout << p_b << "  ";
	}
	cout << endl;
	}

	cout << "see me" << endl;

	for (int iy = 2; iy < (GridY_P - 2); iy++)
	{
	for (int ix = 1; ix < (GridX_P - 1); ix++)
	{
	int i = ix + iy * GridX_P;

	cout << p_b[i] << "  ";
	}
	cout << endl;
	}
	*/
	//看起来误差可控

	/*
	//on U Grid
	for (int iy = 2; iy < (GridY_U - 2); iy++)
	{
	i_0 = iy * GridX_U;
	i_p_0 = 1 + iy * GridX_P;

	u_dyy = (u_n[i_0 + GridX_U] - 2 * u_n[i_0] + u_n[i_0 - GridX_U]) / (dy_h * dy_h);

	if (iy == 2) v_dxy = (-3 * (-3 * u_s[i_0] + 4 * u_s[i_0 + 1] - u_s[i_0 + 2])
	+ 4 * (-3 * u_s[i_0 + GridX_U] + 4 * u_s[i_0 + GridX_U + 1] - u_s[i_0 + GridX_U + 2])
	- 1 * (-3 * u_s[i_0 + 2 * GridX_U] + 4 * u_s[i_0 + 2 * GridX_U + 1] - u_s[i_0 + 2 * GridX_U + 2])) / (4 * dx_h * dy_h);
	else if (iy == (GridY_U - 3)) v_dxy = (3 * (-3 * u_s[i_0] + 4 * u_s[i_0 + 1] - u_s[i_0 + 2])
	- 4 * (-3 * u_s[i_0 - GridX_U] + 4 * u_s[i_0 - GridX_U + 1] - u_s[i_0 - GridX_U + 2])
	+ 1 * (-3 * u_s[i_0 - 2 * GridX_U] + 4 * u_s[i_0 - 2 * GridX_U + 1] - u_s[i_0 - 2 * GridX_U + 2])) / (4 * dx_h * dy_h);
	else v_dxy = ((-3 * u_s[i_0 + GridX_U] + 4 * u_s[i_0 + GridX_U + 1] - u_s[i_0 + GridX_U + 2])
	- (-3 * u_s[i_0 - GridX_U] + 4 * u_s[i_0 - GridX_U + 1] - u_s[i_0 - GridX_U + 2])) / (4 * dx_h * dy_h);

	pu_b[i_0] = dy_h * (G_u[i_0] - gamma_0_invdt * u[i_0] - KINVIS * (v_dxy - u_dyy));
	p_b[i_p_0] = p_b[i_p_0] - pu_b[i_0];

	i_1 = GridX_U - 1 + iy * GridX_U;
	i_p_1 = GridX_P - 2 + iy * GridX_P;

	u_dyy = (u_n[i_1 + GridX_U] - 2 * u_n[i_1] + u_n[i_1 - GridX_U]) / (dy_h * dy_h);

	if (iy == 2) v_dxy = (-3 * (3 * u_s[i_1] - 4 * u_s[i_1 - 1] + u_s[i_1 - 2])
	+ 4 * (3 * u_s[i_1 + GridX_U] - 4 * u_s[i_1 + GridX_U - 1] + u_s[i_1 + GridX_U - 2])
	- 1 * (3 * u_s[i_1 + 2 * GridX_U] - 4 * u_s[i_1 + 2 * GridX_U - 1] + u_s[i_1 + 2 * GridX_U - 2])) / (4 * dx_h * dy_h);
	else if (iy == (GridY_U - 3)) v_dxy = (3 * (3 * u_s[i_1] - 4 * u_s[i_1 - 1] + u_s[i_1 - 2])
	- 4 * (3 * u_s[i_1 - GridX_U] - 4 * u_s[i_1 - GridX_U - 1] + u_s[i_1 - GridX_U - 2])
	+ 1 * (3 * u_s[i_1 - 2 * GridX_U] - 4 * u_s[i_1 - 2 * GridX_U - 1] + u_s[i_1 - 2 * GridX_U - 2])) / (4 * dx_h * dy_h);
	else  v_dxy = ((3 * u_s[i_1 + GridX_U] - 4 * u_s[i_1 + GridX_U - 1] + u_s[i_1 + GridX_U - 2])
	- (3 * u_s[i_1 - GridX_U] - 4 * u_s[i_1 - GridX_U - 1] + u_s[i_1 - GridX_U - 2])) / (4 * dx_h * dy_h);

	pu_b[i_1] = dy_h * (G_u[i_1] - gamma_0_invdt * u[i_1] - KINVIS * (v_dxy - u_dyy));
	p_b[i_p_1] = p_b[i_p_1] + pu_b[i_1];

	cout << pu_b[i_1] << "  " << pu_b[i_0] << endl;
	}

	int N = GridX_V - 1 - 1;
	for (int iy = 2; iy < (GridY_U - 2); iy++)
	{
	i_0 = iy * GridX_U;

	double y = (iy - (N / 2) - 1 - 1 + 0.5) * dy_h;
	double x_0 = 0;

	i_1 = GridX_U - 1 + iy * GridX_U;
	double x_1 = (GridX_U - 1) * dx_h;

	double t = i_t * dt;

	double  q_x = 2 * PI *cos(PI * x_0) * sin(PI * y) * cos(2 * t);

	cout << pu_b[i_0] <<" ??? "<< dy_h * q_x << "  ";

	q_x = 2 * PI *cos(PI * x_1) * sin(PI * y) * cos(2 * t);
	cout << pu_b[i_1] << " ??? " << dy_h * q_x << "   ";

	}

	//on V Grid
	for (int ix = 1; ix < (GridX_V - 1); ix++)
	{
	i_0 = ix + GridX_V;
	i_p_0 = ix + 2 * GridX_P;

	v_dxx = (v_n[i_0 + 1] - 2 * v_n[i_0] + v_n[i_0 - 1]) / (dx_h * dx_h);

	if (ix == 1) u_dxy = (-3 * (-3 * v_s[i_0] + 4 * v_s[i_0 + GridX_V] - v_s[i_0 + 2 * GridX_V])
	+ 4 * (-3 * v_s[i_0 + 1] + 4 * v_s[i_0 + GridX_V + 1] - v_s[i_0 + 2 * GridX_V + 1])
	- 1 * (-3 * v_s[i_0 + 2] + 4 * v_s[i_0 + GridX_V + 2] - v_s[i_0 + 2 * GridX_V + 2])) / (4 * dx_h * dy_h);
	else if (ix == (GridX_V - 2)) u_dxy = (3 * (-3 * v_s[i_0] + 4 * v_s[i_0 + GridX_V] - v_s[i_0 + 2 * GridX_V])
	- 4 * (-3 * v_s[i_0 - 1] + 4 * v_s[i_0 + GridX_V - 1] - v_s[i_0 + 2 * GridX_V - 1])
	+ 1 * (-3 * v_s[i_0 - 2] + 4 * v_s[i_0 + GridX_V - 2] - v_s[i_0 + 2 * GridX_V - 2])) / (4 * dx_h * dy_h);
	else u_dxy = ((-3 * v_s[i_0 + 1] + 4 * v_s[i_0 + GridX_V + 1] - v_s[i_0 + 2 * GridX_V + 1])
	- (-3 * v_s[i_0 - 1] + 4 * v_s[i_0 + GridX_V - 1] - v_s[i_0 + 2 * GridX_V - 1])) / (4 * dx_h * dy_h);

	pv_b[i_0] = dx_h * (G_v[i_0] - gamma_0_invdt * v[i_0] - KINVIS * (u_dxy - v_dxx));
	p_b[i_p_0] = p_b[i_p_0] - pu_b[i_p_0];

	i_1 = ix + (GridY_V - 2) * GridX_V;
	i_p_1 = ix + (GridY_P - 3)* GridX_P;

	v_dxx = (v_n[i_1 + 1] - 2 * v_n[i_1] + v_n[i_1 - 1]) / (dx_h * dx_h);

	if (ix == 1) u_dxy = (-3 * (3 * v_s[i_1] - 4 * v_s[i_1 - GridX_V] + v_s[i_1 - 2 * GridX_V])
	+ 4 * (3 * v_s[i_1 + 1] - 4 * v_s[i_1 - GridX_V + 1] + v_s[i_1 - 2 * GridX_V + 1])
	- 1 * (3 * v_s[i_1 + 2] - 4 * v_s[i_1 - GridX_V + 2] + v_s[i_1 - 2 * GridX_V + 2])) / (4 * dx_h * dy_h);
	else if (ix == (GridX_V - 2)) u_dxy = (3 * (3 * v_s[i_1] - 4 * v_s[i_1 - GridX_V] + v_s[i_1 - 2 * GridX_V])
	- 4 * (3 * v_s[i_1 - 1] - 4 * v_s[i_1 - GridX_V - 1] + v_s[i_1 - 2 * GridX_V - 1])
	+ 1 * (3 * v_s[i_1 - 2] - 4 * v_s[i_1 - GridX_V - 2] + v_s[i_1 - 2 * GridX_V - 2])) / (4 * dx_h * dy_h);
	else u_dxy = ((3 * v_s[i_1 + 1] - 4 * v_s[i_1 - GridX_V + 1] + v_s[i_1 - 2 * GridX_V + 1])
	- (3 * v_s[i_1 - 1] - 4 * v_s[i_1 - GridX_V - 1] + v_s[i_1 - 2 * GridX_V - 1])) / (4 * dx_h * dy_h);

	pv_b[i_1] = dx_h * (G_v[i_1] - gamma_0_invdt * v[i_1] - KINVIS * (u_dxy - v_dxx));
	p_b[i_p_1] = p_b[i_p_1] + pu_b[i_1];

	cout << pv_b[i_1]<<"  "<< pv_b[i_0]<<endl;
	}

	*/

}

void Set_PBC(double *p, double *pu_b, double *pv_b, int GridX_P, int GridY_P, int GridX_U, int GridY_U, int GridX_V, int GridY_V) {

	int i_0 = 0, i_1 = 0, i_0_u = 0, i_1_u = 0, i_0_v = 0, i_1_v = 0;
	int ix = 0, iy = 0;

	for (iy = 2; iy < (GridY_P - 2); iy++)
	{
		i_0 = iy * GridX_P, i_0_u = iy * GridX_U;
		i_1 = GridX_P - 1 + iy * GridX_P, i_1_u = GridX_U - 1 + iy * GridX_U;

		p[i_0] = p[i_0 + 1] - pu_b[i_0_u];
		p[i_1] = p[i_1 - 1] + pu_b[i_1_u];
	}

	for (ix = 1; ix < (GridX_P - 1); ix++)
	{
		i_0 = ix + 2 * GridX_P, i_0_v = ix + 2 * GridX_V;
		i_1 = ix + (GridY_P - 1) * GridX_P, i_1_v = ix + (GridY_V - 1)* GridX_V;

		p[i_0] = p[i_0 + GridX_P] - pu_b[i_0_u];
		p[i_1] = p[i_1 - GridX_P] + pu_b[i_1_u];
	}

	//set 4 corner point use dp/dx=0, dp/dy=0
	p[GridX_P] = p[GridX_P + 1];
	p[GridX_P - 1] = p[GridX_P - 2];
	p[(GridY_P - 2) * GridX_P] = p[(GridY_P - 2) * GridX_P + 1];
	p[(GridY_P - 1) * GridX_P - 1] = p[(GridY_P - 1) * GridX_P - 2];

}

void Display_error_P(double *p_c, double *p, double *p_real, int GridX_Central, int GridY_Central, int GridX_P, int GridY_P) {

	for (int iy = 1; iy < (GridY_Central - 1); iy++)
	{
		for (int ix = 0; ix < GridX_Central; ix++)
		{

			int i_c = ix + iy * GridX_Central;
			int i_p = ix + iy * GridX_P;

			p_c[i_c] = 0.25 * (p[i_p] + p[i_p + 1] + p[i_p + GridX_P] + p[i_p + GridX_P + 1]);
		}
	}

	for (int iy = 1; iy < (GridY_Central - 1); iy++)
	{
		for (int ix = 0; ix < GridX_Central; ix++)
		{

			int i_c = ix + iy * GridX_Central;
			cout << p_c[i_c] << ")_??_(" << p_real[i_c] << "    ";
		}
		cout << endl;
	}
}

void printf_vec(double *vec, int GridX, int GridY) {

	cout << "Vec_Start" << endl;

	for (int iy = 0; iy < GridY; iy++)
	{
		for (int ix = 0; ix < GridX; ix++)
		{
			int i = ix + iy * GridX;

			cout << vec[i] << "  ";
		}
		cout << endl;
	}
	cout << "Vec_End" << endl;
}

int main()
{
	int N = 32, N_halo = 2;

	double dx_h = 2.0 / N, dy_h = dx_h;

	int GridX_Central = N + 1, GridY_Central = N + N_halo + 1;

	int N_elem_Central = GridX_Central * GridY_Central;

	int N_Mem_tot_Central = ((N_elem_Central - 1) / 32 + 1) * 32;

	double *u_c = new double[N_Mem_tot_Central]; //store the result on Domain Grid
	double *v_c = new double[N_Mem_tot_Central];
	double *p_c = new double[N_Mem_tot_Central];

	memset(u_c, 0, N_Mem_tot_Central * sizeof(double));
	memset(v_c, 0, N_Mem_tot_Central * sizeof(double));
	memset(p_c, 0, N_Mem_tot_Central * sizeof(double));

	int GridX_U = GridX_Central, GridY_U = GridY_Central + 1;
	int GridX_V = GridX_Central + 1, GridY_V = GridY_Central;
	int GridX_P = GridX_Central + 1, GridY_P = GridY_Central + 1;

	int N_elem_U = GridX_U * GridY_U;
	int N_elem_V = GridX_V * GridY_V;
	int N_elem_P = GridX_P * GridY_P;

	int N_Mem_tot_U = ((N_elem_U - 1) / 32 + 1) * 32;
	int N_Mem_tot_V = ((N_elem_V - 1) / 32 + 1) * 32;
	int N_Mem_tot_P = ((N_elem_P - 1) / 32 + 1) * 32;

	double *u = new double[N_Mem_tot_U]; //store the result on t_n+1
	double *u_n = new double[N_Mem_tot_U]; //store the u* on t-n
	double *u_s = new double[N_Mem_tot_U];
	double *u_0 = new double[N_Mem_tot_U];
	double *u_b = new double[N_Mem_tot_U];
	double *u_hat = new double[N_Mem_tot_U];
	double *u_star = new double[N_Mem_tot_U];
	double *pu_b = new double[N_Mem_tot_U];

	memset(u, 0, N_Mem_tot_U * sizeof(double));
	memset(u_n, 0, N_Mem_tot_U * sizeof(double));
	memset(u_s, 0, N_Mem_tot_U * sizeof(double));
	memset(u_0, 0, N_Mem_tot_U * sizeof(double));
	memset(u_b, 0, N_Mem_tot_U * sizeof(double));
	memset(u_hat, 0, N_Mem_tot_U * sizeof(double));
	memset(u_star, 0, N_Mem_tot_U * sizeof(double));
	memset(pu_b, 0, N_Mem_tot_U * sizeof(double));

	double *v = new double[N_Mem_tot_V]; //store the result on t_n+1
	double *v_n = new double[N_Mem_tot_V]; //store the u on t-n
	double *v_s = new double[N_Mem_tot_V];
	double *v_0 = new double[N_Mem_tot_V];
	double *v_b = new double[N_Mem_tot_V];
	double *v_hat = new double[N_Mem_tot_V];
	double *v_star = new double[N_Mem_tot_V];
	double *pv_b = new double[N_Mem_tot_V];

	memset(v, 0, N_Mem_tot_V * sizeof(double));
	memset(v_n, 0, N_Mem_tot_V * sizeof(double));
	memset(v_s, 0, N_Mem_tot_V * sizeof(double));
	memset(v_0, 0, N_Mem_tot_V * sizeof(double));
	memset(v_b, 0, N_Mem_tot_V * sizeof(double));
	memset(v_hat, 0, N_Mem_tot_V * sizeof(double));
	memset(v_star, 0, N_Mem_tot_V * sizeof(double));
	memset(pv_b, 0, N_Mem_tot_V * sizeof(double));

	double *p = new double[N_Mem_tot_P];
	double *p_0 = new double[N_Mem_tot_P];
	double *p_s = new double[N_Mem_tot_P];
	double *p_b = new double[N_Mem_tot_P];

	memset(p, 0, N_Mem_tot_P * sizeof(double));
	memset(p_0, 0, N_Mem_tot_P * sizeof(double));
	memset(p_s, 0, N_Mem_tot_P * sizeof(double));
	memset(p_b, 0, N_Mem_tot_P * sizeof(double));

	double *igrid_x_U = new double[N_Mem_tot_U]; //store the grid-setting
	double *igrid_y_U = new double[N_Mem_tot_U];

	double *igrid_x_V = new double[N_Mem_tot_V]; //store the grid-setting
	double *igrid_y_V = new double[N_Mem_tot_V];

	double *igrid_x_P = new double[N_Mem_tot_P]; //store the grid-setting for scaler vector
	double *igrid_y_P = new double[N_Mem_tot_P];

	memset(igrid_x_U, 0, N_Mem_tot_U * sizeof(double));
	memset(igrid_y_U, 0, N_Mem_tot_U * sizeof(double));
	memset(igrid_x_V, 0, N_Mem_tot_V * sizeof(double));
	memset(igrid_y_V, 0, N_Mem_tot_V * sizeof(double));
	memset(igrid_x_P, 0, N_Mem_tot_P * sizeof(double));
	memset(igrid_y_P, 0, N_Mem_tot_P * sizeof(double));

	double *aw_U = new double[N_Mem_tot_U];
	double *ae_U = new double[N_Mem_tot_U];
	double *an_U = new double[N_Mem_tot_U];
	double *as_U = new double[N_Mem_tot_U];
	double *ap_U = new double[N_Mem_tot_U];

	memset(aw_U, 0, N_Mem_tot_U * sizeof(double));
	memset(ae_U, 0, N_Mem_tot_U * sizeof(double));
	memset(an_U, 0, N_Mem_tot_U * sizeof(double));
	memset(as_U, 0, N_Mem_tot_U * sizeof(double));
	memset(ap_U, 0, N_Mem_tot_U * sizeof(double));

	double *aw_V = new double[N_Mem_tot_V];
	double *ae_V = new double[N_Mem_tot_V];
	double *an_V = new double[N_Mem_tot_V];
	double *as_V = new double[N_Mem_tot_V];
	double *ap_V = new double[N_Mem_tot_V];

	memset(aw_V, 0, N_Mem_tot_V * sizeof(double));
	memset(ae_V, 0, N_Mem_tot_V * sizeof(double));
	memset(an_V, 0, N_Mem_tot_V * sizeof(double));
	memset(as_V, 0, N_Mem_tot_V * sizeof(double));
	memset(ap_V, 0, N_Mem_tot_V * sizeof(double));

	double *aw_P = new double[N_Mem_tot_P];
	double *ae_P = new double[N_Mem_tot_P];
	double *an_P = new double[N_Mem_tot_P];
	double *as_P = new double[N_Mem_tot_P];
	double *ap_P = new double[N_Mem_tot_P];

	memset(aw_P, 0, N_Mem_tot_P * sizeof(double));
	memset(ae_P, 0, N_Mem_tot_P * sizeof(double));
	memset(an_P, 0, N_Mem_tot_P * sizeof(double));
	memset(as_P, 0, N_Mem_tot_P * sizeof(double));
	memset(ap_P, 0, N_Mem_tot_P * sizeof(double));

	double *F_u = new double[N_Mem_tot_U]; //store the Source-term for u, v, p
	double *F_v = new double[N_Mem_tot_V];

	memset(F_u, 0, N_Mem_tot_U * sizeof(double));
	memset(F_v, 0, N_Mem_tot_V * sizeof(double));

	double *G_u = new double[N_Mem_tot_U];
	double *G_v = new double[N_Mem_tot_V];

	memset(G_u, 0, N_Mem_tot_U * sizeof(double));
	memset(G_v, 0, N_Mem_tot_V * sizeof(double));

	double *G_u_real = new double[N_Mem_tot_U];
	double *G_v_real = new double[N_Mem_tot_V];

	memset(G_u_real, 0, N_Mem_tot_U * sizeof(double));
	memset(G_v_real, 0, N_Mem_tot_V * sizeof(double));

	double *u_real = new double[N_Mem_tot_U]; //store the real value for u, v, p
	double *v_real = new double[N_Mem_tot_V];
	double *p_real = new double[N_Mem_tot_P];

	memset(u_real, 0, N_Mem_tot_U * sizeof(double));
	memset(v_real, 0, N_Mem_tot_V * sizeof(double));
	memset(p_real, 0, N_Mem_tot_P * sizeof(double));

	double *u_c_real = new double[N_Mem_tot_Central]; //store the real value for u, v, p
	double *v_c_real = new double[N_Mem_tot_Central];
	double *p_c_real = new double[N_Mem_tot_Central];

	memset(u_c_real, 0, N_Mem_tot_Central * sizeof(double));
	memset(v_c_real, 0, N_Mem_tot_Central * sizeof(double));
	memset(p_c_real, 0, N_Mem_tot_Central * sizeof(double));

	int iter_start_step = 0;

	//2020-0207-0951 备注开始
	//由于交错网格与原有网格不一致 故控制容积处有区分
	//Set_Grid_And_Constant_Equ_Params 所有参数暂时不用

	//prepost-setgrid and Equ const params
	Set_Grid_And_Constant_Equ_Params_P(aw_P, ae_P, an_P, as_P, ap_P,
		igrid_x_P, igrid_y_P, GridX_P, GridY_P, dx_h, dy_h);

	//Set_Grid_And_Constant_Equ_Params(aw_U, ae_U, an_U, as_U, ap_U,
	//igrid_x_U, igrid_y_U, GridX_U, GridY_U, dx_h, dy_h);

	//Set_Grid_And_Constant_Equ_Params(aw_V, ae_V, an_V, as_V, ap_V,
	//igrid_x_V, igrid_y_V, GridX_V, GridY_V, dx_h, dy_h);

	// ap_UVP may be needed to update during the time

	//2020-0207-0951 备注结束

	//Real-value Load
	Real_Load_Central(u_real, v_real, p_real, GridX_Central, GridY_Central, dx_h, dy_h, iter_start_step);
	Boundary_Load_on_Central(u_c, v_c, u_real, v_real, GridX_Central, GridY_Central);

	//Initializing vectors
	Init_U(u_n, u_c, GridX_U, GridY_U, GridX_Central, GridY_Central, dx_h, dy_h, iter_start_step);
	Init_V(v_n, v_c, GridX_V, GridY_V, GridX_Central, GridY_Central, dx_h, dy_h, iter_start_step);

	//time-iteration
	if (1)
	{
		for (int iter_t = 0; iter_t < N_t; iter_t++)
		{
			double gamma_0 = 1;
			double gamma_0_invdt = gamma_0 / dt;

			int i_t = iter_t + iter_start_step;

			cout << "-----------------------Next Step----------------------" << endl;
			cout << "TIME_STEP = " << (i_t + 1) << endl;

			/*--------------------------------------G__Update-----------------------------------------------*/
			Get_Source_U(F_u, GridX_U, GridY_U, dx_h, dy_h, i_t);
			Get_Source_V(F_v, GridX_V, GridY_V, dx_h, dy_h, i_t);

			Gu_Update(G_u, F_u, u_n, v_n, GridX_U, GridY_U, GridX_V, dx_h, dy_h);
			Gv_Update(G_v, F_v, u_n, v_n, GridX_U, GridX_V, GridY_V, dx_h, dy_h);

			//1st-test 验证 G_u, G_v
			//Get_G_real_without_F(u_c, v_c, u_real, v_real, GridX_Central, GridY_Central, dx_h, dy_h, i_t);

			//Display_errors(G_u, G_v, u_c, v_c, GridX_Central, GridY_Central, GridX_U, GridY_U, GridX_V, GridY_V);
			//Display_errors(G_u, G_v, u_c, v_c, GridX_Central, GridY_Central, GridX_U, GridX_V);

			/*--------------------------------------PossionEqu__Solve-----------------------------------------------*/
			cout << "P_solver iteration " << endl;
			//计算泊松方程的边界条件
			Real_Load_Central(u_real, v_real, p_real, GridX_Central, GridY_Central, dx_h, dy_h, i_t + 1);
			//Boundary_Load_on_Central(u_c, v_c, u_real, v_real, GridX_Central, GridY_Central);
			//Set_DBC_UV_for_PBC(u, v, u_c, v_c, GridX_U, GridY_U, GridX_V, GridY_V, GridX_Central, GridY_Central);

			//采用边界不插值方案
			Set_DBC_UV_for_PBC_without_interp(u, v, GridX_U, GridY_U, GridX_V, GridY_V, dx_h, dy_h, i_t + 1);

			Set_Us_for_PBC(u_s, v_s, u_n, v_n, GridX_U, GridY_U, GridX_V, GridY_V);
			//20200207-2301-ap_P-if U_DBC ap_P 不变了 / if U_OBC, ap_P update with time(因为动力粘度是变的)

			NSE_P_RHS(p_b, G_u, G_v, u_s, v_s, u_n, v_n, pu_b, pv_b, u, v, dx_h, dy_h,
				GridX_P, GridY_P, GridX_U, GridY_U, GridX_V, GridY_V, gamma_0_invdt, i_t);

			printf_vec(p_b, GridX_P, GridY_P);
			printf_vec(p_0, GridX_P, GridY_P);
			printf_vec(p_s, GridX_P, GridY_P);

			RB_GS_solver(p, p_0, p_s, p_b, aw_P, ae_P, an_P, as_P, ap_P, GridX_P, GridY_P, dx_h, dy_h);

			Set_PBC(p, pu_b, pv_b, GridX_P, GridY_P, GridX_U, GridY_U, GridX_V, GridY_V);

			Display_error_P(p_c, p, p_real, GridX_Central, GridY_Central, GridX_P, GridY_P);

		}
	}
	system("Pause");
	return 0;
}

