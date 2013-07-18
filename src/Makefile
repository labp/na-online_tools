CXX=g++
CXXFLAGS=-O2 -fPIC

# Set library paths
EIGEN_INCLUDE_PATH=/opt/include/eigen311
MNE_INCLUDE_PATH=/opt/include/MNE
MNE_LIBRARY_PATH=/opt/lib/MNE
QT5_INCLUDE_PATH=/opt/include/qt5
QT5CORE_INCLUDE_PATH=/opt/include/qt5/QtCore
QT5_LIBRARY_PATH=/opt/lib/qt5

# Prepare compiler and linker variables
INCLUDES=-I$(EIGEN_INCLUDE_PATH) -I$(MNE_INCLUDE_PATH) -I$(QT5_INCLUDE_PATH) -I$(QT5CORE_INCLUDE_PATH)
LDFLAGS=-L$(MNE_LIBRARY_PATH) -L$(QT5_LIBRARY_PATH)
LIBS=-lQt5Core -lMNE1Fiff -lQt5Network -lMNE1Mne -lMNE1Utils -lMNE1Fs

# Setup project files
OBJS=eeg_sensor_generator.o ReaderBND.o
EXEC=eeg_sensor_generator

# make rules
all: $(EXEC) 

$(EXEC): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(EXEC) $(OBJS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< 

clean:
	$(RM) *.o *.so *.a $(EXEC)