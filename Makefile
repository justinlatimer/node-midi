all:
	node-waf clean || true; node-waf configure build
