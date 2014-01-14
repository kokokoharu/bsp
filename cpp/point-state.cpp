#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <iomanip>

//#include "callisto.h"
#include "matrix.h"
#include "utils.h"

#include <timer/Timer.h>

extern "C" {
#include "symeval.h"
#include "stateMPC.h"
}

#define DT 1.0
#define X_DIM 2
#define U_DIM 2
#define Z_DIM 2
#define Q_DIM 2
#define R_DIM 2

#define S_DIM (((X_DIM+1)*X_DIM)/2)
#define B_DIM (X_DIM+S_DIM)

Matrix<X_DIM> x0;
Matrix<X_DIM,X_DIM> Sigma0;
Matrix<X_DIM> xGoal;
Matrix<X_DIM> xMin, xMax;
Matrix<U_DIM> uMin, uMax;

const int T = 15;
const double INFTY = 1e10;
const double alpha_belief = 10, alpha_final_belief = 10, alpha_control = 1;

// stateMPC vars
stateMPC_FLOAT **H, **f, **lb, **ub, **C, **e, **z;

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

double *inputVars;
std::vector<int> maskIndices;

#ifdef WIN32
// Callisto variables
int cal_env, cal_goal, cal_domain, cal_traj;
#endif

inline Matrix<X_DIM> dynfunc(const Matrix<X_DIM>& x, const Matrix<U_DIM>& u, const Matrix<U_DIM>& q)
{  
	Matrix<X_DIM> xNew = x + u*DT + 0.01*q;
	return xNew;
}

// Observation model
inline Matrix<Z_DIM> obsfunc(const Matrix<X_DIM>& x, const Matrix<R_DIM>& r)
{
	double intensity = sqrt(sqr(0.5*x[0]) + 1e-6);
	Matrix<Z_DIM> z = x + intensity*r;
	return z;
}

// TODO: Find better way to do this using macro expansions?
void setupStateMPCVars(stateMPC_params& problem, stateMPC_output& output)
{
	// inputs
	H = new stateMPC_FLOAT*[T];
	f = new stateMPC_FLOAT*[T];
	lb = new stateMPC_FLOAT*[T];
	ub = new stateMPC_FLOAT*[T];
	C = new stateMPC_FLOAT*[T-1];
	e = new stateMPC_FLOAT*[T];

	// output
	z = new stateMPC_FLOAT*[T];

	H[0] = problem.H01; f[0] = problem.f01; lb[0] = problem.lb01; ub[0] = problem.ub01; C[0] = problem.C01; e[0] = problem.e01;
	H[1] = problem.H02; f[1] = problem.f02; lb[1] = problem.lb02; ub[1] = problem.ub02; C[1] = problem.C02; e[1] = problem.e02;
	H[2] = problem.H03; f[2] = problem.f03; lb[2] = problem.lb03; ub[2] = problem.ub03; C[2] = problem.C03; e[2] = problem.e03;
	H[3] = problem.H04; f[3] = problem.f04; lb[3] = problem.lb04; ub[3] = problem.ub04; C[3] = problem.C04; e[3] = problem.e04;
	H[4] = problem.H05; f[4] = problem.f05; lb[4] = problem.lb05; ub[4] = problem.ub05; C[4] = problem.C05; e[4] = problem.e05;
	H[5] = problem.H06; f[5] = problem.f06; lb[5] = problem.lb06; ub[5] = problem.ub06; C[5] = problem.C06; e[5] = problem.e06;
	H[6] = problem.H07; f[6] = problem.f07; lb[6] = problem.lb07; ub[6] = problem.ub07; C[6] = problem.C07; e[6] = problem.e07;
	H[7] = problem.H08; f[7] = problem.f08; lb[7] = problem.lb08; ub[7] = problem.ub08; C[7] = problem.C08; e[7] = problem.e08;
	H[8] = problem.H09; f[8] = problem.f09; lb[8] = problem.lb09; ub[8] = problem.ub09; C[8] = problem.C09; e[8] = problem.e09;
	H[9] = problem.H10; f[9] = problem.f10; lb[9] = problem.lb10; ub[9] = problem.ub10; C[9] = problem.C10; e[9] = problem.e10;
	H[10] = problem.H11; f[10] = problem.f11; lb[10] = problem.lb11; ub[10] = problem.ub11; C[10] = problem.C11; e[10] = problem.e11;
	H[11] = problem.H12; f[11] = problem.f12; lb[11] = problem.lb12; ub[11] = problem.ub12; C[11] = problem.C12; e[11] = problem.e12;
	H[12] = problem.H13; f[12] = problem.f13; lb[12] = problem.lb13; ub[12] = problem.ub13; C[12] = problem.C13; e[12] = problem.e13;
	H[13] = problem.H14; f[13] = problem.f14; lb[13] = problem.lb14; ub[13] = problem.ub14; C[13] = problem.C14; e[13] = problem.e14;
	H[14] = problem.H15; f[14] = problem.f15; lb[14] = problem.lb15; ub[14] = problem.ub15;                      e[14] = problem.e15;

	z[0] = output.z1; z[1] = output.z2; z[2] = output.z3; z[3] = output.z4; z[4] = output.z5;
	z[5] = output.z6; z[6] = output.z7; z[7] = output.z8; z[8] = output.z9; z[9] = output.z10; 
	z[10] = output.z11; z[11] = output.z12; z[12] = output.z13; z[13] = output.z14; z[14] = output.z15; 
}

