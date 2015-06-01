#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cmath>
#include <sstream>
#include <cstdlib>
#include <time.h>
#include <new>

using namespace std;

template <typename T>
class Node {
public:
	T data;
	Node<T>* next;
	Node<T>* previous;
	
	Node(T data) {
		this->data = data;
		this->next = NULL;
		this->previous = NULL;
	}
};

template <typename T>
class List {
public:
	int size;
	
	Node<T>* start;

	List() {
		start = NULL;
		size = 0;
	}

	Node<T>* insert(T data) {
		Node<T>* new_node = new Node<T>(data);
		// adding new node to the front of the list
		new_node->next = start;
		new_node->previous = NULL;
		if (start != NULL) {
			start->previous = new_node;
		}
		start = new_node;

		size += 1;

		return new_node;
	}

	void remove(Node<T>* node) {
		if (node != NULL) {
			// removes the node provided
			if (node == start) {
				start = start->next;
			}
			if (node->next != NULL) {
				node->next->previous = node->previous;
			}
			if (node->previous != NULL) {
				node->previous->next = node->next;
			}
			
			delete node;

			size -= 1;
		}
		else {
			// we are deleting nothing
			return;
		}
	}

	void remove_data(T data) {
		// first search for the node and then remove it
		remove(find(data));
	}

	Node<T>* find(T data) {
		if (size == 0) {
			// it cannot be there
			return NULL;
		}
		else {
			Node<T>* current = start;
			// go through the nodes and see the contents
			while (current != NULL) {
				if (current->data == data) {
					return current;
				} else {
					current = current->next;
				}
			}
			// if it is not encountered
			return NULL;
		}
	}
};

class HalfEdge;
class Triangle;

class Point { // stores points_node
public:
	double x;
	double y;
	int counter;
	double random_id; // for sorting of the input points to ensure randomness

	Triangle* in_triangle; // in which triangle it originally is
	Node<Point*>* points_node; // the node of this in points list

	List<HalfEdge*>* half_edges; // half-edges containing this point

	Point(double x, double y, int counter, double random_id) {
		this->x = x;
		this->y = y;
		this->counter = counter;
		this->random_id = random_id;
		
		in_triangle = NULL;
		points_node = NULL;

		half_edges = new List<HalfEdge*>();
	}

	~Point() {
		delete half_edges;
	}

	double distance_from(Point* b) {
		// euclidean distance
		double ax = this->x;
		double bx = b->x;
		
		double ay = this->y;
		double by = b->y;
		
		double adx = abs(ax - bx);
		double ady = abs(ay - by);
		return sqrt(pow(adx, 2.0) + pow(ady, 2.0));
	}

	string to_string() {
		stringstream s;
		s << "(" << x << ", " << y << ")";
		return s.str();
	}

	// two points are equal if they have the same x and y
	bool operator== (const Point* rhs) const {
		return ((this->x==rhs->x) && (this->y==rhs->y));
	}
	
	bool operator!= (const Point* rhs) const {
		return !(this==rhs);
	}
};

class HalfEdge { // stores half_edges_node
public:
	Point* source;
	Point* target;
	
	HalfEdge* twin;
	HalfEdge* next;
	
	bool helper; // useful for delaunay output

	Triangle* of_triangle;
	Node<HalfEdge*>* half_edges_node; // what node in half-edges list this is

	Point* midpoint;
	
	HalfEdge(Point* source, Point* target) {
		this->source = source;
		this->target = target;
		
		((this->source)->half_edges)->insert(this);

		twin = NULL;
		next = NULL;

		// is this a helper?
		helper = false;
		if (source->counter < 0 || target->counter < 0) {
			helper = true;
		}

		of_triangle = NULL;
		half_edges_node = NULL;

		double mx = (target->x+source->x)/2.0;
		double my = (target->y+source->y)/2.0;
		midpoint = new Point(mx,my,-1234,-1234);
	}
	
	~HalfEdge() {
		((this->source)->half_edges)->remove_data(this); 
		// fails quietly if not there

		delete midpoint;
	}
	
