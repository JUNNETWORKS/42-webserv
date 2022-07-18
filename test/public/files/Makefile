FILES =	empty-file \
		n-line-file \
		rand-file \
		no-read-permission-file \
		fifo-file \
		symbolic-link-file

.PHONY: all
all: $(FILES)

# FILES
# ------------------------------------------------------------------------------------------
empty-file:
	touch empty-file
	@echo

n-line-file: N_LINE = 1000
n-line-file:
	for i in {1..$(N_LINE)}; do echo Line : $${i}; done > n-line-file
	@echo

rand-file: RAND_FILE_SIZE = 10000
rand-file:
	head -c $(RAND_FILE_SIZE) /dev/urandom > rand-file
	@echo

no-read-permission-file:
	echo no-read-permission > no-read-permission-file
	chmod -r no-read-permission-file
	@echo

fifo-file:
	mkfifo fifo-file
	@echo

symbolic-link-file:
	echo "link-target" > link-target-file
	ln -s link-target-file symbolic-link-file
	@echo

# 
# ------------------------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f $(FILES)
	rm -f link-target-file

.PHONY: fclean
fclean: clean

.PHONY: re
re: fclean all
