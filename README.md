NA-Online: Tools
================

WWW: http://www.labp.htwk-leipzig.de   
SRC: https://bitbucket.org/labp   


EEG Sensor Generator
--------------------

This tool generates virtual EEG sensor positions out of a BEM layer.
These positions can be used to compute a high resolution leadfield.
This leadfield can be used for a leadfield interpolation.


EEG Sensor Transformator
------------------------

Rotates and translates the EEG channel positions.


Compile
-------

    cd build
    cmake ../src
    make
    
    
Run
---

    cd build
    ./nao_eeg_sensor_generator -h
    ./nao_eeg_sensor_transformator -h