	// helper function
	double area(Point* p) {
		// positive = true if anticlockwise direction
		Point* a = this->source;
		Point* b = this->target;
		Point* c = p;

		double xa = a->x;
		double ya = a->y;
		double xb = b->x;
		double yb = b->y;
		double xc = c->x;
		double yc = c->y;

		double sum = (0.5)*((xa*(yb-yc))+(xb*(yc-ya))+(xc*(ya-yb)));

		return sum;
		// if it is zero, it will not get included
		// if a point is on the line, it is not in any triangle
		// therefore, input_points have to be given general position
	}

	string to_string() {
		stringstream s;
		s << "(" << source->counter << ", " << target->counter << ")";
		return s.str();
	}

	// two half-edges are equal if they have the same source and target
	bool operator== (const HalfEdge* rhs) const {
		return ((this->source==rhs->source) && (this->target==rhs->target));
	}
	
	bool operator!= (const HalfEdge* rhs) const {
		return !(this==rhs);
	}
};

class Triangle { // stores triangles_node
public:
	HalfEdge* ab; // primary half_edge
	HalfEdge* bc;
	HalfEdge* ca;

	bool helper; // useful for voronoi

	Point* center; // used if voronoi option employed

	double radius;

	Node<Triangle*>* triangles_node; // the node of this in triangles list

	List<Point*>* points; // list of unassigned points in the triangle

	Triangle(HalfEdge* ab, HalfEdge* bc, HalfEdge* ca) {
		// is this a helper?
		// if at least one half-edge is a helper, this is a helper
		helper = (ab->helper || bc->helper || ca->helper);

		// connect half-edges together
		ab->next = bc;
		bc->next = ca;
		ca->next = ab;

		// add the half-edges to the triangle
		this->ab = ab;
		ab->of_triangle = this;
		this->bc = bc;
		bc->of_triangle = this;
		this->ca = ca;
		ca->of_triangle = this;

		// calculate center
		Point* a = ab->source;
		Point* b = bc->source;
		Point* c = ca->source;
		
		double ax = a->x;
		double ay = a->y;
		double bx = b->x;
		double by = b->y;
		double cx = c->x;
		double cy = c->y;
		
		double d = 2*(ax*(by-cy) + bx*(cy-ay) + cx*(ay-by));
		
		double ux = ((ax*ax + ay*ay) * (by-cy)
				  + (bx*bx + by*by) * (cy-ay)
				  + (cx*cx + cy*cy) * (ay-by))
				  / d;

		double uy = ((ax*ax + ay*ay) * (cx-bx)
				  + (bx*bx + by*by) * (ax-cx)
				  + (cx*cx + cy*cy) * (bx-ax))
				  / d;
		
		center = new Point(ux,uy,-999,-999);
		
		// radius is the distance between a point and the center
		radius = a->distance_from(center);

		triangles_node = NULL;

		points = new List<Point*>();
	}

	~Triangle() {
		delete center;
		delete points;
	}

	string to_string() {
		stringstream s;
		s << "(" << ab->source->counter << ", "
		  << bc->source->counter << ", "
		  << ca->source->counter << ")";
		return s.str();
	}
};

class Delaunay { // includes delaunay and voronoi
private:
	List<Point*>* input_points;
	List<Point*>* random_input_points; // shuffled input_pts to ensure O(nlogn)

	List<Point*>* points;
	List<HalfEdge*>* half_edges;
	List<Triangle*>* triangles;

	string input_address;
	string output_address;

