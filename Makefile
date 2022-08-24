makeAll = language_versions/quine_cpp_python.py\
					language_versions/quine_cpp_python_scheme.py\
					language_versions/quine_cpp_python_scheme.scm


all: $(makeAll)

clean:
	-@rm -Rf language_versions/*
	-@rm -Rf bin/*

bin/quine_cpp_python: quine_cpp_python.cpp
	g++ --std=gnu++11 -o $@ $^

language_versions/quine_cpp_python.py: bin/quine_cpp_python
	./bin/quine_cpp_python --python > $@

bin/quine_cpp_python_scheme: quine_cpp_python_scheme.cpp
	g++ --std=gnu++11 -o $@ $^

language_versions/quine_cpp_python_scheme.py: bin/quine_cpp_python_scheme
	./bin/quine_cpp_python_scheme --python > $@

language_versions/quine_cpp_python_scheme.scm: bin/quine_cpp_python_scheme
	./bin/quine_cpp_python_scheme --scheme > $@
