/*
FORCES - Fast interior point code generation for multistage problems.
Copyright (C) 2011-14 Alexander Domahidi [domahidi@control.ee.ethz.ch],
Automatic Control Laboratory, ETH Zurich.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __pr2MPC_H__
#define __pr2MPC_H__


/* DATA TYPE ------------------------------------------------------------*/
typedef double pr2MPC_FLOAT;


/* SOLVER SETTINGS ------------------------------------------------------*/
/* print level */
#ifndef pr2MPC_SET_PRINTLEVEL
#define pr2MPC_SET_PRINTLEVEL    (0)
#endif

/* timing */
#ifndef pr2MPC_SET_TIMING
#define pr2MPC_SET_TIMING    (0)
#endif

/* Numeric Warnings */
/* #define PRINTNUMERICALWARNINGS */

/* maximum number of iterations  */
#define pr2MPC_SET_MAXIT         (100)	

/* scaling factor of line search (affine direction) */
#define pr2MPC_SET_LS_SCALE_AFF  (0.9)      

/* scaling factor of line search (combined direction) */
#define pr2MPC_SET_LS_SCALE      (0.95)  

/* minimum required step size in each iteration */
#define pr2MPC_SET_LS_MINSTEP    (1E-08)

/* maximum step size (combined direction) */
#define pr2MPC_SET_LS_MAXSTEP    (0.995)

/* desired relative duality gap */
#define pr2MPC_SET_ACC_RDGAP     (0.0001)

/* desired maximum residual on equality constraints */
#define pr2MPC_SET_ACC_RESEQ     (1E-06)

/* desired maximum residual on inequality constraints */
#define pr2MPC_SET_ACC_RESINEQ   (1E-06)

/* desired maximum violation of complementarity */
#define pr2MPC_SET_ACC_KKTCOMPL  (1E-06)


/* RETURN CODES----------------------------------------------------------*/
/* solver has converged within desired accuracy */
#define pr2MPC_OPTIMAL      (1)

/* maximum number of iterations has been reached */
#define pr2MPC_MAXITREACHED (0)

/* no progress in line search possible */
#define pr2MPC_NOPROGRESS   (-7)




/* PARAMETERS -----------------------------------------------------------*/
/* fill this with data before calling the solver! */
typedef struct pr2MPC_params
{
    /* diagonal matrix of size [14 x 14] (only the diagonal is stored) */
    pr2MPC_FLOAT H1[14];

    /* vector of size 14 */
    pr2MPC_FLOAT f1[14];

    /* vector of size 14 */
    pr2MPC_FLOAT lb1[14];

    /* vector of size 14 */
    pr2MPC_FLOAT ub1[14];

    /* vector of size 7 */
    pr2MPC_FLOAT c1[7];

    /* diagonal matrix of size [14 x 14] (only the diagonal is stored) */
    pr2MPC_FLOAT H2[14];

    /* vector of size 14 */
    pr2MPC_FLOAT f2[14];

    /* vector of size 14 */
    pr2MPC_FLOAT lb2[14];

    /* vector of size 14 */
    pr2MPC_FLOAT ub2[14];

    /* diagonal matrix of size [14 x 14] (only the diagonal is stored) */
    pr2MPC_FLOAT H3[14];

    /* vector of size 14 */
    pr2MPC_FLOAT f3[14];

    /* vector of size 14 */
    pr2MPC_FLOAT lb3[14];

    /* vector of size 14 */
    pr2MPC_FLOAT ub3[14];

    /* diagonal matrix of size [14 x 14] (only the diagonal is stored) */
    pr2MPC_FLOAT H4[14];

    /* vector of size 14 */
    pr2MPC_FLOAT f4[14];

    /* vector of size 14 */
    pr2MPC_FLOAT lb4[14];

    /* vector of size 14 */
    pr2MPC_FLOAT ub4[14];

    /* diagonal matrix of size [7 x 7] (only the diagonal is stored) */
    pr2MPC_FLOAT H5[7];

    /* vector of size 7 */
    pr2MPC_FLOAT f5[7];

    /* vector of size 7 */
    pr2MPC_FLOAT lb5[7];

    /* vector of size 7 */
    pr2MPC_FLOAT ub5[7];

} pr2MPC_params;


/* OUTPUTS --------------------------------------------------------------*/
/* the desired variables are put here by the solver */
typedef struct pr2MPC_output
{
    /* vector of size 14 */
    pr2MPC_FLOAT z1[14];

    /* vector of size 14 */
    pr2MPC_FLOAT z2[14];

    /* vector of size 14 */
    pr2MPC_FLOAT z3[14];

    /* vector of size 14 */
    pr2MPC_FLOAT z4[14];

    /* vector of size 7 */
    pr2MPC_FLOAT z5[7];

} pr2MPC_output;


/* SOLVER INFO ----------------------------------------------------------*/
/* diagnostic data from last interior point step */
typedef struct pr2MPC_info
{
    /* iteration number */
    int it;
	
    /* inf-norm of equality constraint residuals */
    pr2MPC_FLOAT res_eq;
	
    /* inf-norm of inequality constraint residuals */
    pr2MPC_FLOAT res_ineq;

    /* primal objective */
    pr2MPC_FLOAT pobj;	
	
    /* dual objective */
    pr2MPC_FLOAT dobj;	

    /* duality gap := pobj - dobj */
    pr2MPC_FLOAT dgap;		
	
    /* relative duality gap := |dgap / pobj | */
    pr2MPC_FLOAT rdgap;		

    /* duality measure */
    pr2MPC_FLOAT mu;

	/* duality measure (after affine step) */
    pr2MPC_FLOAT mu_aff;
	
    /* centering parameter */
    pr2MPC_FLOAT sigma;
	
    /* number of backtracking line search steps (affine direction) */
    int lsit_aff;
    
    /* number of backtracking line search steps (combined direction) */
    int lsit_cc;
    
    /* step size (affine direction) */
    pr2MPC_FLOAT step_aff;
    
    /* step size (combined direction) */
    pr2MPC_FLOAT step_cc;    

	/* solvertime */
	pr2MPC_FLOAT solvetime;   

} pr2MPC_info;


/* SOLVER FUNCTION DEFINITION -------------------------------------------*/
/* examine exitflag before using the result! */
int pr2MPC_solve(pr2MPC_params* params, pr2MPC_output* output, pr2MPC_info* info);


#endif