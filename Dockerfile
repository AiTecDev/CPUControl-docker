FROM ubuntu:18.04
WORKDIR ./
RUN mkdir /usr/share/CPUControl
RUN chmod -R 774 /usr/share/CPUControl
RUN mkdir /usr/share/CPUControl/display
COPY display /usr/share/CPUControl/display
RUN apt-get update
RUN apt-get -y upgrade
RUN apt-get install -yq python3.6 python3-pip net-tools
RUN python3 -m pip install -U pip
RUN pip3 install Adafruit-GPIO Adafruit-PureIO Adafruit-SSD1306 Flask Jinja2 MarkupSafe Werkzeug click itsdangerous Pillow  numpy
WORKDIR /usr/share/CPUControl
expose 8000
CMD ["python3", "/usr/share/CPUControl/display/display-server.py"]