	bool delaunay;
	bool voronoi;

public:
	Delaunay(string input_address, string output_address,
			 bool delaunay, bool voronoi) {

		// seed the random number generator
		srand(time(NULL));

		input_points = new List<Point*>();
		random_input_points = new List<Point*>();

		points = new List<Point*>();
		half_edges = new List<HalfEdge*>();
		triangles = new List<Triangle*>();

		this->input_address = input_address;
		this->output_address = output_address;

		this->delaunay = delaunay;
		this->voronoi = voronoi;
		
		// make initial helper points
		Point* a = new Point(0,0,-1,-1);
		Point* b = new Point(100,0,-2,-1);
		Point* c = new Point(0,100,-3,-1);
		Point* d = new Point(100,100,-4,-1);

		// include these helper points in the points list
		a->points_node = points->insert(a);
		b->points_node = points->insert(b);
		c->points_node = points->insert(c);
		d->points_node = points->insert(d);

		// add helper half edges to the half edges list
		HalfEdge* ab = new HalfEdge(a,b);
		ab->half_edges_node = half_edges->insert(ab);
		HalfEdge* bc = new HalfEdge(b,c);
		bc->half_edges_node = half_edges->insert(bc);
		HalfEdge* ca = new HalfEdge(c,a);
		ca->half_edges_node = half_edges->insert(ca);
		
		HalfEdge* dc = new HalfEdge(d,c);
		dc->half_edges_node = half_edges->insert(dc);
		HalfEdge* cb = new HalfEdge(c,b);
		cb->half_edges_node = half_edges->insert(cb);
		HalfEdge* bd = new HalfEdge(b,d);
		bd->half_edges_node = half_edges->insert(bd);
		
		// add twins
		bc->twin = cb;
		cb->twin = bc;
		
		// creating the helper triangles from the new half edges
		Triangle* triangle1 = new Triangle(ab,bc,ca);
		triangle1->triangles_node = triangles->insert(triangle1);
		Triangle* triangle2 = new Triangle(dc,cb,bd);
		triangle2->triangles_node = triangles->insert(triangle2);

		// input the points from file address
		read_file(input_address);

		// shuffle input points
		shuffle_input();
	}

	~Delaunay() {
		// delete lists
		// also delete all contents of the lists

		// delete input points
		Node<Point*>* current_p_node = input_points->start;
		while (input_points->size != 0) {
			Node<Point*>* old_current_p_node = current_p_node;
			current_p_node = current_p_node->next;
			input_points->remove(old_current_p_node);
		}
		delete input_points;

		// delete random input points
		current_p_node = random_input_points->start;
		while (random_input_points->size != 0) {
			Node<Point*>* old_current_p_node = current_p_node;
			current_p_node = current_p_node->next;
			random_input_points->remove(old_current_p_node);
		}
		delete random_input_points;

		// delete points
		current_p_node = points->start;
		while (points->size != 0) {	
			Node<Point*>* old_current_p_node = current_p_node;
			current_p_node = current_p_node->next;
			points->remove(old_current_p_node);
		}
		delete points;

		// delete half edges
		Node<HalfEdge*>* current_e_node = half_edges->start;
		while (half_edges->size != 0) {
			Node<HalfEdge*>* old_current_e_node = current_e_node;
			current_e_node = current_e_node->next;
			half_edges->remove(old_current_e_node);
		}
		delete half_edges;

		// delete triangles
		Node<Triangle*>* current_t_node = triangles->start;
		while (triangles->size != 0) {
			Node<Triangle*>* old_current_t_node = current_t_node;
			current_t_node = current_t_node->next;
			(triangles)->remove(old_current_t_node);
		}
		delete triangles;
	}

	void read_file(string file_address) {
		// read the input file and put points into list
		// the input file has the format of x_coordinate\ty_coordinate
		string line;
		ifstream input_file(file_address.c_str(), ios::in);
		if (input_file.is_open()) {
			int counter = 0;
			while (getline(input_file, line)) {

				// split the string to strings with x and y coordinates
				int pos = line.find("\t");

				string x_str = line.substr(0, pos);
				string y_str = line.substr(pos);

				// read the numbers
				// we assume valid inputs - if invalid, returns 0.0
				double x = atof(x_str.c_str());
				double y = atof(y_str.c_str());

				// make a random double
				double random_id = (double) rand() / RAND_MAX;

				// create corresponding point and insert to input_points
				if (x <= 100 && x >= 0 && y <= 100 && y >= 0) {
					Point* p = new Point(x, y, counter, random_id);
					input_points->insert(p);

					// put the input point in correct triangle
					Node<Triangle*>* current = triangles->start;
					while (current != NULL) {
						Triangle* t = current->data;
						if (is_in(t, p)) {
							(t->points)->insert(p);
							p->in_triangle = t;
							break;
						}
						
						current = current->next;
					}
				}
				else {
					// if the point is out of range
					cout << "Warning: Point with counter " << counter
						 << " was ignored!\n";
				}

				counter += 1;
			}
			input_file.close();
		}
		else {
			cout << "Error: Could not open input file!\n";
		}
	}

