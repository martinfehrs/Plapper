FROM ubuntu:questing

RUN DEBIAN_FRONTEND="noninteractive" apt-get update && apt-get -y install tzdata
RUN apt-get update && apt-get install -y build-essential gcc g++ gdb clang ninja-build catch2 locales-all wget && apt-get clean
RUN wget https://github.com/Kitware/CMake/releases/download/v4.2.0-rc1/cmake-4.2.0-rc1-linux-x86_64.sh
RUN chmod a+x cmake-4.2.0-rc1-linux-x86_64.sh
RUN ./cmake-4.2.0-rc1-linux-x86_64.sh --skip-license --exclude-subdir --prefix=/usr