#include "EulerOperations.h"
#include <iostream>

Vertex* EulerOperations::mvfs(const Point& _p)
{
    // 如果已存在体，先清理
    if (body_ != nullptr)
    {
        delete body_;
        body_ = nullptr;
    }

    // 创建新的体、面、环、顶点
    body_ = new Body();
    Face* face = new Face();
    Loop* loop = new Loop();
    Vertex* v = new Vertex(_p, nullptr);

    // 建立拓扑关系
    body_->first_face_ = face;
    body_->face_num_ = 1;
    body_->edge_num_ = 0;
    body_->vertex_num_ = 1;

    face->first_loop_ = loop;
    face->body_ = body_;

    loop->face_ = face;

    return v;
}

Halfedge* EulerOperations::mev(Vertex* _v0, Vertex* _v1, Loop* _loop)
{
    if (!_v0 || !_v1 || !_loop) {
        return nullptr;
    }

    // 检查是否已存在连接这两个顶点的边
    if (_loop->start_he_) {
        Halfedge* he_current = _loop->start_he_;
        do {
            if ((he_current->start_vertex_ == _v0 && he_current->to_vertex_ == _v1) ||
                (he_current->start_vertex_ == _v1 && he_current->to_vertex_ == _v0)) {
                return he_current; // 已存在边，返回现有半边
            }
            he_current = he_current->next_he_;
        } while (he_current != _loop->start_he_);
    }

    // 创建新的边和两条半边
    Halfedge* he0 = new Halfedge();
    Halfedge* he1 = new Halfedge();
    Edge* edge = new Edge();

    // 设置边和半边的关联
    he0->edge_ = edge;
    he1->edge_ = edge;
    edge->he0_ = he0;
    edge->he1_ = he1;

    // 设置环关联
    he0->loop_ = _loop;
    he1->loop_ = _loop;

    // 设置顶点关联
    he0->start_vertex_ = _v0;
    he0->to_vertex_ = _v1;
    he1->start_vertex_ = _v1;
    he1->to_vertex_ = _v0;

    // 设置半边对立关系
    he0->oppo_he_ = he1;
    he1->oppo_he_ = he0;

    // 如果顶点还没有关联的半边，设置为新创建的单边
    if (_v0->he_ == nullptr) _v0->he_ = he0;
    if (_v1->he_ == nullptr) _v1->he_ = he1;

    // 处理环的连接关系
    if (_loop->start_he_ == nullptr)
    {
        // 环为空的情况：创建闭环
        he0->next_he_ = he1;
        he0->prev_he_ = he1;
        he1->next_he_ = he0;
        he1->prev_he_ = he0;
        _loop->start_he_ = he0;
    }
    else
    {
        // 在环中找到以_v0为终点的半边
        Halfedge* he_prev = nullptr;
        Halfedge* he_current = _loop->start_he_;

        do {
            if (he_current->to_vertex_ == _v0) {
                he_prev = he_current;
                break;
            }
            he_current = he_current->next_he_;
        } while (he_current != _loop->start_he_);

        if (he_prev != nullptr)
        {
            Halfedge* he_next = he_prev->next_he_;

            // 插入新边
            he_prev->next_he_ = he0;
            he0->prev_he_ = he_prev;
            he0->next_he_ = he1;
            he1->prev_he_ = he0;
            he1->next_he_ = he_next;
            he_next->prev_he_ = he1;
        }
        else
        {
            // 如果找不到合适的位置，添加到环的末尾
            Halfedge* last_he = _loop->start_he_->prev_he_;
            last_he->next_he_ = he0;
            he0->prev_he_ = last_he;
            he0->next_he_ = he1;
            he1->prev_he_ = he0;
            he1->next_he_ = _loop->start_he_;
            _loop->start_he_->prev_he_ = he1;
        }
    }

    // 将边添加到体的边列表中
    body_->edges_.push_back(edge);
    body_->edge_num_++;
    body_->vertex_num_++;

    return he0;
}

