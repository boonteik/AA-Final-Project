#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include "vertex.h"
#include "edge.h"

using namespace std;

class graph
{
	private:
		int _n, _m;
		vertex* _vertices;
		edge* _edges;

	public:
		graph(int n, int m);
		
		void add_edge(int v1, int v2, int upper12j);
		void display_upper();
		void display_flow();
		void write_graph(string name);
 		bool is_valid_flow();
 		
		int flow();
		int n() { return _n; }
		int m() { return _m; }
		vertex* s() { return &_vertices[0]; }
		vertex* t() { return &_vertices[1]; }
		vertex* v() { return _vertices; }
		edge* e() { return _edges; }
};

graph::graph(int n, int m) : _n(n), _m(0)
{
	_vertices = new vertex[n];
	_edges = new edge[m*2];
	
	for (int i = 0; i < n; i++) {
		_vertices[i].set_index(i);
	}
}

void graph::add_edge(int v1, int v2, int upper12)
{
	_edges[_m].set_edge(&(_vertices[v1]), &(_vertices[v2]), upper12, &(_edges[_m+1]));
	_edges[_m+1].set_edge(&(_vertices[v2]), &(_vertices[v1]), 0, &(_edges[_m]));
	_m += 2;
}

void graph::display_upper()
{
	for (int i = 0; i < _m; i++) {
		edge* e = &(_edges[i]);
		vertex* v = e->v();
		int v_index = e->v()->index();

		if (e->upper() != 0)
			cout << v_index << '\t' << e->reverse()->v()->index() << '\t' << e->upper() <<  endl;
	}

}

void graph::display_flow()
{
	for (int i = 0; i < _m; i++) {
		edge* e = &(_edges[i]);
		vertex* v = e->v();
		int v_index = e->v()->index();
		
		if (e->flow() > 0)
			cout << v_index << '\t' << e->reverse()->v()->index() << '\t' << e->upper() <<","<< e->flow() <<  endl;
	}

}

bool graph::is_valid_flow()
{
	bool flag = true;
	for (int i = 2; i < _n; i++){
		int excess = 0;
        	int n_outgoing_edges = _vertices[i].edges()->size();
        	for (int j = 0; j < n_outgoing_edges; j++){
				edge* e = _vertices[i].edges()->at(j);
				int flow = e->flow();
				excess += flow;
        	}
		if (excess != 0){
			cout << "flow is not conserved at vertex: " << i << " excess: " << excess << endl;
			flag = false;
		}
	
	}
	for (int i = 0; i < _m; i++){
		edge* e = &(_edges[i]);
		if (  e->flow() > e->upper() ){
                	cout << "flow exceeds upper bound from vertex: " << e->v()->index() << " to vertex: " << e->v_op()->index()
                        <<  " flow: " << e->flow() << " upper: " <<  e->upper() << endl;
			flag = false;
        }
	}

	return flag;
}

int graph::flow()
{
// 	int flow = 0;
// 	int n_outgoing_edges = _vertices[0].edges()->size();
// 	for (int i = 0; i < n_outgoing_edges; i++){
// 		flow += _vertices[0].edges()->at(i)->flow(&(_vertices[0]));
// 	}
// 	return flow;
	return t()->excess();
}

void graph::write_graph(string name)
{
	ofstream f;
	f.open(name.c_str());
	f << "p max " << _n << '\t' << _m/2 << '\n';
	f << "n\t" << 1 << "\ts\n";
	f << "n\t" << _n << "\tt\n";
	
	for (int i = 0; i < _m; i+=2) {
		edge* e = _edges + i;
		int v1 = e->v()->index();
		int v2 = e->v_op()->index();
		int upper = e->upper();
		
		if (v1 == 0) {
			v1 = 1;
		} else if (v1 == 1) {
			v1 = _n;
		}
		
		if (v2 == 0) {
			v2 = 1;
		} else if (v2 == 1) {
			v2 = _n;
		}
		
		f << "a\t" << v1 << '\t' << v2 << '\t' << upper <<'\n';
	}
	
	f.close();
}

#endif
