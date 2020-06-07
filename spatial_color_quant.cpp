/* Copyright (c) 2006 Derrick Coetzee

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <vector>
#include <deque>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <limits>

using namespace std;

template <typename T, int length>
class vector_fixed
{
public:
    vector_fixed()
    {
        for(int i=0; i<length; i++) {
	    data[i] = 0;
	}
    }

    vector_fixed(const vector_fixed<T, length>& rhs)
    {
        for(int i=0; i<length; i++) {
            data[i] = rhs.data[i];
	}
    }

    vector_fixed(const vector<T>& rhs)
    {
        for(int i=0; i<length; i++) {
            data[i] = rhs[i];
	}
    }

    T& operator()(int i)
    {
        return data[i];
    }

    int get_length() { return length; }

    vector_fixed<T, length>& operator=(const vector_fixed<T, length> rhs)
    {
        for(int i=0; i<length; i++) {
            data[i] = rhs.data[i];
	}
	return *this;
    }

    vector_fixed<T, length> direct_product(vector_fixed<T, length>& rhs) {
        vector_fixed<T, length> result;
        for(int i=0; i<length; i++) {
            result(i) = (*this)(i) * rhs(i);
        }
        return result;
    }

    double dot_product(vector_fixed<T, length> rhs) {
        T result = 0;
        for(int i=0; i<length; i++) {
            result += (*this)(i) * rhs(i);
        }
        return result;
    }

    vector_fixed<T, length>& operator+=(vector_fixed<T, length> rhs) {
        for(int i=0; i<length; i++) {
            data[i] += rhs(i);
        }
	return *this;
    }

    vector_fixed<T, length> operator+(vector_fixed<T, length> rhs) {
	vector_fixed<T, length> result(*this);
	result += rhs;
	return result;
    }

    vector_fixed<T, length>& operator*=(T scalar) {
        for(int i=0; i<length; i++) {
            data[i] *= scalar;
        }
	return *this;
    }

    vector_fixed<T, length> operator*(T scalar) {
	vector_fixed<T, length> result(*this);
	result *= scalar;
	return result;
    }

private:
    T data[length];
};

template <typename T, int length>
vector_fixed<T, length> operator*(T scalar, vector_fixed<T, length> vec) {
    return vec*scalar;
}


template <typename T, int length>
ostream& operator<<(ostream& out, vector_fixed<T, length> vec) {
    out << "(";
    int i;
    for (i=0; i<length - 1; i++) {
	out << vec(i) << ", ";
    }
    out << vec(i) << ")";
    return out;
}

template <typename T>
class array2d
{
public:
    array2d(int width, int height)
    {
        this->width = width;
        this->height = height;
        data = new T[width * height];
    }

    array2d(const array2d<T>& rhs)
    {
        width = rhs.width;
        height = rhs.height;
        data = new T[width * height];
        for(int i=0; i<width; i++) {
	    for(int j=0; j<height; j++) {
		(*this)(i,j) = rhs.data[j*width + i];
	    }
	}
    }

    ~array2d()
    {
	delete [] data;
    }

    T& operator()(int col, int row)
    {
        return data[row*width + col];
    }

    int get_width() { return width; }
    int get_height() { return height; }

    array2d<T>& operator*=(T scalar) {
        for(int i=0; i<width; i++) {
	    for(int j=0; j<height; j++) {
		(*this)(i,j) *= scalar;
	    }
	}
	return *this;
    }

    array2d<T> operator*(T scalar) {
	array2d<T> result(*this);
	result *= scalar;
	return result;
    }

    vector<T> operator*(vector<T> vec) {
	vector<T> result;
	T sum;
	for(int row=0; row<get_height(); row++) {
	    sum = 0;
	    for(int col=0; col<get_width(); col++) {
		sum += (*this)(col,row) * vec[col];
	    }
	    result.push_back(sum);
	}
	return result;
    }

    array2d<T>& multiply_row_scalar(int row, double mult) {
	for(int i=0; i<get_width(); i++) {
	    (*this)(i,row) *= mult;
	}
	return *this;
    }

    array2d<T>& add_row_multiple(int from_row, int to_row, double mult) {
	for(int i=0; i<get_width(); i++) {
	    (*this)(i,to_row) += mult*(*this)(i,from_row);
	}
	return *this;
    }

    // We use simple Gaussian elimination - perf doesn't matter since
    // the matrices will be K x K, where K = number of palette entries.
    array2d<T> matrix_inverse() {
	array2d<T> result(get_width(), get_height());
	array2d<T>& a = *this;

	// Set result to identity matrix
	result *= 0;
        for(int i=0; i<get_width(); i++) {
	    result(i,i) = 1;
	}
	// Reduce to echelon form, mirroring in result
        for(int i=0; i<get_width(); i++) {
	    result.multiply_row_scalar(i, 1/a(i,i));
	    multiply_row_scalar(i, 1/a(i,i));
	    for(int j=i+1; j<get_height(); j++) {
		result.add_row_multiple(i, j, -a(i,j));
		add_row_multiple(i, j, -a(i,j));
	    }
	}
	// Back substitute, mirroring in result
        for(int i=get_width()-1; i>=0; i--) {
	    for(int j=i-1; j>=0; j--) {
		result.add_row_multiple(i, j, -a(i,j));
		add_row_multiple(i, j, -a(i,j));
	    }
	}
	// result is now the inverse
	return result;
    }

private:
    T* data;
    int width, height;
};

template <typename T>
array2d<T> operator*(T scalar, array2d<T> a) {
    return a*scalar;
}


template <typename T>
ostream& operator<<(ostream& out, array2d<T>& a) {
    out << "(";
    int i, j;
    for (j=0; j<a.get_height(); j++) {
	out << "(";
	for (i=0; i<a.get_width() - 1; i++) {
	    out << a(i, j) << ", ";
	}
	if (j == a.get_height() - 1) {
	    out << a(i, j) << "))" << endl;
	} else {
	    out << a(i, j) << ")," << endl << " ";
	}
    }
    return out;
}

template <typename T>
class array3d
{
public:
    array3d(int width, int height, int depth)
    {
        this->width = width;
        this->height = height;
        this->depth = depth;
        data = new T[width * height * depth];
    }

    array3d(const array3d<T>& rhs)
    {
        width = rhs.width;
        height = rhs.height;
        depth = rhs.depth;
        data = new T[width * height * depth];
        for(int i=0; i<width; i++) {
	    for(int j=0; j<height; j++) {
		for(int k=0; k<depth; k++) {
		    (*this)(i,j,k) = rhs.data[j*width*depth + i*depth + k];
		}
	    }
	}
    }

    ~array3d()
    {
	delete [] data;
    }

    T& operator()(int col, int row, int layer)
    {
        return data[row*width*depth + col*depth + layer];
    }

    int get_width() { return width; }
    int get_height() { return height; }
    int get_depth() { return depth; }

private:
    T* data;
    int width, height, depth;
};

template <typename T>
ostream& operator<<(ostream& out, array3d<T>& a) {
    out << "(";
    int i, j, k;
    out << "(";
    for (j=0; j<=a.get_height() - 1; j++) {
	out << "(";
	for (i=0; i<=a.get_width() - 1; i++) {
	    out << "(";
	    for (k=0; k<=a.get_depth() - 1; k++) {
		out << a(i, j, k);
		if (k < a.get_depth() - 1) out << ", ";
	    }
	    out << ")";
	    if (i < a.get_height() - 1) out << ",";
	}
	out << ")";
	if (j < a.get_height() - 1) out << ", " << endl;
    }
    out << ")" << endl;
    return out;
}

int compute_max_coarse_level(int width, int height) {
    // We want the coarsest layer to have at most MAX_PIXELS pixels
    const int MAX_PIXELS = 4000;
    int result = 0;
    while (width * height > MAX_PIXELS) {
        width  >>= 1;
        height >>= 1;
        result++;
    }
    return result;
}

void fill_random(array3d<double>& a) {
    for(int i=0; i<a.get_width(); i++) {
	for(int j=0; j<a.get_height(); j++) {
            for(int k=0; k<a.get_depth(); k++) {
                a(i,j,k) = ((double)rand())/RAND_MAX;
	    }
	}
    }
}

double get_initial_temperature() {
    return 2.0; // TODO: Figure out what to make this
}

double get_final_temperature() {
    return 0.02; // TODO: Figure out what to make this
}

void random_permutation(int count, vector<int>& result) {
    result.clear();
    for(int i=0; i<count; i++) {
        result.push_back(i);
    }
    random_shuffle(result.begin(), result.end());
}

void random_permutation_2d(int width, int height, deque< pair<int, int> >& result) {
    vector<int> perm1d;
    random_permutation(width*height, perm1d);
    while(!perm1d.empty()) {
        int idx = perm1d.back();
        perm1d.pop_back();
        result.push_back(pair<int,int>(idx % width, idx / width));
    }
}

void compute_b_array(array2d< vector_fixed<double, 3> >& filter_weights,
                     array2d< vector_fixed<double, 3> >& b)
{
    // Assume that the pixel i is always located at the center of b,
    // and vary pixel j's location through each location in b.
    int radius_width = (filter_weights.get_width() - 1)/2,
        radius_height = (filter_weights.get_height() - 1)/2;
    int offset_x = (b.get_width() - 1)/2 - radius_width;
    int offset_y = (b.get_height() - 1)/2 - radius_height;
    for(int j_y = 0; j_y < b.get_height(); j_y++) {
	for(int j_x = 0; j_x < b.get_width(); j_x++) {
	    for(int k_y = 0; k_y < filter_weights.get_height(); k_y++) {
		for(int k_x = 0; k_x < filter_weights.get_width(); k_x++) {
		    if (k_x+offset_x >= j_x - radius_width &&
			k_x+offset_x <= j_x + radius_width &&
		        k_y+offset_y >= j_y - radius_width &&
			k_y+offset_y <= j_y + radius_width)
		    {
			b(j_x,j_y) += filter_weights(k_x,k_y).direct_product(filter_weights(k_x+offset_x-j_x+radius_width,k_y+offset_y-j_y+radius_height));
		    }
		}
	    }	    
	}
    }
}

vector_fixed<double, 3> b_value(array2d< vector_fixed<double, 3> >& b,
			 	 int i_x, int i_y, int j_x, int j_y)
{
    int radius_width = (b.get_width() - 1)/2,
        radius_height = (b.get_height() - 1)/2;
    int k_x = j_x - i_x + radius_width;
    int k_y = j_y - i_y + radius_height;
    if (k_x >= 0 && k_y >= 0 && k_x < b.get_width() && k_y < b.get_height())
	return b(k_x, k_y);
    else
	return vector_fixed<double, 3>();
}

void compute_a_image(array2d< vector_fixed<double, 3> >& image,
                     array2d< vector_fixed<double, 3> >& b,
                     array2d< vector_fixed<double, 3> >& a)
{
    int radius_width = (b.get_width() - 1)/2,
        radius_height = (b.get_height() - 1)/2;
    for(int i_y = 0; i_y < a.get_height(); i_y++) {
	for(int i_x = 0; i_x < a.get_width(); i_x++) {
	    for(int j_y = i_y - radius_height; j_y <= i_y + radius_height; j_y++) {
		if (j_y < 0) j_y = 0;
		if (j_y >= a.get_height()) break;

		for(int j_x = i_x - radius_width; j_x <= i_x + radius_width; j_x++) {
		    if (j_x < 0) j_x = 0;
		    if (j_x >= a.get_width()) break;

		    a(i_x,i_y) += b_value(b, i_x, i_y, j_x, j_y).
			              direct_product(image(j_x,j_y));
		}
	    }
	    a(i_x, i_y) *= -2.0;
	}
    }
}

void sum_coarsen(array2d< vector_fixed<double, 3> >& fine,
		 array2d< vector_fixed<double, 3> >& coarse)
{
    for(int y=0; y<coarse.get_height(); y++) {
	for(int x=0; x<coarse.get_width(); x++) {
	    double divisor = 1.0;
	    vector_fixed<double, 3> val = fine(x*2, y*2);
	    if (x*2 + 1 < fine.get_width())  {
		divisor += 1; val += fine(x*2 + 1, y*2);
	    }
	    if (y*2 + 1 < fine.get_height()) {
		divisor += 1; val += fine(x*2, y*2 + 1);
	    }
	    if (x*2 + 1 < fine.get_width() &&
		y*2 + 1 < fine.get_height()) {
		divisor += 1; val += fine(x*2 + 1, y*2 + 1);
	    }
	    coarse(x, y) = /*(1/divisor)**/val;
	}
    }
}

