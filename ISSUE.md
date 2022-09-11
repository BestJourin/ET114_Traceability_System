# NIT SSL Code

> 已知问题


-----
第三方库运行找不到问题
* 使用Ubuntu的小伙伴们切记将需要的动态库移至本地/usr/lib中
```bash
cd lib/paddle_inference/third_party/install
sudo cp mklml/lib/libmklml_intel.so /usr.lib
sudo cp mkldnn/lib/libdnnl.so.2 /usr.lib
sudo cp onnxruntime/lib/libonnxruntime.so.1.11.1 /usr.lib
sudo cp paddle2onnx/lib/libpaddle2onnx.so.1.0.0rc2 /usr.lib
```
执行上述语句后方可直接找到第三方库
-----
