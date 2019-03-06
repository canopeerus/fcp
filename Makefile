all : fcp-server fcp-client

install:
	cp -f fcp-server fcp-client ~/bin/

clean:
	rm -f fcp-server fcp-client
