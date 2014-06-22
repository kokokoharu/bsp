//#ifndef __CAMERA_H__
//#define __CAMERA_H__
//
//#include <Eigen/Eigen>
//using namespace Eigen;
//
//#include <openrave-core.h>
//namespace rave = OpenRAVE;
//
//#include "geometry3d.h"
//#include "rave-utils.h"
//#include "../../util/Timer.h"
//
//#define HEIGHT 480
//#define WIDTH 640
//
//#define H_SUB 48
//#define W_SUB 64
//#define N (H_SUB*W_SUB)
//
//class Camera {
//public:
//	Camera(rave::RobotBasePtr r, rave::SensorBasePtr s, double mr);
//
//	Matrix<double,N,3> get_directions();
//	std::vector<std::vector<Beam3d> > get_beams();
//	std::vector<Triangle3d> get_border(const std::vector<std::vector<Beam3d> >& beams);
//	double signed_distance(const Vector3d& p, std::vector<std::vector<Beam3d> >& beams, std::vector<Triangle3d>& border);
//
//private:
//	rave::RobotBasePtr robot;
//	rave::SensorBasePtr sensor;
//
//	int height, width;
//	double f, F, max_range;
//	double H, W;
//
//};
//
//#endif
