#include "core/context.h"
#include "funcs/concat.h"
#include "test_saber_func.h"
#include "test_saber_base.h"
#include "tensor_op.h"
#include "saber_types.h"
#include <vector>

using namespace anakin::saber;

int axis_in = 1;
int num_in = 1;
int ch_in = 32;
int h_in = 112;
int w_in = 112;
template <typename dtype,typename TargetType_D,typename TargetType_H>
void concat_nv_basic(const std::vector<Tensor<TargetType_H>*>& inputs, std::vector<Tensor<TargetType_H>*>& outputs, ConcatParam<TargetType_D>& param){

    int axis = param.axis;
    int num = outputs[0]->num();
    int channel = outputs[0]->channel();
    int height = outputs[0]->height();
    int width = outputs[0]->width();

    Shape out_sh = outputs[0]->valid_shape();
    int out_concat_axis = out_sh[axis];
    int num_concats = inputs[0]->count_valid(0, param.axis);
    int concat_input_size = inputs[0]->count_valid(param.axis + 1, inputs[0]->dims());

    dtype* dout = (dtype*)outputs[0]->mutable_data();
/*
    for (int i = 0; i < inputs.size(); i++){
        const dtype* din = (dtype*)inputs[i]->data();
        LOG(INFO) << "i: " << i;
        for(int j = 0; j < inputs[i]->count_valid(0, 4); j++){
            LOG(INFO) << "j: "<< j << ", data: " << din[j];
        }
    }
*/
    int total_size = out_concat_axis * concat_input_size;
   // LOG(INFO) << "out_concat_axis: " << out_concat_axis;
   // LOG(INFO) << "num_concats: " << num_concats;
   // LOG(INFO) << "concat_input_size: " << concat_input_size;
    for(int k = 0; k < num_concats; k++){
        dtype* dout_ptr = dout + k * total_size;
        int out_size = 0;
        for(int i = 0; i < inputs.size(); i++){
            Shape in_sh = inputs[i]->valid_shape();
            int size = in_sh[axis] * concat_input_size;
            const dtype* din = (dtype*)inputs[i]->data();
            const dtype* din_ptr = din + k * size;
            dtype* dout_ptr_axis = dout_ptr + out_size;
            for(int j = 0; j < size; j++){
                dout_ptr_axis[j] = din_ptr[j];
            }
            out_size += size;
         //   LOG(INFO) << "out_size: " << size;
        }
    }
}

TEST(TestSaberFunc, test_func_concat) {
    int num = num_in;
    int channel = ch_in;
    int height = h_in;
    int width = w_in;
    int axis1 = axis_in;
   
#ifdef USE_CUDA
   //Init the test_base
    TestSaberBase<NV, NVHX86, AK_FLOAT, Concat, ConcatParam> testbase(2,1);
    Shape input_shape({num, channel, height, width}, Layout_NCHW);
    Shape input_shape2({2, 2, 2, 2}, Layout_NCHW);

    for(auto shape: {input_shape, input_shape2}){
        for(auto axis: {0,1,2,3, axis1}){
            ConcatParam<NV> param(axis);
            testbase.set_param(param);//set param
            //testbase.set_rand_limit(255,255);
            std::vector<Shape> shape_v;
            shape_v.push_back(shape);
            Shape shin = shape;
            shin[axis] = 2;
            shape_v.push_back(shin);
            Shape shin2 = shape;
            shin2[axis] = 4;
            shape_v.push_back(shin2);
            testbase.set_input_shape(shape_v);//add some input shape
            testbase.run_test(concat_nv_basic<float, NV, NVHX86>);//run test
        }
    }

#endif
#ifdef USE_X86_PLACE
    TestSaberBase<X86, X86, AK_FLOAT, Concat, ConcatParam> testbase(2,1);
    Shape input_shape({num, channel, height, width}, Layout_NCHW);
    Shape input_shape2({2, 2, 2, 2}, Layout_NCHW);

    for(auto shape: {input_shape, input_shape2}){
        for(auto axis: {0,1,2,3, axis1}){
            ConcatParam<X86> param(axis);
            testbase.set_param(param);//set param
            //testbase.set_rand_limit(255,255);
            std::vector<Shape> shape_v;
            shape_v.push_back(shape);
            Shape shin = shape;
            shin[axis] = 2;
            shape_v.push_back(shin);
            Shape shin2 = shape;
            shin2[axis] = 4;
            shape_v.push_back(shin2);
            testbase.set_input_shape(shape_v);//add some input shape
            testbase.run_test(concat_nv_basic<float, X86, X86>);//run test
            LOG(INFO) << "X86 run end";
        }
    }
#endif
}


int main(int argc, const char** argv) {
    if (argc >= 2) {
        axis_in = atoi(argv[1]);
    }
    if(argc >= 3) {
        if (argc < 6) {
            LOG(ERROR) << "usage: ./" << argv[0] << "axis " << \
                " num ch_in h_in w_in" ;
            return 0;
        }
        num_in = atoi(argv[2]);
        ch_in = atoi(argv[3]);
        h_in = atoi(argv[4]);
        w_in = atoi(argv[5]);
    }
    // initial logger
    //logger::init(argv[0]);
    InitTest();
    RUN_ALL_TESTS(argv[0]);

    return 0;
}