	void shuffle_input() {
		// sorts according to points' random_id's
		// since the id's are random, it makes the input order random

		// uses quicksort
		if (input_points->size > 0) {
			// make a point* array of input_points
			Point** input = new (nothrow) Point* [input_points->size];

			Node<Point*>* current = input_points->start;
			int counter = 0;
			while (current != NULL) {
				// add the input_points to the array one by one
				input[counter] = current->data;
				current = current->next;
				counter += 1;
			}

			// perform quicksort on the input array
			input_quicksort(input, 0, input_points->size-1);

			for (int i = 0; i < input_points->size; i++) {
				// insert input_points to random_input_points one by one
				random_input_points->insert(input[i]);
			}

			// delete the input array
			delete[] input;
		}
	}

	// the recursive sort operation
	void input_quicksort(Point** input, int l, int r) {
		// the array is changed in-place
		int orig_l = l;
		int orig_r = r;

		int size = r - l;

		double pivot = input[orig_l]->random_id;

		// swap things around while pointers are at correct relative positions
		while (l <= r && l < size) {
			bool l_stayed = true;
			bool r_stayed = true;

			if (input[l]->random_id <= pivot) {
				l += 1;
				l_stayed = false;
			}
			if (input[r]->random_id > pivot) {
				r -= 1;
				r_stayed = false;
			}

			// if both pointers cannot move - we can swap
			if (l_stayed && r_stayed) {
				Point* swap = input[l];
				input[l] = input[r];
				input[r] = swap;
			}
		}

		// finally, we swap the right pointer's current position with first
		Point* swap = input[r];
		input[r] = input[orig_l];
		input[orig_l] = swap;

		// do quicksort for the two resulting paths
		if (r != orig_l) {
			input_quicksort(input, orig_l, r-1);
		}
		if (l < size) {
			input_quicksort(input, l, orig_r);
		}
	}

	bool is_in(Triangle* t, Point* p) {
		// uses the sign of areas
		// triangle is made of each half edge and p
		// if the sign of all of these triangles is the same = inside
		// if at least one of them is different = outside
		HalfEdge* ab = t->ab;
		HalfEdge* bc = t->bc;
		HalfEdge* ca = t->ca;

		return (((ab->area(p) > 0) == (bc->area(p) > 0)) &&
				((bc->area(p) > 0) == (ca->area(p) > 0)));
	}

	bool circumcircle_empty(Triangle* t) {
		bool val = true;
		
		Node<Point*>* current_p_node = points->start;
		// go through all points
		// could be more localized
		while (current_p_node != NULL) {
			Point* p = current_p_node->data;
			if (p != t->ab->source &&
				p != t->bc->source &&
				p != t->ca->source) {
				// only if p is not one of the points of the triangle
				if (p->distance_from(t->center) < t->radius) {
					// when it's on the circumcircle, it doesn't count
					val = false;
					// one of the points is in = circumcircle not empty
					break;
				}
			}
			current_p_node = current_p_node->next;
		}
				
		return val;
	}

