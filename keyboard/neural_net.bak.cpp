vector<float> X { 5.1, 3.5, 1.4, 0.2, 4.9, 3.0, 1.4, 0.2, 6.2, 3.4, 5.4, 2.3, 5.9, 3.0, 5.1, 1.8 };

vector<float> y { 0, 0, 1, 1 };

vector<float> W { 0.5, 0.5, 0.5, 0.5 };

vector<float> sigmoid_d(const vector<float> &m1)
{
    /*
      Returns the value of the sigmoid function derivative f'(x) = f(x)(1 - f(x)),
      where f(x) is sigmoid function.
      Input: m1, a vector.
      Output: x(1 - x) for every element of the input matrix m1.
    */

    const uint64_t VECTOR_SIZE = m1.size();
    vector<float> output(VECTOR_SIZE);

    for(uint64_t i = 0; i < VECTOR_SIZE; i++)
	output[i] = m1[i] * (1 - m1[i]);

    return(output);
}

vector<float> sigmoid(const vector<float> &m1)
{
    /*
      Returns the value of the sigmoid function f(x) = 1/(1 + e^-x).
      Input: m1, a vector.
      Output: 1/(1 + e^-x) for every element of the input matrix m1.
    */
    const uint64_t VECTOR_SIZE = m1.size();
    vector<float> output (VECTOR_SIZE);

    for(uint64_t i = 0; i < VECTOR_SIZE; i++) 
	output[i] = 1 / (1 + exp(-m1[i]));
    
    return(output);
}

vector<float> operator+(const vector<float> &m1, const vector<float> &m2)
{
    /*
      Returns the elementwise sum of two vectors.
      Inputs:
      m1: a vector
      m2: a vector
      Output: a vector, sum of the vectors m1 and m2.
    */

    const uint64_t VECTOR_SIZE = m1.size();
    vector<float> sum(VECTOR_SIZE);

    for (uint64_t i = 0; i < VECTOR_SIZE; ++i)
	sum[i] = m1[i] + m2[i];

    return(sum);
}

vector<float> operator-(const vector<float> &m1, const vector<float> &m2)
{
    /*
      Returns the difference between two vectors.
      Inputs:
      m1: vector
      m2: vector
      Output: vector, m1 - m2, difference between two vectors m1 and m2.
    */

    const uint64_t VECTOR_SIZE = m1.size();
    vector<float> difference(VECTOR_SIZE);

    for (uint64_t i = 0; i < VECTOR_SIZE; i++)
	difference[i] = m1[i] - m2[i];

    return(difference);
}

vector<float> operator*(const vector<float> &m1, const vector<float> &m2){

    /*
      Returns the product of two vectors (elementwise multiplication).
      Inputs:
      m1: vector
      m2: vector
      Output: vector, m1 * m2, product of two vectors m1 and m2
    */
    const uint64_t VECTOR_SIZE = m1.size();
    vector <float> product(VECTOR_SIZE);

    for (uint64_t i = 0; i < VECTOR_SIZE; i++)
	product[i] = m1[i] * m2[i];
    
    return product;
}

vector<float> transpose(float *m, const int C, const int R)
{
    /*
      Returns a transpose matrix of input matrix.
      Inputs:
      m: vector, input matrix
      C: int, number of columns in the input matrix
      R: int, number of rows in the input matrix
      Output: vector, transpose matrix mT of input matrix m
    */

    vector<float> mT(C*R);

    for(uint64_t n = 0; n < C*R; n++)
    {
	uint64_t i = n/C;
	uint64_t j = n%C;
	mT[n] = m[R*j + i];
    }
    
    return mT;
}

vector<float> dot(const vector<float> &m1, const vector<float> &m2, const int m1_rows, const int m1_columns, const int m2_columns)
{
    /*
      Returns the product of two matrices: m1 x m2.
      Inputs:
      m1: vector, left matrix of size m1_rows x m1_columns
      m2: vector, right matrix of size m1_columns x m2_columns (the number of rows in the right matrix
      must be equal to the number of the columns in the left one)
      m1_rows: int, number of rows in the left matrix m1
      m1_columns: int, number of columns in the left matrix m1
      m2_columns: int, number of columns in the right matrix m2
      Output: vector, m1 * m2, product of two vectors m1 and m2, a matrix of size m1_rows x m2_columns
    */

    vector <float> output (m1_rows*m2_columns);

    for(int row = 0; row < m1_rows; row++)
    {
	for(int col = 0; col < m2_columns; col++)
	{
	    output[row * m2_columns + col] = 0.0f;
	    for(int k = 0; k < m1_columns; k++)
		output[ row * m2_columns + col ] += m1[ row * m1_columns + k ] * m2[ k * m2_columns + col ];
	}
    }

    return output;
}