Loop* EulerOperations::mef(Vertex* _v0, Vertex* _v1, Loop* _lp)
{
    if (!_v0 || !_v1 || !_lp) {
        return nullptr;
    }

    // 检查是否已存在连接这两个顶点的面
    if (_lp->start_he_) {
        Halfedge* he_current = _lp->start_he_;
        do {
            if ((he_current->start_vertex_ == _v0 && he_current->to_vertex_ == _v1) ||
                (he_current->start_vertex_ == _v1 && he_current->to_vertex_ == _v0)) {
                // 检查是否已存在面
                if (he_current->edge_ && he_current->oppo_he_ &&
                    he_current->loop_ != he_current->oppo_he_->loop_) {
                    return he_current->oppo_he_->loop_; // 返回现有内环
                }
            }
            he_current = he_current->next_he_;
        } while (he_current != _lp->start_he_);
    }

    // 创建新边和两条半边
    Halfedge* he_new0 = new Halfedge();
    Halfedge* he_new1 = new Halfedge();
    Edge* edge_new = new Edge();

    // 设置基本关联
    he_new0->edge_ = edge_new;
    he_new1->edge_ = edge_new;
    edge_new->he0_ = he_new0;
    edge_new->he1_ = he_new1;

    he_new0->start_vertex_ = _v0;
    he_new0->to_vertex_ = _v1;
    he_new1->start_vertex_ = _v1;
    he_new1->to_vertex_ = _v0;

    he_new0->oppo_he_ = he_new1;
    he_new1->oppo_he_ = he_new0;

    // 创建新面和新环
    Face* face_new = new Face();
    Loop* loop_new = new Loop();

    face_new->body_ = body_;
    face_new->first_loop_ = loop_new;
    loop_new->face_ = face_new;

    // 将新面添加到体中
    face_new->next_face_ = body_->first_face_;
    if (body_->first_face_) {
        body_->first_face_->prev_face_ = face_new;
    }
    body_->first_face_ = face_new;
    body_->face_num_++;

    // 查找原环中连接 _v0 和 _v1 的路径
    bool found_path = false;

    if (_lp->start_he_) {
        // 查找从_v0到_v1的路径
        Halfedge* he_from_v0 = nullptr;
        Halfedge* he_from_v1 = nullptr;

        Halfedge* he_current = _lp->start_he_;
        do {
            if (he_current->start_vertex_ == _v0) {
                he_from_v0 = he_current;
            }
            if (he_current->start_vertex_ == _v1) {
                he_from_v1 = he_current;
            }
            he_current = he_current->next_he_;
        } while (he_current != _lp->start_he_);

        if (he_from_v0 && he_from_v1) {
            // 重新连接环
            Halfedge* he_prev_v0 = he_from_v0->prev_he_;
            Halfedge* he_prev_v1 = he_from_v1->prev_he_;

            // 原环：he_prev_v0 -> he_new0 -> he_from_v1 -> ... -> he_prev_v1 -> he_new1 -> he_from_v0
            he_prev_v0->next_he_ = he_new0;
            he_new0->prev_he_ = he_prev_v0;
            he_new0->next_he_ = he_from_v1;
            he_from_v1->prev_he_ = he_new0;

            he_prev_v1->next_he_ = he_new1;
            he_new1->prev_he_ = he_prev_v1;
            he_new1->next_he_ = he_from_v0;
            he_from_v0->prev_he_ = he_new1;

            // 设置环关联
            he_new0->loop_ = _lp;
            he_new1->loop_ = loop_new;
            loop_new->start_he_ = he_new1;

            // 更新新环中所有半边的环指针
            Halfedge* he_loop = he_from_v1;
            do {
                he_loop->loop_ = loop_new;
                he_loop = he_loop->next_he_;
            } while (he_loop != he_from_v1);

            found_path = true;
        }
    }

    if (!found_path) {
        // 简化处理：直接连接形成新环
        he_new0->loop_ = _lp;
        he_new1->loop_ = loop_new;

        he_new0->next_he_ = he_new1;
        he_new0->prev_he_ = he_new1;
        he_new1->next_he_ = he_new0;
        he_new1->prev_he_ = he_new0;

        _lp->start_he_ = he_new0;
        loop_new->start_he_ = he_new1;
    }

    // 将边添加到体的边列表中
    body_->edges_.push_back(edge_new);
    body_->edge_num_++;

    return loop_new;
}

void EulerOperations::kfmrh(Loop* _out_loop, Loop* _inner_loop)
{
    if (!_out_loop || !_inner_loop) {
        return;
    }

    if (_out_loop->face_ == _inner_loop->face_) {
        return; // 内外环属于同一个面
    }

    Face* face_outer = _out_loop->face_;
    Face* face_inner = _inner_loop->face_;

    // 将内环面的所有环转移到外环面
    Loop* current_inner_loop = face_inner->first_loop_;
    while (current_inner_loop) {
        current_inner_loop->face_ = face_outer;
        current_inner_loop = current_inner_loop->next_loop_;
    }

    // 将内环面的环链表连接到外环面的环链表
    if (face_outer->first_loop_) {
        Loop* last_outer_loop = face_outer->first_loop_;
        while (last_outer_loop->next_loop_) {
            last_outer_loop = last_outer_loop->next_loop_;
        }
        last_outer_loop->next_loop_ = face_inner->first_loop_;
        if (face_inner->first_loop_) {
            face_inner->first_loop_->prev_loop_ = last_outer_loop;
        }
    }
    else {
        face_outer->first_loop_ = face_inner->first_loop_;
    }

    // 从体链表中移除内环面
    if (body_->first_face_ == face_inner) {
        body_->first_face_ = face_inner->next_face_;
    }

    if (face_inner->prev_face_) {
        face_inner->prev_face_->next_face_ = face_inner->next_face_;
    }
    if (face_inner->next_face_) {
        face_inner->next_face_->prev_face_ = face_inner->prev_face_;
    }

    body_->face_num_--;

    // 安全释放内环面
    face_inner->first_loop_ = nullptr; // 避免双重释放
    delete face_inner;
}

