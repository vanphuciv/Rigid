﻿#include "rigid_body.h"

Rigid_Body::Rigid_Body(vec3 _position, vec3 _velocity,Quaternion _rotation,vec3 _angular_velocity,float _coefficient_of_restitution/*=0.5*/)
    : position(_position), velocity(_velocity), rotation(_rotation), angular_velocity(_angular_velocity), coefficient_of_restitution(_coefficient_of_restitution)
{
    //no-op
}

void Rigid_Body::Set_Cube_Inertia(float _mass,float _side)
{
    mass=_mass;
    float factor=10*mass*_side*_side/6.;
    inertia=(factor*mat33::identity2D());
    inertia_inverse=mat33::identity2D()/factor;
}

void Rigid_Body::Apply_Impulse(vec3 impulse, vec3 location)
{
    velocity+=impulse/mass;
    vec3 angular_impulse=(location-position)^impulse;
    //angular_velocity+=inertia_inverse*angular_impulse;

    mat33 world_space_inertia_inverse=World_Space_Inertia_Tensor_Inverse(rotation);
    angular_velocity+=world_space_inertia_inverse*angular_impulse;

    //std::cout<<"angular velocity "<<angular_velocity[0]<<" "<<angular_velocity[1]<<" "<<angular_velocity[2]<<std::endl;
}

void Rigid_Body::Apply_Gravity(float dt)
{
    velocity+=vec3(0,-9.8,0)*dt;
}

void Rigid_Body::Advance(float dt)
{
    position+=dt*velocity;
    Quaternion rotation1(angular_velocity,0);
    rotation=rotation+rotation1*rotation*dt*.5;
    rotation=rotation/rotation.norm();
    //std::cout<<"quat after "<<rotation.w()<<" "<<rotation.x()<<" "<<rotation.y()<<" "<<rotation.z()<<std::endl;
}

void Rigid_Body::Collide_Cube_Ground(float l)
{
    vec3 points[8];
    points[0]=vec3( -0.5*l, -0.5*l, -0.5*l );       
    points[1]=vec3( -0.5*l,  0.5*l, -0.5*l );       
    points[2]=vec3(  0.5*l,  0.5*l, -0.5*l );       
    points[3]=vec3(  0.5*l, -0.5*l, -0.5*l );       
    points[4]=vec3(  0.5*l, -0.5*l,  0.5*l );
    points[5]=vec3(  0.5*l,  0.5*l,  0.5*l );
    points[6]=vec3( -0.5*l,  0.5*l,  0.5*l );
    points[7]=vec3( -0.5*l, -0.5*l,  0.5*l );
    float max_depth=0;
    vec3 deepest_point;
    for(int i=0;i<8;i++){
        //std::cout<<"quat "<<rotation.w()<<" "<<rotation.x()<<" "<<rotation.y()<<" "<<rotation.z()<<std::endl;
        vec3 world_space_point=rotation.rotatedVector(points[i])+position;
        //std::cout<<"position point "<<i<<" "<<world_space_point[0]<<" "<<world_space_point[1]<<" "<<world_space_point[2]<<std::endl;
        if(world_space_point[1]<max_depth){ max_depth=world_space_point[1];
            deepest_point=world_space_point;	//Điểm sâu nhất	
        }
    }
    //std::cout<<"max depth "<<max_depth<<std::endl;
    if(max_depth<0){
		vec3 impulse;
        vec3 normal(0.,1.,0.);		//Pháp tuyến mặt phẳng
        vec3 r=deepest_point-position;	//Bán kính điểm sâu nhất và tâm khối lập phương	
        vec3 temp=angular_velocity^r;		//Mô men quay
        vec3 vp=velocity+temp;				//Tổng mô men hướng va chạm tiếp tuyến vận tốc

        vec3 vp_n=(vp*normal)*normal;			//Vector momen theo hướng pháp tuyến
        vec3 vp_t=vp-vp_n;						//Vector momen theo hướng tiếp tuyến
        vec3 tangent=vp_t;tangent.normalize();		//Hướng tiếp tuyến động đơn vị

        mat33 K=Impulse_Factor(deepest_point);
        mat33 K_inverse=K.inverse();

        //TODO: WRITE YOUR CODE HERE
        double coef_of_friction=0.2;//TODO: adjust this value as necessary
        vec3 vp_rel =  -vp_n*(1+coefficient_of_restitution); //TODO: incorporate tangential component of velocity
        vec3 impulse_n=K_inverse*vp_rel; //NOTE: this impulse is calculated without friction
		vec3 impulse_p = K_inverse*vp;
		vec3 impulse_duKien = impulse_n - K_inverse*vp;
		
		if (
			//impulse_n.length()*0.4>impulse_p.length()
			(impulse_duKien -(impulse_duKien*vp_n)*vp_n).length() <= 0.4*impulse_duKien*vp_n
			) {
			impulse = impulse_n;
		}
		else {
			impulse = impulse_n  - 0.4* impulse_p;
		}


        //END SECTION FOR YOUR CODE

        Apply_Impulse(impulse,deepest_point);
        position[1]-=max_depth;
    }
}


mat33 Rigid_Body::World_Space_Inertia_Tensor_Inverse(const Quaternion& orientation)
{
    mat33 object_to_world_rotation=orientation.rotationMatrix();
    return object_to_world_rotation * inertia_inverse * object_to_world_rotation.transpose();
}

mat33 Rigid_Body::Cross_Product_Matrix(const vec3& v)
{
    return mat33( 0.  , -v[2],  v[1],
                  v[2], 0.   , -v[0],
                 -v[1],  v[0], 0.);
}

mat33 Rigid_Body::Impulse_Factor(const vec3& location)
{
    mat33 cross_product_matrix=Cross_Product_Matrix(location-position);
    mat33 world_space_inertia_inverse=World_Space_Inertia_Tensor_Inverse(rotation);

    return cross_product_matrix*world_space_inertia_inverse*cross_product_matrix.transpose()+1./mass*mat33::identity2D();
}
