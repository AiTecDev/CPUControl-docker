FROM ubuntu
WORKDIR ./
RUN mkdir /usr/share/CPUControl
RUN chmod -R 774 /usr/share/CPUControl
COPY install.sh /usr/share/CPUControl
RUN mkdir /usr/share/CPUControl/display
COPY display /usr/share/CPUControl/display
RUN ls -la /usr/share/CPUControl
RUN apt-get update
RUN apt-get -y upgrade
RUN apt-get install python3.6
RUN apt-get -y install python3-pip
RUN python3 -m pip install -U --force-reinstall pip
RUN pip3 install Adafruit-GPIO Adafruit-PureIO Adafruit-SSD1306 Flask Jinja2 MarkupSafe Werkzeug click itsdangerous Pillow  numpy
RUN chmod 774 ./usr/share/CPUControl/install.sh
RUN /usr/share/CPUControl/install.sh klmn13klmn
WORKDIR /usr/share/CPUControl
expose 8000
CMD ["python3", "/usr/share/CPUControl/display/display-server.py"]