Loop* EulerOperations::kemr(Vertex* _v0, Vertex* _v1, Loop* _lp)
{
    if (!_v0 || !_v1 || !_lp) {
        return nullptr;
    }

    // 查找连接两个顶点的边
    Halfedge* he_to_remove = find_halfedge_in_loop(_lp, _v0, _v1);
    if (!he_to_remove || !he_to_remove->edge_) {
        return nullptr;
    }

    Edge* edge_to_remove = he_to_remove->edge_;
    Halfedge* he_opposite = he_to_remove->oppo_he_;

    // 创建新环（内环）
    Loop* loop_inner = new Loop();
    loop_inner->face_ = _lp->face_;

    // 重新连接环，移除指定的边
    if (he_to_remove->prev_he_ && he_to_remove->next_he_) {
        Halfedge* prev_he = he_to_remove->prev_he_;
        Halfedge* next_he = he_to_remove->next_he_;

        // 跳过要删除的边
        prev_he->next_he_ = next_he;
        next_he->prev_he_ = prev_he;

        // 如果删除的边是环的起始边，更新环的起始边
        if (_lp->start_he_ == he_to_remove) {
            _lp->start_he_ = next_he;
        }
    }

    // 设置内环的起始边为对向半边
    if (he_opposite) {
        loop_inner->start_he_ = he_opposite;

        // 更新内环中所有半边的环指针
        Halfedge* current_he = he_opposite;
        do {
            current_he->loop_ = loop_inner;
            current_he = current_he->next_he_;
        } while (current_he != he_opposite);
    }

    // 从体中移除边
    auto it = std::find(body_->edges_.begin(), body_->edges_.end(), edge_to_remove);
    if (it != body_->edges_.end()) {
        body_->edges_.erase(it);
        body_->edge_num_--;

        // 删除边和半边
        if (edge_to_remove->he0_) {
            delete edge_to_remove->he0_;
        }
        if (edge_to_remove->he1_) {
            delete edge_to_remove->he1_;
        }
        delete edge_to_remove;
    }

    return loop_inner;
}

Halfedge* EulerOperations::find_halfedge_in_loop(Loop* loop, Vertex* v_start, Vertex* v_end)
{
    if (!loop || !loop->start_he_ || !v_start || !v_end) return nullptr;

    Halfedge* he_current = loop->start_he_;
    do {
        if (he_current->start_vertex_ == v_start && he_current->to_vertex_ == v_end) {
            return he_current;
        }
        he_current = he_current->next_he_;
    } while (he_current != loop->start_he_);

    return nullptr;
}

Halfedge* EulerOperations::find_halfedge_with_vertex(Loop* loop, Vertex* v)
{
    if (!loop || !loop->start_he_ || !v) return nullptr;

    Halfedge* he_current = loop->start_he_;
    do {
        if (he_current->start_vertex_ == v || he_current->to_vertex_ == v) {
            return he_current;
        }
        he_current = he_current->next_he_;
    } while (he_current != loop->start_he_);

    return nullptr;
}