void print(const vector <float>& m, int n_rows, int n_columns)
{
    /*
      "Couts" the input vector as n_rows x n_columns matrix.
      Inputs:
      m: vector, matrix of size n_rows x n_columns
      n_rows: int, number of rows in the left matrix m1
      n_columns: int, number of columns in the left matrix m1
    */

    for(int i = 0; i < n_rows; i++)
    {
	for(int j = 0; j < n_columns; j++)
	    cout << m[ i * n_columns + j ] << " ";
	
	cout << '\n';
    }
    cout << endl;
}

typedef vector<Mat> filter3d;

filter3d operator+(const filter3d &r, const filter3d &l)
{
    filter3d result;
    for(int i = 0; i < r.size(); i++)
	result.push_back(r[i] + l[i]);
    return(result);
}

filter3d operator-(const filter3d &r, const filter3d &l)
{
    filter3d result;
    for(int i = 0; i < r.size(); i++)
	result.push_back(r[i] - l[i]);
    return(result);
}

Mat operator*(const filter3d &r, const filter3d &l)
{
    Mat result;
    for(int i = 0; i < r.size(); i++)
    {
	Mat con;
	filter2D(r[i], con, -1, l[i]);
	result += con;
    }
    return(result);
}

Mat operator*(const Mat &r, const filter3d &l)
{
    Mat result;
    for(int i = 0; i < l.size(); i++)
    {
	Mat con;
	filter2D(r, con, -1, l[i]);
	result += con;
    }
    return(result);
}

filter3d operator!(const filter3d &f)
{
    filter3d result;
    for(const auto &i : f)
    {
	Mat flipped_filter;
	rotate(f, flipped_filter, ROTATE_180);
	result.push_back(flipped_filter);
    }
    return(result);
}

enum LayerType
{
    LayerConv,
    LayerFullyConnected,    
};

enum ActivationType
{
    ActivationRelu,
};

struct layer
{
    LayerType type;
    ActivationType activation;
    vector<filter3d> filters; //If it's fully connected, there'll just be one entry
    filter3d bias;
};

Mat relu(const Mat &m)
{
    Mat result(m);
    result.setTo(0, result < 0);
    return(result);
}

Mat relu_grad(Mat activation)
{
    return(activation > 0);
}

Mat activation_function(const Mat &m, ActivationType activation)
{
    switch(activation)
    {
    case ActivationRelu:
	return(relu(m));
    }
}

Mat activation_function_grad(const Mat &m, ActivationType activation)
{
    switch(activation)
    {
    case ActivationRelu:
	return(relu_grad(m));
    }
}

vector<Mat> convolve(const vector<Mat> &inputs, const vector<filter3d> &filters)
{
    vector<Mat> result;
    for(int i = 0; i < inputs.size(); i++)
    {
	Mat out = inputs * filters[i];
	result.push_back(out);
    }
    return(result);
}

pair<vector<Mat>, vector<Mat> > conv_layer(const vector<Mat> &inputs, const layer &l)
{
    auto zs = convolve(inputs, l.filters);
    vector<Mat> as;
    //TODO(sasha): assert that result.size() == l.bias.size()
    for(int i = 0; i < zs.size(); i++)
    {
	Mat a = zs[i].clone();
	activation_function(a, l.activation);
	a += l.bias[i];
	as.push_back(a);
    }
    return(make_pair(as, zs));
}

filter3d conv_delta(const vector<filter3d> &zs, const vector<filter3d> &deltas, const vector<layer> &layers, int i)
{
    filter3d result;

    for(int j = 0; j < deltas[i + 1].size(); j++)
    {
	const Mat &delta = deltas[i + 1][j];
	result.push_back(delta * !layers[i + 1].filters[j]);
    }
    for(Mat &m : result)
	m = activation_function_grad(m, layers[i + 1].activation);
    return(result);
}

vector<vector<Mat> > conv_grad(const vector<Mat> &activation, const vector<vector<Mat> > &filters)
{
    vector<vector<Mat> > result;
    for(const auto &act : activation)
    {
	for(const auto &filter : filters)
	{
	    vector<Mat> grads;
	    for(const auto &filter2d : filter)
	    {
		Mat flipped_filter;
		rotate(filter2d, flipped_filter, ROTATE_180);
		Mat grad;
		filter2D(act, grad, -1, flipped_filter);
		grads.push_back(grad);
	    }
	    result.push_back(grads);
	}
    }
    return(result);
}

