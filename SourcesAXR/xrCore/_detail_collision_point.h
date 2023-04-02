#pragma once
#include "_vector3d.h"
#include "_types.h"
#include <cmath>

struct DetailCollisionPoint;

ENGINE_API extern xr_vector<DetailCollisionPoint> level_detailcoll_points;
ENGINE_API extern float ps_detail_collision_dist;
ENGINE_API extern float ps_detail_collision_time;

typedef xr_vector<DetailCollisionPoint>::iterator VecDetailCollIter;
typedef xr_vector<DetailCollisionPoint> VecDetailColl;


struct DetailCollisionPoint
{
public:
    Fvector pos;
    u16 id;
    float radius = ps_detail_collision_dist;
    float rot_time_in = ps_detail_collision_time;
    float rot_time_out = ps_detail_collision_time * 1.5f; //-- сделать отдельно _in && _out

    bool is_explosion{};

    //-- это промежуточное для зоны взрыва
    bool bNoExplCollision{};
    float fExplCollisionTimeIn{};
    float fExplCollisionTimeOut{};

    //-- Время опускания вниз зоны взрыва, аномалии и тп
    float fExpPointLoweringTime{};

    DetailCollisionPoint() = default;
    DetailCollisionPoint(Fvector pos, u16 id, float radius = ps_detail_collision_dist, float rot_time_in = ps_detail_collision_time,
                         float rot_time_out = ps_detail_collision_time * 1.5f, bool is_explosion = false, float fExpPointLoweringTime = 1.5f)
    { 
        this->pos = pos;
        this->id = id;
        this->radius = radius;
        this->is_explosion = is_explosion;
        this->rot_time_in = rot_time_in;
        this->rot_time_out = rot_time_out;
        this->fExpPointLoweringTime = fExpPointLoweringTime;
    }
};

static DetailCollisionPoint* GetDetailCollisionPointById(const u16& ID)
{
    for (auto& point : level_detailcoll_points)
        if (point.id == ID)
            return &point;
    return nullptr;
}

static void EraseDetailCollPointIfExists(const u16& ID)
{
    VecDetailCollIter it = level_detailcoll_points.begin();
    for (; it != level_detailcoll_points.end(); ++it)
        if ((*it).id == ID)
        {
            level_detailcoll_points.erase(it);
            break;
        }
}

static bool DetailCollisonPointExist(const u16& ID)
{ 
    for (auto& i : level_detailcoll_points)
        if (i.id == ID)
            return true;
    return false;
}

/* Для поворота вокруг точки колизии на потом
static void GetDetailDirAndAngle(Fmatrix matrix, Fvector& vec, float& angle)
{
    matrix.i.normalize();
    matrix.j.normalize();
    matrix.k.normalize();
    angle = acos(((matrix._11 + matrix._22 + matrix._33) - 1.f) * 0.5f);
    float omegaPreCalc = 1.0f / (2.f * sin(angle));
    vec.x = omegaPreCalc * (matrix._23 - matrix._32);
    vec.y = omegaPreCalc * (matrix._31 - matrix._13);
    vec.z = omegaPreCalc * (matrix._12 - matrix._21);
}
static void GetDetailDir(Fmatrix matrix, Fvector& vec)
{
    matrix.i.normalize();
    matrix.j.normalize();
    matrix.k.normalize();
  
    float omegaPreCalc = 1.0f / (2.f * sin(acos(((matrix._11 + matrix._22 + matrix._33) - 1.f) * 0.5f)));
    vec.x = omegaPreCalc * (matrix._23 - matrix._32);
    vec.y = omegaPreCalc * (matrix._31 - matrix._13);
    vec.z = omegaPreCalc * (matrix._12 - matrix._21);
}*/