template <typename T, int length>
array2d<T> extract_vector_layer_2d(array2d< vector_fixed<T, length> > s, int k)
{
    array2d<T> result(s.get_width(), s.get_height());
    for(int i=0; i < s.get_width(); i++) {
	for(int j=0; j < s.get_height(); j++) {
	    result(i,j) = s(i,j)(k);
	}
    }
    return result;
}

template <typename T, int length>
vector<T> extract_vector_layer_1d(vector< vector_fixed<T, length> > s, int k)
{
    vector<T> result;
    for(unsigned int i=0; i < s.size(); i++) {
	result.push_back(s[i](k));
    }
    return result;
}

int best_match_color(array3d<double>& vars, int i_x, int i_y,
		     vector< vector_fixed<double, 3> >& palette)
{
    int max_v = 0;
    double max_weight = vars(i_x, i_y, 0);
    for (unsigned int v=1; v < palette.size(); v++) {
	if (vars(i_x, i_y, v) > max_weight) {
	    max_v = v;
	    max_weight = vars(i_x, i_y, v);
	}
    }
    return max_v;
}

void zoom_double(array3d<double>& small, array3d<double>& big)
{
    for(int y=0; y<small.get_height(); y++) {
	for(int x=0; x<small.get_width(); x++) {
	    for(int z=0; z<small.get_depth(); z++) {
		big(2*x, 2*y, z) = small(x, y, z);
		big(2*x + 1, 2*y, z) = small(x, y, z);
		big(2*x, 2*y + 1, z) = small(x, y, z);
		big(2*x + 1, 2*y + 1, z) = small(x, y, z);
	    }
	}
    }
    if ((big.get_width() % 2) == 1) {
	int x = big.get_width() - 1;
	for(int y=0; y<big.get_height(); y++) {
	    for(int z=0; z<big.get_depth(); z++) {
		big(x,y,z) = big(x-1,y,z);
	    }
	}
    }
    if ((big.get_height() % 2) == 1) {
	int y = big.get_height() - 1;
	for(int x=0; x<big.get_width(); x++) {
	    for(int z=0; z<big.get_depth(); z++) {
		big(x,y,z) = big(x,y-1,z);
	    }
	}
    }
}

