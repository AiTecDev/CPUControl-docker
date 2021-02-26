Displaying device information like Host IP, CPU and Memory workload on the OLED Display

Running CPU Control Container:
# Prerequisite
cp /opt/nvidia .

# Pull nvidia base image and its docker build 
(https://ngc.nvidia.com/catalog/containers/nvidia:l4t-base)
- sudo docker build -t nvidia/aitech-base-image -f Dockerfile.base .

# Build display container and run it.
- sudo docker build -t  aitecdev/cpu-control-display -f Dockerfile .
- sudo docker run --network host --runtime nvidia -e DISPLAY=$DISPLAY -v /tmp/.X11-unix/:/tmp/.X11-unix --privileged aitecdev/cpu-control-display:latest

