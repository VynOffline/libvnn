################ Config

NAME_STATIC = lib/libvnn.a
NAME_DYNAMIC = lib/libvnn.so

COMPILER = g++
O_FLAGS = 
FLAGS = 
ARCHIVER = ar
ARCHIVER_FLAGS = rc

# Theses are specials variables for UE4 projects
COMPILER_UE4 = /home/vyn/Software/UnrealEngine/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v13_clang-7.0.1-centos7/x86_64-unknown-linux-gnu/bin/clang++ # Change for your need
O_FLAGS_UE4 = -stdlib=libc++ # UE4 needs libc++ instead of libstdc++
FLAGS_UE4 = -stdlib=libc++ -lpthread # libc++ needs libpthread, i don't remember why, i may be wrong

ARCHIVER_UE4 = /home/vyn/Software/UnrealEngine/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v13_clang-7.0.1-centos7/x86_64-unknown-linux-gnu/bin/llvm-ar # Change for your need
ARCHIVER_UE4_FLAGS = rc

#COMPILER = $(COMPILER_UE4) # Uncomment to use UE4 compiler (clang)
#O_FLAGS = $(O_FLAGS_UE4) # Uncomment to use UE4 flags for .o compilation (clang flags)
#FLAGS = $(FLAGS_UE4) # Uncomment to use UE4 flags (clang flags)

#ARCHIVER = $(ARCHIVER_UE4) # Uncomment to use UE4 archiver (llvm-ar)
#ARCHIVER_FLAGS = $(ARCHIVER_UE4_FLAGS)
#----------------------------------------------

INCLUDES =	-I include/Vyn/NeuralNetwork -I include/Vyn/File

SOURCES = 	Neuron.cpp \
			Connection.cpp \
			Layer.cpp \
			Network.cpp \
			Network_Fit.cpp \
			Network_Propagate.cpp \
			Population.cpp \
			Functions/ActivationFunctions.cpp \
			Functions/CostFunctions.cpp \
			Functions/CrossOverFunctions.cpp \
			File/Csv.cpp \
			Utils/Utils.cpp

################ Setup paths

SRCS = $(addprefix source/, $(SOURCES)) # All sources are in the 'source' folder
OBJS = $(SRCS:source/%.cpp=obj/%.o) # All objects are ine 'obj' folder, mirrored from 'source'
OBJDIRS := $(dir $(OBJS)) # Get all directories to create them in the 'obj' folder. (remove files from the string)

################ Rules

all: createdir $(NAME_STATIC)
	
createdir:
	if [ ! -d "obj" ]; then mkdir obj; fi
	if [ ! -d "lib" ]; then mkdir lib; fi
	-mkdir $(OBJDIRS)

obj/%.o: source/%.cpp include/Vyn/NeuralNetwork/*
	$(COMPILER) $(O_FLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME_STATIC)
	rm -f $(NAME_DYNAMIC)
	rm -f test_program

######## Static library version

static: createdir $(NAME_STATIC)

$(NAME_STATIC): $(OBJS)
	$(ARCHIVER) $(ARCHIVER_FLAGS) $(NAME_STATIC) $(OBJS)

######## Dynamic/Shared library version

dynamic: setup_dymanic createdir $(NAME_DYNAMIC)

setup_dymanic:
	$(eval O_FLAGS = $(O_FLAGS) -fPIC)

$(NAME_DYNAMIC): $(OBJS)
	$(COMPILER) $(FLAGS) -shared $(OBJS) -o $(NAME_DYNAMIC)

######## Tests

test: static
	$(COMPILER) -I include tests/*.cpp lib/libvnn.a -o test_program
	./test_program