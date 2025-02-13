#ifndef DISCHARGE
#define DISCHARGE

#include <cstdlib>
#include <cassert>
#include <iostream>
#include <vector>
#include <queue>
#include <cilk/cilk.h>
#include "graph.h"
#include "limits.h"

using namespace std;

extern int heighest;
extern vector<int> counter;

void push(vertex* v, edge *e)
{
	assert(e->v() == v);
	assert(v->excess() > 0);
	assert(e->residue() > 0);
	assert(v->height() == e->v_op()->height() + 1);
	
	int d = min(v->excess(), e->residue());
	e->push_flow(d);
	vertex* w = e->v_op();
	v->update_excess(-d);
	w->update_excess(d);
}

void relabel(vertex* v)
{
	vector<edge*>* edges = v->edges();
	int numOfEdges = edges->size();
	int minHeight = INT_MAX;

	for( int i = 0; i < numOfEdges; i++){
		edge* currentEdge = edges->at(i);
		vertex* w = currentEdge->v_op();
		if (currentEdge->residue() > 0 && w->height()+1 < minHeight) {
			minHeight = w->height()+1;
		}
	}
	v->set_height(minHeight);
	
	assert(minHeight != INT_MAX);
	
}

int push_relabel(vertex* v, int* push_counter, int* relabel_counter)
{
	assert(v->excess() > 0);	// must be active
	edge* e = v->cur_edge();

	if (e->residue() > 0 && v->height() == e->v_op()->height() + 1) {
		push(v, e);
		*push_counter += 1;
	} else {
		if (!v->is_last()) {
			v->next_edge();
		} else {
			v->next_edge();
			int vh = v->height();
			counter[vh]--;
			relabel(v);
			if (v->height() != counter.size()){
				if (v->height() > heighest)
					heighest = v->height();
				counter[v->height()]++;
			}
			*relabel_counter += 1;
			if ( counter[vh] == 0 && heighest > vh )
                                return vh;
		}
	}

	return -1;

}

void discharge(queue<vertex*>* Q, vertex* source, vertex* sink, int* push_counter, int* relabel_counter)
{
	vertex* v = Q->front();
	Q->pop();
	int h = v->height();
	int i = 0;
 	while (v->excess() != 0) { // && v->height() == h){
 		vertex* w = v->cur_edge()->v_op();
		int old_excess = w->excess();
		push_relabel(v, push_counter, relabel_counter);
		if ( old_excess == 0 && w->excess() > 0 && w != source && w!= sink ) {
			Q->push(w);
		}
 	}
// 	if ( v->excess() > 0 )
// 		Q->push(v);
}

void discharge(priority_queue<vertex*, vector<vertex*>, CompareVertex>* Q, 
	       vertex* source, vertex* sink, int* push_counter, int* relabel_counter)
{
	vertex* v = Q->top();
	Q->pop();
 	while (v->excess() != 0) {
 		vertex* w = v->cur_edge()->v_op();
		int old_excess = w->excess();
		push_relabel(v, push_counter, relabel_counter);
		if ( old_excess == 0 && w->excess() > 0 && w != source && w!= sink ) {
			Q->push(w);
		}
 	}
}

#endif
