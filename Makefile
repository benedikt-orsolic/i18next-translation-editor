create-target:
	mkdir target;
compile: 
	gcc $( pkg-config --cflags gtk4 ) -o target/main main.c $( pkg-config --libs gtk4 )
