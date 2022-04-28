#include "ibex.h"
#include "vibes.cpp"
#include <cstdlib>
#include <chrono>

using namespace std::chrono;
using namespace std;
using namespace ibex;

class ManualRing : public Ctc {

	public:

		ManualRing(int nb_var) : Ctc(nb_var) {}

		void contract(IntervalVector& X) {
;
            Interval& x = X[0];
            Interval& y = X[1];
            Interval cx(1,3);
            Interval cy(2,4);
            Interval r(4,5);

            Interval a = (x - cx);
            Interval b = (y - cy);
            Interval a2 = sqr(a);
            Interval b2 = sqr(b);
            Interval r2 = sqr(r);

            bwd_add(r2, a2, b2);
		    bwd_sqr(a2, a);
		    bwd_sqr(b2, b);
		    bwd_sub(a, x, cx);
		    bwd_sub(b, y, cy);
            return;
		}
};

CtcFwdBwd TreeRing(){

	Interval cx(1,3);
    Interval cy(2,4);
	Interval r(4, 5);

	Variable x,y;
	Function f(x,y,sqr(x - cx) + sqr(y - cy));
	CtcFwdBwd ctr(f, sqr(r));
	return ctr;
}

float rand_FloatRange(float a, float b){

    return ((b - a) * ((float)rand() / RAND_MAX)) + a;
}

float min(float a, float b){

	float val = (a <= b ) ? a : b;
	return val;
}

std::vector<IntervalVector> random_boxes(IntervalVector space, float maxwidth, size_t nb){

	if(space.size() != 2){

		std::cout << "only search space of dim 2 is supported" << std::endl;
		return std::vector<IntervalVector>();
	}

	std::vector<IntervalVector> randomboxes;
	IntervalVector box(2);

	srand ( time(NULL) );
	for(int i = 0; i < nb; i++){

		float baseX = rand_FloatRange(space[0].lb(), space[0].ub());
		float baseY = rand_FloatRange(space[1].lb(), space[1].ub());

		float widthX = rand_FloatRange(0, min(maxwidth, space[0].ub() - baseX));
		float widthY = rand_FloatRange(0, min(maxwidth, space[1].ub() - baseY));

		box[0]=Interval(baseX ,baseX + widthX);
		box[1]=Interval(baseY ,baseY + widthY);
		randomboxes.push_back(box);
	}

	return randomboxes;
}

void comparison(Ctc* manual_ctc, CtcFwdBwd* tree_ctc, std::vector<IntervalVector> test_set){


	std::vector<IntervalVector> manual_boxes = test_set;
	std::vector<IntervalVector> tree_boxes = test_set;

	auto start_manual = high_resolution_clock::now();
	for(IntervalVector& el : manual_boxes){
		manual_ctc->contract(el);
	}
	auto end_manual = high_resolution_clock::now();
	auto duration_manual = duration_cast<milliseconds>(end_manual - start_manual);
	std::cout << "manual contractor took " << duration_manual.count() << " ms for 1e6 boxes" << std::endl;

	auto start_tree = high_resolution_clock::now();
	for(IntervalVector& el : tree_boxes){
		tree_ctc->contract(el);
	}

	auto end_tree = high_resolution_clock::now();
	auto duration_tree = duration_cast<milliseconds>(end_tree - start_tree);
	std::cout << "tree took " << duration_tree.count() << " ms for 1e6 boxes" << std::endl;

	size_t origin_diff = 0;
	size_t equal_count = 0;

	for(size_t i = 0; i < test_set.size(); i++){

		bool equal_dim1 = (manual_boxes[0] == tree_boxes[0]);
		bool equal_dim2 = (manual_boxes[1] == tree_boxes[1]);

		if(equal_dim1 && equal_dim2){

			equal_count++;
		}
	}

	std::cout << "percentage of equal contractions between manual and tree : " << ((float)equal_count/test_set.size())*100 << std::endl;
	return;
}

int main() {

	IntervalVector box(2);
	box[0]=Interval(-10,10);
	box[1]=Interval(-10,10);

	ManualRing rctc(2);
	CtcFwdBwd ictc = TreeRing();

	std::vector<IntervalVector> randomboxes = random_boxes(box, 3, 1000000);
	comparison(&rctc, &ictc, randomboxes);
	return 0;
}
