#ifndef PARALLEL_SOLVER
#define PARALLEL_SOLVER

#include <iostream>
#include <cassert>
#include <cstdlib>
#include <cilk/cilk.h>
#include <cilk/reducer_opadd.h>
#include <cilk/reducer_min.h>
#include <pthread.h>

#define print(x) std::cout << x << std::endl

class parallel_solver
{
private:
	graph *g;
	vector<vertex*>* vertices;
	cilk::reducer_opadd<int> *excess_tmp;
	vector<int> ex;
	vector<pthread_mutex_t> vlocks;
	cilk::reducer_opadd<int> active;
//	int active;
	pthread_mutex_t alock;

public:
	parallel_solver(graph *G) : g(G), vertices(G->v()) 
	{
		vlocks.resize(g->n());
		ex.resize(g->n(), 0);
		excess_tmp = new cilk::reducer_opadd<int>[g->n()];
		g->s()->set_height(g->n());
//		active = 0;
		pthread_mutex_init(&alock, NULL);
		for (int i = 0; i < g->n(); ++i) {
			pthread_mutex_init(&vlocks[i], NULL);
		}
	}

	~parallel_solver() { delete[] excess_tmp; }
	int solve_maxflow();
	int pulse(int);
	void push(int);
	void relabel(int);
};

int parallel_solver::solve_maxflow()
{
	vertex *s = g->s();
	vector<edge*>* s_adj = s->edges();
	edge *e;
	int a = 0;
	for (int i = 0; i < s_adj->size(); ++i) {
		e = s_adj->at(i);
		e->update_flow(s, e->upper(s));
		e->opposite(s)->update_excess(e->upper(s));
		if (e->opposite(s)->excess() > 0)
			++a;
	}
//	cilk_for (int i = 0; i < g->n(); ++i) {
//		if (excess_tmp[i].get_value() != 0)
//			print("initialize: excess reducer not 0");
//	}
	int counter = 0;
	while (a > 0) {
		a = pulse(counter);
//		g->display_flow();
//		print("count: " << counter << ", active: " << a);
		counter++;
	}
	return 0;
}

int parallel_solver::pulse(int c)
{
//	cilk::reducer_opadd<int> active;
	int active = 0;
	vector<edge*>* edges;
//	cilk_for (int i = 2; i < vertices->size(); ++i) {
//		int e = excess_tmp[i].get_value();
//		assert(ex[i] == 0);
//		if (e != 0)
//			print("excess for " << i << " not cleaned before push");
//	}
	cilk_for (int i = 2; i < vertices->size(); ++i) {
		push(i);
	}			
	cilk_sync;
	cilk_for(int i = 2; i < vertices->size(); ++i) {
		vertex *v = vertices->at(i);
		if (v->excess() > 0) {
			relabel(i);
		}
		int exc = excess_tmp[i].get_value();
		exc = ex[i];
		assert(exc >= 0);
//		if (exc != ex[i])
//			print("count: " << c << " v " << i << " exc: " << exc << " ex: " << ex[i]);
		v->update_excess(exc);
		ex[i] = 0;
		excess_tmp[i] -= exc;
//		if (excess_tmp[i].get_value() != 0)
//			print(i << "excess not cleaned: " << excess_tmp[i].get_value());
//		excess_tmp[i].get_value();
		assert(excess_tmp[i].get_value() == 0);
		if (v->excess() > 0) {
			pthread_mutex_lock(&alock);
//			print(i << " is active with excess " << v->excess());
			active += 1;
			pthread_mutex_unlock(&alock);
		}
	}
	cilk_sync;
	return active;
//	return active.get_value();
}

//stage 1
void parallel_solver::push(int i)
{
	vertex *v = vertices->at(i);
	int excess = v->excess();
	if (excess <= 0)
		return;
	vector<edge*>* edges = v->edges();
	int h = v->height();
	cilk_for (int j = 0; j < edges->size(); ++j) {
		edge *e = edges->at(j);
		vertex *op = e->opposite(v);
		int res = e->residue(v);
		if (h == op->height()+1 && res > 0)
		{
			bool updated = false;
			int d;
			pthread_mutex_lock(&vlocks[i]);
			if (excess > 0) {
				d = min(excess, res);
				assert(d > 0);
				excess -= d;
				updated = true;
			}
			pthread_mutex_unlock(&vlocks[i]);
			if (updated) {
				e->update_flow(v, d);
				int k = e->opposite(v)->index();
				excess_tmp[k] += d;
				pthread_mutex_lock(&vlocks[k]);
				ex[k] += d;
				pthread_mutex_unlock(&vlocks[k]);
			}
		}
	}
	cilk_sync;
	v->update_excess(excess - v->excess());
}

void parallel_solver::relabel(int i)
{
	assert(i > 1);
	vertex* v = vertices->at(i);
	assert(v->excess() > 0);
	cilk::reducer_min<int> h(g->n()*2);
	vector<edge*>* edges = v->edges();
	cilk_for (int i = 0; i < edges->size(); ++i) {
		edge *e = edges->at(i);
		if (e->residue(v) > 0) {
			int k = e->opposite(v)->height();
			assert(v->height() <= k);
			cilk::min_of(h, e->opposite(v)->height());
		}
	}
	v->set_height(h.get_value() + 1);

//	int height = v->height();
//	int newh = h.get_value();
//	if (height < newh) {
//		v->set_height(newh);
//	}
}

#endif
