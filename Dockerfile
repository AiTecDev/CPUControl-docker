FROM nvidia/aitech-base-image:latest
COPY nvidia /opt/
RUN mkdir /usr/share/CPUControl
RUN chmod -R 774 /usr/share/CPUControl
COPY ./ /usr/share/CPUControl
RUN apt-get update
RUN apt-get -y upgrade
RUN apt-get install python3.6
RUN apt-get -y install python3-pip
RUN python3 -m pip install -U --force-reinstall pip
RUN pip3 install -r /usr/share/CPUControl/requirements.txt
RUN chmod 774 ./usr/share/CPUControl/install.sh
RUN ./usr/share/CPUControl/install.sh klmn13klmn
WORKDIR /usr/share/CPUControl
expose 8000

CMD ["jupyter", "notebook", "--port=8888", "--no-browser", "--ip=0.0.0.0", "--allow-root"] && \
    python3 /usr/share/CPUControl/display/display-server.py
