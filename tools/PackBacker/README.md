PackBacker
==========

PackBacker is a light tool to download and install 3rd party libraries.   
You can use it to simplify and automate your project setup, e.g. to download internal dependencies for your project.


Concept
-------

Job script with one instruction per line:
```
$ cat jobs/example.bb
cxxtest: dest_dir=~;
eigen3: dest_dir=~;
```

Run installation job:
```
$ python3 packbacker.py jobs/example.pb
```


Examples of use
---------------

Have a look in the `tools` folder and `install_dependencies.sh`:

* https://github.com/cpieloth/CppMath/
* https://bitbucket.org/labp/na-online_ow-toolbox/


Installation & Usage
--------------------

* Clone repository.
```
$ git clone https://github.com/cpieloth/PackBacker.git PackBacker
```
* Create a new branch for your installation jobs (optional).
```
$ cd PackBacker
$ git checkout -b jobs
```
* Create an installation job.
```
$ cp jobs/example.bb jobs/myjob.bb
$ vi jobs/myjob.bb
```
* Commit your setup (optional).
* Run your installation job
```
$ python3 packbacker.py jobs/myjob.pb
```


Extend PackBacker with your own installer
-----------------------------------------

Let's implement an installer for the dependency `mydep`:

1. Place the Python file `mydep.py` in: `PackBacker/packbacker/installers/`
2. The file must contain a class, which implements/extends `packbacker.installer.Installer`, e.g.:
```
from packbacker.installer import Installer
class MyDep(Installer)
```
3. Give the installer a name and label, e.g.:
```
class MyDep(Installer):
    def __init__(self):
        Installer.__init__(self, 'mydep', 'My Dependency')
```
4. Implement at least these methods: 
```
def _install(self)
@classmethod
def instance(cls, params)
@classmethod
def prototype(cls)
```
5. Now, you can use your installer by using the specified name `mydep` in your job file:
```
mydep: arg1=value, ...;
```
