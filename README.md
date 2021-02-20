# Displaying device information like Host IP, CPU and Memory workload on the OLED Display

# Running CPU Control Container:
# Prerequisite
cp /opt/nvidia .

# Pull nvidia base image and its docker build (https://ngc.nvidia.com/catalog/containers/nvidia:l4t-base)
- docker pull nvcr.io/nvidia/l4t-base:r32.5.0
- sudo docker build -t nvidia/aitech-base-image -f Dockerfile.base .

# Build display container and run it.
- sudo docker build -t nvidia/cpu-control-display -f Dockerfile .
- sudo docker run -p 8000:8000 -v /dev:/dev --privileged nvidia/cpu-control-display:latest
