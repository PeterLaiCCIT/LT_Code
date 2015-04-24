NAME=encoder
LIBNAME=encoder
DIRNAME=encoderDir

CC=g++
CCOPT=-W -Wall -pipe

OBJDIR=.obj/
SOURCEs=random_generator.cpp bit_matrix.cpp distribution.cpp codec.cpp
MAIN=my-decoder.cpp

.PHONY : all new clean tar

OBJs=$(patsubst %.cpp, $(OBJDIR)%.o, $(SOURCEs))
OBJMAIN=$(patsubst %.cpp, $(OBJDIR)%.o, $(MAIN))
NAMES=$(patsubst %.cpp, %, $(MAIN))
HEADERS=$(patsubst %.cpp, %.h, $(SOURCEs))
FULLLIBNAME=lib$(LIBNAME).a

all: $(OBJDIR) $(FULLLIBNAME) $(OBJMAIN) $(NAMES) $(NAME)
	@echo -e "\nCompiled without errors\n"
#	cp my-decoder results_TXpolicies
	@echo -e "Enjoy ;-)\n"

new: clean all 

$(FULLLIBNAME): Makefile $(OBJs)
	ar cr $(FULLLIBNAME) $(OBJs)

$(OBJs): $(OBJDIR)%.o: %.cpp %.h
	$(CC) $(CCOPT) -c $< -o $@

$(OBJMAIN): $(OBJDIR)%.o: %.cpp $(HEADERS)
	$(CC) $(CCOPT) -c $< -o $@

$(NAMES): %: $(OBJDIR)%.o $(FULLLIBNAME)
	$(CC) -L. $< -o $@ -l$(LIBNAME)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

clean:
	@rm -fr .obj
	@rm -fr $(OBJMAIN) $(FULLLIBNAME)
	@rm -fr core
	@echo -e "\nClear all\n"
#	@rm *~

tar:
	mkdir -p $(DIRNAME)
	cp $(HEADERS) $(SOURCEs) $(MAIN) *dd Makefile README $(DIRNAME)/
	tar zcvf encoder.tar.gz $(DIRNAME)