vector<Mat> fully_connected(const vector<Mat> &inputs, const Mat &weights)
{
    Mat unrolled;
    vector<Mat> result;
    for(const Mat &i : inputs)
	unrolled.push_back(i.reshape(1));

    unrolled = unrolled * weights;
    result.push_back(unrolled);
    return(result);
}

pair<vector<Mat>, vector<Mat> > fully_connected_layer(const vector<Mat> &inputs, const layer &l)
{
    auto zs = fully_connected(inputs, l.filters[0][0]);
    vector<Mat> as;
    Mat a = zs[0].clone();
    activation_function(a, l.activation);
    a += l.bias[0];
    as.push_back(a);  

    return(make_pair(as, zs));
}

filter3d fully_connected_delta(const vector<filter3d> &zs, const vector<filter3d> &deltas, const vector<layer> &layers, int i)
{
    Mat delta = (layers[i + 1].filters[0][0].t() * deltas[i + 1][0]).mul(activation_function_grad(zs[0][i], layers[i].activation));
    return(delta);
}

pair<vector<Mat>, vector<Mat> > execute_layer(const vector<Mat> &inputs, const layer &l)
{
    switch(l.type)
    {
    case LayerFullyConnected:
	return(fully_connected_layer(inputs, l));
    case LayerConv:
	return(conv_layer(inputs, l));
    }
    return(make_pair(vector<Mat>(), vector<Mat>()));
}

filter3d layer_delta(const vector<filter3d> &zs, const vector<filter3d> &deltas, const vector<layer> &layers, int i)
{
    switch(layers[i].type)
    {
    case LayerConv:
	return(conv_delta(zs, deltas, layers, i));
    case LayerFullyConnected:
	return(fully_connected_delta(zs, deltas, layers, i));
    default:
	return(filter3d());
    }
}

vector<filter3d> comp_deltas(const vector<Mat> &expected_output, const vector<Mat> &nn_out,
			     vector<filter3d> &zs, const vector<layer> &layers)
{
    vector<filter3d> result(layers.size());
    result.back() = expected_output - nn_out;
    for(auto &i : result.back())
	i = i.mul(activation_function_grad(i, layers.back().activation));

    for(int i = layers.size() - 2; i >= 0; i--)
    {
	result.push_back(layer_delta(zs, result, layers, i));
    }
    return(result);
}

vector<vector<filter3d> > comp_grads(const vector<filter3d> &deltas, const vector<filter3d> &zs)
{
    vector<vector<filter3d> > grads;
    for(int i = 0; i < deltas.size(); i++)
    {
    }
}

pair<vector<filter3d>, vector<filter3d> > for_prop(const filter3d &input, const vector<layer> &layers)
{
    vector<filter3d> activations;
    vector<filter3d> zs;
    activations.push_back(input);
    for(int i = 0; i < layers.size(); i++)
    {
	auto act = execute_layer(activations[i], layers[i]);
	activations.push_back(act.first);
	zs.push_back(act.second);
    }
    return(make_pair(activations, zs));
}

void back_prop(const vector<Mat> &input, vector<layer> &layers, const filter3d &expected_output)
{
    auto prop = for_prop(input, layers);
    auto activations = prop.first;
    auto zs = prop.second;
    auto deltas = comp_deltas(expected_output, activations.back(), zs, layers);
    
    
}

vector<vector<Mat> > random_conv_layer_initialization(int filter_size, int num_inputs, int num_filters)
{
    vector<vector<Mat> > result;
    for(int i = 0; i < num_filters; i++)
    {
	vector<Mat> filter3d;
	for(int j = 0; j < num_inputs; j++)
	{
	    Mat filter(filter_size, filter_size, CV_32FC1);
	    randu(filter, -0.001, 0.001);
	    filter3d.push_back(filter);
	}
	result.push_back(filter3d);
    }
    return(result);
}

void train_nn()
{
    auto data_pair = read_data("data.yml");
    auto data = data_pair.first;
    auto filenames = data_pair.second;

    for(int i = 0; i < filenames.size(); i++)
    {
	Mat _img = imread(filenames[i]);
	Mat img;
	_img.convertTo(img, CV_32FC3);
	Mat bgr[3];
	split(img, bgr);
    }
}