void spatial_color_quant(array2d< vector_fixed<double, 3> >& image,
                         array2d< vector_fixed<double, 3> >& filter_weights,
                         array2d< int >& quantized_image,
                         vector< vector_fixed<double, 3> >& palette,
			 array3d<double>*& p_coarse_variables,
			 double initial_temperature,
			 double final_temperature)
{
    int max_coarse_level = //1;
        compute_max_coarse_level(image.get_width(), image.get_height());
    p_coarse_variables = new array3d<double>(
	image.get_width()  >> max_coarse_level,
	image.get_height() >> max_coarse_level,
	palette.size());
    // For syntactic convenience
    array3d<double>& coarse_variables = *p_coarse_variables;
    fill_random(coarse_variables);

    double temperature = initial_temperature;

    // Compute a_i, b_{ij} according to (11)
    int extended_neighborhood_width = filter_weights.get_width()*2 - 1;
    int extended_neighborhood_height = filter_weights.get_height()*2 - 1;
    array2d< vector_fixed<double, 3> > b0(extended_neighborhood_width,
                                          extended_neighborhood_height);
    compute_b_array(filter_weights, b0);

    array2d< vector_fixed<double, 3> > a0(image.get_width(), image.get_height());
    compute_a_image(image, b0, a0);

    // Compute a_I^l, b_{IJ}^l according to (18)
    vector< array2d< vector_fixed<double, 3> > > a_vec, b_vec;
    a_vec.push_back(a0);
    b_vec.push_back(b0);

    int coarse_level;
    for(coarse_level=1; coarse_level <= max_coarse_level; coarse_level++)
    {
	int radius_width  = (filter_weights.get_width() - 1)/2,
	    radius_height = (filter_weights.get_height() - 1)/2;
	array2d< vector_fixed<double, 3> >
	    bi(max(3, b_vec.back().get_width()-2),
	       max(3, b_vec.back().get_height()-2));
	for(int J_y=0; J_y<bi.get_height(); J_y++) {
	    for(int J_x=0; J_x<bi.get_width(); J_x++) {
		for(int i_y=radius_height*2; i_y<radius_height*2+2; i_y++) {
		    for(int i_x=radius_width*2; i_x<radius_width*2+2; i_x++) {
			for(int j_y=J_y*2; j_y<J_y*2+2; j_y++) {
			    for(int j_x=J_x*2; j_x<J_x*2+2; j_x++) {
				bi(J_x,J_y) += b_value(b_vec.back(), i_x, i_y, j_x, j_y);
			    }
			}
		    }
		}
	    }
	}
	b_vec.push_back(bi);

	array2d< vector_fixed<double, 3> >
	    ai(image.get_width() >> coarse_level,
	       image.get_height() >> coarse_level);
	sum_coarsen(a_vec.back(), ai);
	a_vec.push_back(ai);
    }

    // Multiscale annealing
    coarse_level = max_coarse_level;
    const int iters_per_level = 3;
    double temperature_multiplier = pow(final_temperature/initial_temperature, 1.0/(max_coarse_level*iters_per_level));
#if TRACE
    cout << "Temperature multiplier: " << temperature_multiplier << endl;
#endif
    int iters_at_current_level = 0;
    while (coarse_level >= 0 || temperature > final_temperature) {
	// Need to reseat this reference in case we changed p_coarse_variables
	array3d<double>& coarse_variables = *p_coarse_variables;
	array2d< vector_fixed<double, 3> >& a = a_vec[coarse_level];
	array2d< vector_fixed<double, 3> >& b = b_vec[coarse_level];
	vector_fixed<double,3> middle_b = b_value(b,0,0,0,0);
#if TRACE
	cout << "Temperature: " << temperature << endl;
#endif
	int center_x = (b.get_width()-1)/2, center_y = (b.get_height()-1)/2;
	int step_counter = 0;
	for(int repeat=0; repeat<1; repeat++)
	{
	    int pixels_changed = 0;
	    deque< pair<int, int> > visit_queue;
	    random_permutation_2d(coarse_variables.get_width(), coarse_variables.get_height(), visit_queue);

	    // Compute 2*sum(j in extended neighborhood of i, j != i) b_ij

	    while(!visit_queue.empty())
	    {
		int i_x = visit_queue.front().first, i_y = visit_queue.front().second;
		visit_queue.pop_front();

		// Compute (25)
		vector_fixed<double,3> p_i;
		vector_fixed<double, 3> a_i = a(i_x, i_y);
		for (int y=0; y<b.get_height(); y++) {
		    for (int x=0; x<b.get_width(); x++) {
			int j_x = x - center_x + i_x, j_y = y - center_y + i_y;
			if (i_x == j_x && i_y == j_y) continue;
			if (j_x < 0 || j_y < 0 || j_x >= coarse_variables.get_width() || j_y >= coarse_variables.get_height()) continue;
			vector_fixed<double, 3> palette_sum = vector_fixed<double, 3>();
			for (unsigned int alpha=0; alpha < palette.size(); alpha++) {
			    palette_sum += coarse_variables(j_x,j_y,alpha)*palette[alpha];
			}
			p_i += b_value(b, i_x, i_y, j_x, j_y).direct_product(palette_sum);
		    }
		}
		p_i *= 2.0;
		p_i += a_i;

		vector<double> meanfield_logs, meanfields;
		double max_meanfield_log = -numeric_limits<double>::infinity();
		double meanfield_sum = 0.0;
		for (unsigned int v=0; v < palette.size(); v++) {
		    // Update m_{pi(i)v}^I according to (23)
		    // We can subtract an arbitrary factor to prevent overflow,
		    // since only the weight relative to the sum matters, so we
		    // will choose a value that makes the maximum e^100.
		    meanfield_logs.push_back(-(palette[v].dot_product(
			p_i + middle_b.direct_product(
			    palette[v])))/temperature);
		    if (meanfield_logs.back() > max_meanfield_log) {
			max_meanfield_log = meanfield_logs.back();
		    }
		}
		max_meanfield_log -= 100;
		for (unsigned int v=0; v < palette.size(); v++) {
		    meanfields.push_back(exp(meanfield_logs[v]-max_meanfield_log));
		    meanfield_sum += meanfields.back();
		}
		if (meanfield_sum == 0) {
		    cout << "Fatal error: Meanfield sum underflowed. Please contact developer." << endl;
		    exit(-1);
		}
		int old_max_v = best_match_color(coarse_variables, i_x, i_y, palette);
		for (unsigned int v=0; v < palette.size(); v++) {
		    double new_val = meanfields[v]/meanfield_sum;
		    // Prevent the matrix S from becoming singular
		    if (new_val <= 0) new_val = 1e-250;
		    if (new_val >= 1) new_val = 1 - 1e-16;
		    coarse_variables(i_x,i_y,v) = new_val;
		}
		int max_v = best_match_color(coarse_variables, i_x, i_y, palette);
		if (old_max_v != max_v) {
		    pixels_changed++;
		    // We don't add the outer layer of pixels , because
		    // there isn't much weight there, and if it does need
		    // to be visited, it'll probably be added when we visit
		    // neighboring pixels.
		    for (int y=1; y<b.get_height()-1; y++) {
			for (int x=1; x<b.get_width()-1; x++) {
			    int j_x = x - center_x + i_x, j_y = y - center_y + i_y;
			    if (j_x < 0 || j_y < 0 || j_x >= coarse_variables.get_width() || j_y >= coarse_variables.get_height()) continue;
			    visit_queue.push_back(pair<int,int>(j_x,j_y));
			}
		    }
		}

		// Show progress with dots - in a graphical interface,
		// we'd show progressive refinements of the image instead,
		// and maybe a palette preview.
		step_counter++;
		if ((step_counter % 10000) == 0) {
		    cout << ".";
		    cout.flush();
		}
	    }
#if TRACE
	    cout << "Pixels changed: " << pixels_changed << endl;
#endif

	    array2d< vector_fixed<double,3> > s(palette.size(), palette.size());
	    for (int i_y=0; i_y<coarse_variables.get_height(); i_y++) {
		for (int i_x=0; i_x<coarse_variables.get_width(); i_x++) {
		    int max_j_x = min(coarse_variables.get_width(), i_x - center_x + b.get_width());
		    int max_j_y = min(coarse_variables.get_height(), i_y - center_y + b.get_height());
		    for (int j_y=max(0, i_y - center_y); j_y<max_j_y; j_y++) {
			for (int j_x=max(0, i_x - center_x); j_x<max_j_x; j_x++) {
			    if (i_x == j_x && i_y == j_y) continue;
			    for (unsigned int v=0; v<palette.size(); v++) {
				for (unsigned int alpha=v; alpha<palette.size(); alpha++) {
				    s(v,alpha) += coarse_variables(i_x,i_y,v)*coarse_variables(j_x,j_y,alpha)*b_value(b,i_x,i_y,j_x,j_y);
				}
			    }
			}
		    }	    
		}
	    }
	    vector_fixed<double,3> center_b = b_value(b,0,0,0,0);
	    for (int i_y=0; i_y<coarse_variables.get_height(); i_y++) {
		for (int i_x=0; i_x<coarse_variables.get_width(); i_x++) {
		    for (unsigned int v=0; v<palette.size(); v++) {
			s(v,v) += coarse_variables(i_x,i_y,v)*center_b;
		    }
		}
	    }
	    for (unsigned int v=0; v<palette.size(); v++) {
		for (unsigned int alpha=0; alpha<v; alpha++) {
		    s(v,alpha) = s(alpha,v);
		}
	    }
            //cout << s << endl;

	    vector< vector_fixed<double,3> > r(palette.size());
	    for (unsigned int v=0; v<palette.size(); v++) {
		for (int i_y=0; i_y<coarse_variables.get_height(); i_y++) {
		    for (int i_x=0; i_x<coarse_variables.get_width(); i_x++) {
			r[v] += coarse_variables(i_x,i_y,v)*a(i_x,i_y);
		    }
		}
	    }

	    for (unsigned int k=0; k<3; k++) {
		array2d<double> S_k = extract_vector_layer_2d(s, k);
		vector<double> R_k = extract_vector_layer_1d(r, k);
		vector<double> palette_channel = -1.0*((2.0*S_k).matrix_inverse())*R_k;
		for (unsigned int v=0; v<palette.size(); v++) {
		    palette[v](k) = palette_channel[v];
		}		
	    }
#if TRACE
	    for (unsigned int v=0; v<palette.size(); v++) {
		cout << palette[v] << endl;
	    }
#endif
        }

	iters_at_current_level++;
	if ((temperature <= final_temperature || coarse_level > 0) &&
	    iters_at_current_level >= iters_per_level)
	{
	    coarse_level--;
	    if (coarse_level < 0) break;
	    array3d<double>* p_new_coarse_variables = new array3d<double>(
		image.get_width()  >> coarse_level,
		image.get_height() >> coarse_level,
		palette.size());
	    zoom_double(coarse_variables, *p_new_coarse_variables);
	    delete p_coarse_variables;
	    p_coarse_variables = p_new_coarse_variables;
	    iters_at_current_level = 0;
#ifdef TRACE
	    cout << "Image size: " << p_coarse_variables->get_width() << " " << p_coarse_variables->get_height() << endl;
#endif
	}
	if (temperature > final_temperature) {
	    temperature *= temperature_multiplier;
	}
    }

    while (coarse_level > 0) {
	coarse_level--;
	array3d<double>* p_new_coarse_variables = new array3d<double>(
	    image.get_width()  >> coarse_level,
	    image.get_height() >> coarse_level,
	    palette.size());
	zoom_double(*p_coarse_variables, *p_new_coarse_variables);
	delete p_coarse_variables;
	p_coarse_variables = p_new_coarse_variables;
    }

    {
    // Need to reseat this reference in case we changed p_coarse_variables
    array3d<double>& coarse_variables = *p_coarse_variables;

    for(int i_x = 0; i_x < image.get_width(); i_x++) {
	for(int i_y = 0; i_y < image.get_height(); i_y++) {
	    quantized_image(i_x,i_y) =
		best_match_color(coarse_variables, i_x, i_y, palette);
	}
    }
    for (unsigned int v=0; v<palette.size(); v++) {
	for (unsigned int k=0; k<3; k++) {
	    if (palette[v](k) > 1.0) palette[v](k) = 1.0;
	    if (palette[v](k) < 0.0) palette[v](k) = 0.0;
	}
#ifdef TRACE
	cout << palette[v] << endl;
#endif
    }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 1 + 5 || argc > 1 + 6) {
	printf("Usage: spatial_color_quant <source image.rgb> <width> <height> <desired palette size> <output image.rgb> [filter size (1/3/5)]\n");
	return -1;
    }

    srand(time(NULL));

    const int width = atoi(argv[2]), height = atoi(argv[3]);
    if (width <= 0 || height <= 0) {
	printf("Must specify a valid positive image width and height.\n");
	return -1;
    }

    array2d< vector_fixed<double, 3> > image(width, height);
    array2d< vector_fixed<double, 3> > filter1_weights(1, 1);
    array2d< vector_fixed<double, 3> > filter3_weights(3, 3);
    array2d< vector_fixed<double, 3> > filter5_weights(5, 5);
    array2d< int > quantized_image(width, height);
    vector< vector_fixed<double, 3> > palette;

    double gaussian3[3][3] = {{0.0705917, 0.106818, 0.0705917},
			     {0.1068180, 0.290361, 0.1068180},
			     {0.0705917, 0.106818, 0.0705917}};
    double gaussian5[5][5] =
	{{0.0116424, 0.0210523, 0.0266577, 0.0210523, 0.0116424},
	 {0.0210523, 0.0478881, 0.0724633, 0.0478881, 0.0210523},
	 {0.0266577, 0.0724633, 0.1969760, 0.0724633, 0.0266577},
	 {0.0210523, 0.0478881, 0.0724633, 0.0478881, 0.0210523},
	 {0.0116424, 0.0210523, 0.0266577, 0.0210523, 0.0116424}};

    for(int k=0; k<3; k++) {
	filter1_weights(0,0)(k) = 1.0;
    }
    for(int i=0; i<3; i++) {
	for(int j=0; j<3; j++) {
	    for(int k=0; k<3; k++) {
		filter3_weights(i,j)(k) = gaussian3[i][j];
	    }
	}
    }
    for(int i=0; i<5; i++) {
	for(int j=0; j<5; j++) {
	    for(int k=0; k<3; k++) {
		filter5_weights(i,j)(k) = gaussian5[i][j];
	    }
	}
    }

    int num_colors = atoi(argv[4]);
    if (num_colors <= 1 || num_colors > 256) {
	printf("Number of colors must be at least 2 and no more than 256.\n");
	return -1;
    }
    for (int i=0; i<atoi(argv[4]); i++) {
	vector_fixed<double, 3> v;
	v(0) = ((double)rand())/RAND_MAX;
	v(1) = ((double)rand())/RAND_MAX;
	v(2) = ((double)rand())/RAND_MAX;
	palette.push_back(v);
    }

#if TRACE
    for (unsigned int v=0; v<palette.size(); v++) {
	cout << palette[v] << endl;
    }
#endif

    {
    unsigned char c[3];
    FILE* in = fopen(argv[1], "rb");
    if (in == NULL) {
	printf("Could not open input file '%s'.\n", argv[1]);
	return -1;
    }
    for(int y=0; y<height; y++) {
	for (int x=0; x<width; x++) {
	    fread(c, 3, 1, in);
	    for(int ci=0; ci<3; ci++) {
		image(x,y)(ci) = c[ci]/((double)255);
	    }
	}
    }
    fclose(in);
    }

    // Check the output file before we begin the long part
    FILE* out = fopen(argv[5], "wb");
    if (out == NULL) {
	printf("Could not open output file '%s'.\n", argv[5]);
	return -1;
    }
    fclose(out);
    

    array3d<double>* coarse_variables;
    int filter_size = 3;
    if (argc > 6) {
	filter_size = atoi(argv[6]); 
	if (filter_size != 1 && filter_size != 3 && filter_size != 5) {
	    printf("Filter size must be one of 1, 3, or 5.\n");
	    return -1;
	}
    }
    array2d< vector_fixed<double, 3> >* filters[] =
	{NULL, &filter1_weights, NULL, &filter3_weights,
	 NULL, &filter5_weights};
    spatial_color_quant(image, *filters[filter_size], quantized_image, palette, coarse_variables, 1.0, 0.001);
    //spatial_color_quant(image, filter3_weights, quantized_image, palette, coarse_variables, 0.05, 0.02);

    cout << endl;

    {
    FILE* out = fopen(argv[5], "wb");
    if (out == NULL) {
	printf("Could not open output file '%s'.\n", argv[5]);
	return -1;
    }
    unsigned char c[3] = {0,0,0};
    for(int y=0; y<height; y++) {
	for (int x=0; x<width; x++) {
	    c[0] = (unsigned char)(255*palette[quantized_image(x,y)](0));
	    c[1] = (unsigned char)(255*palette[quantized_image(x,y)](1));
	    c[2] = (unsigned char)(255*palette[quantized_image(x,y)](2));
	    fwrite(c, 3, 1, out);
	}
    }
    fclose(out);
    }

    return 0;
}