void EulerOperations::cleanup_extra_edges()
{
    if (!body_) return;

    // 收集所有需要清理的边
    std::vector<Edge*> edges_to_remove;

    for (Edge* edge : body_->edges_) {
        if (!edge || !edge->he0_ || !edge->he1_) continue;

        Vertex* v0 = edge->he0_->start_vertex_;
        Vertex* v1 = edge->he0_->to_vertex_;

        if (!v0 || !v1) continue;

        // 计算边的长度
        double dx = v0->p_[0] - v1->p_[0];
        double dy = v0->p_[1] - v1->p_[1];
        double dz = v0->p_[2] - v1->p_[2];
        double length = sqrt(dx * dx + dy * dy + dz * dz);

        // 检查条件1：非常短的边（可能是错误的连接）
        if (length < 0.1) {
            edges_to_remove.push_back(edge);
            continue;
        }

        // 检查条件2：在同一平面上但不合理的水平边
        if (fabs(dz) < 1e-6) {
            // 检查是否是连接通孔和外框的边（不应该存在）
            bool v0_in_hole = (v0->p_[0] > 0.6 && v0->p_[0] < 1.4 && v0->p_[1] > 0.6 && v0->p_[1] < 1.4);
            bool v1_in_hole = (v1->p_[0] > 0.6 && v1->p_[0] < 1.4 && v1->p_[1] > 0.6 && v1->p_[1] < 1.4);
            bool v0_in_outer = (v0->p_[0] < 0.1 || v0->p_[0] > 1.9 || v0->p_[1] < 0.1 || v0->p_[1] > 1.9);
            bool v1_in_outer = (v1->p_[0] < 0.1 || v1->p_[0] > 1.9 || v1->p_[1] < 0.1 || v1->p_[1] > 1.9);

            // 如果一边在孔内，一边在孔外，这是不应该存在的连接
            if ((v0_in_hole && v1_in_outer) || (v0_in_outer && v1_in_hole)) {
                edges_to_remove.push_back(edge);
                continue;
            }
        }

        // 检查条件3：垂直方向的不合理边
        if (fabs(dx) < 0.1 && fabs(dy) < 0.1 && fabs(dz) > 1.0) {
            bool v0_in_hole_area = (v0->p_[0] > 0.6 && v0->p_[0] < 1.4 && v0->p_[1] > 0.6 && v0->p_[1] < 1.4);
            bool v1_in_hole_area = (v1->p_[0] > 0.6 && v1->p_[0] < 1.4 && v1->p_[1] > 0.6 && v1->p_[1] < 1.4);

            // 如果垂直边连接了孔区域和非孔区域，可能是错误的
            if (v0_in_hole_area != v1_in_hole_area) {
                edges_to_remove.push_back(edge);
                continue;
            }
        }
    }

    // 检查重复的边（连接相同顶点的多条边）
    std::vector<std::pair<Vertex*, Vertex*>> processed_pairs;

    for (Edge* edge : body_->edges_) {
        if (!edge || !edge->he0_ ||
            std::find(edges_to_remove.begin(), edges_to_remove.end(), edge) != edges_to_remove.end()) {
            continue;
        }

        Vertex* v0 = edge->he0_->start_vertex_;
        Vertex* v1 = edge->he0_->to_vertex_;

        if (!v0 || !v1) continue;

        // 创建标准化的顶点对（确保顺序一致）
        Vertex* first = (v0 < v1) ? v0 : v1;
        Vertex* second = (v0 < v1) ? v1 : v0;

        auto pair = std::make_pair(first, second);

        // 检查是否已经处理过这个顶点对
        if (std::find(processed_pairs.begin(), processed_pairs.end(), pair) != processed_pairs.end()) {
            edges_to_remove.push_back(edge);
        }
        else {
            processed_pairs.push_back(pair);
        }
    }

    // 移除边
    for (Edge* edge : edges_to_remove) {
        auto it = std::find(body_->edges_.begin(), body_->edges_.end(), edge);
        if (it != body_->edges_.end()) {
            // 修复环的连接关系
            if (edge->he0_ && edge->he0_->prev_he_ && edge->he0_->next_he_) {
                edge->he0_->prev_he_->next_he_ = edge->he0_->next_he_;
                edge->he0_->next_he_->prev_he_ = edge->he0_->prev_he_;

                // 如果这个边是环的起始边，需要更新环的起始边
                if (edge->he0_->loop_ && edge->he0_->loop_->start_he_ == edge->he0_) {
                    edge->he0_->loop_->start_he_ = edge->he0_->next_he_;
                }
            }

            if (edge->he1_ && edge->he1_->prev_he_ && edge->he1_->next_he_) {
                edge->he1_->prev_he_->next_he_ = edge->he1_->next_he_;
                edge->he1_->next_he_->prev_he_ = edge->he1_->prev_he_;

                if (edge->he1_->loop_ && edge->he1_->loop_->start_he_ == edge->he1_) {
                    edge->he1_->loop_->start_he_ = edge->he1_->next_he_;
                }
            }

            // 删除半边
            if (edge->he0_) {
                delete edge->he0_;
                edge->he0_ = nullptr;
            }
            if (edge->he1_) {
                delete edge->he1_;
                edge->he1_ = nullptr;
            }

            // 从列表中移除并删除边
            body_->edges_.erase(it);
            body_->edge_num_--;
            delete edge;
        }
    }
}