void setupDstarInterface() 
{
	// instantiations
	// alpha_belief, alpha_control, alpha_final_belief
	int nparams = 3;
	// T*X_DIM + (T-1)*U_DIM + zeros for Q_DIM,R_DIM + Sigma0 (X_DIM*X_DIM) + nparams
	int nvars = T * X_DIM + (T - 1) * U_DIM + Q_DIM + R_DIM + (X_DIM * X_DIM) + nparams;

	inputVars = new double[nvars];
	
	std::ifstream fptr("masks.txt");
	int val;
	for(int i = 0; i < nvars; ++i) {
		fptr >> val;
		if (val == 1) {
			maskIndices.push_back(i);
		}
	}
	// Read in Jacobian and Hessian masks here
	fptr.close();
}

void cleanup()
{
	delete[] inputVars;

	delete[] H;
	delete[] f;
	delete[] lb; 
	delete[] ub; 
	delete[] C;
	delete[] e;
	delete[] z;
}

void computeCostGradDiagHess(const std::vector< Matrix<X_DIM> >& X, const std::vector< Matrix<U_DIM> >& U, double* result)
{
	int idx = 0;	
	for (int t = 0; t < T; ++t) {
		for (int i = 0; i < X_DIM; ++i) {
			inputVars[idx++] = X[t][i];
		}
	}
	for (int t = 0; t < (T - 1); ++t) {
		for (int i = 0; i < U_DIM; ++i) {
			inputVars[idx++] = U[t][i];
		}
	}
	for (int i = 0; i < (Q_DIM+R_DIM); ++i) {
		inputVars[idx++] = 0;
	}
	for (int i = 0; i < (X_DIM+X_DIM); ++i) {
		inputVars[idx++] = Sigma0[i];
	}
	inputVars[idx++] = alpha_belief; inputVars[idx++] = alpha_control; inputVars[idx++] = alpha_final_belief;
	
	int nvars = (int)maskIndices.size();
	double* vars = new double[nvars];

	// For evaluation
	idx = 0;
	for (int i = 0; i < nvars; ++i) {
		vars[idx++] = inputVars[maskIndices[i]];
	}
	evalCostGradDiagHess(result, vars);
}

