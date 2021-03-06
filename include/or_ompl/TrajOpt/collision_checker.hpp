#pragma once
#include <set>
#include <utility>
#include <iostream>
#include <openrave/openrave.h>
#include "configuration_space.hpp"

namespace trajopt {

enum CastCollisionType {
  CCType_None,
  CCType_Time0,
  CCType_Time1,
  CCType_Between
};

struct Collision {
  const OR::KinBody::Link* linkA;
  const OR::KinBody::Link* linkB;
  OR::Vector ptA, ptB, normalB2A; /* normal points from 2 to 1 */
  OR::Vector ptB1;
  double distance; /* pt1 = pt2 + normal*dist */
  float weight, time;
  CastCollisionType cctype;
  Collision(const OR::KinBody::Link* linkA, const OR::KinBody::Link* linkB, 
            const OR::Vector& ptA, const OR::Vector& ptB, 
            const OR::Vector& normalB2A, double distance, float weight=1, float time=0) :
    linkA(linkA), linkB(linkB), ptA(ptA), ptB(ptB), normalB2A(normalB2A), distance(distance), weight(weight), time(0), cctype(CCType_None) {}
};
 std::ostream& operator<<(std::ostream&, const Collision&);

enum CollisionFilterGroups {
  RobotFilter = 1,
  KinBodyFilter = 2,
};

/** 
 * Each CollisionChecker object has a copy of the world. For performance, don't make too many copies. */ 
class  CollisionChecker : public OR::UserData {
public:

  /** check everything vs everything else */
  virtual void AllVsAll(std::vector<Collision>& collisions)=0;
  /** check link vs everything else */
  virtual void LinkVsAll(const OR::KinBody::Link& link, std::vector<Collision>& collisions, short filterMask)=0;
  virtual void LinksVsAll(const std::vector<OR::KinBody::LinkPtr>& links, std::vector<Collision>& collisions, short filterMask)=0;

  /** check robot vs everything else. includes attached bodies */
  void BodyVsAll(const OR::KinBody& body, std::vector<Collision>& collisions, short filterMask=-1) {
    LinksVsAll(body.GetLinks(), collisions, filterMask);
  }
  /** contacts of distance < (arg) will be returned */
  virtual void SetContactDistance(float distance)  = 0;
  virtual double GetContactDistance() = 0;
  
  virtual void PlotCollisionGeometry(std::vector<OpenRAVE::GraphHandlePtr>&) {throw std::runtime_error("not implemented");}

  virtual void ContinuousCheckTrajectory(const DblMatrix& traj, Configuration& rad, std::vector<Collision>& collisions) {throw std::runtime_error("not implemented");}
  
  /** Find contacts between swept-out shapes of robot links and everything in the environment, as robot goes from startjoints to endjoints */ 
  virtual void CastVsAll(Configuration& rad, const std::vector<OR::KinBody::LinkPtr>& links, const DblVec& startjoints, const DblVec& endjoints, std::vector<Collision>& collisions) {throw std::runtime_error("not implemented");}

  /** Finds all self collisions when all joints are set to zero, and ignore collisions between the colliding links */
  void IgnoreZeroStateSelfCollisions();
  void IgnoreZeroStateSelfCollisions(OR::KinBodyPtr body);

  /** Prevent this pair of links from colliding */
  virtual void ExcludeCollisionPair(const OR::KinBody::Link& link0, const OR::KinBody::Link& link1) = 0;
  virtual void IncludeCollisionPair(const OR::KinBody::Link& link0, const OR::KinBody::Link& link1) = 0;

  /** Check whether a raycast hits the environment */
  virtual bool RayCastCollision(const OpenRAVE::Vector& point1, const OpenRAVE::Vector& point2) = 0;


  OpenRAVE::EnvironmentBaseConstPtr GetEnv() {return m_env;}

  virtual ~CollisionChecker() {}
  /** Get or create collision checker for this environment */
  static boost::shared_ptr<CollisionChecker> GetOrCreate(OR::EnvironmentBase& env);
protected:
  CollisionChecker(OpenRAVE::EnvironmentBaseConstPtr env) : m_env(env) {}
  OpenRAVE::EnvironmentBaseConstPtr m_env;
};
typedef boost::shared_ptr<CollisionChecker> CollisionCheckerPtr;

CollisionCheckerPtr  CreateCollisionChecker(OR::EnvironmentBaseConstPtr env);

 void PlotCollisions(const std::vector<Collision>& collisions, OR::EnvironmentBase& env, std::vector<OR::GraphHandlePtr>& handles, double safe_dist);

} // namespace trajopt

