CC=gcc

batracios: batracios.c libbatracios.a batracios.h
	$(CC) -m32 batracios.c libbatracios.a batracios.h -o batracios -lm

upload:
	git add * && git commit -m "Upload!" && git push

clean:
	rm *.o batracios && clear