void stateCollocation(std::vector< Matrix<X_DIM> >& X, std::vector< Matrix<U_DIM> >& U, stateMPC_params& problem, stateMPC_output& output, stateMPC_info& info)
{
	int maxIter = 10;
	double Xeps = 1;
	double Ueps = 1;

	// box constraint around goal
	double delta = 0.01;

	Matrix<X_DIM,1> x0 = X[0];

	double prevcost, optcost;

	int dim = T*X_DIM + (T-1)*U_DIM;
	double* result = new double[2*dim + 1];

	double Hzbar[4];

	//std::cout << "Initialization trajectory cost: " << std::setprecision(10) << prevcost << std::endl;

	for(int it = 0; it < maxIter; ++it)
	{
		//std::cout << "Iter: " << it << std::endl;
		computeCostGradDiagHess(X, U, result);

		// linearize belief dynamics constraint here
		for (int t = 0; t < T-1; ++t)
		{
			Matrix<X_DIM>& xt = X[t];
			Matrix<U_DIM>& ut = U[t];

			Matrix<X_DIM+U_DIM> zbar;
			zbar.insert(0,0,xt);
			zbar.insert(X_DIM,0,ut);

			H[t][0] = result[1+dim+t*X_DIM];
			H[t][1] = result[1+dim+t*X_DIM+1];
			H[t][2] = result[1+dim+T*X_DIM+t*U_DIM];
			H[t][3] = result[1+dim+T*X_DIM+t*U_DIM+1];

			for(int i = 0; i < (X_DIM+U_DIM); ++i) {
				Hzbar[i] = H[t][i]*zbar[i];
			}

			f[t][0] = result[1+t*X_DIM] - Hzbar[0];
			f[t][1] = result[1+t*X_DIM+1] - Hzbar[1];
			f[t][2] = result[1+T*X_DIM+t*U_DIM] - Hzbar[2];
			f[t][3] = result[1+T*X_DIM+t*U_DIM+1] - Hzbar[3];



			// Fill in lb, ub, C, e
			lb[t][0] = MAX(xMin[0], xt[0] - Xeps);
			lb[t][1] = MAX(xMin[1], xt[1] - Xeps);
			lb[t][2] = MAX(uMin[0], ut[0] - Ueps);
			lb[t][3] = MAX(uMin[1], ut[1] - Ueps);

			ub[t][0] = MIN(xMax[0], xt[0] + Xeps);
			ub[t][1] = MIN(xMax[1], xt[1] + Xeps);
			ub[t][2] = MIN(uMax[0], ut[0] + Ueps);
			ub[t][3] = MIN(uMax[1], ut[1] + Ueps);

			Matrix<X_DIM,X_DIM+U_DIM> CMat;
			Matrix<X_DIM> eVec;

			CMat.insert<X_DIM,X_DIM>(0,0,identity<X_DIM>());
			CMat.insert<X_DIM,U_DIM>(0,X_DIM,DT*identity<U_DIM>());
			int idx = 0;
			for(int c = 0; c < (X_DIM+U_DIM); ++c) {
				for(int r = 0; r < X_DIM; ++r) {
					C[t][idx++] = CMat[c + r*(X_DIM+U_DIM)];
				}
			}

			if (t == 0) {
				e[t][0] = x0[0]; e[t][1] = x0[1];
			} else {
				e[t][0] = 0; e[t][1] = 0;
			}
		} //setting up problem

		Matrix<X_DIM>& xT = X[T-1];

		H[T-1][0] = result[1+dim+(T-1)*X_DIM];
		H[T-1][1] = result[1+dim+(T-1)*X_DIM+1];

		for(int i = 0; i < X_DIM; ++i) {
			Hzbar[i] = H[T-1][i]*xT[i];
		}

		f[T-1][0] = result[1+(T-1)*X_DIM] - Hzbar[0];
		f[T-1][1] = result[1+(T-1)*X_DIM+1] - Hzbar[1];

		// Fill in lb, ub, C, e
		lb[T-1][0] = MAX(xGoal[0] - delta, xT[0] - Xeps);
		lb[T-1][1] = MAX(xGoal[1] - delta, xT[1] - Xeps);

		ub[T-1][0] = MIN(xGoal[0] + delta, xT[0] + Xeps);
		ub[T-1][1] = MIN(xGoal[1] + delta, xT[1] + Xeps);

		e[T-1][0] = 0; e[T-1][1] = 0;

		// Verify problem inputs

		//int num;
		//std::cin >> num;

		int exitflag = stateMPC_solve(&problem, &output, &info);
		if (exitflag == 1) {
			for(int t = 0; t < T-1; ++t) {
				Matrix<X_DIM>& xt = X[t];
				Matrix<U_DIM>& ut = U[t];

				for(int i = 0; i < X_DIM; ++i) {
					xt[i] = z[t][i];
				}
				for(int i = 0; i < U_DIM; ++i) {
					ut[i] = z[t][X_DIM+i];
				}
				optcost = info.pobj;
			}
			Matrix<X_DIM>& xt = X[T-1];
			xt[0] = z[T-1][0]; xt[1] = z[T-1][1];
		}
		else {
			std::cerr << "Some problem in solver" << std::endl;
			std::exit(-1);
		}
		//std::cout << "Optimized cost: " << optcost << std::endl;

		if ((optcost > prevcost) | (abs(optcost - prevcost)/prevcost < 0.01))
			break;
		else {
			prevcost = optcost;
			// TODO: integrate trajectory?
			// TODO: plot trajectory
		}

		//int num;
		//std::cin >> num;
	}
	delete[] result;
}

int main(int argc, char* argv[])
{
	x0[0] = -3.5; x0[1] = 2;
	Sigma0 = identity<X_DIM>();
	xGoal[0] = -3.5; xGoal[1] = -2;

	xMin[0] = -5; xMin[1] = -3; 
	xMax[0] = 5; xMax[1] = 3;
	uMin[0] = -1; uMin[1] = -1;
	uMax[0] = 1; uMax[1] = 1;

	Matrix<U_DIM> uinit;
	uinit[0] = (xGoal[0] - x0[0]) / (T-1);
	uinit[1] = (xGoal[1] - x0[1]) / (T-1);

	std::vector<Matrix<U_DIM> > U(T-1, uinit); 
	std::vector<Matrix<X_DIM> > X(T);

	X[0] = x0;
	for (int t = 0; t < T-1; ++t) {
		X[t+1] = dynfunc(X[t], U[t], zeros<Q_DIM,1>());
		//std::cout << ~X[t+1] << std::endl;
	}

	setupDstarInterface();
	
	stateMPC_params problem;
	stateMPC_output output;
	stateMPC_info info;

	setupStateMPCVars(problem, output);

	Timer t;
	t.start();

	// compute cost for the trajectory
	stateCollocation(X, U, problem, output, info);

	t.stop();
	//std::cout << "Cost: " << std::setprecision(10) << cost << std::endl;
	std::cout << "Compute time: " << t.getElapsedTimeInMilliSec() << " mS" << std::endl;

	cleanup();

	int k;
	std::cin >> k;

	//CAL_End();
	return 0;
}
