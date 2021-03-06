#include "ball.h"
// GMlib library
#include <gmOpenglModule>
#include <gmSceneModule>
#include <gmParametricsModule>
#include <parametrics/gmpsphere>
#include <parametrics/gmpplane>
// Qt library
#include <QTimerEvent>
#include <QRectF>
#include <QMouseEvent>
#include <QDebug>
// stl library
#include <stdexcept>
#include <thread>
#include <mutex>


/* Update the ball position by time varriable dt
   this is called from the localsimulate of ball and physicalobject class */

void Ball::updateStep(double dt)
{
    _Surf->estimateClpPar(this->getPos(),_u,_v);
    if(!_go)
    {


        //        std::cout<< "name-- " << this->getName()<< " pos-- " <<getPos()<<std::endl;
        //        std::cout<< "u " << _u<< " v " << _v <<std::endl;

        _go = true;
    }

    const GMlib::Vector<float,3> _g(0.0, 0.0, -9.81);

    _ds = dt*_velocity + (0.5*dt*dt)*_g;  //球在重力加速度作用下移动的距离

    _p = this->getPos() + _ds;            //球在重力加速度作用下到达的新位置（点），在曲面以下

    _Surf->getClosestPoint( _p, _u, _v );  //得到球在曲面上最近的点


    GMlib::DMatrix<GMlib::Vector<float,3> > _m = _Surf->evaluate(_u,_v,1,1); //evaluate方程是为了创建一个矩阵

    _normal = _m[0][1]^_m[1][0];        //得到垂直于曲面的向量
    _normal.normalize();
    _ds=_m[0][0] + this->getRadius()*_normal-this->getPos();  //移动的距离

    //******* for debug *****
    //    double vv=_ds.getLength();
    //    if(vv>5)
    //    {
    //     std::cout<< "Name " << getName()<< " step " << vv <<std::endl;
    //     std::cout<< "Normal " << _normal <<std::endl;
    //     std::cout<< "M " << _m[0][0] <<std::endl;
    //    }

    double checkV1 = _velocity*_velocity + 2.0*(_g*_ds);
//    calculate the velocity of a moving ball
    _velocity  += dt*_g;                        //得到一个因为重力加速度引起的速度向量
    _velocity  -= (_normal*_velocity)*_normal ; //得到跟曲面方向一致的速度向量

    double checkV2 = _velocity*_velocity;

    if(checkV2 > 0.0001)
    {
        if(checkV1 > 0.0001)
            _velocity *= std::sqrt(checkV1/checkV2);
    }

}
void Ball::moveUp()
{
    GMlib::Vector<float,3> newVelVect = this->getVelocity();
    if (newVelVect[1] < 8.0 && newVelVect[1] > -8.0)
    {
        if (newVelVect[1] < 0.0)
        {
            newVelVect[1] = 0.0;
        }

        newVelVect[1] += 1.0;
        newVelVect[0] *= 0.5;

        this->setVelocity(newVelVect);

    }
    else
    {
        while (newVelVect[1] >= 8.0 || newVelVect[1] <= -8.0)
        {
            newVelVect[1] *= 0.9;
            this->setVelocity(newVelVect);
        }
    }
}
void Ball::moveDown()
{
    GMlib::Vector<float,3> newVelVect = this->getVelocity();
    if (newVelVect < 8.0 && newVelVect > -8.0)
    {
        if (newVelVect[1] > 0.0)
        {
            newVelVect[1] = 0.0;
        }

        newVelVect[1] -= 1.0;
        newVelVect[0] *= 0.5;
        //newVelVect[2] *= 0.5;

        this->setVelocity(newVelVect);
    }
    else
    {
        while (newVelVect[1] >= 8.0 || newVelVect[1] <= -8.0)
        {
            newVelVect[1] *= 0.9;
            this->setVelocity(newVelVect);
        }
    }
}
void Ball::moveRight()
{
    GMlib::Vector<float,3> newVelVect = this->getVelocity();
    if (newVelVect[0] < 8.0 && newVelVect[0] > -8.0)
    {
        if (newVelVect[0] < 0.0)
        {
            newVelVect[0] = 0.0;
        }

        newVelVect[0] += 1.0;
        newVelVect[1] *= 0.5;

        this->setVelocity(newVelVect);
    }
    else
    {
        while (newVelVect[0] >= 8.0 || newVelVect[0] <= -8.0)
        {
            newVelVect[0] *= 0.9;
            this->setVelocity(newVelVect);
        }
    }
}
void Ball::moveLeft()
{
    GMlib::Vector<float,3> newVelVect = this->getVelocity();
    if (newVelVect[0] < 8.0 && newVelVect[0] > -8.0)
    {
        if (newVelVect[0] > 0.0)
        {
            newVelVect[0] = 0.0;
        }

        newVelVect[0] -= 1.0;
        newVelVect[1] *= 0.5;
        //newVelVect[2] *= 0.5;

        this->setVelocity(newVelVect);
    }
    else
    {
        while (newVelVect[0] >= 8.0 || newVelVect[0] <= -8.0)
        {
            newVelVect[0] *= 0.9;
            this->setVelocity(newVelVect);
        }
    }
}



void Ball::localSimulate(double dt)   //球跟底面的关系
{
    rotateGlobal(_ds.getLength(),this->getSurfNormal()^_ds);   //调用球的转动方程
    translateGlobal(_ds);
    // std::cout<< "Name " << getName()<< " Pos " << this->getPos() <<std::endl;


}

Ball::~Ball()
{

}

GMlib::Vector<float,3> Ball::getds()
{
    return _ds;
}

GMlib::Vector<float,3> Ball::getSurfNormal()
{
    _Surf->getClosestPoint(this->getPos(),_u,_v);
    GMlib::DMatrix<GMlib::Vector<float,3> > matrix=_Surf->evaluate(_u,_v,1,1);
    GMlib::UnitVector<float,3> normal=matrix[0][1]^matrix[1][0];
    return normal;

}


/*  Set the velocity of the ball */

void Ball::setVelocity(GMlib::Vector<float, 3> newVelocity)
{
    _velocity = newVelocity;
}


/* Get the velocity of the ball */

GMlib::Vector<float,3> Ball::getVelocity()
{
    return _velocity;
}


float Ball::getMass()
{
    return _mass;
}


/* Get the x value */
double Ball::getX()
{
    return _x;
}


/* Set the x value */
void Ball::setX(double x)
{
    _x = x;
}



