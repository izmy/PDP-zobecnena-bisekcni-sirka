FROM gcc
COPY . /usr/src/myapp
WORKDIR /usr/src/myapp
RUN g++ -fopenmp -o myapp main.cpp
CMD ["./myapp"]
