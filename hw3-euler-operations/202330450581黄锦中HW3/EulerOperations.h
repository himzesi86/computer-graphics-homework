#ifndef _EULER_OPERATIONS_H_
#define _EULER_OPERATIONS_H_

#include "SolidModel.h"



class EulerOperations
{
public:
	EulerOperations() {}
	~EulerOperations() 
	{
		if (body_ != nullptr)
		{
			delete body_;
			body_ = nullptr;
		}
	}

	Body* get_body()
	{
		return body_;
	}
public:
	//--- 实现以下的欧拉操作 ---//

	Vertex* mvfs(const Point & _p);
	Halfedge* mev(Vertex* _v0, Vertex * _v1, Loop* _lp);
	Loop* mef(Vertex* _v0, Vertex* _v1, Loop* _lp);
	Loop* kemr(Vertex* _v0, Vertex* _v1, Loop* _lp);
	void kfmrh(Loop* _out_loop, Loop* _loop);

	// 在 EulerOperations 类中添加清理多余边的函数
	void cleanup_extra_edges();

private:
	Body* body_ = nullptr;

	// 在环中查找连接两个顶点的半边
	Halfedge* find_halfedge_in_loop(Loop* loop, Vertex* v_start, Vertex* v_end);
	// 在环中查找包含指定顶点的半边
	Halfedge* find_halfedge_with_vertex(Loop* loop, Vertex* v);

	
};


#endif // !_EULER_OPERATIONS_H_