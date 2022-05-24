FROM	debian:buster

# -- no-install-recommends
# && rm -rf /var/lib/apt/lists/*;
RUN			set -eux; \
			apt-get update && apt-get install -y \
				clang \
				curl \
				doxygen \
				gcc \
				git \
				graphviz \
				libreadline-dev \
				lsof \
				make \
				procps \
				python3 \
				python3-pip \
				telnet \
				tree \
				valgrind \
				vim \
				wget \
			&& apt-get clean

RUN			mkdir -p /root/workdir

WORKDIR		/root/workdir

CMD			make && ./webserv file
