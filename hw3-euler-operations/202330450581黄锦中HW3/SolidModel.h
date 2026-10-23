#ifndef _SOLID_MODEL_H_
#define _SOLID_MODEL_H_

#include <vector>
#include <algorithm>

struct Point;
struct Vertex;
struct Halfedge;
struct Edge;
struct Loop;
struct Face;
struct Body;

typedef struct Point
{
    Point(double _x, double _y, double _z)
    {
        coordinate_[0] = _x;
        coordinate_[1] = _y;
        coordinate_[2] = _z;
    }
    Point(const double _coord[3])
    {
        coordinate_[0] = _coord[0];
        coordinate_[1] = _coord[1];
        coordinate_[2] = _coord[2];
    }
    // 添加默认构造函数
    Point() {
        coordinate_[0] = 0;
        coordinate_[1] = 0;
        coordinate_[2] = 0;
    }
    double& operator[] (const int& _idx)
    {
        return coordinate_[_idx];
    }
    const double& operator[] (const int& _idx)const
    {
        return coordinate_[_idx];
    }
    double x()
    {
        return coordinate_[0];
    }
    double y()
    {
        return coordinate_[1];
    }
    double z()
    {
        return coordinate_[2];
    }
    double coordinate_[3] = { 0, 0, 0 }; // 顶点三个坐标
}Point;

typedef struct Vertex
{
    Point     p_;            // 顶点几何位置
    Halfedge* he_ = nullptr; // 与顶点相关的一条半边

    // 添加构造函数
    Vertex() : p_(Point()), he_(nullptr) {}
    Vertex(const Point& p, Halfedge* he) : p_(p), he_(he) {}
}Vertex;

typedef struct Halfedge
{
    Vertex* start_vertex_ = nullptr;  // 起点
    Vertex* to_vertex_ = nullptr;       // to vertex
    Halfedge* next_he_ = nullptr;       // 前一条半边
    Halfedge* prev_he_ = nullptr;       // 前一条半边
    Halfedge* oppo_he_ = nullptr;       // 反向对边
    Edge* edge_ = nullptr;       // 对应的边
    Loop* loop_ = nullptr;       // 半边所在的 Loop
}Halfedge;

typedef struct Edge
{
    Halfedge* he0_;
    Halfedge* he1_;

    // 添加构造函数
    Edge() : he0_(nullptr), he1_(nullptr) {}
}Edge;

typedef struct Loop
{
    Halfedge* start_he_ = nullptr; // 起始边
    Loop* next_loop_ = nullptr; // 下一个 Loop
    Loop* prev_loop_ = nullptr; // 前一个 Loop
    Face* face_ = nullptr; // Loop 所在的面

    // 添加构造函数
    Loop() : start_he_(nullptr), next_loop_(nullptr), prev_loop_(nullptr), face_(nullptr) {}
}Loop;

typedef struct Face
{
    Loop* first_loop_ = nullptr; // 第一个 Loop
    Face* next_face_ = nullptr; // 下一个面
    Face* prev_face_ = nullptr; // 前一个面
    Body* body_ = nullptr; // 对应的 Body 

    // 添加构造函数
    Face() : first_loop_(nullptr), next_face_(nullptr), prev_face_(nullptr), body_(nullptr) {}
}Face;

typedef struct Body
{
    Face* first_face_ = nullptr; // 第一个 Face 
    std::vector<Edge*> edges_;   // 记录所有的边，用于线框显示    
    int face_num_ = 0;           // 面的个数
    int edge_num_ = 0;           // 边数
    int vertex_num_ = 0;         // 顶点数

    /** 删除记录的边 */
    void delete_edge(Edge* _e)
    {
        auto it = std::find(edges_.begin(), edges_.end(), _e);
        if (it != edges_.end())
        {
            if (*it)
            {
                delete* it;
                *it = nullptr;
            }
            edges_.erase(it);
        }
    }

    Body() : first_face_(nullptr), face_num_(0), edge_num_(0), vertex_num_(0) {}

    ~Body(){ }

}Body;

#endif // !_SOLIDMODEL_H_