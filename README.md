# Complex Real-time 3D Motion Capture tool (WIP)
### Dependencies
- OpenGL (Should be present by default)
- OpenCL (Should be present by default)
- [OpenCV](https://opencv.org/get-started) (tl;dr: `libopencv-dev`)
- [GTKMM 4.0](https://gtkmm.org/en/download.html) (tl;dr: `libgtkmm-4.0-dev`)
- [spdlog](https://github.com/gabime/spdlog) (tl;dr: `libspdlog-dev`)
- [v4l2](https://trac.gateworks.com/wiki/linux/v4l2) (tl;dr: `libv4l-dev`) (linux)
- [argparse](https://github.com/p-ranav/argparse#positional-arguments) (includes)
- [glm](https://github.com/g-truc/glm) (includes)

## TODO:
- Multi-camera setup (full 3D)
- Tensorflow blazepose model (body + face + hands landmarks)
- Region segmentation (humans only)
- API: Ableton/Max/PureData/Unity/UE

## Cooking in progress

#### Real-time 3D visualization (video)
[![IMAGE ALT TEXT](https://img.youtube.com/vi/YNmbTUxgt3U/0.jpg)](https://www.youtube.com/watch?v=YNmbTUxgt3U "Real-time 3D visualization")

#### Point cloud reconstruction, export to PLY (meshlab)
![1](https://raw.githubusercontent.com/henryco/eox-capture/master/media/1.png)
![2](https://raw.githubusercontent.com/henryco/eox-capture/master/media/2.png)
![3](https://raw.githubusercontent.com/henryco/eox-capture/master/media/3.png)
![4](https://raw.githubusercontent.com/henryco/eox-capture/master/media/4.png)

#### Disparity map + filtering
![Cooking](https://raw.githubusercontent.com/henryco/eox-capture/master/media/cooking.png)