	void split_triangle(Triangle* t, Point* p) {
		// split that triangle into three new triangles
		// therefore, make three triangles inside and connect

		// edges of the old triangle
		HalfEdge* old_e1 = t->ab;
		HalfEdge* old_e2 = t->bc;
		HalfEdge* old_e3 = t->ca;

		// make new edges and insert them to half edges list
		HalfEdge* new_e1_1 = new HalfEdge(p, old_e1->source);
		HalfEdge* new_e1_2 = new HalfEdge(old_e1->source, p);
		new_e1_1->twin = new_e1_2;
		new_e1_2->twin = new_e1_1;
		new_e1_1->half_edges_node = half_edges->insert(new_e1_1);
		new_e1_2->half_edges_node = half_edges->insert(new_e1_2);

		HalfEdge* new_e2_1 = new HalfEdge(p, old_e2->source);
		HalfEdge* new_e2_2 = new HalfEdge(old_e2->source, p);
		new_e2_1->twin = new_e2_2;
		new_e2_2->twin = new_e2_1;
		new_e2_1->half_edges_node = half_edges->insert(new_e2_1);
		new_e2_2->half_edges_node = half_edges->insert(new_e2_2);

		HalfEdge* new_e3_1 = new HalfEdge(p, old_e3->source);
		HalfEdge* new_e3_2 = new HalfEdge(old_e3->source, p);
		new_e3_1->twin = new_e3_2;
		new_e3_2->twin = new_e3_1;
		new_e3_1->half_edges_node = half_edges->insert(new_e3_1);
		new_e3_2->half_edges_node = half_edges->insert(new_e3_2);

		// make new triangles using the old and new edges
		Triangle* new_t_1 = new Triangle(old_e1,new_e2_2,new_e1_1);
		new_t_1->triangles_node = triangles->insert(new_t_1);
		Triangle* new_t_2 = new Triangle(old_e2,new_e3_2,new_e2_1);
		new_t_2->triangles_node = triangles->insert(new_t_2);
		Triangle* new_t_3 = new Triangle(old_e3,new_e1_2,new_e3_1);
		new_t_3->triangles_node = triangles->insert(new_t_3);

		// reallocate the remaining input_points in original triangle
		// so that they are still in a corresponding triangle
		// also, remove p from old triangle's input_points

		// loop through the triangle's points
		Node<Point*>* current_t_p_node = t->points->start;
		while ((t->points)->size != 0) {

			Point* t_p = current_t_p_node->data;

			if (t_p != p) { // one of the points returned will be p itself
				if (is_in(new_t_1, t_p)) {
					(new_t_1->points)->insert(t_p);
					t_p->in_triangle = new_t_1;
				}
				else if (is_in(new_t_2, t_p)) {
					(new_t_2->points)->insert(t_p);
					t_p->in_triangle = new_t_2;
				}
				else if (is_in(new_t_3, t_p)) {
					(new_t_3->points)->insert(t_p);
					t_p->in_triangle = new_t_3;
				}
				else {
					cout << "Error: Wrong point position.\n";
					// Can happen if points are not in general position
				}
			}

			// remove the p from the triangle
			Node<Point*>* old_current_t_p_node = current_t_p_node;
			current_t_p_node = current_t_p_node->next;
			(t->points)->remove(old_current_t_p_node);
		}

		// for each half-edge of the triangle, check for flip
		// if there are flips, check the adjacent half-edges too = recurse
		// (inside the flip function, reallocate the input_points)
		flip(old_e1);
		flip(old_e2);
		flip(old_e3);
		
		// finally, delete the original big triangle
		triangles->remove(t->triangles_node);
	}

