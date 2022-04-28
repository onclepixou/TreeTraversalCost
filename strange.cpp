#include "ibex.h"
#include "vibes.cpp"
#include <cstdlib>
#include <chrono>

using namespace std::chrono;
using namespace std;
using namespace ibex;

class ManualStrange : public Ctc {

	public:

		ManualStrange(int nb_var) : Ctc(nb_var) {}

		void contract(IntervalVector& X) {

            Interval& x = X[0];
            Interval& y = X[1];

            Interval v0 = x * y;
            Interval v1 = exp(v0);
            Interval v2 = x - y;
            Interval v3 = sin(v2);
            Interval v4 = v1 - v3;

            Interval r = Interval(-0.2, 0.2);

            bwd_sub(r, v1, v3);
            bwd_exp(v1, v0);
            bwd_mul(v0, x, y);
            bwd_sin(v3, v2);
            bwd_sub(v2, x, y);
            return;
		}
};

CtcFwdBwd TreeStrange(){

	Variable x,y;
	Function f(x,y,exp(x * y) - sin(x - y));
	CtcFwdBwd ctr(f, Interval(-0.2, 0.2));
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
	box[0]=Interval(-0.5,3);
	box[1]=Interval(-3,0.5);

	ManualStrange rctc(2);
	CtcFwdBwd ictc = TreeStrange();

	std::vector<IntervalVector> randomboxes = random_boxes(box, 3, 1000000);
	comparison(&rctc, &ictc, randomboxes);
	return 0;
}