	void flip(HalfEdge* ab) {
		// switching to the most advantageous edge - flipping if necessary

		// definitions that are for sure
		Point* c = (ab->next)->target;

		HalfEdge* bc = ab->next;
		HalfEdge* ca = bc->next;
		
		Triangle* old_1 = ab->of_triangle;
		
		if (ab->twin != NULL && !circumcircle_empty(old_1)) {
			// it's enough to check one triangle
			// if one triangle fails, they both do

			// definitions that only apply if a twin exists
			Point* d = ((ab->twin)->next)->target;
			
			HalfEdge* ba = ab->twin;
			HalfEdge* ad = ba->next;
			HalfEdge* db = ad->next;
			
			Triangle* old_2 = ba->of_triangle;

			// make new edges across
			HalfEdge* cd = new HalfEdge(c, d);
			HalfEdge* dc = new HalfEdge(d, c);

			// make them twins
			cd->twin = dc;
			dc->twin = cd;

			// insert them into half edges list
			cd->half_edges_node = half_edges->insert(cd);
			dc->half_edges_node = half_edges->insert(dc);

			// make two new triangles
			Triangle* t_1 = new Triangle(db,bc,cd);
			t_1->triangles_node = triangles->insert(t_1);
			Triangle* t_2 = new Triangle(ad,dc,ca);
			t_2->triangles_node = triangles->insert(t_2);

			// transfer input_points to these triangles
			// from the first old triangle
			Node<Point*>* current_o_1_p_node = (old_1->points)->start;
			while (old_1->points->size != 0) {

				Point* o_1_p = current_o_1_p_node->data;
				if (is_in(t_1, o_1_p)) {
					// the old point is in the first new trianagle
					(t_1->points)->insert(o_1_p);
					o_1_p->in_triangle = t_1;
				}
				else if (is_in(t_2, o_1_p)) {
					// the old point is in the second new trianagle
					(t_2->points)->insert(o_1_p);
					o_1_p->in_triangle = t_2;
				}
				else {
					cout << "Error: Wrong point position.\n";
					// Can happen if points are not in general position
				}

				// remove the point from the triangle
				Node<Point*>* old_current_o_1_p_node = current_o_1_p_node;
				current_o_1_p_node = current_o_1_p_node->next;
				(old_1->points)->remove(old_current_o_1_p_node);
			}

			// from the second old triangle
			Node<Point*>* current_o_2_p_node = (old_2->points)->start;
			while (old_2->points->size != 0) {

				Point* o_2_p = current_o_2_p_node->data;
				if (is_in(t_1, o_2_p)) {
					// the old point is in the first new trianagle
					(t_1->points)->insert(o_2_p);
					o_2_p->in_triangle = t_1;
				}
				else if (is_in(t_2, o_2_p)) {
					// the old point is in the second new trianagle
					(t_2->points)->insert(o_2_p);
					o_2_p->in_triangle = t_2;
				}
				else {
					cout << "Error: Wrong point position.\n";
					// Can happen if points are not in general position
				}

				// remove the point from the triangle
				Node<Point*>* old_current_o_2_p_node = current_o_2_p_node;
				current_o_2_p_node = current_o_2_p_node->next;
				(old_2->points)->remove(old_current_o_2_p_node);
			}

			// remove old triangles
			triangles->remove(old_1->triangles_node);
			triangles->remove(old_2->triangles_node);

			// remove old half edges
			Point* a = ab->half_edges_node->data->source;
			Point* b = ba->half_edges_node->data->source;

			a->half_edges->remove_data(ab->half_edges_node->data);
			half_edges->remove(ab->half_edges_node);
			
			b->half_edges->remove_data(ba->half_edges_node->data);
			half_edges->remove(ba->half_edges_node);

			// recurse:
			// the ab was set to be the so far unaffected edge
			// not one of the new edges in the split triangle
			// not one of the new edges made during flipping
			// but the other one
			// that edge needs to be checked
			flip(t_1->ab);
			flip(t_2->ab);	
		}
	}

	void insert_points() {
		// taking points one by one from the random input points
		// and adding them to points
		// has to re-do the delaunay connections

		Node<Point*>* current_p_node = random_input_points->start;
		while (random_input_points->size != 0) {

			// inserting the point into points
			Point* p = current_p_node->data;
			p->points_node = points->insert(p);

			// what triangle this input_point is in
			Triangle* t = p->in_triangle;

			// split that triangle into three new triangles
			split_triangle(t,p);

			// add another point
			Node<Point*>* old_current_p_node = current_p_node;
			current_p_node = current_p_node->next;

			// delete the input point
			random_input_points->remove(old_current_p_node);
		}		
	}

	void write_output() {
		// writes the output for gnuplot to read

		ofstream output_file(output_address.c_str(), ios::out);
		if (output_file.is_open()) {
			output_file << "#!/usr/local/bin/gnuplot\n";
			output_file << "reset\n";
			output_file << "set terminal png\n";
			output_file << "set yrange [0:100]\n";
			output_file << "set xrange [0:100]\n";
			output_file << "unset colorbox\n";
			// delaunay connections:
			output_file << "set style arrow 1 nohead lc rgb \'black\'\n";
			// voronoi boundaries:
			output_file << "set style arrow 2 nohead lc rgb \'red\'\n";

			// go through all points
			Node<Point*>* current_p_node = points->start;
			while (current_p_node != NULL) {

				// go through all of the point's half edges
				Point* p = current_p_node->data;
				Node<HalfEdge*>* current_e_node = p->half_edges->start;
				while (current_e_node != NULL) {

					HalfEdge* e = current_e_node->data;
					if (e->target->counter > p->counter || e->twin == NULL) {
						// only does draw operations for one of the twins

						// draw delaunay:
						if (delaunay) {
							// draw delaunay connections
							output_file << "set arrow from "
										<< e->source->x << ","
										<< e->source->y << " to "
										<< e->target->x << ","
										<< e->target->y
										<< " as 1\n";
						}
						else {
							// only draw delaunay points:
							// for source point of half edges
							output_file << "set label "
										<< (e->source->counter + 4 + 1)
										<< " \"\" at "
										<< e->source->x << "," << e->source->y
										<< " point pointtype 5 "
										<< "lc rgb \'black\'\n";

							// for target point of half edge
							output_file << "set label "
										<< (e->target->counter + 4 + 1)
										<< " \"\" at "
										<< e->target->x << "," << e->target->y
										<< " point pointtype 5 "
										<< "lc rgb \'black\'\n";
						}

						// draw voronoi:
						if (voronoi && (e->twin != NULL)) {
							// draw voronoi boundary between triangle centers
							output_file << "set arrow from "
										<< e->of_triangle->center->x
										<< ","
										<< e->of_triangle->center->y
										<< " to "
										<< e->twin->of_triangle->center->x
										<< ","
										<< e->twin->of_triangle->center->y
										<< " as 2\n";
						}
						else if (voronoi && (e->twin == NULL)) {
							// draw voronoi boundary between center & midpoint

							// making virtual point to which boundary will aim
							double mx_offset = 0;
							double my_offset = 0;

							// determining to which direction to offset
							if (e->midpoint->x == 0) {
								mx_offset -= 1000;
							}
							else if (e->midpoint->x == 100) {
								mx_offset += 1000;
							}

							if (e->midpoint->y == 0) {
								my_offset -= 1000;
							}
							else if (e->midpoint->y == 100) {
								my_offset += 1000;
							}

							// draw the actual line
							output_file << "set arrow from ";
							output_file << e->of_triangle->center->x << ","
										<< e->of_triangle->center->y << " to "
										<< (e->midpoint->x+mx_offset) << ","
										<< (e->midpoint->y+my_offset)
										<< " as 2\n";
						}
					}
					
					current_e_node = current_e_node->next;
				}					
				
				current_p_node = current_p_node->next;
			}

			// ending of the file:
			output_file << "plot NaN notitle\n";
			//output_file << "pause 1000\n"; // ensure that X11 stays visible

			output_file.close();
		}
		else {
			cout << "Error: Could not access output file for writing!\n";
		}
	}
};

// can be built and run by:
// g++ delaunay.cpp -o delaunay
// ./delaunay <input_address output_address <delaunay voronoi>>
int main (int argc, char * const argv[]) {
	if (argc == 1) {
		// using default values
		// needs the two files in the same directory
		// draws voronoi only

		Delaunay* d = new Delaunay("input.txt", "output.txt", false, true);
		d->insert_points();
		d->write_output();
		delete d;
	}
	else if (argc == 3) {
		// standard
		// uses file addresses provided
		// draws voronoi only

		string input = argv[1];
		string output = argv[2];

		Delaunay* d = new Delaunay(input, output, false, true);
		d->insert_points();
		d->write_output();
		delete d;
	}
	else if (argc == 5) {
		// uses file values provided
		// uses dealunay and voronoi booleans to draw/not draw delaunay/voronoi

		string input = argv[1];
		string output = argv[2];
		string delaunay_s = argv[3];
		string voronoi_s = argv[4];

		if ((delaunay_s == "true" || delaunay_s == "false") &&
			(voronoi_s == "true" || voronoi_s == "false")) {

			// parse boolean values
			bool delaunay = (delaunay_s == "true") ? true : false;
			bool voronoi = (voronoi_s == "true") ? true : false;

			Delaunay* d = new Delaunay(input, output, delaunay, voronoi);
			d->insert_points();
			d->write_output();
			delete d;
		}
		else {
			// values provided are not booleans
			cout << "Error: Invalid input for delaunay or voronoi argument!\n";
			cout << "Make sure delaunay and voronoi arguments booleans\n";
		}
		
	}
	else {
		cout << "Usage: <input output <delaunay voronoi>>\n";
	}

	return 0;
}